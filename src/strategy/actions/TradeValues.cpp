/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TradeValues.h"
#include "ItemUsageValue.h"
#include "Playerbot.h"

list<Item*> ItemsUsefulToGiveValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    list<Item*> giveItems;

    if (ai->HasActivePlayerMaster() || !player->GetPlayerbotAI())
        return giveItems;

    list<ItemUsage> myUsages = { ITEM_USAGE_NONE , ITEM_USAGE_VENDOR, ITEM_USAGE_AH, ITEM_USAGE_DISENCHANT };

    for (auto& myUsage : myUsages)
    {
        list<Item*> myItems = AI_VALUE2(list<Item*>, "inventory items", "usage " + to_string(myUsage));
        myItems.reverse();

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetObjectGuid()))
                continue;

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", item->GetEntry());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}
