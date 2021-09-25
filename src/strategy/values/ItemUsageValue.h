/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ITEMUSAGEVALUE_H
#define _PLAYERBOT_ITEMUSAGEVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class Player;
class PlayerbotAI;

struct ItemTemplate;

enum ItemUsage
{
    ITEM_USAGE_NONE         = 0,
    ITEM_USAGE_EQUIP        = 1,
    ITEM_USAGE_REPLACE      = 2,
    ITEM_USAGE_BAD_EQUIP    = 3,
    ITEM_USAGE_QUEST        = 4,
    ITEM_USAGE_SKILL        = 5,
    ITEM_USAGE_USE          = 6,
    ITEM_USAGE_GUILD_TASK   = 7,
    ITEM_USAGE_DISENCHANT   = 8,
    ITEM_USAGE_AH           = 9,
    ITEM_USAGE_VENDOR       = 10,
    ITEM_USAGE_AMMO         = 11
};

class ItemUsageValue : public CalculatedValue<ItemUsage>, public Qualified
{
	public:
        ItemUsageValue(PlayerbotAI* botAI) : CalculatedValue<ItemUsage>(botAI) { }

        ItemUsage Calculate() override;

    private:
        ItemUsage QueryItemUsageForEquip(ItemTemplate const* proto);
        uint32 GetSmallestBagSize();
        bool IsItemUsefulForQuest(Player const* player, uint32 itemId);
        bool IsItemUsefulForSkill(ItemTemplate const* proto);
};

#endif
