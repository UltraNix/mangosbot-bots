/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CastCustomSpellAction.h"
#include "ChatHelper.h"
#include "Event.h"
#include "Playerbot.h"

uint32 FindLastSeparator(std::string const& text, std::string const& sep)
{
    uint32 pos = text.rfind(sep);
    if (pos == std::string::npos)
        return pos;

    uint32 lastLinkBegin = text.rfind("|H");
    uint32 lastLinkEnd = text.find("|h|r", lastLinkBegin + 1);
    if (pos >= lastLinkBegin && pos <= lastLinkEnd)
        pos = text.find_last_of(sep, lastLinkBegin);

    return pos;
}

static inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

bool CastCustomSpellAction::Execute(Event event)
{
    Player* master = GetMaster();
    Unit* target = nullptr;
    std::string text = event.getParam();

    GuidVector gos = chat->parseGameobjects(text);
    if (!gos.empty())
    {
        for (auto go : gos)
        {
            if (!target)
                target = botAI->GetUnit(go);

            chat->eraseAllSubStr(text, chat->formatWorldobject(botAI->GetUnit(go)));
        }

        ltrim(text);
    }

    if (!target)
        if (master && master->GetTarget())
            target = botAI->GetUnit(master->GetTarget());

    if (!target)
        target = bot;

    Item* itemTarget = nullptr;

    uint32 pos = FindLastSeparator(text, " ");
    uint32 castCount = 1;
    if (pos != std::string::npos)
    {
        std::string param = text.substr(pos + 1);
        std::list<Item*> items = InventoryAction::parseItems(param, ITERATE_ITEMS_IN_BAGS);
        if (!items.empty())
            itemTarget = *items.begin();
        else
        {
            castCount = atoi(param.c_str());
            if (castCount > 0)
                text = text.substr(0, pos);
        }
    }

    uint32 spell = AI_VALUE2(uint32, "spell id", text);

    std::ostringstream msg;
    if (!spell)
    {
        msg << "Unknown spell " << text;
        botAI->TellError(msg.str());
        return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if (!spellInfo)
    {
        msg << "Unknown spell " << text;
        botAI->TellError(msg.str());
        return false;
    }

    if (target != bot && !bot->HasInArc(CAST_ANGLE_IN_FRONT, target, sPlayerbotAIConfig->sightDistance))
    {
        bot->SetFacingToObject(target);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);

        msg << "cast " << text;
        botAI->HandleCommand(CHAT_MSG_WHISPER, msg.str(), master);
        return true;
    }

    std::ostringstream spellName;
    spellName << ChatHelper::formatSpell(spellInfo) << " on ";

    if (bot->GetTrader())
        spellName << "trade item";
    else if (itemTarget)
        spellName << chat->formatItem(itemTarget->GetTemplate());
    else if (target == bot)
        spellName << "self";
    else
        spellName << target->GetName();

    if (!bot->GetTrader() && !botAI->CanCastSpell(spell, target, true, itemTarget))
    {
        msg << "Cannot cast " << spellName.str();
        botAI->TellError(msg.str());
        return false;
    }

    bool result = spell ? botAI->CastSpell(spell, target, itemTarget) : botAI->CastSpell(text, target, itemTarget);
    if (result)
    {
        msg << "Casting " << spellName.str();

        if (castCount > 1)
        {
            std::ostringstream cmd;
            cmd << castString(target) << " " << text << " " << (castCount - 1);
            botAI->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), master);
            msg << "|cffffff00(x" << (castCount-1) << " left)|r";
        }

        botAI->TellMasterNoFacing(msg.str());
    }
    else
    {
        msg << "Cast " << spellName.str() << " is failed";
        botAI->TellError(msg.str());
    }

    return result;
}

bool CastCustomNcSpellAction::isUseful()
{
    return !bot->IsInCombat();
}

std::string const& CastCustomNcSpellAction::castString(WorldObject* target)
{
    return "castnc " + chat->formatWorldobject(target);
}

bool CastRandomSpellAction::Execute(Event event)
{
    PlayerSpellMap const& spellMap = bot->GetSpellMap();
    Player* master = GetMaster();

    Unit* target = nullptr;
    GameObject* got = nullptr;

    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    std::vector<ObjectGuid> wos = chat->parseGameobjects(name);

    for (auto wo : wos)
    {
        target = botAI->GetUnit(wo);
        got = botAI->GetGameObject(wo);

        if (got || target)
            break;
    }

    if (!got && !target && bot->GetTarget())
    {
        target = botAI->GetUnit(bot->GetTarget());
        got = botAI->GetGameObject(bot->GetTarget());
    }

    if (!got && !target)
        if (master && master->GetTarget())
            target = botAI->GetUnit(master->GetTarget());

    if (!got && !target)
        target = bot;

    std::vector<std::pair<uint32, std::pair<Unit*, GameObject*>>> spellList;

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second->State == PLAYERSPELL_REMOVED || !spell.second->Active)
            continue;

        SpellInfo const* pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->IsPassive() || !AcceptSpell(pSpellInfo))
            continue;

        if (pSpellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL || pSpellInfo->Effects[0].Effect == SPELL_EFFECT_TRADE_SKILL)
            continue;

        if (bot->HasSpell(spellId))
        {
            if (target && botAI->CanCastSpell(spellId, target, true))
                spellList.push_back(std::make_pair(spellId, std::make_pair(target, nullptr)));
            if (got && botAI->CanCastSpell(spellId, got->GetPositionX(), got->GetPositionY(), got->GetPositionZ(), true))
                spellList.push_back(std::make_pair(spell.first, std::make_pair(nullptr, got)));
            if (botAI->CanCastSpell(spellId, bot, true))
                spellList.push_back(std::make_pair(spellId, std::make_pair(bot, nullptr)));
        }
    }

    if (spellList.empty())
        return false;

    bool isCast = false;

    for (uint32 i = 0; i < 5; i++)
    {
        uint32 rnd = urand(0, spellList.size() - 1);

        uint32 spellId = spellList[rnd].first;

        Unit* unit = spellList[rnd].second.first;
        GameObject* go = spellList[rnd].second.second;

        if (unit)
            isCast = botAI->CastSpell(spellId, unit);
        else
            isCast = botAI->CastSpell(spellId, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());

        if (isCast)
        {
            if (MultiCast && ((unit && bot->HasInArc(CAST_ANGLE_IN_FRONT, unit, sPlayerbotAIConfig->sightDistance)) ||
                (go && bot->HasInArc(CAST_ANGLE_IN_FRONT, unit, sPlayerbotAIConfig->sightDistance))))
            {
                std::ostringstream cmd;
                if (unit)
                    cmd << "castnc " << chat->formatWorldobject(unit) + " " << spellId << " " << 19;
                else
                    cmd << "castnc " << chat->formatWorldobject(go) + " " << spellId << " " << 19;

                botAI->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), bot);
            }
            return true;
        }
    }

    return false;
}

bool CraftRandomItemAction::AcceptSpell(SpellInfo const* pSpellInfo)
{
    return pSpellInfo->Effects[0].Effect == SPELL_EFFECT_CREATE_ITEM;
}
