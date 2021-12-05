/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReviveFromCorpseAction.h"
#include "Event.h"
#include "FleeManager.h"
#include "GameGraveyard.h"
#include "Playerbot.h"
#include "PlayerbotFactory.h"
#include "ServerFacade.h"

bool ReviveFromCorpseAction::Execute(Event event)
{
    Player* master = ai->GetGroupMaster();
    Corpse* corpse = bot->GetCorpse();

    // follow master when master revives
    WorldPacket& p = event.getPacket();
    if (!p.empty() && p.GetOpcode() == CMSG_RECLAIM_CORPSE && master && !corpse && bot->IsAlive())
    {
        if (sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig->farDistance))
        {
            if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
            {
                botAI->TellMasterNoFacing("Welcome back!");
                botAI->ChangeStrategy("+follow,-stay", BOT_STATE_NON_COMBAT);
                return true;
            }
        }
    }

    if (!corpse)
        return false;

    if (corpse->GetGhostTime() + bot->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP) > time(nullptr))
        return false;

    if (master)
    {
        if (!master->GetPlayerbotAI() && master->isDead() && master->GetCorpse()
            && sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig->farDistance))
            return false;
    }

    if (!ai->HasRealPlayerMaster())
    {
        uint32 dCount = AI_VALUE(uint32, "death count");

        if (dCount >= 5)
        {
            return ai->DoSpecificAction("spirit healer");
        }
    }

    LOG_INFO("playerbots", "Bot %s %s:%d <%s> revives at body", bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();

    WorldPacket packet(CMSG_RECLAIM_CORPSE);
    packet << bot->GetGUID();
    bot->GetSession()->HandleReclaimCorpseOpcode(packet);

    return true;
}

bool FindCorpseAction::Execute(Event event)
{
    if (bot->InBattleground())
        return false;

    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
        return false;

    if (Player* master = ai->GetGroupMaster())
    {
        if (!master->GetPlayerbotAI() &&
            sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig->farDistance))
            return false;
    }

    uint32 dCount = AI_VALUE(uint32, "death count");

    if (!botAI->HasRealPlayerMaster())
    {
        if (dCount >= 5)
        {
            LOG_INFO("playerbots", "Bot %s %s:%d <%s>: died too many times and was sent to an inn",
                bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());
            context->GetValue<uint32>("death count")->Set(0);
            sRandomPlayerbotMgr->RandomTeleportForRpg(bot);
            return true;
        }
    }

    WorldPosition botPos(bot),corpsePos(corpse), moveToPos = corpsePos, masterPos(master);
    float reclaimDist = CORPSE_RECLAIM_RADIUS - 5.0f;
    float corpseDist = botPos.distance(corpsePos);
    int64 deadTime = time(nullptr) - corpse->GetGhostTime();

    bool moveToMaster = master && master != bot && masterPos.fDist(corpsePos) < reclaimDist;

    //Should we ressurect? If so, return false.
    if (corpseDist < reclaimDist)
    {
        if (moveToMaster) //We are near master.
        {
            if (botPos.fDist(masterPos) < sPlayerbotAIConfig.spellDistance)
                return false;
        }
        else if (deadTime > 8 * MINUTE) //We have walked too long already.
            return false;
        else
        {
            list<ObjectGuid> units = AI_VALUE(list<ObjectGuid>, "possible targets no los");

            if (botPos.getUnitsAggro(units, bot) == 0) //There are no mobs near.
                return false;
        }
    }

    // If we are getting close move to a save ressurrection spot instead of just the corpse.
    if (corpseDist < sPlayerbotAIConfig.reactDistance)
    {
        if (moveToMaster)
            moveToPos = masterPos;
        else
        {
            FleeManager manager(bot, reclaimDist, 0.0, urand(0, 1), moveToPos);

            if (manager.isUseful())
            {
                float rx, ry, rz;
                if (manager.CalculateDestination(&rx, &ry, &rz))
                    moveToPos = WorldPosition(moveToPos.getMapId(), rx, ry, rz, 0.0);
                else if (!moveToPos.GetReachableRandomPointOnGround(bot, reclaimDist, urand(0, 1)))
                    moveToPos = corpsePos;
            }
        }
    }

    //Actual mobing part.
    bool moved = false;

    if (!ai->AllowActivity(ALL_ACTIVITY))
    {
        uint32 delay = sServerFacade.GetDistance2d(bot, corpse) / bot->GetSpeed(MOVE_RUN); //Time a bot would take to travel to it's corpse.
        delay = min(delay, uint32(10 * MINUTE)); //Cap time to get to corpse at 10 minutes.

        if (deadTime > delay)
        {
            bot->GetMotionMaster()->Clear();
            bot->TeleportTo(moveToPos.getMapId(), moveToPos.getX(), moveToPos.getY(), moveToPos.getZ(), 0);
        }

            moved = true;
    }
    else
    {
        if (bot->IsMoving())
            moved = true;
        else
        {
            if (deadTime < 10 * MINUTE && dCount < 5) // Look for corpse up to 30 minutes.
            {
                moved = MoveTo(moveToPos.getMapId(), moveToPos.getX(), moveToPos.getY(), moveToPos.getZ(), false, false);
            }

            if (!moved)
            {
                moved = botAI->DoSpecificAction("spirit healer");
            }
        }
    }

    return moved;
}

