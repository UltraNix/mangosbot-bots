/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ItemUsageValue.h"
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
        case ITEM_CLASS_CONSUMABLE:
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

    return ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemTemplate const* item)
{
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
        return ITEM_USAGE_NONE;

    if (item->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->getLevel(), item))
        return ITEM_USAGE_NONE;

    Item* existingItem = bot->GetItemByPos(dest);
    if (!existingItem)
        return ITEM_USAGE_EQUIP;

    ItemTemplate const* oldItem = existingItem->GetTemplate();
    if (oldItem->ItemId != item->ItemId && (oldItem->ItemLevel < item->ItemLevel || oldItem->Quality < item->Quality))
    {
        switch (item->Class)
        {
            case ITEM_CLASS_ARMOR:
                if (oldItem->SubClass <= item->SubClass)
                {
                    return ITEM_USAGE_REPLACE;
                }
                break;
            default:
                return ITEM_USAGE_EQUIP;
        }
    }

    return ITEM_USAGE_NONE;
}

bool ItemUsageValue::IsItemUsefulForSkill(ItemTemplate const*  proto)
{
    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        {
            if (botAI->HasSkill(SKILL_TAILORING) && auctionbot.IsUsedBySkill(proto, SKILL_TAILORING))
                return true;
            if (botAI->HasSkill(SKILL_LEATHERWORKING) && auctionbot.IsUsedBySkill(proto, SKILL_LEATHERWORKING))
                return true;
            if (botAI->HasSkill(SKILL_ENGINEERING) && auctionbot.IsUsedBySkill(proto, SKILL_ENGINEERING))
                return true;
            if (botAI->HasSkill(SKILL_BLACKSMITHING) && auctionbot.IsUsedBySkill(proto, SKILL_BLACKSMITHING))
                return true;
            if (botAI->HasSkill(SKILL_ALCHEMY) && auctionbot.IsUsedBySkill(proto, SKILL_ALCHEMY))
                return true;
            if (botAI->HasSkill(SKILL_ENCHANTING) && auctionbot.IsUsedBySkill(proto, SKILL_ENCHANTING))
                return true;
            if (botAI->HasSkill(SKILL_FISHING) && auctionbot.IsUsedBySkill(proto, SKILL_FISHING))
                return true;
            if (botAI->HasSkill(SKILL_FIRST_AID) && auctionbot.IsUsedBySkill(proto, SKILL_FIRST_AID))
                return true;
            if (botAI->HasSkill(SKILL_COOKING) && auctionbot.IsUsedBySkill(proto, SKILL_COOKING))
                return true;
            if (botAI->HasSkill(SKILL_JEWELCRAFTING) && auctionbot.IsUsedBySkill(proto, SKILL_JEWELCRAFTING))
                return true;
            if (botAI->HasSkill(SKILL_MINING) && (auctionbot.IsUsedBySkill(proto, SKILL_MINING) || auctionbot.IsUsedBySkill(proto, SKILL_BLACKSMITHING) ||
                auctionbot.IsUsedBySkill(proto, SKILL_JEWELCRAFTING) || auctionbot.IsUsedBySkill(proto, SKILL_ENGINEERING)))
                return true;
            if (botAI->HasSkill(SKILL_SKINNING) && (auctionbot.IsUsedBySkill(proto, SKILL_SKINNING) || auctionbot.IsUsedBySkill(proto, SKILL_LEATHERWORKING)))
                return true;
            if (botAI->HasSkill(SKILL_HERBALISM) && (auctionbot.IsUsedBySkill(proto, SKILL_HERBALISM) || auctionbot.IsUsedBySkill(proto, SKILL_ALCHEMY)))
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
