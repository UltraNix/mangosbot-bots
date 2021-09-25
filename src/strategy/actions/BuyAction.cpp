/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BuyAction.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ItemCountValue.h"
#include "ItemUsageValue.h"
#include "Playerbot.h"

bool BuyAction::Execute(Event event)
{
    bool buyUseful = false;
    ItemIds itemIds;
    std::string const& link = event.getParam();

    if (link == "vendor")
        buyUseful = true;
    else
    {
        itemIds = chat->parseItems(link);

        Player* master = GetMaster();
        if (!master)
            return false;
    }

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    bool vendored = false;
    bool result = false;
    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        if (buyUseful)
        {
            VendorItemData const* tItems = pCreature->GetVendorItems();

            if (!tItems)
                continue;

            for (auto& tItem : tItems->m_items)
            {
                ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", tItem->item);
                if (usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_AMMO)
                {
                    itemIds.insert(tItem->item);
                }
                else if (usage == ITEM_USAGE_SKILL)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(tItem->item);

                    if (!bot->HasItemCount(tItem->item, proto->Stackable))
                        itemIds.insert(tItem->item);
                }
                else
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(tItem->item);
                    if (!proto)
                        continue;

                    //temp needs to move to itemusage value
                    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
                    {
                        uint32 entry = botAI->GetBot()->GetQuestSlotQuestId(slot);
                        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
                        if (!quest)
                            continue;

                        for (int i = 0; i < 4; i++)
                        {
                            if (quest->RequiredItemId[i] == tItem->item)
                            {
                                if (!botAI->GetMaster() || !sPlayerbotAIConfig->syncQuestWithPlayer)
                                    if (AI_VALUE2(uint8, "item count", proto->Name1) < quest->RewardChoiceItemCount[i])
                                        itemIds.insert(tItem->item);
                            }
                        }
                    }
                }
            }

            if (itemIds.empty())
                return true;
        }

        if (itemIds.empty())
            return false;

        vendored = true;

        for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
        {
            uint32 itemId = *i;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            VendorItemData const* tItems = pCreature->GetVendorItems();
            result |= BuyItem(pCreature->GetVendorItems(), vendorguid, proto);

            if (!result)
            {
                std::ostringstream out;
                out << "Nobody sells " << ChatHelper::formatItem(proto) << " nearby";
                botAI->TellMaster(out.str());
            }
        }
    }

    if (!vendored)
    {
        botAI->TellError("There are no vendors nearby");
        return false;
    }

    return true;
}

bool BuyAction::BuyItem(VendorItemData const* tItems, ObjectGuid vendorguid, ItemTemplate const* proto)
{
    if (!tItems)
        return false;

    uint32 itemId = proto->ItemId;
    for (uint32 slot = 0; slot < tItems->GetItemCount(); slot++)
    {
        if (tItems->GetItem(slot)->item == itemId)
        {
            bool couldBuy = false;
            couldBuy = bot->BuyItemFromVendorSlot(vendorguid, slot, itemId, 1, NULL_BAG, NULL_SLOT);

            if (couldBuy)
            {
                std::ostringstream out;
                out << "Buying " << ChatHelper::formatItem(proto);
                botAI->TellMaster(out.str());
            }

            return true;
        }
    }

    return false;
}
