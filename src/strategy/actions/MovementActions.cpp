/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"
#include "MovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "LastMovementValue.h"
#include "PositionValue.h"
#include "Stances.h"
#include "FleeManager.h"
#include "LootObjectStack.h"
#include "ServerFacade.h"
#include "TravelMgr.h"
#include "TravelNode.h"
#include "Transport.h"

MovementAction::MovementAction(PlayerbotAI* botAI, std::string const& name) : Action(botAI, name)
{
    bot = botAI->GetBot();
}

void MovementAction::CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 5000.0f; // 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;

    //if(!important)
    //    delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, delay);
    if (!important)
        wpCreature->SetObjectScale(0.2f);
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance)
{
    float angle = GetFollowAngle();
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z);
}

bool MovementAction::MoveNear(WorldObject* target, float distance)
{
    if (!target)
        return false;

    distance += target->GetCombatReach();

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    float followAngle = GetFollowAngle();

    for (float angle = followAngle; angle <= followAngle + 2 * M_PI; angle += M_PI / 4.f)
    {
        float x = target->GetPositionX() + cos(angle) * distance;
        float y = target->GetPositionY() + sin(angle) * distance;
        float z = target->GetPositionZ();

        if (!bot->IsWithinLOS(x, y, z))
            continue;

        bool moved = MoveTo(target->GetMapId(), x, y, z);
        if (moved)
            return true;
    }

    //botAI->TellError("All paths not in LOS");
    return false;
}

