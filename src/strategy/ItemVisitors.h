/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Common.h"
#include "Item.h"
#include "../ChatHelper.h"

class Player;

char* strstri(char const* str1, char const* str2);

enum IterateItemsMask : uint32
{
    ITERATE_ITEMS_IN_BAGS   = 1,
    ITERATE_ITEMS_IN_EQUIP  = 2,
    ITERATE_ITEMS_IN_BANK   = 4,
    ITERATE_ALL_ITEMS       = 255
};

class IterateItemsVisitor
{
    public:
        IterateItemsVisitor() { }

        virtual bool Visit(Item* item) = 0;
};

class FindItemVisitor : public IterateItemsVisitor
{
    public:
        FindItemVisitor() : IterateItemsVisitor(), result(nullptr) { }

        bool Visit(Item* item) override
        {
            if (!Accept(item->GetTemplate()))
                return true;

            result.push_back(item);
            return true;
        }

        std::vector<Item*>& GetResult() { return result; }

    protected:
        virtual bool Accept(ItemTemplate const* proto) = 0;

    private:
        std::vector<Item*> result;
};

class FindUsableItemVisitor : public FindItemVisitor
{
    public:
        FindUsableItemVisitor(Player* bot) : FindItemVisitor(), bot(bot) { }

        bool Visit(Item* item) override
        {
            if (bot->CanUseItem(item->GetTemplate()) == EQUIP_ERR_OK)
                return FindItemVisitor::Visit(item);

            return true;
        }

    private:
        Player* bot;
};

class FindItemsByQualityVisitor : public IterateItemsVisitor
{
    public:
        FindItemsByQualityVisitor(uint32 quality, uint32 count) : IterateItemsVisitor(), quality(quality), count(count) { }

        bool Visit(Item* item) override
        {
            if (item->GetTemplate()->Quality != quality)
                return true;

            if (result.size() >= (size_t)count)
                return false;

            result.push_back(item);
            return true;
        }

        std::vector<Item*>& GetResult()
        {
            return result;
        }

    private:
        uint32 quality;
        uint32 count;
        std::vector<Item*> result;
};

class FindItemsToTradeByQualityVisitor : public FindItemsByQualityVisitor
{
    public:
        FindItemsToTradeByQualityVisitor(uint32 quality, uint32 count) : FindItemsByQualityVisitor(quality, count) { }

        bool Visit(Item* item) override
        {
            if (item->IsSoulBound())
                return true;

            return FindItemsByQualityVisitor::Visit(item);
        }
};

class FindItemsToTradeByClassVisitor : public IterateItemsVisitor
{
    public:
        FindItemsToTradeByClassVisitor(uint32 itemClass, uint32 itemSubClass, uint32 count)
            : IterateItemsVisitor(), count(count), itemClass(itemClass), itemSubClass(itemSubClass) { }

        bool Visit(Item* item) override
        {
            if (item->IsSoulBound())
                return true;

            if (item->GetTemplate()->Class != itemClass || item->GetTemplate()->SubClass != itemSubClass)
                return true;

            if (result.size() >= (size_t)count)
                return false;

            result.push_back(item);
            return true;
        }

        std::vector<Item*>& GetResult()
        {
            return result;
        }

    private:
        uint32 itemClass;
        uint32 itemSubClass;
        uint32 count;
        std::vector<Item*> result;
};

class QueryItemCountVisitor : public IterateItemsVisitor
{
    public:
        QueryItemCountVisitor(uint32 itemId) : count(0), itemId(itemId) { }

        bool Visit(Item* item) override
        {
            if (item->GetTemplate()->ItemId == itemId)
                count += item->GetCount();

            return true;
        }

        uint32 GetCount() { return count; }

    protected:
        uint32 count;
        uint32 itemId;
};

class QueryNamedItemCountVisitor : public QueryItemCountVisitor
{
    public:
        QueryNamedItemCountVisitor(std::string const& name) : QueryItemCountVisitor(0), name(name) { }

        bool Visit(Item* item) override
        {
            ItemTemplate const* proto = item->GetTemplate();
            if (proto && proto->Name1.c_str() && strstri(proto->Name1.c_str(), name.c_str()))
                count += item->GetCount();

            return true;
        }

    private:
        std::string name;
};

class FindNamedItemVisitor : public FindItemVisitor
{
    public:
        FindNamedItemVisitor(Player* bot, std::string const& name) : FindItemVisitor(), name(name) { }

        bool Accept(ItemTemplate const* proto) override
        {
            return proto && proto->Name1.c_str() && strstri(proto->Name1.c_str(), name.c_str());
        }

    private:
        std::string name;
};

class FindItemByIdVisitor : public FindItemVisitor
{
    public:
        FindItemByIdVisitor(uint32 id) : FindItemVisitor(), id(id) { }

        bool Accept(ItemTemplate const* proto) override
        {
            return proto->ItemId == id;
        }

    private:
        uint32 id;
};

class FindItemByIdsVisitor : public FindItemVisitor
{
    public:
        FindItemByIdsVisitor(ItemIds ids) : FindItemVisitor(), ids(ids) { }

        bool Accept(ItemTemplate const* proto) override
        {
            return ids.find(proto->ItemId) != ids.end();
        }

    private:
        ItemIds ids;
};

class ListItemsVisitor : public IterateItemsVisitor
{
    public:
        ListItemsVisitor() : IterateItemsVisitor() { }

        std::map<uint32, uint32> items;
        std::map<uint32, bool> soulbound;

        bool Visit(Item* item) override
        {
            uint32 id = item->GetTemplate()->ItemId;

            if (items.find(id) == items.end())
                items[id] = 0;

            items[id] += item->GetCount();
            soulbound[id] = item->IsSoulBound();
            return true;
        }
};

class ItemCountByQuality : public IterateItemsVisitor
{
    public:
        ItemCountByQuality() : IterateItemsVisitor()
        {
            for (uint32 i = 0; i < MAX_ITEM_QUALITY; ++i)
                count[i] = 0;
        }

        bool Visit(Item* item) override
        {
            count[item->GetTemplate()->Quality]++;
            return true;
        }

    public:
        std::map<uint32, uint32> count;
};

class FindPotionVisitor : public FindUsableItemVisitor
{
    public:
        FindPotionVisitor(Player* bot, uint32 effectId) : FindUsableItemVisitor(bot), effectId(effectId) { }

        bool Accept(ItemTemplate const* proto) override;

    private:
        uint32 effectId;
};

class FindFoodVisitor : public FindUsableItemVisitor
{
    public:
        FindFoodVisitor(Player* bot, uint32 spellCategory, bool conjured = false) : FindUsableItemVisitor(bot),
            spellCategory(spellCategory), conjured(conjured) { }

        bool Accept(ItemTemplate const* proto) override
        {
            return proto->Class == ITEM_CLASS_CONSUMABLE && (proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD) &&
                proto->Spells[0].SpellCategory == spellCategory && (!conjured || proto->IsConjuredConsumable());
        }

    private:
        uint32 spellCategory;
        bool conjured;
};

class FindMountVisitor : public FindUsableItemVisitor
{
    public:
        FindMountVisitor(Player* bot) : FindUsableItemVisitor(bot) { }

        bool Accept(ItemTemplate const* proto) override;

    private:
        uint32 effectId;
};

class FindPetVisitor : public FindUsableItemVisitor
{
    public:
        FindPetVisitor(Player* bot) : FindUsableItemVisitor(bot) { }

        bool Accept(ItemTemplate const* proto) override;
};
