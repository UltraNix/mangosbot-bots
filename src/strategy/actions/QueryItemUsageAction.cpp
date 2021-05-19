/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "QueryItemUsageAction.h"
#include "../Event.h"
#include "../values/ItemUsageValue.h"
#include "../../ChatHelper.h"
#include "../../Playerbot.h"

bool QueryItemUsageAction::Execute(Event event)
{
    if (!GetMaster())
        return false;

    WorldPacket& data = event.getPacket();
    if (!data.empty())
    {
        data.rpos(0);

        ObjectGuid guid;
        data >> guid;
        if (guid != bot->GetGUID())
            return false;

        uint32 received;
        uint32 created;
        uint32 isShowChatMessage;
        uint32 notUsed;
        uint32 itemId;
        uint32 suffixFactor;
        uint32 itemRandomPropertyId;
        uint32 count;
        uint32 invCount;
        uint8 bagSlot;

        data >> received;                               // 0=looted, 1=from npc
        data >> created;                                // 0=received, 1=created
        data >> isShowChatMessage;                      // IsShowChatMessage
        data >> bagSlot;                                // item slot, but when added to stack: 0xFFFFFFFF

        data >> notUsed;
        data >> itemId;
        data >> suffixFactor;
        data >> itemRandomPropertyId;
        data >> count;
        // data >> invCount; // [-ZERO] count of items in inventory

        ItemTemplate const* item = sObjectMgr->GetItemTemplate(itemId);
        if (!item)
            return false;

        botAI->TellMaster(QueryItem(item, count, GetCount(item)));
        return true;
    }

    std::string const& text = event.getParam();
    ItemIds items = chat->parseItems(text);
    for (uint32 itemId : items)
    {
        ItemTemplate const* item = sObjectMgr->GetItemTemplate(itemId);
        if (!item)
            continue;

        botAI->TellMaster(QueryItem(item, 0, GetCount(item)));
    }

    return true;
}

uint32 QueryItemUsageAction::GetCount(ItemTemplate const* item)
{
    uint32 total = 0;

    std::list<Item*> items = InventoryAction::parseItems(item->Name1);
    if (!items.empty())
    {
        for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            total += (*i)->GetCount();
        }
    }

    return total;
}

std::string const& QueryItemUsageAction::QueryItem(ItemTemplate const* item, uint32 count, uint32 total)
{
    std::ostringstream out;
    std::string usage = QueryItemUsage(item);
    std::string const& quest = QueryQuestItem(item->ItemId);
    std::string const& price = QueryItemPrice(item);
    if (usage.empty())
        usage = (quest.empty() ? "Useless" : "Quest");

    out << chat->formatItem(item, count, total) << ": " << usage;
    if (!quest.empty())
        out << ", " << quest;

    if (!price.empty())
        out << ", " << price;

    return out.str();
}

std::string const& QueryItemUsageAction::QueryItemUsage(ItemTemplate const* item)
{
    std::ostringstream out;
    out << item->ItemId;
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
    switch (usage)
    {
        case ITEM_USAGE_EQUIP:
            return "Equip";
        case ITEM_USAGE_REPLACE:
            return "Equip (replace)";
        case ITEM_USAGE_SKILL:
            return "Tradeskill";
        case ITEM_USAGE_USE:
            return "Use";
	    case ITEM_USAGE_GUILD_TASK:
		    return "Guild task";
	    case ITEM_USAGE_DISENCHANT:
		    return "Disenchant";
	}

    return "";
}

std::string const& QueryItemUsageAction::QueryItemPrice(ItemTemplate const* item)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return "";

    if (item->Bonding == BIND_WHEN_PICKED_UP)
        return "";

    std::ostringstream msg;
    std::list<Item*> items = InventoryAction::parseItems(item->Name1);
    int32 sellPrice = 0;
    if (!items.empty())
    {
        for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            Item* sell = *i;
            int32 price = sell->GetCount() * auctionbot.GetSellPrice(sell->GetTemplate()) * sRandomPlayerbotMgr->GetSellMultiplier(bot);
            if (!sellPrice || sellPrice > price)
                sellPrice = price;
        }
    }
    if (sellPrice)
        msg << "Sell: " << chat->formatMoney(sellPrice);

    std::ostringstream out;
    out << item->ItemId;
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
    if (usage == ITEM_USAGE_NONE)
        return msg.str();

    int32 buyPrice = auctionbot.GetBuyPrice(item) * sRandomPlayerbotMgr->GetBuyMultiplier(bot);
    if (buyPrice)
    {
        if (sellPrice)
            msg << " ";

        msg << "Buy: " << chat->formatMoney(buyPrice);
    }

    return msg.str();
}

std::string const& QueryItemUsageAction::QueryQuestItem(uint32 itemId)
{
    Player* bot = botAI->GetBot();
    QuestStatusMap& questMap = bot->getQuestStatusMap();
    for (QuestStatusMap::const_iterator i = questMap.begin(); i != questMap.end(); i++)
    {
        Quest const* questTemplate = sObjectMgr->GetQuestTemplate(i->first);
        if (!questTemplate )
            continue;

        uint32 questId = questTemplate->GetQuestId();
        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || (status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questId)))
        {
            QuestStatusData const& questStatus = i->second;
            std::string const& usage = QueryQuestItem(itemId, questTemplate, &questStatus);
            if (!usage.empty())
                return usage;
        }
    }

    return "";
}

std::string const& QueryItemUsageAction::QueryQuestItem(uint32 itemId, Quest const* questTemplate, QuestStatusData const* questStatus)
{
    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (questTemplate->ReqItemId[i] != itemId)
            continue;

        uint32 required = questTemplate->RequiredItemCount[i];
        uint32 available = questStatus->ItemCount[i];
        if (!required)
            continue;

        return chat->formatQuestObjective(chat->formatQuest(questTemplate), available, required);
    }

    return "";
}
