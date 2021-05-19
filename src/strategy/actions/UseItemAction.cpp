/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UseItemAction.h"
#include "../Event.h"
#include "../../ChatHelper.h"
#include "../../Playerbot.h"

bool UseItemAction::Execute(Event event)
{
    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", name);
    GuidVector gos = chat->parseGameobjects(name);

    if (gos.empty())
    {
        if (items.size() > 1)
        {
            std::vector<Item*>::iterator i = items.begin();
            Item* item = *i++;
            Item* itemTarget = *i;
            return UseItemOnItem(item, itemTarget);
        }
        else if (!items.empty())
            return UseItemAuto(*items.begin());
    }
    else
    {
        if (items.empty())
            return UseGameObject(*gos.begin());
        else
            return UseItemOnGameObject(*items.begin(), *gos.begin());
    }

    botAI->TellError("No items (or game objects) available");
    return false;
}

bool UseItemAction::UseGameObject(ObjectGuid guid)
{
    GameObject* go = botAI->GetGameObject(guid);
    if (!go || !go->isSpawned() || go->GetGoState() != GO_STATE_READY)
        return false;

    go->Use(bot);

    std::ostringstream out;
    out << "Using " << chat->formatGameobject(go);
    botAI->TellMasterNoFacing(out.str());
    return true;
}

bool UseItemAction::UseItemAuto(Item* item)
{
    return UseItem(item, ObjectGuid::Empty, nullptr);
}

bool UseItemAction::UseItemOnGameObject(Item* item, ObjectGuid go)
{
    return UseItem(item, go, nullptr);
}

bool UseItemAction::UseItemOnItem(Item* item, Item* itemTarget)
{
    return UseItem(item, ObjectGuid::Empty, itemTarget);
}

bool UseItemAction::UseItem(Item* item, ObjectGuid goGuid, Item* itemTarget)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 spell_index = 0;
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 unk_flags = 0;
    uint32 targetFlag = TARGET_FLAG_NONE;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << spell_index;
    packet << cast_count << item_guid;

    bool targetSelected = false;

    std::ostringstream out;
    out << "Using " << chat->formatItem(item->GetTemplate());

    if (item->GetTemplate()->Stackable > 1)
    {
        uint32 count = item->GetCount();
        if (count > 1)
            out << " (" << count << " available) ";
        else
            out << " (the last one!)";
    }

    if (goGuid)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || !go->isSpawned())
            return false;

        targetFlag = TARGET_FLAG_GAMEOBJECT;

        packet << targetFlag;
        packet << goGuid.WriteAsPacked();
        out << " on " << chat->formatGameobject(go);
        targetSelected = true;
    }

    if (itemTarget)
    {
        targetFlag = TARGET_FLAG_ITEM;
        packet << targetFlag;
        packet << itemTarget->GetGUID().WriteAsPacked();
        out << " on " << chat->formatItem(itemTarget->GetTemplate());
        targetSelected = true;
    }

	Player* master = GetMaster();
	if (!targetSelected && item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE && master)
	{
		if (ObjectGuid masterSelection = master->GetTarget())
		{
			if (Unit* unit = botAI->GetUnit(masterSelection))
			{
			    targetFlag = TARGET_FLAG_UNIT;
				packet << targetFlag << masterSelection.WriteAsPacked();
				out << " on " << unit->GetName();
				targetSelected = true;
			}
		}
	}

    if (uint32 questid = item->GetTemplate()->StartQuest)
    {
        if (Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid))
        {
            WorldPacket packet(CMSG_QUESTGIVER_ACCEPT_QUEST, 8 + 4 + 4);
            packet << item_guid;
            packet << questid;
            packet << uint32(0);
            bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);

            std::ostringstream out;
            out << "Got quest " << chat->formatQuest(qInfo);
            botAI->TellMasterNoFacing(out.str());
            return true;
        }
    }

    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    if (bot->isMoving())
    {
        bot->StopMoving();
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        return false;
    }

    for (uint8 i = 0; i<MAX_ITEM_PROTO_SPELLS; i++)
    {
        uint32 spellId = item->GetTemplate()->Spells[i].SpellId;
        if (!spellId)
            continue;

        if (!botAI->CanCastSpell(spellId, bot, false))
            continue;

		SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
		if (spellInfo->Targets & TARGET_FLAG_ITEM)
        {
            Item* itemForSpell = AI_VALUE2(Item*, "item for spell", spellId);
            if (!itemForSpell)
                continue;

            if (itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                continue;

            if (bot->GetTrader())
            {
                if (selfOnly)
                    return false;

                targetFlag = TARGET_FLAG_TRADE_ITEM;
                packet << targetFlag << (uint8)1 << ObjectGuid((uint64)TRADE_SLOT_NONTRADED).WriteAsPacked();
                targetSelected = true;
                out << " on traded item";
            }
            else
            {
                targetFlag = TARGET_FLAG_ITEM;
                packet << targetFlag;
                packet << itemForSpell->GetGUID().WriteAsPacked();
                targetSelected = true;
                out << " on "<< chat->formatItem(itemForSpell->GetTemplate());
            }

            Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
            botAI->WaitForSpellCast(spell);
            delete spell;
        }

        break;
    }

    if (!targetSelected)
    {
        targetFlag = TARGET_FLAG_NONE;
        packet << targetFlag;
        packet << bot->GetPackGUID();
        targetSelected = true;
        out << " on self";
    }

    ItemTemplate const* proto = item->GetTemplate();
    bool isDrink = proto->Spells[0].SpellCategory == 59;
    bool isFood = proto->Spells[0].SpellCategory == 11;
    if (proto->Class == ITEM_CLASS_CONSUMABLE && (proto->SubClass == ITEM_SUBCLASS_FOOD || proto->SubClass == ITEM_SUBCLASS_CONSUMABLE) && (isFood || isDrink))
    {
        if (bot->IsInCombat())
            return false;

        bot->AddUnitState(UNIT_STAND_STATE_SIT);
        botAI->InterruptSpell();

        float hp = bot->GetHealthPct();
        float mp = bot->GetPower(POWER_MANA) * 100.0f / bot->GetMaxPower(POWER_MANA);
        float p = 0.f;
        if (isDrink && isFood)
        {
            p = min(hp, mp);
            TellConsumableUse(item, "Feasting", p);
        }
        else if (isDrink)
        {
            p = mp;
            TellConsumableUse(item, "Drinking", p);
        }
        else if (isFood)
        {
            p = hp;
            TellConsumableUse(item, "Eating", p);
        }

        botAI->SetNextCheckDelay(27000.0f * (100 - p) / 100.0f);
        bot->GetSession()->HandleUseItemOpcode(packet);

        return true;
    }

    botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
    botAI->TellMasterNoFacing(out.str());
    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

void UseItemAction::TellConsumableUse(Item* item, std::string const& action, float percent)
{
    std::ostringstream out;
    out << action << " " << chat->formatItem(item->GetTemplate());

    if (item->GetTemplate()->Stackable > 1)
        out << "/x" << item->GetCount();

    out  << " (" << round(percent) << "%)";
    botAI->TellMasterNoFacing(out.str());
}

bool UseItemAction::isPossible()
{
    return getName() == "use" || AI_VALUE2(uint8, "item count", getName()) > 0;
}

bool UseSpellItemAction::isUseful()
{
    return AI_VALUE2(bool, "spell cast useful", getName());
}

bool UseHealingPotion::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target");
}

bool UseManaPotion::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target");
}
