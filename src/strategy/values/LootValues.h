/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTVALUES_H
#define _PLAYERBOT_LOOTVALUES_H

#include "LootMgr.h"
#include "NamedObjectContext.h"
#include "ItemUsageValue.h"
#include "Value.h"

class PlayerbotAI;

//Cheat class copy to hack into the loot system
class LootTemplateAccess
{
    public:
        class LootGroup;                                   // A set of loot definitions for items (refs are not allowed inside)
        typedef std::vector<LootGroup> LootGroups;
        LootStoreItemList Entries;                          // not grouped only
        LootGroups        Groups;                           // groups have own (optimized) processing, grouped entries go there
};

//                   itemId, entry
typedef unordered_map<uint32, int32> DropMap;

//Returns the loot map of all entries
class DropMapValue : public SingleCalculatedValue<DropMap*>
{
    public:
        DropMapValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "drop map") {}

        static LootTemplateAccess const* GetLootTemplate(ObjectGuid guid, LootType type = LOOT_CORPSE);

        DropMap* Calculate() override;
};

//Returns the entries that drop a specific item
class ItemDropListValue : public SingleCalculatedValue<list<int32>>, public Qualified
{
    public:
        ItemDropListValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "item drop list") {}

        list<int32> Calculate() override;
};

//Returns the items a specific entry can drop
class EntryLootListValue : public SingleCalculatedValue<list<uint32>>, public Qualified
{
    public:
        EntryLootListValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "entry loot list") {}

        list<uint32> Calculate() override;
};

typedef unordered_map<ItemUsage, vector<uint32>> itemUsageMap;

class EntryLootUsageValue : public CalculatedValue<itemUsageMap>, public Qualified
{
    public:
        EntryLootUsageValue(PlayerbotAI* ai) : CalculatedValue(ai, "entry loot usage", 2) {}

        itemUsageMap Calculate() override;
};

class HasUpgradeValue : public BoolCalculatedValue, public Qualified
{
    public:
        HasUpgradeValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has upgrade", 2) {}

        bool Calculate() override;
};

#endif