bool MovementAction::MoveToLOS(WorldObject* target, bool ranged)
{
    if (!target)
        return false;

    //std::ostringstream out; out << "Moving to LOS!";
    //bot->Say(out.str(), LANG_UNIVERSAL);

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    //Use standard pathfinder to find a route.
    PathFinder path(bot);
    path.calculate(x, y, z, false);
    PathType type = path.getPathType();
    if (type != PATHFIND_NORMAL && type != PATHFIND_INCOMPLETE)
        return false;

    if (!ranged)
        return MoveTo((Unit*)target, target->GetObjectBoundingRadius());

    float dist = FLT_MAX;
    PositionInfo dest;

    if (!path.getPath().empty())
    {
        for (auto& point : path.getPath())
        {
            if (botAI->HasStrategy("debug", BOT_STATE_NON_COMBAT))
                CreateWp(bot, point.x, point.y, point.z, 0.0, 15631);

            float distPoint = target->GetDistance(point.x, point.y, point.z, DIST_CALC_NONE);
            if (distPoint < dist && target->IsWithinLOS(point.x, point.y, point.z + bot->GetCollisionHeight()))
            {
                dist = distPoint;
                dest.Set(point.x, point.y, point.z, target->GetMapId());

                if (ranged)
                    break;
            }
        }
    }

    if (dest.isSet())
        return MoveTo(dest.mapId, dest.x, dest.y, dest.z);
    else
        botAI->TellError("All paths not in LOS");

    return false;
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react)
{
    UpdateMovementState();

    bool detailedMove = botAI->AllowActive(DETAILED_MOVE_ACTIVITY);
    if (!detailedMove)
    {
        time_t now = time(nullptr);
        if (AI_VALUE(LastMovement&, "last movement").nextTeleport > now) //We can not teleport yet. Wait.
            return true;
    }

    float minDist = sPlayerbotAIConfig->targetPosRecalcDistance; //Minium distance a bot should move.
    float maxDist = sPlayerbotAIConfig->reactDistance;           //Maxium distance a bot can move in one single action.
    bool generatePath = !bot->IsFlying() && !bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING) && !bot->IsInWater() && !bot->IsUnderWater();
    if (generatePath)
    {
        z += CONTACT_DISTANCE;
        bot->UpdateAllowedPositionZ(x, y, z);
    }

    if (!IsMovingAllowed() && bot->isDead())
    {
        bot->StopMoving();
        return false;
    }

    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    WorldPosition startPosition = WorldPosition(bot);             //Current location of the bot
    WorldPosition endPosition = WorldPosition(mapId, x, y, z, 0); //The requested end location
    WorldPosition movePosition;                                   //The actual end location

    float totalDistance = startPosition.distance(endPosition);    //Total distance to where we want to go
    float maxDistChange = totalDistance * 0.1;                    //Maximum change between previous destination before needing a recalulation

    if (totalDistance < minDist)
    {
        if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange)
            AI_VALUE(LastMovement&, "last movement").clear();

        bot->StopMoving();
        return false;
    }

    TravelPath movePath;

    if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange && startPosition.distance(lastMove.lastMoveShort) < maxDist) //The last short movement was to the same place we want to move now.
        movePosition = endPosition;
    else if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPosition) < maxDistChange) //The last long movement was to the same place we want to move now.
    {
        movePath = lastMove.lastPath;
    }
    else
    {
        movePosition = endPosition;

        std::vector<WorldPosition> beginPath;
        std::vector<WorldPosition> endPath;

        if (totalDistance > maxDist)
        {
            if (!sTravelNodeMap.getNodes().empty())
            {
                //[[Node pathfinding system]]
                //We try to find nodes near the bot and near the end position that have a route between them.
                //Then bot has to move towards/along the route.
                sTravelNodeMap.m_nMapMtx.lock_shared();

                //Find the route of nodes starting at a node closest to the start position and ending at a node closest to the endposition.
                //Also returns longPath: The path from the start position to the first node in the route.
                TravelNodeRoute route = sTravelNodeMap.getRoute(&startPosition, &endPosition, beginPath, bot);
                if (sPlayerbotAIConfig->hasLog("bot_pathfinding.csv"))
                {
                    sPlayerbotAIConfig->log("bot_pathfinding.csv", route.print().str().c_str());
                }

                if (route.isEmpty())
                {
                    sTravelNodeMap.m_nMapMtx.unlock_shared();

                    //We have no path. Beyond 450yd the standard pathfinder will probably move the wrong way.
                    if (sServerFacade->IsDistanceGreaterThan(totalDistance, maxDist * 3))
                    {
                        movePath.clear();
                        movePath.addPoint(endPosition);
                        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

                        bot->StopMoving();
                        if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
                            botAI->TellMasterNoFacing("I have no path");
                        return false;
                    }

                    movePosition = endPosition;
                }
                else
                {
                    endPath = route.getNodes().back()->getPosition()->getPathTo(endPosition, nullptr);
                    movePath = route.buildPath(beginPath, endPath);

                    if (sPlayerbotAIConfig->hasLog("bot_pathfinding.csv"))
                    {
                        sPlayerbotAIConfig->log("bot_pathfinding.csv", movePath.print().str().c_str());
                    }
                    sTravelNodeMap.m_nMapMtx.unlock_shared();
                }
            }
            else
            {
                //Use standard pathfinder to find a route.
                movePosition = endPosition;
            }
        }
    }

    if (movePath.empty() && movePosition.distance(startPosition) > maxDist)
    {
        //Use standard pathfinder to find a route.
        PathFinder path(bot);
        path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = path.getPathType();
        PointsArray& points = path.getPath();
        movePath.addPath(startPosition.fromPointsArray(points));
    }

    if (!movePath.empty())
    {
        if (movePath.makeShortCut(startPosition, maxDist))
            if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
                botAI->TellMasterNoFacing("Found a shortcut.");

        if (movePath.empty())
        {
            AI_VALUE(LastMovement&, "last movement").setPath(movePath);

            if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
                botAI->TellMasterNoFacing("Too far from path. Rebuilding.");

            return true;
        }

        bool isTeleport, isTransport;
        uint32 entry;
        movePosition = movePath.getNextPoint(startPosition, maxDist, isTeleport, isTransport, entry);

        if (isTeleport)// && !botAI->IsRealPlayer())
        {
            //Log bot movement
            if (sPlayerbotAIConfig->hasLog("bot_movement.csv"))
            {
                WorldPosition telePos;
                if (entry)
                {
                    AreaTrigger const* at = sObjectMgr.GetAreaTrigger(entry);
                    if (at)
                        telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
                }
                else
                    telePos = movePosition;

                std::ostringstream out;
                out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
                out << bot->GetName() << ",";
                if (telePos && telePos != movePosition)
                    startPosition.printWKT({ startPosition, movePosition, telePos }, out, 1);
                else
                    startPosition.printWKT({ startPosition, movePosition }, out, 1);

                out << std::to_string(bot->getRace()) << ",";
                out << std::to_string(bot->getClass()) << ",";
                out << bot->getLevel() << ",";
                out << (entry ? -1 : entry);

                sPlayerbotAIConfig->log("bot_movement.csv", out.str().c_str());
            }

            if (entry)
            {
                AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = entry;
            }
            else
                return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), movePosition.getO(), 0);
        }

        if (isTransport && entry)
        {
            if (!bot->GetTransport())
            {
                for (auto& transport : movePosition.getTransports(entry))
                    if (movePosition.sqDistance2d(WorldPosition((WorldObject*)transport)) < 5 * 5)
                        transport->AddPassenger(bot, true);
            }
            WaitForReach(100.0f);
            return true;
        }
        //if (!isTransport && bot->GetTransport())
        //    bot->GetTransport()->RemovePassenger(bot);
    }

    AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    if (movePosition == WorldPosition())
    {
        movePath.clear();

        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
            botAI->TellMasterNoFacing("No point. Rebuilding.");

        return false;
    }

    //Visual waypoints
    if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    {
        if (!movePath.empty())
        {
            float cx = x;
            float cy = y;
            float cz = z;
            for (auto i : movePath.getPath())
            {
                CreateWp(bot, i.point.getX(), i.point.getY(), i.point.getZ(), 0.f, 15631);

                cx = i.point.getX();
                cy = i.point.getY();
                cz = i.point.getZ();
            }
        }
        else
            CreateWp(bot, movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0, 15631, true);
    }

    //Log bot movement
    if (sPlayerbotAIConfig->hasLog("bot_movement.csv") && lastMove.lastMoveShort != movePosition)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        startPosition.printWKT({ startPosition, movePosition }, out, 1);
        out << std::to_string(bot->getRace()) << ",";
        out << std::to_string(bot->getClass()) << ",";
        out << bot->getLevel();
        out << 0;

        sPlayerbotAIConfig->log("bot_movement.csv", out.str().c_str());
    }

    if (!react)
        if (totalDistance > maxDist)
            WaitForReach(startPosition.distance(movePosition) - 10.0f);
        else
            WaitForReach(startPosition.distance(movePosition));

    bot->HandleEmoteState(0);
    if (bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCasted(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    if (lastMove.lastMoveShort.distance(movePosition) < minDist)
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
    }

    if (!detailedMove && !botAI->HasPlayerNearby(&movePosition)) //Why walk if you can fly?
    {
        time_t now = time(nullptr);

        AI_VALUE(LastMovement&, "last movement").nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));

        return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    }

    // walk if master walks and is close
    bool masterWalking = false;
    if (botAI->GetMaster())
    {
        if (botAI->GetMaster()->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE) && sServerFacade->GetDistance2d(bot, botAI->GetMaster()) < 20.0f)
            masterWalking = true;
    }

    bot->GetMotionMaster()->MovePoin(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);

    AI_VALUE(LastMovement&, "last movement").setShort(movePosition);

    if (!idle)
        ClearIdleState();

    return true;
}

