/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomItemMgr.h"
#include "Playerbot.h"

char* strstri(char const* str1, char const* str2);

uint64 BotEquipKey::GetKey()
{
    return level + 100 * clazz + 10000 * slot + 1000000 * quality;
}

class RandomItemGuildTaskPredicate : public RandomItemPredicate
{
    public:
        bool Apply(ItemTemplate const* proto) override
        {
            if (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
                return false;

            if (proto->Quality < ITEM_QUALITY_NORMAL)
                return false;

            if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) && proto->Quality >= ITEM_QUALITY_RARE)
                return true;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_CONSUMABLE)
                return true;

            return false;
        }
};

class RandomItemGuildTaskRewardPredicate : public RandomItemPredicate
{
    public:
        RandomItemGuildTaskRewardPredicate(bool equip, bool rare) : equip(equip), rare(rare) { }

        bool Apply(ItemTemplate const* proto) override
        {
            if (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
                return false;

            if (proto->Class == ITEM_CLASS_QUEST)
                return false;

            if (equip)
            {
                uint32 desiredQuality = rare ? ITEM_QUALITY_RARE : ITEM_QUALITY_UNCOMMON;
                if (proto->Quality < desiredQuality || proto->Quality >= ITEM_QUALITY_EPIC)
                    return false;

                if (proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON)
                    return true;
            }
            else
            {
                uint32 desiredQuality = rare ? ITEM_QUALITY_UNCOMMON : ITEM_QUALITY_NORMAL;
                if (proto->Quality < desiredQuality || proto->Quality >= ITEM_QUALITY_RARE)
                    return false;

                if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_CONSUMABLE)
                    return true;
            }

            return false;
        }

    private:
        bool equip;
        bool rare;
};

RandomItemMgr::RandomItemMgr()
{
    predicates[RANDOM_ITEM_GUILD_TASK] = new RandomItemGuildTaskPredicate();
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN] = new RandomItemGuildTaskRewardPredicate(true, false);
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE] = new RandomItemGuildTaskRewardPredicate(true, true);
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_TRADE] = new RandomItemGuildTaskRewardPredicate(false, false);
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE] = new RandomItemGuildTaskRewardPredicate(false, true);

    viableSlots[EQUIPMENT_SLOT_HEAD].insert(INVTYPE_HEAD);
    viableSlots[EQUIPMENT_SLOT_NECK].insert(INVTYPE_NECK);
    viableSlots[EQUIPMENT_SLOT_SHOULDERS].insert(INVTYPE_SHOULDERS);
    viableSlots[EQUIPMENT_SLOT_BODY].insert(INVTYPE_BODY);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_CHEST);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_ROBE);
    viableSlots[EQUIPMENT_SLOT_WAIST].insert(INVTYPE_WAIST);
    viableSlots[EQUIPMENT_SLOT_LEGS].insert(INVTYPE_LEGS);
    viableSlots[EQUIPMENT_SLOT_FEET].insert(INVTYPE_FEET);
    viableSlots[EQUIPMENT_SLOT_WRISTS].insert(INVTYPE_WRISTS);
    viableSlots[EQUIPMENT_SLOT_HANDS].insert(INVTYPE_HANDS);
    viableSlots[EQUIPMENT_SLOT_FINGER1].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_FINGER2].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_TRINKET1].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_TRINKET2].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_SHIELD);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_HOLDABLE);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGED);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_THROWN);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGEDRIGHT);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RELIC);
    viableSlots[EQUIPMENT_SLOT_TABARD].insert(INVTYPE_TABARD);
    viableSlots[EQUIPMENT_SLOT_BACK].insert(INVTYPE_CLOAK);

}

void RandomItemMgr::Init()
{
    BuildEquipCache();
    BuildAmmoCache();
    BuildPotionCache();
    BuildFoodCache();
    BuildTradeCache();
}

void RandomItemMgr::InitAfterAhBot()
{
    BuildRandomItemCache();
    BuildRarityCache();
}

