/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RpgAction.h"
#include "BattlegroundMgr.h"
#include "ChatHelper.h"
#include "ChooseRpgTargetAction.h"
#include "EmoteAction.h"
#include "GossipDef.h"
#include "Event.h"
#include "PossibleRpgTargetsValue.h"
#include "Playerbot.h"

bool RpgAction::Execute(Event event)
{
    ObjectGuid guid = AI_VALUE(ObjectGuid, "rpg target");
    WorldObject* wo = botAI->GetWorldObject(guid);
    Unit* unit = botAI->GetUnit(guid);
    GameObject* go = botAI->GetGameObject(guid);
    if (!wo)
    {
        context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);
        return false;
    }

    if (bot->isMoving())
        return true;

    if (bot->GetMapId() != wo->GetMapId())
    {
        context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);
        return false;
    }

    if (!wo->HasInArc(CAST_ANGLE_IN_FRONT, bot, sPlayerbotAIConfig->sightDistance) && !bot->HasUnitState(UNIT_STATE_IN_FLIGHT) && !bot->IsFlying())
    {
        bot->SetFacingToObject(wo);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        return true;
    }

    if (unit && !bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE))
    {
        context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);
        return false;
    }

    if (bot->GetShapeshiftForm() > 0)
        bot->SetShapeshiftForm(FORM_NONE);

    //Random taxi action.
    if (unit && unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER) && !botAI->HasRealPlayerMaster())
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        taxi(guid);
        return true;
    }

    std::vector<RpgElement> elements;
    if (bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD2 ||
        bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD ||
        bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD_REP ||
        bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_AVAILABLE)
        elements.push_back(&RpgAction::quest);
    if (unit)
    {
        if (unit->IsVendor())
            elements.push_back(&RpgAction::trade);
        if (unit->IsArmorer() && (AI_VALUE(uint8, "durability") < 100 && AI_VALUE(uint32, "repair cost") < bot->GetMoney()))
            elements.push_back(&RpgAction::repair);
        if (CanTrain(guid))
            elements.push_back(&RpgAction::train);
        if (unit->GetHealthPct() < 100 && (bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_SHAMAN))
            elements.push_back(&RpgAction::heal);
        if (unit->IsInnkeeper() && AI_VALUE2(float, "distance", "home bind") > 1000.0f)
            elements.push_back(&RpgAction::homebind);
        if (unit->IsBattleMaster() && CanQueueBg(guid))
            elements.push_back(&RpgAction::queuebg);
    }
    else
    {
        if (go->GetGoState() == GO_STATE_READY)
        {
            elements.push_back(&RpgAction::use);
            elements.push_back(&RpgAction::work);
            elements.push_back(&RpgAction::spell);
            elements.push_back(&RpgAction::craft);
        }
    }

    if (AddIgnore(guid))
    {
        if (elements.empty() && ChooseRpgTargetAction::isFollowValid(bot, wo))
        {
            elements.push_back(&RpgAction::emote);
            elements.push_back(&RpgAction::stay);
            elements.push_back(&RpgAction::work);
            elements.push_back(&RpgAction::spell);
            elements.push_back(&RpgAction::craft);
        }
    }
    else
        elements.push_back(&RpgAction::cancel);

    if (elements.empty())
        elements.push_back(&RpgAction::cancel);

    RpgElement element = elements[urand(0, elements.size() - 1)];
    (this->*element)(guid);
    return true;
}

bool RpgAction::AddIgnore(ObjectGuid guid)
{
    if (HasIgnore(guid))
    {
        return false;
    }

    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();
    ignoreList.insert(guid);

    if (ignoreList.size() > 50)
        ignoreList.erase(ignoreList.begin());

    context->GetValue<GuidSet&>("ignore rpg target")->Set(ignoreList);

    return true;
}
bool RpgAction::RemIgnore(ObjectGuid guid)
{
    if (!HasIgnore(guid))
        return false;

    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();
    ignoreList.erase(ignoreList.find(guid));

    context->GetValue<GuidSet&>("ignore rpg target")->Set(ignoreList);

    return true;
}

