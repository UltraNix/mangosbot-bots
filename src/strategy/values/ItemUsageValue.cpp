/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ItemUsageValue.h"
#include "ChatHelper.h"
#include "GuildTaskMgr.h"
#include "Playerbot.h"
#include "RandomItemMgr.h"

ItemUsage ItemUsageValue::Calculate()
{
    uint32 itemId = atoi(qualifier.c_str());
    if (!itemId)
        return ITEM_USAGE_NONE;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return ITEM_USAGE_NONE;

    if (IsItemUsefulForSkill(proto))
        return ITEM_USAGE_SKILL;

    switch (proto->Class)
    {
        case ITEM_CLASS_KEY:
            return ITEM_USAGE_USE;
    }

    if (proto->Class == ITEM_CLASS_CONSUMABLE)
    {
        std::string foodType = "";
        if (proto->Spells[0].SpellCategory == 11)
            foodType = "food";
        else if (proto->Spells[0].SpellCategory == 59)
            foodType = "drink";
        else if (proto->Spells[0].SpellCategory == SPELL_EFFECT_ENERGIZE)
            foodType = "mana potion";
        else if (proto->Spells[0].SpellCategory == SPELL_EFFECT_HEAL)
            foodType = "healing potion";

        std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", foodType);

        bool foundBetter = false;;

        for (auto& otherItem : items)
        {
            ItemTemplate const* otherProto = otherItem->GetTemplate();

            if (otherProto->Class != ITEM_CLASS_CONSUMABLE || otherProto->SubClass != proto->SubClass)
                continue;

            if (otherProto->ItemLevel < proto->ItemLevel)
                continue;

            if (otherProto->ItemId == proto->ItemId)
                continue;

            foundBetter = true;
        }

        if (!foundBetter)
            return ITEM_USAGE_USE;
    }

    if (bot->GetGuildId() && sGuildTaskMgr->IsGuildTaskItem(itemId, bot->GetGuildId()))
        return ITEM_USAGE_GUILD_TASK;

    ItemUsage equip = QueryItemUsageForEquip(proto);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) && proto->Bonding != BIND_WHEN_PICKED_UP &&
            botAI->HasSkill(SKILL_ENCHANTING) && proto->Quality >= ITEM_QUALITY_UNCOMMON)
        return ITEM_USAGE_DISENCHANT;

    //While sync is on, do not loot quest items that are also Useful for master. Master
    if (!botAI->GetMaster() || !sPlayerbotAIConfig->syncQuestWithPlayer || !IsItemUsefulForQuest(botAI->GetMaster(), itemId))
        if (IsItemUsefulForQuest(bot, itemId))
            return ITEM_USAGE_QUEST;

    if (proto->Class == ITEM_CLASS_PROJECTILE)
        if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR)
        {
            if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            {
                uint32 subClass = 0;
                switch (pItem->GetTemplate()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_GUN:
                        subClass = ITEM_SUBCLASS_BULLET;
                        break;
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                        subClass = ITEM_SUBCLASS_ARROW;
                        break;
                }

                if (proto->SubClass == subClass)
                    return ITEM_USAGE_AMMO;
            }

            return ITEM_USAGE_NONE;
        }

    //Need to add something like free bagspace or item value.
    if (proto->SellPrice > 0)
    {
        if (proto->Quality > ITEM_QUALITY_NORMAL)
        {
            return ITEM_USAGE_AH;
        }
        else
        {
            return ITEM_USAGE_VENDOR;
        }
    }

    return ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemTemplate const* item)
{
    bool shouldEquip = true;
    bool existingShouldEquip = true;

    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return ITEM_USAGE_NONE;

    if (item->InventoryType == INVTYPE_NON_EQUIP)
        return ITEM_USAGE_NONE;

    Item* pItem = Item::CreateItem(item->ItemId, 1, bot);
    if (!pItem)
        return ITEM_USAGE_NONE;

    uint16 dest;
    InventoryResult result = bot->CanEquipItem(NULL_SLOT, dest, pItem, true, false);
    pItem->RemoveFromUpdateQueueOf(bot);
    delete pItem;

    if (result != EQUIP_ERR_OK )
        return ITEM_USAGE_NONE;

    if (item->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot->getClass(), item))
        shouldEquip = false;

    if (item->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->getLevel(), item))
        shouldEquip = false;

    Item* existingItem = bot->GetItemByPos(dest);
    if (!existingItem)
    {
        if (shouldEquip)
            return ITEM_USAGE_EQUIP;
        else
            return ITEM_USAGE_BAD_EQUIP;
    }

    ItemTemplate const* oldItem = existingItem->GetTemplate();
    if (oldItem->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot->getClass(), oldItem))
        existingShouldEquip = false;

    if (oldItem->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->getLevel(), oldItem))
        existingShouldEquip = false;

    if (item->Class == ITEM_CLASS_CONTAINER)
    {
        if (item->SubClass != ITEM_SUBCLASS_CONTAINER)
            return ITEM_USAGE_NONE; //Todo add logic for non-bag containers. We want to look at professions/class and only replace if non-bag is larger than bag.

        if (GetSmallestBagSize() >= item->ContainerSlots)
            return ITEM_USAGE_NONE;
    }

    if (item->Class == ITEM_CLASS_QUIVER)
        if (bot->getClass() != CLASS_HUNTER)
            return ITEM_USAGE_NONE;
        else
            return ITEM_USAGE_EQUIP; //Todo add logic for replacing larger quiver. Quiver is probably better than a bag as long as it is equal or bigger than current bag.

    if (oldItem->ItemId != item->ItemId && //Item is not identical
        (shouldEquip || !existingShouldEquip) && //New item is optimal or old item was already sub-optimal
        (oldItem->ItemLevel + oldItem->Quality * 5 < item->ItemLevel + item->Quality * 5)) // Item is upgrade
    {
        switch (item->Class)
        {
            case ITEM_CLASS_ARMOR:
                if (oldItem->SubClass <= item->SubClass)
                {
                    if (shouldEquip)
                        return ITEM_USAGE_REPLACE;
                    else
                        return ITEM_USAGE_BAD_EQUIP;
                }
                break;
            default:
            {
                if (shouldEquip)
                    return ITEM_USAGE_EQUIP;
                else
                    return ITEM_USAGE_BAD_EQUIP;
            }
        }
    }

    return ITEM_USAGE_NONE;
}

