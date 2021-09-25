/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMITEMMGR_H
#define _PLAYERBOT_RANDOMITEMMGR_H

#include "Common.h"

class ChatHandler;

struct ItemTemplate;

enum EquipmentSlots : uint32;
enum InventoryType : uint32;

enum RandomItemType
{
    RANDOM_ITEM_GUILD_TASK,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE
};

class RandomItemPredicate
{
    public:
        virtual ~RandomItemPredicate() { };

        virtual bool Apply(ItemTemplate const* proto) = 0;
};

typedef std::vector<uint32> RandomItemList;
typedef std::map<RandomItemType, RandomItemList> RandomItemCache;

class BotEquipKey
{
    public:
        BotEquipKey() : level(0), clazz(0), slot(0), quality(0), key(GetKey()) { }
        BotEquipKey(uint32 level, uint8 clazz, uint8 slot, uint32 quality) : level(level), clazz(clazz), slot(slot), quality(quality), key(GetKey()) { }
        BotEquipKey(BotEquipKey const& other)  : level(other.level), clazz(other.clazz), slot(other.slot), quality(other.quality), key(GetKey()) { }

        bool operator<(BotEquipKey const& other) const
        {
            return other.key < this->key;
        }

        uint32 level;
        uint8 clazz;
        uint8 slot;
        uint32 quality;
        uint64 key;

    private:
        uint64 GetKey();
};

typedef std::map<BotEquipKey, RandomItemList> BotEquipCache;

class RandomItemMgr
{
    public:
        RandomItemMgr();
        virtual ~RandomItemMgr();
        static RandomItemMgr* instance()
        {
            static RandomItemMgr instance;
            return &instance;
        }

	public:
        void Init();
        void InitAfterAhBot();
        static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
        RandomItemList Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate);
        RandomItemList Query(uint32 level, uint8 clazz, uint8 slot, uint32 quality);
        uint32 GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate = nullptr);
        uint32 GetAmmo(uint32 level, uint32 subClass);
        uint32 GetRandomPotion(uint32 level, uint32 effect);
        uint32 GetRandomFood(uint32 level, uint32 category);
        uint32 GetFood(uint32 level, uint32 category);
        uint32 GetRandomTrade(uint32 level);
        bool CanEquipArmor(uint8 clazz, uint32 level, ItemTemplate const* proto);
        bool CanEquipWeapon(uint8 clazz, ItemTemplate const* proto);
        float GetItemRarity(uint32 itemId);

        static bool IsUsedBySkill(ItemTemplate const* proto, uint32 skillId);

    private:
        void BuildRandomItemCache();
        void BuildEquipCache();
        void BuildAmmoCache();
        void BuildFoodCache();
        void BuildPotionCache();
        void BuildTradeCache();
        void BuildRarityCache();
        bool CanEquipItem(BotEquipKey key, ItemTemplate const* proto);
        void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank);
        bool CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank);

    private:
        std::map<uint32, RandomItemCache> randomItemCache;
        std::map<RandomItemType, RandomItemPredicate*> predicates;
        BotEquipCache equipCache;
        std::map<EquipmentSlots, std::set<InventoryType>> viableSlots;
        std::map<uint32, std::map<uint32, uint32> > ammoCache;
        std::map<uint32, std::map<uint32, std::vector<uint32> > > potionCache;
        std::map<uint32, std::map<uint32, std::vector<uint32> > > foodCache;
        std::map<uint32, std::vector<uint32> > tradeCache;
        std::map<uint32, float> rarityCache;

        static std::set<uint32> itemCache;
};

#define sRandomItemMgr RandomItemMgr::instance()

#endif
