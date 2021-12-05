/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RangeTriggers.h"
#include "Playerbot.h"
#include "ServerFacade.h"

bool EnemyTooCloseForSpellTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target)
    {
        if (target->GetTarget() == bot && !bot->GetGroup() && !target->IsRooted() && target->GetSpeedInMotion() > bot->GetSpeedInMotion() * 0.65f)
            return false;

        bool isBoss = false;
        bool isRaid = false;
        float targetDistance = sServerFacade.GetDistance2d(bot, target) + bot->GetCombinedCombatReach(target, false);
        if (target->IsCreature())
        {
            Creature* creature = ai->GetCreature(target->GetObjectGuid());
                if (creature)
                {
                    isBoss = creature->IsWorldBoss();
                }
        }

        if (bot->GetMap() && bot->GetMap()->IsRaid())
            isRaid = true;

        if (isBoss || isRaid)
            return sServerFacade.IsDistanceLessThan(targetDistance, ai->GetRange("spell"));

        return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, (ai->GetRange("spell") / 2));
    }
    return false;
}

bool EnemyTooCloseForShootTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (target->GetTarget() == bot && !bot->GetGroup() && !target->IsRooted() && target->GetSpeedInMotion() > bot->GetSpeedInMotion() * 0.65f)
        return false;

    bool isBoss = false;
    float targetDistance = sServerFacade.GetDistance2d(bot, target) + bot->GetCombinedCombatReach(target, false);
    if (target->IsCreature())
    {
        Creature* creature = ai->GetCreature(target->GetObjectGuid());
        if (creature)
        {
            isBoss = creature->IsWorldBoss();
        }
    }

    if (isBoss)
        return sServerFacade.IsDistanceLessThan(targetDistance, ai->GetRange("shoot"));

    return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, (ai->GetRange("shoot") / 2));
}

bool EnemyTooCloseForMeleeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsPlayer())
        return false;

    return target && AI_VALUE2(bool, "inside target", "current target");
}

bool EnemyIsCloseTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), sPlayerbotAIConfig->tooCloseDistance);
}

bool OutOfRangeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    return target && sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", GetTargetName()), distance);
}

bool EnemyOutOfSpellRangeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    return target && (sServerFacade.GetDistance2d(bot, target) > distance || !bot->IsWithinLOSInMap(target, true));
}

bool EnemyOutOfMeleeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (!target)
        return false;

    float targetDistance = sServerFacade.GetDistance2d(bot, target);
    return target && (targetDistance > max(5.0f, bot->GetCombinedCombatReach(target, true)) || (!bot->IsWithinLOSInMap(target, true) && targetDistance > 5.0f));
}

bool PartyMemberToHealOutOfSpellRangeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (!target)
        return false;

    float combatReach = bot->GetCombinedCombatReach(target, false);
    return target && (sServerFacade.GetDistance2d(bot, target) > (distance + combatReach) || !bot->IsWithinLOSInMap(target, true));
}

bool FarFromMasterTrigger::IsActive()
{
    return sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), distance);
}