//Return smaltest bag size equipped
uint32 ItemUsageValue::GetSmallestBagSize()
{
    int8 curSlot = 0;
    int8 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START + 1; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            if (curSlot > 0 && curSlots < pBag->GetBagSize())
                continue;

            curSlot = pBag->GetSlot();
            curSlots = pBag->GetBagSize();
        }
        else
            return 0;
    }

    return curSlots;
}

bool ItemUsageValue::IsItemUsefulForQuest(Player const* player, uint32 itemId)
{
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = player->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            continue;

        for (uint8 i = 0; i < 4; i++)
        {
            if (quest->RequiredItemId[i] == itemId)
            {
                return true;
            }
        }
    }

    return false;
}

bool ItemUsageValue::IsItemUsefulForSkill(ItemTemplate const*  proto)
{
    switch (proto->ItemId)
    {
        case 2901: //Mining pick
            return botAI->HasSkill(SKILL_MINING);
        case 5956: //Blacksmith Hammer
            return botAI->HasSkill(SKILL_BLACKSMITHING) || botAI->HasSkill(SKILL_ENGINEERING);
        case 6219: //Arclight Spanner
            return botAI->HasSkill(SKILL_ENGINEERING);
        case 16207: //Runed Arcanite Rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 7005: //Skinning Knife
            return botAI->HasSkill(SKILL_SKINNING);
        case 4471: //Flint and Tinder
            return botAI->HasSkill(SKILL_COOKING);
        case 4470: //Simple Wood
            return botAI->HasSkill(SKILL_COOKING);
        case 6256: //Fishing Rod
            return botAI->HasSkill(SKILL_FISHING);
    }

    uint32 maxStack = proto->GetMaxStackSize();

    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", chat->formatItem(proto));

    uint32 itemCount = 0;
    for (auto stack : found)
    {
        itemCount += stack->GetCount();
        if (itemCount > maxStack)
            return false;
    }

    if (AI_VALUE(uint8, "bag space") > 50)
        return false;

    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
        {
            if (botAI->HasSkill(SKILL_TAILORING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_TAILORING))
                return true;
            if (botAI->HasSkill(SKILL_LEATHERWORKING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING))
                return true;
            if (botAI->HasSkill(SKILL_ENGINEERING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING))
                return true;
            if (botAI->HasSkill(SKILL_BLACKSMITHING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING))
                return true;
            if (botAI->HasSkill(SKILL_ALCHEMY) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY))
                return true;
            if (botAI->HasSkill(SKILL_ENCHANTING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ENCHANTING))
                return true;
            if (botAI->HasSkill(SKILL_FISHING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_FISHING))
                return true;
            if (botAI->HasSkill(SKILL_FIRST_AID) && RandomItemMgr::IsUsedBySkill(proto, SKILL_FIRST_AID))
                return true;
            if (botAI->HasSkill(SKILL_COOKING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_COOKING))
                return true;
            if (botAI->HasSkill(SKILL_JEWELCRAFTING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING))
                return true;
            if (botAI->HasSkill(SKILL_MINING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_MINING) || RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING) ||
                RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING) || RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING)))
                return true;
            if (botAI->HasSkill(SKILL_SKINNING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_SKINNING) || RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING)))
                return true;
            if (botAI->HasSkill(SKILL_HERBALISM) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_HERBALISM) || RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY)))
                return true;

            return false;
        }
        case ITEM_CLASS_RECIPE:
        {
            if (bot->HasSpell(proto->Spells[2].SpellId))
                break;

            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                    return botAI->HasSkill(SKILL_LEATHERWORKING);
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                    return botAI->HasSkill(SKILL_TAILORING);
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                    return botAI->HasSkill(SKILL_ENGINEERING);
                case ITEM_SUBCLASS_BLACKSMITHING:
                    return botAI->HasSkill(SKILL_BLACKSMITHING);
                case ITEM_SUBCLASS_COOKING_RECIPE:
                    return botAI->HasSkill(SKILL_COOKING);
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                    return botAI->HasSkill(SKILL_ALCHEMY);
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                    return botAI->HasSkill(SKILL_FIRST_AID);
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                    return botAI->HasSkill(SKILL_ENCHANTING);
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return botAI->HasSkill(SKILL_FISHING);
            }
        }
    }

    return false;
}
