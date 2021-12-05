/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildCreateActions.h"
#include "ArenaTeam.h"
#include "BudgetValues.h"
#include "Event.h"
#include "Playerbot.h"
#include "RandomPlayerbotFactory.h"

bool BuyPetitionAction::Execute(Event event)
{
    list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest npcs")->Get();
    bool vendored = false, result = false;
    for (list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_PETITIONER);
        if (!pCreature)
            continue;

        string guildName = RandomPlayerbotFactory::CreateRandomGuildName();
        if (guildName.empty())
            continue;

        WorldPacket data(CMSG_PETITION_BUY);

        data << pCreature->GetObjectGuid();
        data << uint32(0);
        data << uint64(0);
        data << guildName.c_str();
        data << std::string("");
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint16(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);

        for (uint8 i = 0; i < 10; ++i)
            data << std::string("");

        data << uint32(0); // index
        data << uint32(0);

        bot->GetSession()->HandlePetitionBuyOpcode(data);

        return true;
    }

    return false;
}

bool BuyPetitionAction::isUseful()
{
    return canBuyPetition(bot);
};

bool BuyPetitionAction::canBuyPetition(Player* bot)
{
    if (bot->GetGuildId())
        return false;

    if (bot->GetGuildIdInvited())
        return false;

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (AI_VALUE2(uint32, "item count", "Hitem:5863:"))
        return false;

    if (ai->GetGuilderType() == GuilderType::SOLO)
        return false;

    if (ai->GetGrouperType() == GrouperType::SOLO)
        return false;

    if (!ai->HasStrategy("guild", BOT_STATE_NON_COMBAT))
        return false;

    uint32 cost = 1000; //GUILD_CHARTER_COST;

    if (AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) < cost)
        return false;

    return true;
}

bool PetitionOfferAction::Execute(Event event)
{
    uint32 petitionEntry = 5863; //GUILD_CHARTER
    list<Item*> petitions = AI_VALUE2(list<Item*>, "inventory items", chat->formatQItem(5863));

   if (petitions.empty())
        return false;

    ObjectGuid guid = event.getObject();

    Player* master = GetMaster();
    if (!master)
    {
        if (!guid)
            guid = bot->GetSelectionGuid();
    }
    else {
        if (!guid)
            guid = master->GetSelectionGuid();
    }

    if (!guid)
        return false;

    Player* player = sObjectMgr.GetPlayer(guid);

    if (!player)
        return false;

    WorldPacket data(CMSG_OFFER_PETITION);

    data << uint32(0);
    data << petitions.front()->GetObjectGuid();
    data << guid;

    QueryResult* result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE player_account = '%u' AND petitionguid = '%u'", player->GetSession()->GetAccountId(), petitions.front()->GetObjectGuid().GetCounter());

    if (result)
    {
        return false;
    }

    bot->GetSession()->HandleOfferPetitionOpcode(data);

    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitions.front()->GetObjectGuid().GetCounter());
    uint8 signs = result ? (uint8)result->GetRowCount() : 0;

    context->GetValue<uint8>("petition signs")->Set(signs);

    return true;
}

bool PetitionOfferAction::isUseful();
{
    return !bot->GetGuildId();
}

bool PetitionOfferNearbyAction::Execute(Event event)
{
    uint32 found = 0;

    list<ObjectGuid> nearGuids = ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest friendly players")->Get();
    for (auto& i : nearGuids)
    {
        Player* player = sObjectMgr.GetPlayer(i);

        if (!player)
            continue;

        if (player->GetGuildId())
            continue;

        if (player->GetGuildIdInvited())
            continue;

        PlayerbotAI* botAi = player->GetPlayerbotAI();

        if (botAi)
        {
            /*
            if (botAi->GetGrouperType() == SOLO && !botAi->HasRealPlayerMaster()) //Do not invite solo players.
                continue;

            */
            if (botAi->HasActivePlayerMaster()) //Do not invite alts of active players.
                continue;
        }
        else
        {
            if (!sPlayerbotAIConfig.randomBotGroupNearby)
                return false;
        }

        if (sServerFacade.GetDistance2d(bot, player) > sPlayerbotAIConfig.sightDistance)
            continue;

        //Parse rpg target to quest action.
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        p << i;
        p.rpos(0);

        if (PetitionOfferAction::Execute(Event("petition offer nearby", p)))
            found++;
    }

    return found > 0;
}

bool PetitionOfferNearbyAction::isUseful()
{
    return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) && AI_VALUE(uint8, "petition signs") < sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS);
}

bool PetitionTurnInAction::Execute(Event event)
{
    list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest npcs")->Get();
    bool vendored = false, result = false;
    list<Item*> petitions = AI_VALUE2(list<Item*>, "inventory items", chat->formatQItem(5863));

    if (petitions.empty())
        return false;

    for (list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_PETITIONER);
        if (!pCreature)
            continue;

        WorldPacket data(CMSG_TURN_IN_PETITION, 8);

        Item* petition = petitions.front();

        if (!petition)
            return false;

        data << petition->GetObjectGuid();

        bot->GetSession()->HandleTurnInPetitionOpcode(data);

        if (bot->GetGuildId())
        {
            Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
            uint32 st, cl, br, bc, bg;
            bg = urand(0, 51);
            bc = urand(0, 17);
            cl = urand(0, 17);
            br = urand(0, 7);
            st = urand(0, 180);
            guild->SetEmblem(st, cl, br, bc, bg);

            //LANG_GUILD_VETERAN -> can invite
            guild->SetRankRights(2, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK | GR_RIGHT_INVITE);
        }

        return true;
    }

    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    //Select a new target to travel to.
    TravelTarget newTarget = TravelTarget(ai);

    bool foundTarget = SetNpcFlagTarget(&newTarget, { UNIT_NPC_FLAG_PETITIONER });

    if (!foundTarget || !newTarget.isActive())
        return false;

    newTarget.setRadius(INTERACTION_DISTANCE);

    setNewTarget(&newTarget, oldTarget);

    return true;
}

bool PetitionTurnInAction::isUseful() override
{
    bool inCity = false;
    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(bot->GetAreaId());
    if (areaEntry)
    {
        if (areaEntry->zone)
            areaEntry = GetAreaEntryByAreaID(areaEntry->zone);

        if (areaEntry && areaEntry->flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    return inCity && !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) &&
        AI_VALUE(uint8, "petition signs") >= sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS) && !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}

bool BuyTabardAction::Execute(Event event)
{
    bool canBuy = ai->DoSpecificAction("buy", Event("buy tabard", "Hitem:5976:"));
    if (canBuy && AI_VALUE2(uint32, "item count", chat->formatQItem(5976)))
        return true;

    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    //Select a new target to travel to.
    TravelTarget newTarget = TravelTarget(ai);

    bool foundTarget = SetNpcFlagTarget(&newTarget, { UNIT_NPC_FLAG_TABARDDESIGNER }, "Tabard Vendor", { 5976 });

    if (!foundTarget || !newTarget.isActive())
        return false;

    newTarget.setRadius(INTERACTION_DISTANCE);

    setNewTarget(&newTarget, oldTarget);

    return true;
};

bool BuyTabardAction::isUseful()
{
    bool inCity = false;
    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(bot->GetAreaId());
    if (areaEntry)
    {
        if (areaEntry->zone)
            areaEntry = GetAreaEntryByAreaID(areaEntry->zone);

        if (areaEntry && areaEntry->flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    return inCity && bot->GetGuildId() && !AI_VALUE2(uint32, "item count", chat->formatQItem(5976)) &&
        AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) >= 10000 && !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}