RandomItemMgr::~RandomItemMgr()
{
    for (std::map<RandomItemType, RandomItemPredicate*>::iterator i = predicates.begin(); i != predicates.end(); ++i)
        delete i->second;

    predicates.clear();
}

bool RandomItemMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        LOG_ERROR("playerbots",  "Usage: rnditem");
        return false;
    }

    return false;
}

RandomItemList RandomItemMgr::Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList& list = randomItemCache[(level - 1) / 10][type];

    RandomItemList result;
    for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
    {
        uint32 itemId = *i;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        if (predicate && !predicate->Apply(proto))
            continue;

        result.push_back(itemId);
    }

    return result;
}

void RandomItemMgr::BuildRandomItemCache()
{
    QueryResult results = PlayerbotDatabase.PQuery("SELECT lvl, type, item FROM playerbot_rnditem_cache");
    if (results)
    {
        LOG_INFO("playerbots", "Loading random item cache");
        uint32 count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 level = fields[0].GetUInt32();
            uint32 type = fields[1].GetUInt32();
            uint32 itemId = fields[2].GetUInt32();

            RandomItemType rit = (RandomItemType)type;
            randomItemCache[level][rit].push_back(itemId);
            ++count;

        } while (results->NextRow());

        LOG_INFO("playerbots", "Equipment cache loaded from %d records", count);
    }
    else
    {
        ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
        LOG_INFO("playerbots", "Building random item cache from %u items", itemTemplates->size());

        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (strstri(proto->Name1.c_str(), "qa") || strstri(proto->Name1.c_str(), "test") || strstri(proto->Name1.c_str(), "deprecated"))
                continue;

            if (!proto->ItemLevel)
                continue;

            if (!proto->SellPrice)
                continue;

            uint32 level = proto->ItemLevel;
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemType rit = (RandomItemType)type;
                if (predicates[rit] && !predicates[rit]->Apply(proto))
                    continue;

                randomItemCache[level / 10][rit].push_back(itr.first);

                PlayerbotDatabase.PExecute("INSERT INTO playerbot_rnditem_cache (lvl, type, item) VALUES (%u, %u, %u)", level / 10, type, itr.first);
            }
        }

        uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
        if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

        for (uint32 level = 0; level < maxLevel / 10; level++)
        {
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemList list = randomItemCache[level][(RandomItemType)type];
                LOG_INFO("playerbots", "    Level %d..%d Type %d - %zu random items cached", level * 10, level * 10 + 9, type, list.size());

                for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
                {
                    uint32 itemId = *i;
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (!proto)
                        continue;

                    LOG_INFO("playerbots", "        [%d] %s", itemId, proto->Name1.c_str());
                }
            }
        }
    }
}

uint32 RandomItemMgr::GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList const& list = Query(level, type, predicate);
    if (list.empty())
        return 0;

    uint32 index = urand(0, list.size() - 1);
    uint32 itemId = list[index];

    return itemId;
}

bool RandomItemMgr::CanEquipItem(BotEquipKey key, ItemTemplate const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Quality != key.quality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    std::set<InventoryType> slots = viableSlots[(EquipmentSlots)key.slot];
    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
        return false;

    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
        return false;

    uint32 level = key.level;
    uint32 delta = 2;
    if (level < 15)
        delta = urand(7, 15);
    else if (proto->Class == ITEM_CLASS_WEAPON || proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        delta = urand(2, 3);
    else if (!(level % 10) || (level % 10) == 9)
        delta = 2;
    else if (level < 40)
        delta = urand(5, 10);
    else if (level < 60)
        delta = urand(3, 7);
    else if (level < 70)
        delta = urand(2, 5);
    else if (level < 80)
        delta = urand(2, 4);

    if (key.quality > ITEM_QUALITY_NORMAL && (requiredLevel > level || requiredLevel < level - delta))
        return false;

    for (uint32 gap = 60; gap <= 80; gap += 10)
    {
        if (level > gap && requiredLevel <= gap)
            return false;
    }

    return true;
}


void RandomItemMgr::AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank)
{
    switch (mod)
    {
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
        case ITEM_MOD_MANA:
        case ITEM_MOD_INTELLECT:
        case ITEM_MOD_SPIRIT:
            ++sp;
            break;
    }

    switch (mod)
    {
        case ITEM_MOD_AGILITY:
        case ITEM_MOD_STRENGTH:
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
            ++tank;
            break;
    }

    switch (mod)
    {
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
        case ITEM_MOD_AGILITY:
        case ITEM_MOD_STRENGTH:
            ++ap;
            break;
    }
}