bool MovementAction::MoveTo(Unit* target, float distance)
{
    if (!IsMovingAllowed(target))
    {
        //botAI->TellError("Seems I am stuck");
        return false;
    }

    float bx = bot->GetPositionX(), by = bot->GetPositionY(), bz = bot->GetPositionZ();
    float tx = target->GetPositionX(), ty = target->GetPositionY(), tz = target->GetPositionZ();

    if (bot->IsHostileTo(target))
    {
        Stance* stance = AI_VALUE(Stance*, "stance");

        WorldLocation const& loc = stance->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.GetMapId() == -1)
        {
            //botAI->TellError("Nowhere to move");
            return false;
        }

        tx = loc.GetPositionX();
        ty = loc.GetPositionY();
        tz = loc.GetPositionZ();
    }

    float distanceToTarget = sServerFacade->GetDistance2d(bot, tx, ty);
    if (sServerFacade->IsDistanceGreaterThan(distanceToTarget, sPlayerbotAIConfig->targetPosRecalcDistance))
    {
        float angle = bot->GetAngle(tx, ty);
        float needToGo = distanceToTarget - distance;

        float maxDistance = botAI->GetRange("spell");
        if (needToGo > 0 && needToGo > maxDistance)
            needToGo = maxDistance;
        else if (needToGo < 0 && needToGo < -maxDistance)
            needToGo = -maxDistance;

        float dx = cos(angle) * needToGo + bx;
        float dy = sin(angle) * needToGo + by;
        float dz = bz + (tz - bz) * needToGo / distanceToTarget;

        return MoveTo(target->GetMapId(), dx, dy, dz);
    }

    return true;
}

