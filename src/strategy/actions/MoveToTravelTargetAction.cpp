/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MoveToTravelTargetAction.h"
#include "ChooseRpgTargetAction.h"
#include "LootObjectStack.h"
#include "PathGenerator.h"
#include "Playerbot.h"
#include "TravelMgr.h"

bool MoveToTravelTargetAction::Execute(Event event)
{
    //Check if we still have to loot. If so, loot first.
    bool addLoot = botAI->DoSpecificAction("add all loot", Event(), true);
    if (addLoot)
        bool addLoot = botAI->DoSpecificAction("loot", Event(), true);

    if (context->GetValue<LootObject>("loot target")->Get().IsLootPossible(bot))
    {
       return false;
    }

    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    WorldPosition botLocation(bot);

    float maxDistance = target->getDestination()->getRadiusMin();

    //Evenly distribute around the target.
    float angle = 2 * M_PI * urand(0, 100) / 100.0;

    WorldLocation location = target->getLocation();

    if (target->getMaxTravelTime() > target->getTimeLeft()) //The bot is late. Speed it up.
    {
        //distance = sPlayerbotAIConfig->fleeDistance;
        //angle = bot->GetAngle(location.GetPositionX(), location.GetPositionY());
        //location = botLocation.getLocation();
    }

    float x = location.GetPositionX();
    float y = location.GetPositionY();
    float z = location.GetPositionZ();
    float mapId = location.GetMapId();

    //Move between 0.5 and 1.0 times the maxDistance.
    float mod = frand(50.f, 100.f) / 100.0f;

    x += cos(angle) * maxDistance * mod;
    y += sin(angle) * maxDistance * mod;

    bool canMove = false;

    if (bot->IsWithinLOS(x, y, z))
        canMove = MoveNear(mapId, x, y, z, 0);
    else
        canMove = MoveTo(mapId, x, y, z, false, false);

    if (!canMove && !target->isForced())
    {
        target->incRetry(true);

        if (target->isMaxRetry(true))
            target->setStatus(TRAVEL_STATUS_COOLDOWN);
    }
    else
        target->setRetry(true);

    return canMove;
}

bool MoveToTravelTargetAction::isUseful()
{
    if (!context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling())
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    if (bot->IsFlying())
        return false;

    if (bot->isMoving())
        return false;

    if (bot->IsInCombat())
        return false;

    if (AI_VALUE2(uint8, "health", "self target") <= sPlayerbotAIConfig->mediumHealth)
        return false;

    if (AI_VALUE2(uint8, "mana", "self target") && AI_VALUE2(uint8, "mana", "self target") <= sPlayerbotAIConfig->mediumMana)
        return false;

    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.IsLootPossible(bot))
        return false;

    if (!ChooseRpgTargetAction::isFollowValid(bot, context->GetValue<TravelTarget*>("travel target")->Get()->getLocation()))
        return false;

    return true;
}

