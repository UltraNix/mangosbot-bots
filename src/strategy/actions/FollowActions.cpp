/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FollowActions.h"
#include "Event.h"
#include "Formations.h"
#include "Playerbot.h"
#include "ServerFacade.h"

bool FollowAction::Execute(Event event)
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const& target = formation->GetTargetName();

    bool moved = false;
    if (!target.empty())
    {
        moved = Follow(AI_VALUE(Unit*, target));
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.GetMapId() == -1)
            return false;

        moved = MoveTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
    }

    //if (moved)
        //botAI->SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);

    return moved;
}

bool FollowAction::isUseful()
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const& target = formation->GetTargetName();

    Unit* fTarget = nullptr;
    if (!target.empty())
        fTarget = AI_VALUE(Unit*, target);
    else
        fTarget = AI_VALUE(Unit*, "master target");

    if (fTarget && (fTarget->HasUnitState(UNIT_STATE_IN_FLIGHT) && (bot->IsAlive() || bot->GetCorpse()) ||
        fTarget->GetGUID() == bot->GetGUID() || fTarget->getDeathState() != bot->getDeathState()))
        return false;

    float distance = 0.f;
    if (!target.empty())
    {
        distance = AI_VALUE2(float, "distance", target);
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || bot->GetMapId() != loc.GetMapId())
            return false;

        distance = sServerFacade->GetDistance2d(bot, loc.GetPositionX(), loc.GetPositionY());
    }

    return sServerFacade->IsDistanceGreaterThan(distance, formation->GetMaxDistance());
}

bool FleeToMasterAction::Execute(Event event)
{
    bool canFollow = Follow(AI_VALUE(Unit*, "master target"));
    if (!canFollow)
    {
        //botAI->SetNextCheckDelay(5000);
        return false;
    }

    botAI->TellMaster("Wait for me!");
    botAI->SetNextCheckDelay(3000);

    return true;
}

bool FleeToMasterAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return botAI->GetGroupMaster() && botAI->GetGroupMaster() != bot &&
        (!target || (target && botAI->GetGroupMaster()->GetTarget() != target->GetGUID())) && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT);
}