float MovementAction::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = master ? master->GetGroup() : bot->GetGroup();
    if (!group)
        return 0.0f;

    uint32 index = 1;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (ref->GetSource() == master)
            continue;

        if (ref->GetSource() == bot)
            return 2 * M_PI / (group->GetMembersCount() -1) * index;

        ++index;
    }

    return 0;
}

bool MovementAction::IsMovingAllowed(Unit* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

    float distance = bot->GetDistance(target);
    if (!bot->InBattleground() && distance > sPlayerbotAIConfig->reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    float distance = bot->GetDistance(x, y, z);
    if (!bot->InBattleground() && distance > sPlayerbotAIConfig->reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed()
{
    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) || bot->IsBeingTeleported() ||
        bot->isInRoots() || bot->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) || bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || bot->IsCharmed() ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->IsFlying() ||
        bot->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return false;

    return bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
}

bool MovementAction::Follow(Unit* target, float distance)
{
    return Follow(target, distance, GetFollowAngle());
}

void MovementAction::UpdateMovementState()
{
    if (bot->IsInWater() || bot->IsUnderWater())
    {
        bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_SWIMMING);
        bot->UpdateSpeed(MOVE_SWIM, true);
    }
    else
    {
        bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_SWIMMING);
        bot->UpdateSpeed(MOVE_SWIM, true);
    }

    // Temporary speed increase in group
    if (botAI->HasRealPlayerMaster())
        bot->SetSpeedRate(MOVE_RUN, 1.1f);
}

bool MovementAction::Follow(Unit* target, float distance, float angle)
{
    UpdateMovementState();

    if (!target)
        return false;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->followDistance))
    {
        //botAI->TellError("No need to follow");
        return false;
    }

    if (!bot->InBattleground()
        && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target->GetPositionX(), target->GetPositionY()), sPlayerbotAIConfig->sightDistance)
        && abs(bot->GetPositionZ() - target->GetPositionZ()) >= sPlayerbotAIConfig->spellDistance && botAI->HasRealPlayerMaster()
        && (target->GetMapId() && bot->GetMapId() != target->GetMapId()))
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        float z = target->GetPositionZ();
        if (target->GetMapId() && bot->GetMapId() != target->GetMapId())
        {
            if ((target->GetMap() && target->GetMap()->IsBattlegroundOrArena()) || (bot->GetMap() && bot->GetMap()->IsBattlegroundOrArena()))
                return false;

            bot->TeleportTo(target->GetMapId(), x, y, z, bot->GetOrientation());
        }
        else
        {
            bot->Relocate(x, y, z, bot->GetOrientation());
        }

        AI_VALUE(LastMovement&, "last movement").Set(target);
        ClearIdleState();
        return true;
    }

    if (!IsMovingAllowed(target) && botAI->HasRealPlayerMaster())
    {
        if ((target->GetMap() && target->GetMap()->IsBattlegroundOrArena()) || (bot->GetMap() && bot->GetMap()->IsBattlegroundOrArena()))
            return false;

        if (sServerFacade->UnitIsDead(bot) && sServerFacade->IsAlive(botAI->GetMaster()))
        {
            bot->ResurrectPlayer(1.0f, false);
            botAI->TellMasterNoFacing("I live, again!");
        }
        else
            botAI->TellError("I am stuck while following");

        bot->CombatStop(true);
        botAI->TellMasterNoFacing("I will there soon.");
		bot->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation());
        return false;
    }

    if (sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->followDistance))
    {
        //botAI->TellError("No need to follow");
        return false;
    }

    if (target->IsFriendlyTo(bot) && bot->IsMounted() && AI_VALUE(GuidVector, "all targets").empty())
        distance += angle;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->followDistance))
    {
        //botAI->TellError("No need to follow");
        return false;
    }

    bot->HandleEmoteCommand(0);

    if (bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCast(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit *currentTarget = sServerFacade->GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetGUID() == target->GetGUID())
            return false;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        bot->GetMotionMaster()->Clear();

    bot->GetMotionMaster()->MoveFollow(target, distance, angle);
    return true;
}

