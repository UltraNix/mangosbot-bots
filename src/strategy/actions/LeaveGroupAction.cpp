/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LeaveGroupAction.h"
#include "Event.h"
#include "Playerbot.h"

bool LeaveGroupAction::Execute(Event event)
{
    return Leave();
}

bool PartyCommandAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 operation;
    std::string member;

    p >> operation >> member;

    if (operation != PARTY_OP_LEAVE)
        return false;

    Player* master = GetMaster();
    if (master && member == master->GetName())
        return Leave();

    return false;
}

bool UninviteAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();
    if (p.GetOpcode() == CMSG_GROUP_UNINVITE)
    {
        p.rpos(0);
        std::string membername;
        p >> membername;

        // player not found
        if (!normalizePlayerName(membername))
        {
            return false;
        }

        if (bot->GetName() == membername)
            return Leave();
    }

    if (p.GetOpcode() == CMSG_GROUP_UNINVITE_GUID)
    {
        p.rpos(0);
        ObjectGuid guid;
        p >> guid;

        if (bot->GetGUID() == guid)
            return Leave();
    }

    return false;
}

bool LeaveGroupAction::Leave()
{
    bool aiMaster = botAI->GetMaster()->GetPlayerbotAI();

    botAI->TellMaster("Goodbye!", PLAYERBOT_SECURITY_TALK);

    WorldPacket p;
    std::string const& member = bot->GetName();
    p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
    bot->GetSession()->HandleGroupDisbandOpcode(p);

    bool randomBot = sRandomPlayerbotMgr->IsRandomBot(bot);
    if (randomBot)
    {
        bot->GetPlayerbotAI()->SetMaster(nullptr);
        sRandomPlayerbotMgr->ScheduleTeleport(bot->GetGUID().GetCounter());
    }

    if (!aiMaster)
        botAI->ResetStrategies(!randomBot);

    botAI->Reset();

    return true;
}

bool LeaveFarAwayAction::isUseful()
{
    if (!sPlayerbotAIConfig->randomBotGroupNearby)
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    if (!bot->GetGroup())
        return false;

    Player* master = botAI->GetGroupMaster();
    Player* trueMaster = botAI->GetMaster();
    if (!master || (bot == master && !botAI->IsRealPlayer()))
        return false;

    if (master && !master->GetPlayerbotAI())
        return false;

    if (trueMaster && !trueMaster->GetPlayerbotAI())
        return false;

    if (botAI->GetGrouperType() == SOLO)
        return true;

    if (abs(int32(master->getLevel() - bot->getLevel())) > 4)
        return true;

    if (master->GetDistance(bot) > sPlayerbotAIConfig->reactDistance * 4)
        return true;

    return false;
}