bool RandomItemMgr::CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank)
{
    switch (clazz)
    {
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            if (!sp || ap > sp || tank > sp)
                return false;
            break;
        case CLASS_PALADIN:
        case CLASS_WARRIOR:
            if ((!ap && !tank) || sp > ap || sp > tank)
                return false;
            break;
        case CLASS_HUNTER:
        case CLASS_ROGUE:
            if (!ap || sp > ap || sp > tank)
                return false;
            break;
    }

    return sp || ap || tank;
}

bool RandomItemMgr::CanEquipArmor(uint8 clazz, uint32 level, ItemTemplate const* proto)
{
    if ((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN || clazz == CLASS_SHAMAN) && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        return true;

    if ((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN) && level >= 40)
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_PLATE && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN) && level < 40) || ((clazz == CLASS_HUNTER || clazz == CLASS_SHAMAN) && level >= 40))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_MAIL && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (((clazz == CLASS_HUNTER || clazz == CLASS_SHAMAN) && level < 40) || (clazz == CLASS_DRUID || clazz == CLASS_ROGUE))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_LEATHER && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (proto->Quality <= ITEM_QUALITY_NORMAL)
        return true;

    uint8 sp = 0, ap = 0, tank = 0;
    for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        // for ItemStatValue != 0
        if (!proto->ItemStat[j].ItemStatValue)
            continue;

        AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    }

    return CheckItemStats(clazz, sp, ap, tank);
}

bool RandomItemMgr::CanEquipWeapon(uint8 clazz, ItemTemplate const* proto)
{
    switch (clazz)
    {
        case CLASS_PRIEST:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE)
                return false;
            break;
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_WARRIOR:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                return false;
            break;
        case CLASS_PALADIN:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_SHAMAN:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                return false;
            break;
        case CLASS_DRUID:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                return false;
            break;
        case CLASS_HUNTER:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW)
                return false;
            break;
        case CLASS_ROGUE:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                return false;
            break;
    }

    return true;
}

