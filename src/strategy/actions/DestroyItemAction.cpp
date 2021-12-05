/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DestroyItemAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbot.h"

bool DestroyItemAction::Execute(Event event)
{
    std::string const& text = event.getParam();
    ItemIds ids = chat->parseItems(text);

    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        DestroyItem(&visitor);
    }

    return true;
}

void DestroyItemAction::DestroyItem(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
	for (Item* item : items)
    {
        bot->DestroyItem(item->GetBagSlot(),item->GetSlot(), true);

        std::ostringstream out;
        out << chat->formatItem(item->GetTemplate()) << " destroyed";
        botAI->TellMaster(out);
    }
}

bool SmartDestroyItemAction::isUseful()
{
    return !ai->HasActivePlayerMaster();
}

bool SmartDestroyItemAction::Execute(Event event)
{
    uint8 bagSpace = AI_VALUE(uint8, "bag space");

    if (bagSpace < 90)
        return false;

    vector<uint32> bestToDestroy = { ITEM_USAGE_NONE }; //First destroy anything useless.

    if (!AI_VALUE(bool, "can sell") && AI_VALUE(bool, "should get money")) // We need money so quest items are less important since they can't directly be sold.
        bestToDestroy.push_back(ITEM_USAGE_QUEST);
    else // We don't need money so destroy the cheapest stuff.
    {
        bestToDestroy.push_back(ITEM_USAGE_VENDOR);
        bestToDestroy.push_back(ITEM_USAGE_AH);
    }

    // If we still need room
    bestToDestroy.push_back(ITEM_USAGE_SKILL); // Items that might help tradeskill are more important than above but still expenable.
    bestToDestroy.push_back(ITEM_USAGE_USE);   // These are more likely to be usefull 'soon' but still expenable.

    for (auto& usage : bestToDestroy)
    {
        list<Item*> items = AI_VALUE2(list<Item*>, "inventory items", "usage " + to_string(usage));

        items.reverse();

        for (auto& item : items)
        {
            FindItemByIdVisitor visitor(item->GetProto()->ItemId);
            DestroyItem(&visitor);

            bagSpace = AI_VALUE(uint8, "bag space");

            if (bagSpace < 90)
                return true;
        }
    }

    return false;
}
