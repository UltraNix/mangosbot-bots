/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReachTargetActions.h"
#include "Event.h"
#include "Playerbot.h"
#include "ServerFacade.h"

bool ReachTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (!target)
        return false;

    UpdateMovementState();

    float combatReach = bot->GetCombinedCombatReach(target, true);
    if (distance < max(5.0f, combatReach))
    {
        return ChaseTo(target, 0.0f, GetFollowAngle());
    }
    else
    {
        combatReach = bot->GetCombinedCombatReach(target, false);
        bool inLos = bot->IsWithinLOSInMap(target, true);
        bool isFriend = sServerFacade.IsFriendlyTo(bot, target);
        float chaseDist = inLos ? distance : isFriend ? distance / 2 : distance;
        return ChaseTo(target, chaseDist + combatReach, bot->GetAngle(target));
    }
}

bool ReachTargetAction::isUseful()
{
    // do not move while casting
    if (bot->IsNonMeleeSpellCasted(true))
        return false;

    Unit* target = AI_VALUE(Unit*, GetTargetName());
    return target && (!bot->IsWithinDistInMap(target, distance) || (bot->IsWithinDistInMap(target, distance) && !bot->IsWithinLOSInMap(target, true)));
}

std::string const& ReachTargetAction::GetTargetName()
{
    return "current target";
}

bool CastReachTargetSpellAction::isUseful()
{
    return sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), (distance + sPlayerbotAIConfig->contactDistance));
}

std::string const& ReachPartyMemberToHealAction::GetTargetName()
{
    return "party member to heal";
}