void RandomItemMgr::BuildEquipCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    QueryResult results = PlayerbotDatabase.PQuery("SELECT clazz, lvl, slot, quality, item FROM playerbot_equip_cache");
    if (results)
    {
        LOG_INFO("playerbots", "Loading equipment cache for %d classes, %d levels, %d slots, %d quality from %zu items",
                MAX_CLASSES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, itemTemplates->size());

        uint32 count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint8 clazz = fields[0].GetUInt8();
            uint32 level = fields[1].GetUInt32();
            uint8 slot = fields[2].GetUInt8();
            uint32 quality = fields[3].GetUInt32();
            uint32 itemId = fields[4].GetUInt32();

            BotEquipKey key(level, clazz, slot, quality);
            equipCache[key].push_back(itemId);
            ++count;

        } while (results->NextRow());

        LOG_INFO("playerbots", "Equipment cache loaded from %d records", count);
    }
    else
    {
        uint64 total = MAX_CLASSES * maxLevel * EQUIPMENT_SLOT_END * ITEM_QUALITY_ARTIFACT;
        LOG_INFO("playerbots", "Building equipment cache for %d classes, %d levels, %d slots, %d quality from %d items (%zu total)",
                MAX_CLASSES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, itemTemplates->size(), total);

        for (uint8 class_ = CLASS_WARRIOR; class_ < MAX_CLASSES; ++class_)
        {
            // skip nonexistent classes
            if (!((1 << (class_ - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(class_))
                continue;

            for (uint32 level = 1; level <= maxLevel; ++level)
            {
                for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    for (uint32 quality = ITEM_QUALITY_POOR; quality <= ITEM_QUALITY_ARTIFACT; ++quality)
                    {
                        BotEquipKey key(level, class_, slot, quality);

                        RandomItemList items;
                        for (auto const& itr : *itemTemplates)
                        {
                            ItemTemplate const* proto = &itr.second;
                            if (!proto)
                                continue;

                            if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_CONTAINER && proto->Class != ITEM_CLASS_PROJECTILE)
                                continue;

                            if (!CanEquipItem(key, proto))
                                continue;

                            if (proto->Class == ITEM_CLASS_ARMOR && (slot == EQUIPMENT_SLOT_HEAD || slot == EQUIPMENT_SLOT_SHOULDERS || slot == EQUIPMENT_SLOT_CHEST ||
                                slot == EQUIPMENT_SLOT_WAIST || slot == EQUIPMENT_SLOT_LEGS || slot == EQUIPMENT_SLOT_FEET || slot == EQUIPMENT_SLOT_WRISTS ||
                                slot == EQUIPMENT_SLOT_HANDS) && !CanEquipArmor(key.clazz, key.level, proto))
                                continue;

                            if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(key.clazz, proto))
                                continue;

                            if (slot == EQUIPMENT_SLOT_OFFHAND && key.clazz == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
                                continue;

                            items.push_back(itr.first);

                            PlayerbotDatabase.PExecute("INSERT INTO playerbot_equip_cache (clazz, lvl, slot, quality, item) VALUES (%u, %u, %u, %u, %u)",
                                    class_, level, slot, quality, itr.first);
                        }

                        equipCache[key] = items;

                        LOG_INFO("playerbots", "Equipment cache for class: %d, level %d, slot %d, quality %d: %d items",
                                class_, level, slot, quality, items.size());
                    }
                }
            }
        }

        LOG_INFO("playerbots", "Equipment cache saved to DB");
    }
}

RandomItemList RandomItemMgr::Query(uint32 level, uint8 clazz, uint8 slot, uint32 quality)
{
    BotEquipKey key(level, clazz, slot, quality);
    return equipCache[key];
}

void RandomItemMgr::BuildAmmoCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("playerbots", "Building ammo cache for %d levels", maxLevel);

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        for (uint32 subClass = ITEM_SUBCLASS_ARROW; subClass <= ITEM_SUBCLASS_BULLET; subClass++)
        {
            QueryResult results = WorldDatabase.PQuery("SELECT entry, RequiredLevel FROM item_template WHERE class = '%u' AND subclass = '%u' AND RequiredLevel <= '%u' "
                "AND quality = '%u' ORDER BY RequiredLevel DESC", ITEM_CLASS_PROJECTILE, subClass, level, ITEM_QUALITY_NORMAL);
            if (!results)
                return;

            Field* fields = results->Fetch();
            uint32 entry = fields[0].GetUInt32();
            ammoCache[level / 10][subClass] = entry;
            ++counter;
        }
    }

    LOG_INFO("playerbots", "Cached %d types of ammo", counter); // TEST
}

uint32 RandomItemMgr::GetAmmo(uint32 level, uint32 subClass)
{
    return ammoCache[(level - 1) / 10][subClass];
}

void RandomItemMgr::BuildPotionCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("playerbots", "Building potion cache for %d levels", maxLevel);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];

            for (auto const& itr : *itemTemplates)
            {
                ItemTemplate const* proto = &itr.second;
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE || (proto->SubClass != ITEM_SUBCLASS_POTION && proto->SubClass != ITEM_SUBCLASS_FLASK) || proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
                    if (!spellInfo)
                        continue;

                    for (uint8 i = 0 ; i < 3; i++)
                    {
                        if (spellInfo->Effects[i].Effect == effect)
                        {
                            potionCache[level / 10][effect].push_back(itr.first);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];
            uint32 size = potionCache[level / 10][effect].size();
            ++counter;

            LOG_INFO("playerbots", "Potion cache for level=%d, effect=%d: %d items", level, effect, size);
        }
    }

    LOG_INFO("playerbots", "Cached %d types of potions", counter); // TEST
}

