/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RpgSubActions.h"
#include "ChooseRpgTargetAction.h"
#include "GuildCreateActions.h"
#include "EmoteAction.h"
#include "Formations.h"
#include "GossipDef.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "Playerbot.h"
#include "PossibleRpgTargetsValue.h"
#include "SocialMgr.h"

void RpgHelper::OnExecute(string nextAction = "rpg")
{
    if (ai->HasRealPlayerMaster() && nextAction == "rpg")
        nextAction = "rpg cancel";

    SET_AI_VALUE(string, "next rpg action", nextAction);
}

void RpgHelper::BeforeExecute()
{
    OnExecute();

    bot->SetSelectionGuid(guidP());

    setFacingTo(guidP());
}

void RpgHelper::AfterExecute(bool doDelay, bool waitForGroup)
{
    OnExecute();

    bot->SetSelectionGuid(guidP());

    setFacingTo(guidP());

    if(doDelay)
        setDelay(waitForGroup);

    setFacing(guidP());
}

GuidPosition RpgHelper::guidP()
{
    return AI_VALUE(GuidPosition, "rpg target");
}

ObjectGuid RpgHelper::guid()
{
    return (ObjectGuid)guidP();
}

bool RpgHelper::InRange()
{
    return guidP() ? (guidP().sqDistance2d(bot) < INTERACTION_DISTANCE * INTERACTION_DISTANCE) : false;
}

void RpgHelper::setFacingTo(GuidPosition guidPosition)
{
    bot->SetFacingTo(guidPosition.getAngleTo(bot)+ M_PI_F);
}

void RpgHelper::setFacing(GuidPosition guidPosition)
{
    if (!guidPosition.IsUnit())
        return;

    if (guidPosition.IsPlayer())
        return;

    Unit* unit = guidPosition.GetUnit();

    unit->SetFacingTo(unit->GetAngle(bot));
}

void RpgHelper::setDelay(bool waitForGroup)
{
    if (!ai->HasRealPlayerMaster() || (waitForGroup && ai->GetGroupMaster() == bot && bot->GetGroup()))
        ai->SetNextCheckDelay(sPlayerbotAIConfig.rpgDelay);
    else
        ai->SetNextCheckDelay(sPlayerbotAIConfig.rpgDelay / 5);
}

bool RpgSubAction::isPossible()
{
    return rpg->guidP() && rpg->guidP().GetWorldObject();
}

bool RpgSubAction::isUseful()
{
    return rpg->InRange();
}

bool RpgSubAction::Execute(Event event)
{
    bool doAction = ai->DoSpecificAction(ActionName(), ActionEvent(event), true);
    rpg->AfterExecute(doAction, true);
    return doAction;
}

string RpgSubAction::ActionName()
{
    return "none";
}

Event RpgSubAction::ActionEvent(Event event)
{
    return event;
}

bool RpgStayAction::isUseful()
{
    return rpg->InRange() && !ai->HasRealPlayerMaster();
}

bool RpgStayAction::Execute(Event event)
{
    if (bot->GetPlayerMenu())
        bot->GetPlayerMenu()->CloseGossip();

    rpg->AfterExecute();
    return true;
}

bool RpgWorkAction::isUseful()
{
    return rpg->InRange() && !ai->HasRealPlayerMaster();
}

bool RpgWorkAction::Execute(Event event)
{
    bot->HandleEmoteCommand(EMOTE_STATE_USESTANDING);
    rpg->AfterExecute();
    return true;
}

bool RpgEmoteAction::isUseful()
{
    return rpg->InRange() && !ai->HasRealPlayerMaster();
}

bool RpgEmoteAction::Execute(Event event)
{
    uint32 type = TalkAction::GetRandomEmote(rpg->guidP().GetUnit());

    WorldPacket p1;
    p1 << rpg->guid();
    bot->GetSession()->HandleGossipHelloOpcode(p1);

    bot->HandleEmoteCommand(type);

    rpg->AfterExecute();

    return true;
}

bool RpgCancelAction::Execute(Event event)
{
    RESET_AI_VALUE(GuidPosition, "rpg target");
    rpg->OnExecute("");
    return true;
}

bool RpgTaxiAction::isUseful()
{
    return rpg->InRange() && !ai->HasRealPlayerMaster();
}

bool RpgTaxiAction::Execute(Event event)
{
    GuidPosition guidP = rpg->guidP();

    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    uint32 node = sObjectMgr.GetNearestTaxiNode(guidP.getX(), guidP.getY(), guidP.getZ(), guidP.getMapId(), bot->GetTeam());

    vector<uint32> nodes;
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i);
        if (entry && entry->from == node && (bot->m_taxi.IsTaximaskNodeKnown(entry->to) || bot->isTaxiCheater()))
        {
            nodes.push_back(i);
        }
    }

    if (nodes.empty())
    {
        sLog.outError("Bot %s - No flight paths available", bot->GetName());
        return false;
    }

    uint32 path = nodes[urand(0, nodes.size() - 1)];
    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 100000);

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
    if (!entry)
        return false;

    TaxiNodesEntry const* nodeFrom = sTaxiNodesStore.LookupEntry(entry->from);
    TaxiNodesEntry const* nodeTo = sTaxiNodesStore.LookupEntry(entry->to);

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
    {
        sLog.outError("Bot %s cannot talk to flightmaster (%zu location available)", bot->GetName(), nodes.size());
        return false;
    }
    if (!bot->ActivateTaxiPathTo({ entry->from, entry->to }, flightMaster, 0))
    {
        sLog.outError("Bot %s cannot fly %u (%zu location available)", bot->GetName(), path, nodes.size());
        return false;
    }
    sLog.outString("Bot #%d <%s> is flying from %s to %s (%zu location available)", bot->GetGUIDLow(), bot->GetName(), nodeFrom->name[0], nodeTo->name[0], nodes.size());
    bot->SetMoney(money);

    rpg->AfterExecute();

    return true;
}

