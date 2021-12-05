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

enum ItemUsage : uint32
{
    ITEM_USAGE_NONE         = 0,
    ITEM_USAGE_EQUIP        = 1,
    ITEM_USAGE_REPLACE      = 2,
    ITEM_USAGE_BAD_EQUIP    = 3,
    ITEM_USAGE_BROKEN_EQUIP = 4,
    ITEM_USAGE_QUEST        = 5,
    ITEM_USAGE_SKILL        = 6,
    ITEM_USAGE_USE          = 7,
    ITEM_USAGE_GUILD_TASK   = 8,
    ITEM_USAGE_DISENCHANT   = 9,
    ITEM_USAGE_AH           = 10,
    ITEM_USAGE_KEEP         = 11,
    ITEM_USAGE_VENDOR       = 12,
    ITEM_USAGE_AMMO         = 13
};

class ItemUsageValue : public CalculatedValue<ItemUsage>, public Qualified
{
	public:
        ItemUsageValue(PlayerbotAI* botAI, string name = "item usage") : CalculatedValue<ItemUsage>(ai, name) {}

        ItemUsage Calculate() override;

    private:
        ItemUsage QueryItemUsageForEquip(ItemTemplate const* proto);
        uint32 GetSmallestBagSize();
        bool IsItemUsefulForQuest(Player* player, ItemPrototype const* proto);
        bool IsItemNeededForSkill(ItemPrototype const* proto);
        bool IsItemUsefulForSkill(ItemTemplate const* proto);
        bool IsItemNeededForUsefullSpell(ItemPrototype const* proto, bool checkAllReagents = false);
        bool HasItemsNeededForSpell(uint32 spellId, ItemPrototype const* proto);
        Item* CurrentItem(ItemPrototype const* proto);
        float CurrentStacks(ItemPrototype const* proto);
        float BetterStacks(ItemPrototype const* proto, string usageType = "");

    public:
        static vector<uint32> SpellsUsingItem(uint32 itemId, Player* bot);
        static bool SpellGivesSkillUp(uint32 spellId, Player* bot);

        static string GetConsumableType(ItemPrototype const* proto, bool hasMana);
};

#endif