bool FindCorpseAction::isUseful()
{
    if (bot->InBattleground())
        return false;

    return bot->GetCorpse();
}

WorldSafeLocsEntry const* SpiritHealerAction::GetGrave(bool startZone)
{
    WorldSafeLocsEntry const* ClosestGrave = nullptr;
    WorldSafeLocsEntry const* NewGrave = nullptr;

    ClosestGrave = sObjectMgr.GetClosestGraveYard(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), bot->GetTeam());

    if (!startZone && ClosestGrave)
        return ClosestGrave;

    if (ai->HasStrategy("follow", BOT_STATE_NON_COMBAT) && ai->GetGroupMaster() && ai->GetGroupMaster() != bot)
    {
        Player* master = ai->GetGroupMaster();
        if (master && master != bot)
        {
            ClosestGrave = sObjectMgr.GetClosestGraveYard(master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), master->GetMapId(), bot->GetTeam());

            if (ClosestGrave)
                return ClosestGrave;
        }
    }
    else if(startZone && AI_VALUE(uint8, "durability"))
    {
        TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

        if (travelTarget->getPosition())
        {
            WorldPosition travelPos = *travelTarget->getPosition();
            ClosestGrave = sObjectMgr.GetClosestGraveYard(travelPos.getX(), travelPos.getY(), travelPos.getZ(), travelPos.getMapId(), bot->GetTeam());

            if (ClosestGrave)
                return ClosestGrave;
        }
    }

    vector<uint32> races;

    if (bot->GetTeam() == ALLIANCE)
        races = { RACE_HUMAN, RACE_DWARF,RACE_GNOME,RACE_NIGHTELF };
    else
        races = { RACE_ORC, RACE_TROLL,RACE_TAUREN,RACE_UNDEAD };

    float graveDistance = -1;

    WorldPosition botPos(bot);

    for (auto race : races)
    {
        for (uint32 cls = 0; cls < MAX_CLASSES; cls++)
        {
            PlayerInfo const* info = sObjectMgr.GetPlayerInfo(race, cls);

            if (!info)
                continue;

            NewGrave = sObjectMgr.GetClosestGraveYard(info->positionX, info->positionY, info->positionZ, info->mapId, bot->GetTeam());

            if (!NewGrave)
                continue;

            WorldPosition gravePos(NewGrave->map_id, NewGrave->x, NewGrave->y, NewGrave->z);

            float newDist = botPos.fDist(gravePos);

            if (graveDistance < 0 || newDist < graveDistance)
            {
                ClosestGrave = NewGrave;
                graveDistance = newDist;
            }
        }
    }

    return ClosestGrave;
}

bool SpiritHealerAction::Execute(Event event)
{
    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
    {
        botAI->TellError("I am not a spirit");
        return false;
    }

    uint32 dCount = AI_VALUE(uint32, "death count");
    int64 deadTime = time(nullptr) - corpse->GetGhostTime();

    WorldSafeLocsEntry const* ClosestGrave = GetGrave(dCount > 10 || deadTime > 15 * MINUTE || AI_VALUE(uint8, "durability") < 10);

    if (bot->GetDistance2d(ClosestGrave->x, ClosestGrave->y) < sPlayerbotAIConfig.sightDistance)
    {
        list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
        for (list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
        {
            Unit* unit = ai->GetUnit(*i);
            if (unit && unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
            {
                sLog.outBasic("Bot #%d %s:%d <%s> revives at spirit healer", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName());
                PlayerbotChatHandler ch(bot);
                bot->ResurrectPlayer(0.5f);
                bot->SpawnCorpseBones();
                bot->SaveToDB();
                context->GetValue<Unit*>("current target")->Set(NULL);
                bot->SetSelectionGuid(ObjectGuid());
                ai->TellMaster("Hello");

                if (dCount > 20)
                    context->GetValue<uint32>("death count")->Set(0);

                return true;
            }
        }
    }

    if (!ClosestGrave)
    {
        return false;
    }

    bool moved = false;

    if (bot->IsWithinLOS(ClosestGrave->x, ClosestGrave->y, ClosestGrave->z))
        moved = MoveNear(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, 0.0);
    else
        moved = MoveTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, false, false);

    if (moved)
        return true;

    if (!botAI->HasActivePlayerMaster())
    {
        context->GetValue<uint32>("death count")->Set(dCount + 1);
        return bot->TeleportTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, ClosestGrave->o);
    }

    LOG_INFO("playerbots", "Bot %s %s:%d <%s> can't find a spirit healer",
        bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    botAI->TellError("Cannot find any spirit healer nearby");
    return false;
}

bool SpiritHealerAction::isUseful()
{
    return bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
}