void RandomItemMgr::BuildFoodCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("playerbots", "Building food cache for %d levels", maxLevel);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 categories[] = { 11, 59 };
        for (int i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];

            for (auto const& itr : *itemTemplates)
            {
                ItemTemplate const* proto = &itr.second;
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE || (proto->SubClass != ITEM_SUBCLASS_FOOD && proto->SubClass != ITEM_SUBCLASS_CONSUMABLE) ||
                    (proto->Spells[0].SpellCategory != category) || proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                foodCache[level / 10][category].push_back(itr.first);
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 categories[] = { 11, 59 };
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];
            uint32 size = foodCache[level / 10][category].size();
            ++counter;
            LOG_INFO("playerbots", "Food cache for level=%d, category=%d: %d items", level, category, size);
        }
    }

    LOG_INFO("playerbots", "Cached %d types of food", counter);
}

uint32 RandomItemMgr::GetRandomPotion(uint32 level, uint32 effect)
{
    std::vector<uint32> potions = potionCache[(level - 1) / 10][effect];
    if (potions.empty())
        return 0;

    return potions[urand(0, potions.size() - 1)];
}

uint32 RandomItemMgr::GetFood(uint32 level, uint32 category)
{
    std::initializer_list<uint32> items;
    std::vector<uint32> food;
    if (category == 11)
    {
        if (level < 5)
            items = { 787, 117, 4540, 2680 };
        else if (level < 15)
            items = { 2287, 4592, 4541, 21072 };
        else if (level < 25)
            items = { 3770, 16170, 4542, 20074 };
        else if (level < 35)
            items = { 4594, 3771, 1707, 4457 };
        else if (level < 45)
            items = { 4599, 4601, 21552, 17222 /*21030, 16168 */ };
        else if (level < 55)
            items = { 8950, 8952, 8957, 21023 /*21033, 21031 */ };
        else if (level < 65)
            items = { 29292, 27859, 30458, 27662 };
        else
            items = { 29450, 29451, 29452 };
    }

    if (category == 59)
    {
        if (level < 5)
            items = { 159, 117 };
        else if (level < 15)
            items = { 1179, 21072 };
        else if (level < 25)
            items = { 1205 };
        else if (level < 35)
            items = { 1708 };
        else if (level < 45)
            items = { 1645 };
        else if (level < 55)
            items = { 8766 };
        else if (level < 65)
            items = { 28399 };
        else
            items = { 27860 };
    }

    food.insert(food.end(), items);
    if (food.empty()) return 0;
    return food[urand(0, food.size() - 1)];
}

uint32 RandomItemMgr::GetRandomFood(uint32 level, uint32 category)
{
    std::vector<uint32> food = foodCache[(level - 1) / 10][category];
    if (food.empty())
        return 0;

    return food[urand(0, food.size() - 1)];
}

void RandomItemMgr::BuildTradeCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("playerbots", "Building trade cache for %d levels", maxLevel);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_TRADE_GOODS || proto->Bonding != NO_BIND)
                continue;

            if (proto->ItemLevel < level)
                continue;

            if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                continue;

            if (proto->RequiredSkill)
                continue;

            tradeCache[level / 10].push_back(itr.first);
        }
    }

    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 size = tradeCache[level / 10].size();
        LOG_INFO("playerbots", "Trade cache for level=%d: %d items", level, size);
        ++counter;
    }

    LOG_INFO("playerbots", "Cached %d trade items", counter); // TEST
}

uint32 RandomItemMgr::GetRandomTrade(uint32 level)
{
    std::vector<uint32> trade = tradeCache[(level - 1) / 10];
    if (trade.empty())
        return 0;

    return trade[urand(0, trade.size() - 1)];
}

