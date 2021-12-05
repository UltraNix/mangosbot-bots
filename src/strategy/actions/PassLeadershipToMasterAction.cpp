/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PassLeadershipToMasterAction.h"
#include "Event.h"
#include "Playerbot.h"

bool PassLeadershipToMasterAction::Execute(Event event)
{
    if (Player* master = GetMaster())
        if (master && master != bot && bot->GetGroup() && bot->GetGroup()->IsMember(master->GetGUID()))
        {
            WorldPacket p(SMSG_GROUP_SET_LEADER, 8);
            p << master->GetGUID();
            bot->GetSession()->HandleGroupSetLeaderOpcode(p);

            if (!message.empty())
                ai->TellMasterNoFacing(message);

            if (sRandomPlayerbotMgr.IsRandomBot(bot))
            {
                ai->ResetStrategies();
                ai->Reset();
            }

            return true;
        }

    return false;
}

bool PassLeadershipToMasterAction::isUseful()
{
    return ai->IsAlt() && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid());
}

bool GiveLeaderAction::isUseful()
{
    return ai->HasActivePlayerMaster() && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid());
}