bool RpgAction::HasIgnore(ObjectGuid guid)
{
    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();
    if (ignoreList.empty())
        return false;

    if (ignoreList.find(guid) == ignoreList.end())
        return false;

    return true;
}

void RpgAction::stay(ObjectGuid guid)
{
    if (bot->PlayerTalkClass)
        bot->PlayerTalkClass->SendCloseGossip();

    botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::work(ObjectGuid guid)
{
    bot->HandleEmoteCommand(EMOTE_STATE_USE_STANDING);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::emote(ObjectGuid guid)
{
    Unit* unit = botAI->GetUnit(guid);
    uint32 type = TalkAction::GetRandomEmote(unit);

    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    WorldPacket p1;
    p1 << guid;
    bot->GetSession()->HandleGossipHelloOpcode(p1);

    bot->HandleEmoteCommand(type);

    if (unit)
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::cancel(ObjectGuid guid)
{
    context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);
}

void RpgAction::taxi(ObjectGuid guid)
{
    Unit* unit = botAI->GetUnit(guid);
    uint32 curloc = sObjectMgr->GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), bot->GetTeamId());

    std::vector<uint32> nodes;
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            if (entry->from == curloc)
            {
                //uint8 field = uint8((i - 1) / 32);
                //if (field < TaxiMaskSize)
                nodes.push_back(i);
            }
    }

    if (nodes.empty())
    {
        LOG_ERROR("playerbots", "Bot %s - No flight paths available", bot->GetName().c_str());
        return;
    }

    context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);

    uint32 path = nodes[urand(0, nodes.size() - 1)];
    //bot->m_taxi.SetTaximaskNode(path); // Causes crash/freeze
    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 100000);

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
    if (!entry)
        return;

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
    {
        LOG_ERROR("playerbots", "Bot %s cannot talk to flightmaster (%zu location available)", bot->GetName().c_str(), nodes.size());
        return;
    }

    if (!bot->ActivateTaxiPathTo({ entry->from, entry->to }, flightMaster, 0))
    {
        LOG_ERROR("playerbots", "Bot %s cannot fly (%zu location available)", bot->GetName().c_str(), nodes.size());
        return;
    }

    LOG_INFO("playerbots", "Bot %s is flying to %u (%zu location available)", bot->GetName().c_str(), path, nodes.size());
    bot->SetMoney(money);
}

void RpgAction::quest(ObjectGuid guid)
{
    WorldObject* wo = botAI->GetWorldObject(guid);
    uint32 type = TalkAction::GetRandomEmote(botAI->GetUnit(guid));

    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    //Parse rpg target to quest action.
    WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
    p << guid;
    p.rpos(0);

    bool retVal = false;

    if (bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_AVAILABLE ||
        bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD_REP)
        retVal = botAI->DoSpecificAction("accept all quests", Event("rpg action", p));

    if (bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD2 ||
        bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD ||
        bot->GetQuestDialogStatus(wo) == DIALOG_STATUS_REWARD_REP)
        retVal = botAI->DoSpecificAction("talk to quest giver", Event("rpg action", p));

    if (!retVal)
        bot->HandleEmoteCommand(type);

    if (retVal)
        RemIgnore(guid);

    bot->HandleEmoteCommand(type);

    if (Unit* unit = wo->ToUnit())
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    //Speed up if
    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);

    cancel(guid);
}