void RandomItemMgr::BuildRarityCache()
{
    QueryResult results = PlayerbotDatabase.PQuery("SELECT item, rarity FROM playerbot_rarity_cache");
    if (results)
    {
        LOG_INFO("playerbots", "Loading item rarity cache");

        uint32 count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            float rarity = fields[1].GetFloat();

            rarityCache[itemId] = rarity;
            ++count;

        } while (results->NextRow());

        LOG_INFO("playerbots", "Item rarity cache loaded from %d records", count);
    }
    else
    {
        ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
        LOG_INFO("playerbots", "Building item rarity cache from %u items", itemTemplates->size());

        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (proto->Quality == ITEM_QUALITY_POOR)
                continue;

            if (strstri(proto->Name1.c_str(), "qa") || strstri(proto->Name1.c_str(), "test") || strstri(proto->Name1.c_str(), "deprecated"))
                continue;

            if (!proto->ItemLevel)
                continue;

            QueryResult results = WorldDatabase.PQuery(
                    "SELECT MAX(q.chance) FROM ( "
                    // "-- Creature "
                    "SELECT  "
                    "AVG ( "
                    "   CASE  "
                    "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                    "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                    "    ELSE   "
                    "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM creature_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (SELECT COUNT(*) FROM creature_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance = 0) "
                    "    END "
                    ") chance, 'creature' type "
                    "FROM creature_loot_template lt "
                    "JOIN creature_template ct ON ct.LootId = lt.entry "
                    "JOIN creature c ON c.id = ct.entry "
                    "WHERE lt.item = '%u' "
                    "union all "
                    // "-- Gameobject "
                    "SELECT  "
                    "AVG ( "
                    "   CASE  "
                    "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                    "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                    "    ELSE   "
                    "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM gameobject_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (SELECT COUNT(*) FROM gameobject_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance = 0) "
                    "    END "
                    ") chance, 'gameobject' type "
                    "FROM gameobject_loot_template lt "
                    "JOIN gameobject_template ct ON ct.data1 = lt.entry "
                    "JOIN gameobject c ON c.id = ct.entry "
                    "WHERE lt.item = '%u' "
                    "union all "
                    // "-- Disenchant "
                    "SELECT  "
                    "AVG ( "
                    "   CASE  "
                    "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                    "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                    "    ELSE   "
                    "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM disenchant_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (SELECT COUNT(*) FROM disenchant_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance = 0) "
                    "    END "
                    ") chance, 'disenchant' type "
                    "FROM disenchant_loot_template lt "
                    "JOIN item_template ct ON ct.DisenchantID = lt.entry "
                    "WHERE lt.item = '%u' "
                    "union all "
                    // "-- Fishing "
                    "SELECT  "
                    "AVG ( "
                    "   CASE  "
                    "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                    "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                    "    ELSE   "
                    "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM fishing_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (SELECT COUNT(*) FROM fishing_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance = 0) "
                    "    END "
                    ") chance, 'fishing' type "
                    "FROM fishing_loot_template lt "
                    "WHERE lt.item = '%u' "
                    "union all "
                    // "-- Skinning "
                    "SELECT  "
                    "AVG ( "
                    "   CASE  "
                    "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                    "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance  "
                    "    ELSE   "
                    "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM skinning_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                    "    * IFNULL((SELECT 1/COUNT(*) FROM skinning_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance = 0), 1) "
                    "    END "
                    ") chance, 'skinning' type "
                    "FROM skinning_loot_template lt "
                    "JOIN creature_template ct ON ct.SkinningLootId = lt.entry "
                    "JOIN creature c ON c.id = ct.entry "
                    "WHERE lt.item = '%u') q; ", itr.first, itr.first, itr.first, itr.first, itr.first);

            if (results)
            {
                Field* fields = results->Fetch();
                float rarity = fields[0].GetFloat();
                if (rarity > 0.01)
                {
                    rarityCache[itr.first] = rarity;

                    PlayerbotDatabase.PExecute("INSERT INTO playerbot_rarity_cache (item, rarity) VALUES (%u, %f)", itr.first, rarity);
                }
            }
        }

        LOG_INFO("playerbots", "Item rarity cache built from %u items", itemTemplates->size());
    }
}

float RandomItemMgr::GetItemRarity(uint32 itemId)
{
    return rarityCache[itemId];
}

inline bool IsCraftedBySpell(ItemTemplate const* proto, uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    return IsCraftedBySpell(proto, spellInfo);
}