bool MovementAction::ChaseTo(WorldObject* obj)
{
    if (bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCast(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveChase((Unit*)obj, botAI->IsRanged(bot) ? 25.0f : 1.5f);
    return true;
}

float MovementAction::MoveDelay(float distance)
{
    return distance / bot->GetSpeed(MOVE_RUN);
}

void MovementAction::WaitForReach(float distance)
{
    float delay = 1000.0f * MoveDelay(distance) + sPlayerbotAIConfig->reactDelay;
    if (delay > sPlayerbotAIConfig->maxWaitForMove)
        delay = sPlayerbotAIConfig->maxWaitForMove;

    Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
    Unit* player = *botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
    if ((player || target) && delay > sPlayerbotAIConfig->globalCoolDown)
        delay = sPlayerbotAIConfig->globalCoolDown;

    if (delay < 0)
        delay = 0;

    botAI->SetNextCheckDelay((uint32)delay);
}

bool MovementAction::Flee(Unit *target)
{
    Player* master = GetMaster();
    if (!target)
        target = master;

    if (!target)
        return false;

    if (!sPlayerbotAIConfig->fleeingEnabled)
        return false;

    if (!IsMovingAllowed())
    {
        botAI->TellError("I am stuck while fleeing");
        return false;
    }

    bool foundFlee = false;
    bool isTarget = false;
    time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;

    if (HostileReference* ref = target->getThreatManager().getCurrentVictim())
        if (ref->getTarget() == bot)
        {
            isTarget = true;

            if (Group* group = bot->GetGroup())
            {
                for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
                {
                    Player* player = gref->GetSource();
                    if (!player || player == bot)
                        continue;

                    if (botAI->IsTank(player))
                    {
                        float distanceToTank = sServerFacade->GetDistance2d(bot, player);
                        float distanceToTarget = sServerFacade->GetDistance2d(bot, target);
                        if (sServerFacade->IsDistanceGreaterThan(distanceToTank, distanceToTarget))
                        {
                            foundFlee = MoveTo(player, sPlayerbotAIConfig->followDistance);
                        }
                    }

                    if (!foundFlee && master)
                    {
                        foundFlee = MoveTo(master, sPlayerbotAIConfig->followDistance);
                    }

                    if (foundFlee)
                    {
                        if (!urand(0, 25))
                        {
                            std::vector<uint32> sounds;
                            sounds.push_back(304); // guard
                            sounds.push_back(306); // flee
                            botAI->PlaySound(sounds[urand(0, sounds.size() - 1)]);
                        }

                        return true;
                    }
                }
            }
        }

    if ((foundFlee || lastFlee) && bot->GetGroup())
    {
        uint32 fleeDelay = sPlayerbotAIConfig->returnDelay / 1000;
        time_t now = time(nullptr);
        if (!lastFlee)
        {
            AI_VALUE(LastMovement&, "last movement").lastFlee = now;
        }
        else
        {
            if ((now - lastFlee) > urand(5, fleeDelay * 2))
            {
                AI_VALUE(LastMovement&, "last movement").lastFlee = 0;
            }
            else
                return false;
        }
    }

    FleeManager manager(bot, botAI->GetRange("flee"), bot->GetAngle(target) + M_PI);
    if (!manager.isUseful())
        return false;

    float rx, ry, rz;
    if (!manager.CalculateDestination(&rx, &ry, &rz))
    {
        botAI->TellError("Nowhere to flee");
        return false;
    }

    bool result = MoveTo(target->GetMapId(), rx, ry, rz);
    if (result && !urand(0, 25))
    {
        std::vector<uint32> sounds;
        sounds.push_back(304); // guard
        sounds.push_back(306); // flee
        botAI->PlaySound(sounds[urand(0, sounds.size() - 1)]);
    }

    if (result)
        AI_VALUE(LastMovement&, "last movement").lastFlee = time(nullptr);

    return result;
}

void MovementAction::ClearIdleState()
{
    context->GetValue<time_t>("stay time")->Set(0);
    context->GetValue<PositionMap&>("position")->Get()["random"].Reset();
}

bool FleeAction::Execute(Event event)
{
    return Flee(AI_VALUE(Unit*, "current target"));
}

bool FleeWithPetAction::Execute(Event event)
{
    if (Pet* pet = bot->GetPet())
    {
        if (CreatureAI* creatureAI = ((Creature*)pet)->AI())
        {
            pet->SetReactState(REACT_PASSIVE);
            pet->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
            pet->AttackStop();
        }
    }

    return Flee(AI_VALUE(Unit*, "current target"));
}

bool RunAwayAction::Execute(Event event)
{
    return Flee(AI_VALUE(Unit*, "master target"));
}

bool MoveToLootAction::Execute(Event event)
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot) || AI_VALUE(bool, "possible ads"))
        return false;

    return MoveNear(loot.GetWorldObject(bot), sPlayerbotAIConfig->contactDistance);
}

bool MoveOutOfEnemyContactAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig->contactDistance);
}

bool MoveOutOfEnemyContactAction::isUseful()
{
    return AI_VALUE2(bool, "inside target", "current target");
}

bool SetFacingTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT) || bot->IsFlying())
        return true;

    sServerFacade->SetFacingTo(bot, target);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
    return true;
}

bool SetFacingTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "facing", "current target");
}

bool SetFacingTargetAction::isPossible()
{
    if (sServerFacade->IsFrozen(bot) || bot->IsPolymorphed() ||
        (sServerFacade->UnitIsDead(bot) && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() ||
        bot->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || sServerFacade->IsCharmed(bot) ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->IsFlying() ||
        bot->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return false;

    return true;
}

bool SetBehindTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float angle = GetFollowAngle() / 3 + target->GetOrientation() + M_PI;

    float distance = sPlayerbotAIConfig->contactDistance;
    float x = target->GetPositionX() + cos(angle) * distance;
    float y = target->GetPositionY() + sin(angle) * distance;
    float z = target->GetPositionZ();
    //bot->UpdateGroundPositionZ(x, y, z);

    return MoveTo(bot->GetMapId(), x, y, z);
}

bool SetBehindTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "behind", "current target");
}

bool SetBehindTargetAction::isPossible() const
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !(target->GetVictim() && target->GetVictim()->GetGUID() == bot->GetGUID());
}

bool MoveOutOfCollisionAction::Execute(Event event)
{
    float angle = M_PI * 2000 / frand(1.f, 1000.f);
    float distance = sPlayerbotAIConfig->followDistance;
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
    return AI_VALUE2(bool, "collision", "self target") && botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get().size() < 15;
}

bool MoveRandomAction::Execute(Event event)
{
    //uint32 randnum = bot->GetGUID().GetCounter();       // Semi-random but fixed number for each bot.
    //uint32 cycle = floor(getMSTime() / (1000 * 60));    // Semi-random number adds 1 each minute.

    //randnum = ((randnum + cycle) % 1000) + 1;

    uint32 randnum = urand(1, 2000);

    float angle = M_PI * (float)randnum / 1000; //urand(1, 1000);
    float distance = urand(20, 200);

    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveRandomAction::isUseful()
{
    return !botAI->HasRealPlayerMaster() && botAI->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest friendly players")->Get().size() > urand(25, 100);
}