void RpgAction::trade(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    botAI->DoSpecificAction("sell", Event("rpg action", "vendor"));
    botAI->DoSpecificAction("buy", Event("rpg action", "vendor"));

    if (Unit* unit = botAI->GetUnit(guid))
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::repair(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    botAI->DoSpecificAction("repair");

    if (Unit* unit = botAI->GetUnit(guid))
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::train(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    botAI->DoSpecificAction("trainer");

    if (Unit* unit = botAI->GetUnit(guid))
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::heal(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            botAI->DoSpecificAction("lesser heal on party");
            /* Example to cast a specific spel on a specif unit. Maybe Useful later.
            if(!botAI->HasAura("power word: fortitude", unit))
                botAI->DoSpecificAction("cast custom spell", Event("rpg action", chat->formatWorldobject(unit) + " power word: fortitude"), true);
            else
                botAI->DoSpecificAction("cast custom spell", Event("rpg action", chat->formatWorldobject(unit) + " 2052"), true);
            */
            break;
        case CLASS_DRUID:
            botAI->DoSpecificAction("healing touch on party");
            break;
        case CLASS_PALADIN:
            botAI->DoSpecificAction("holy light on party");
            break;
        case CLASS_SHAMAN:
            botAI->DoSpecificAction("healing wave on party");
            break;
    }

    if (Unit* unit = botAI->GetUnit(guid))
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::use(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    WorldObject* wo = botAI->GetWorldObject(guid);

    botAI->DoSpecificAction("use", Event("rpg action", chat->formatWorldobject(wo)));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::spell(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    WorldObject* wo = botAI->GetWorldObject(guid);

    botAI->DoSpecificAction("cast random spell", Event("rpg action", chat->formatWorldobject(wo)));

    Unit* unit = botAI->GetUnit(guid);
    if (unit)
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::craft(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    WorldObject* wo = botAI->GetWorldObject(guid);

    bool crafted = botAI->DoSpecificAction("craft random item", Event("rpg action", chat->formatWorldobject(wo)));

    Unit* unit = botAI->GetUnit(guid);
    if (unit)
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (crafted)
        RemIgnore(guid);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::homebind(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    botAI->DoSpecificAction("home");

    Unit* unit = botAI->GetUnit(guid);
    if (unit)
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

void RpgAction::queuebg(ObjectGuid guid)
{
    ObjectGuid oldSelection = bot->GetTarget();

    bot->SetTarget(guid);

    BattlegroundTypeId bgTypeId = CanQueueBg(guid);

    bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("bg type")->Set((uint32)bgTypeId);
    botAI->DoSpecificAction("free bg join");

    Unit* unit = botAI->GetUnit(guid);
    if (unit)
        unit->SetFacingTo(unit->GetAngle(bot));

    if (oldSelection)
        bot->SetTarget(oldSelection);

    if (!botAI->HasRealPlayerMaster())
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
}

bool RpgAction::isUseful()
{
    return context->GetValue<ObjectGuid>("rpg target")->Get();
}

bool RpgAction::CanTrain(ObjectGuid guid)
{
    Creature* creature = botAI->GetCreature(guid);
    if (!creature)
        return false;

    if (!creature->IsValidTrainerForPlayer(bot))
        return false;

    // check present spell in trainer spell list
    TrainerSpellData const* cSpells = creature->GetTrainerSpells();
    if (!cSpells)
    {
        return false;
    }

    float fDiscountMod = bot->GetReputationPriceDiscount(creature);

    TrainerSpellData const* trainer_spells = cSpells;
    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;
        if (!tSpell)
            continue;

        uint32 reqLevel = 0;

        TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
        if (state != TRAINER_SPELL_GREEN)
            continue;

        uint32 spellId = tSpell->spell;
        SpellInfo const* pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        uint32 cost = uint32(floor(tSpell->spellCost * fDiscountMod));
        if (cost > bot->GetMoney())
            continue;

        return true;
    }

    return false;
}

BattlegroundTypeId RpgAction::CanQueueBg(ObjectGuid guid)
{
    for (uint32 i = 1; i < MAX_BATTLEGROUND_QUEUE_TYPES; i++)
    {
        BattlegroundQueueTypeId queueTypeId = (BattlegroundQueueTypeId)i;

        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);

        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
            continue;

        if (bot->getLevel() < bg->GetMinLevel())
            continue;

        // check if already in queue
        if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
            continue;

        std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> battleMastersCache = sRandomPlayerbotMgr->getBattleMastersCache();

        for (auto& entry : battleMastersCache[TEAM_NEUTRAL][bgTypeId])
            if (entry == guid.GetEntry())
                return bgTypeId;

        for (auto& entry : battleMastersCache[bot->GetTeamId()][bgTypeId])
            if (entry == guid.GetEntry())
                return bgTypeId;
    }

    return BATTLEGROUND_TYPE_NONE;
}