inline bool IsCraftedBySpell(ItemTemplate const* proto, SpellInfo const* spellInfo)
{
    for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
    {
        if (spellInfo->Reagent[x] <= 0)
        {
            continue;
        }

        if (proto->ItemId == spellInfo->Reagent[x])
        {
            return true;
        }
    }

    for (uint8 i = 0; i < 3; ++i)
    {
        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM)
        {
            if (spellInfo->Effects[i].ItemType == proto->ItemId)
            {
                return true;
            }
        }
    }

    return false;
}

inline bool IsCraftedBy(ItemTemplate const* proto, uint32 spellId)
{
    if (IsCraftedBySpell(proto, spellId))
        return true;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    for (uint32 effect = 0; effect < 3; ++effect)
    {
        uint32 craftId = spellInfo->Effects[effect].TriggerSpell;
        SpellInfo const* craftSpellInfo = sSpellMgr->GetSpellInfo(craftId);
        if (!craftSpellInfo)
            continue;

        if (IsCraftedBySpell(proto, craftSpellInfo))
            return true;
    }

    return false;
}

inline bool ContainsInternal(ItemTemplate const* proto, uint32 skillId)
{
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine || skillLine->ID != skillId)
            continue;

        if (IsCraftedBy(proto, skillLine->Spell))
            return true;
    }

    CreatureTemplateContainer const* creatureTemplates = sObjectMgr->GetCreatureTemplates();
    for (auto const& itr : *creatureTemplates)
    {
        CreatureTemplate const* co = &itr.second;
        if (!co || co->trainer_type != TRAINER_TYPE_TRADESKILLS)
            continue;

        uint32 trainerId = co->Entry;
        TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);
        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell || tSpell->reqSkill != skillId)
                continue;

            if (IsCraftedBy(proto, tSpell->spell))
                return true;
        }
    }

    std::vector<ItemTemplate*> const* itemTemplates = sObjectMgr->GetItemTemplateStoreFast();
    for (ItemTemplate const* recipe : *itemTemplates)
    {
        if (!recipe)
            continue;

        if (recipe->Class == ITEM_CLASS_RECIPE && (
            (recipe->SubClass == ITEM_SUBCLASS_LEATHERWORKING_PATTERN && skillId == SKILL_LEATHERWORKING) ||
            (recipe->SubClass == ITEM_SUBCLASS_TAILORING_PATTERN && skillId == SKILL_TAILORING) ||
            (recipe->SubClass == ITEM_SUBCLASS_ENGINEERING_SCHEMATIC && skillId == SKILL_ENGINEERING) ||
            (recipe->SubClass == ITEM_SUBCLASS_BLACKSMITHING && skillId == SKILL_BLACKSMITHING) ||
            (recipe->SubClass == ITEM_SUBCLASS_COOKING_RECIPE && skillId == SKILL_COOKING) ||
            (recipe->SubClass == ITEM_SUBCLASS_ALCHEMY_RECIPE && skillId == SKILL_ALCHEMY) ||
            (recipe->SubClass == ITEM_SUBCLASS_FIRST_AID_MANUAL && skillId == SKILL_FIRST_AID) ||
            (recipe->SubClass == ITEM_SUBCLASS_ENCHANTING_FORMULA && skillId == SKILL_ENCHANTING) ||
            (recipe->SubClass == ITEM_SUBCLASS_JEWELCRAFTING_RECIPE && skillId == SKILL_JEWELCRAFTING) ||
            (recipe->SubClass == ITEM_SUBCLASS_FISHING_MANUAL && skillId == SKILL_FISHING)
            ))
        {
            for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                if (IsCraftedBy(proto, recipe->Spells[i].SpellId))
                    return true;
            }
        }
    }

    return false;
}

bool RandomItemMgr::IsUsedBySkill(ItemTemplate const* proto, uint32 skillId)
{
    if (itemCache.find(proto->ItemId) != itemCache.end())
        return true;

    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
            break;
        default:
            return false;
    }

    bool contains = ContainsInternal(proto, skillId);
    if (contains)
        itemCache.insert(proto->ItemId);

    return contains;
}
