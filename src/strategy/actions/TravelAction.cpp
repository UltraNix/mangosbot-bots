/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TravelAction.h"
#include "GridNotifiers.h"
#include "Playerbot.h"

bool TravelAction::Execute(Event event)
{
    TravelTarget * target = AI_VALUE(TravelTarget *, "travel target");

    if (bot->isMoving())
        return false;

    target->setStatus(TRAVEL_STATUS_WORK);

    Unit* newTarget = nullptr;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, sPlayerbotAIConfig->sightDistance * 2);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    bot->VisitNearbyObject(sPlayerbotAIConfig->sightDistance * 2, searcher);

    for (Unit* unit : targets)
    {
        newTarget = unit;
        if (!newTarget)
            continue;

        if (newTarget->GetMapId() != bot->GetMapId())
            continue;

        if (!newTarget->IsAlive())
            continue;

        if (!newTarget->GetEntry() != target->getDestination()->getEntry())
            continue;

        if (newTarget->IsInCombat())
            continue;

        if (bot->IsHostileTo(newTarget))
            context->GetValue<ObjectGuid>("pull target")->Set(newTarget->GetGUID());
        else
            context->GetValue<ObjectGuid>("rpg target")->Set(newTarget->GetGUID());

        break;
    }

    return true;
}

bool TravelAction::isUseful()
{
    return false && context->GetValue<TravelTarget*>("travel target")->Get()->isActive() &&
        (!context->GetValue<ObjectGuid>("rpg target")->Get() || !context->GetValue<ObjectGuid>("pull target")->Get());
}

bool MoveToDarkPortalAction::Execute(Event event)
{
    if (bot->GetGroup())
        if (bot->GetGroup()->GetLeaderGUID() != bot->GetGUID() && !bot->GetPlayerbotAI()->GetGroupMaster()->GetPlayerbotAI())
            return false;

    if (bot->getLevel() > 57)
    {
        if ((bot->GetTeamId() == TEAM_ALLIANCE && bot->GetQuestStatus(10119) == QUEST_STATUS_NONE) || (bot->GetTeamId() == TEAM_HORDE && bot->GetQuestStatus(9407) == QUEST_STATUS_NONE))
        {
            if (!bot->IsInCombat())
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(10119);
                    CreatureData const* dataPair = sRandomPlayerbotMgr->GetCreatureDataByEntry(16841);
                    if (quest && dataPair)
                    {
                        ObjectGuid npcGuid = ObjectGuid(HIGHGUID_UNIT, 16841, dataPair->first);
                        Creature* npc = bot->GetMap()->GetCreature(npcGuid);
                        bot->AddQuest(quest, npc);
                    }
                }
                else
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(9407);
                    CreatureData const* dataPair = sRandomPlayerbotMgr->GetCreatureDataByEntry(19254);
                    if (quest && dataPair)
                    {
                        ObjectGuid npcGuid = ObjectGuid(HIGHGUID_UNIT, 19254, dataPair->first);
                        Creature* npc = bot->GetMap()->GetCreature(npcGuid);
                        bot->AddQuest(quest, npc);
                    }
                }
            }

            return MoveTo(0, -11815.1f, -3190.39f, -30.7338f, false, true);
        }

        return MoveTo(0, -11906.9f, -3208.53f, -14.8616f, false, true);
    }

    return false;
}

bool MoveToDarkPortalAction::isUseful()
{
    return bot->getLevel() > 54;
}

bool DarkPortalAzerothAction::Execute(Event event)
{
    if (bot->getLevel() > 57)
    {
        WorldPacket packet(CMSG_AREATRIGGER);
        packet << 4354;

        return bot->GetPlayerbotAI()->DoSpecificAction("reach area trigger", Event("travel action", packet));
    }

    return false;
}

bool DarkPortalAzerothAction::isUseful()
{
    return bot->getLevel() > 57;
}

bool MoveFromDarkPortalAction::Execute(Event event)
{
    context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid());

    if (bot->GetTeamId() == TEAM_ALLIANCE)
        return MoveTo(530, -319.261f, 1027.213, 54.172638f, false, true);
    else
        return MoveTo(530, -180.444f, 1027.947, 54.181538f, false, true);

    return false;
}