bool RpgDiscoverAction::Execute(Event event)
{
    GuidPosition guidP = rpg->guidP();

    uint32 node = sObjectMgr.GetNearestTaxiNode(guidP.getX(), guidP.getY(), guidP.getZ(), guidP.getMapId(), bot->GetTeam());

    if (!node)
        return false;

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
        return false;

    return bot->GetSession()->SendLearnNewTaxiNode(flightMaster);
}

string RpgStartQuestAction::ActionName()
{
    return "accept all quests";
}

Event RpgStartQuestAction::ActionEvent(Event event)
{
    WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
    p << rpg->guid();
    p.rpos(0);
    return Event("rpg action", p);
}

string RpgEndQuestAction::ActionName()
{
    return "talk to quest giver";
}

Event RpgEndQuestAction::ActionEvent(Event event)
{
    WorldPacket p(CMSG_QUESTGIVER_COMPLETE_QUEST);
    p << rpg->guid();
    p.rpos(0);
    return Event("rpg action", p);
}

string RpgBuyAction::ActionName()
{
    return "buy";
}

Event RpgBuyAction::ActionEvent(Event event)
{
    return Event("rpg action", "vendor");
}

string RpgSellAction::ActionName()
{
    return "sell";
}

Event RpgSellAction::ActionEvent(Event event)
{
    return Event("rpg action", "vendor");
}

string RpgRepairAction::ActionName()
{
    return "repair";
}

string RpgTrainAction::ActionName()
{
    return "trainer";
}

bool RpgHealAction::Execute(Event event)
{
    bool retVal = false;

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            retVal = ai->DoSpecificAction("lesser heal on party", Event(), true);
            break;
        case CLASS_DRUID:
            retVal=ai->DoSpecificAction("healing touch on party", Event(), true);
            break;
        case CLASS_PALADIN:
            retVal=ai->DoSpecificAction("holy light on party", Event(), true);
            break;
        case CLASS_SHAMAN:
            retVal=ai->DoSpecificAction("healing wave on party", Event(), true);
            break;
    }

    return retVal;
}

string RpgHomeBindAction::ActionName()
{
    return "home";
}

string RpgQueueBgAction::ActionName()
{
    SET_AI_VALUE(uint32, "bg type", (uint32) AI_VALUE(BattleGroundTypeId, "rpg bg type"));
    return "free bg join";
}

string RpgBuyPetitionAction::ActionName()
{
    return "buy petition";
}

string RpgUseAction::ActionName()
{
    return "use";
}

Event RpgUseAction::ActionEvent(Event event)
{
    return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject()));
}

string RpgSpellAction::ActionName()
{
    return "cast random spell";
}

Event RpgSpellAction::ActionEvent(Event event)
{
    return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject()));
}

string RpgCraftAction::ActionName()
{
    return "craft random item";
}

Event RpgCraftAction::ActionEvent(Event event)
{
    return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject()));
}

list<Item*> RpgTradeUsefulAction::CanGiveItems(GuidPosition guidPosition)
{
    Player* player = guidPosition.GetPlayer();

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

bool RpgTradeUsefulAction::Execute(Event event)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    list<Item*> items = CanGiveItems(guidP);

    if (items.empty())
        return false;

    Item* item = items.front();

    ostringstream param;

    param << chat->formatWorldobject(player);
    param << " ";
    param << chat->formatItem(item->GetProto());

    bool hasTraded = ai->DoSpecificAction("trade", Event("rpg action", param.str().c_str()), true);

    if (hasTraded || bot->GetTradeData())
    {
        if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetObjectGuid()))
        {
            if (bot->GetGroup() && bot->GetGroup()->IsMember(guidP) && ai->HasRealPlayerMaster())
                ai->TellMasterNoFacing("You can use this " + chat->formatItem(item->GetProto()) + " better than me, " + guidP.GetPlayer()->GetName() /*chat->formatWorldobject(guidP.GetPlayer())*/ + ".");
            else
                bot->Say("You can use this " + chat->formatItem(item->GetProto()) + " better than me, " + player->GetName() /*chat->formatWorldobject(player)*/ + ".", (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

            if (!urand(0, 4) || items.size() < 2)
            {
                //bot->Say("End trade with" + chat->formatWorldobject(player), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                WorldPacket p;
                uint32 status = TRADE_STATUS_TRADE_ACCEPT;
                p << status;
                bot->GetSession()->HandleAcceptTradeOpcode(p);
            }
        }
        else
            bot->Say("Start trade with" + chat->formatWorldobject(player), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        ai->SetNextCheckDelay(sPlayerbotAIConfig.rpgDelay);
        return true;
    }

    return false;
}

bool RpgDuelAction::isUseful()
{
    // do not offer duel in non pvp areas
    if (sPlayerbotAIConfig.IsInPvpProhibitedZone(bot->GetAreaId()))
        return false;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    AreaTableEntry const* casterAreaEntry = GetAreaEntryByAreaID(bot->GetAreaId());
    if (casterAreaEntry && !(casterAreaEntry->flags & AREA_FLAG_DUEL))
    {
        // Dueling isn't allowed here
        return false;
    }

    // Less spammy duels
    if (bot->getLevel() == 1)
        return false;

    return true;
}

bool RpgDuelAction::Execute(Event event)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    return ai->DoSpecificAction("cast custom spell", Event("rpg action", chat->formatWorldobject(player) + " 7266"), true);
}
