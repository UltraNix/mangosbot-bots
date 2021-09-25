/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SeeSpellAction.h"
#include "Event.h"
#include "Formations.h"
#include "PathGenerator.h"
#include "Playerbot.h"
#include "TravelMgr.h"

Creature* SeeSpellAction::CreateWps(Player* wpOwner, float x, float y, float z, float o, uint32 entry, Creature* lastWp, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;

    if (!important)
        delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, delay);
    if (!important)
        wpCreature->SetObjectScale(0.2f);

    return wpCreature;
}

bool SeeSpellAction::Execute(Event event)
{
    WorldPacket p(event.getPacket()); //
    uint32 spellId;
    uint8  castCount, castFlags;

    p.rpos(0);
    p >> castCount >> spellId >> castFlags;

    //botAI->TellMaster(std::to_string(spellId));

    if (spellId != 30758)
        return false;

    SpellCastTargets targets;
    targets.Read(p, botAI->GetMaster());

    Position spellPosition = targets.GetDstPos()->GetPosition();

    if (bot->GetDistance(spellPosition.GetPositionX(), spellPosition.GetPositionY(), spellPosition.GetPositionZ()) <= 10)
    {
        botAI->TellMaster("In range!");
    }
    else
    {
        botAI->TellMaster("Out of range!");
    }

    Player* master = botAI->GetMaster();

    float x = spellPosition.GetPositionX();
    float y = spellPosition.GetPositionY();
    float z = spellPosition.GetPositionZ();

    Formation* formation = AI_VALUE(Formation*, "formation");
    WorldLocation formationLocation = formation->GetLocation();
    if (formationLocation.GetPositionX() != 0 || formationLocation.GetPositionY() != 0)
    {
        x = x - master->GetPositionX() + formationLocation.GetPositionX();
        y = y - master->GetPositionY() + formationLocation.GetPositionY();
        z = z - master->GetPositionZ() + formationLocation.GetPositionZ();
    }

    if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    {
        PathGenerator path(bot);
        path.CalculatePath(x, y, z, false);

        G3D::Vector3 end = path.GetEndPosition();
        G3D::Vector3 aend = path.GetActualEndPosition();

        Movement::PointsArray const& points = path.GetPath();
        PathType type = path.GetPathType();

        std::ostringstream out;
        out << x << ";" << y << ";" << z << " =";
        out << "path is: ";
        out << type;
        out << " of length ";
        out << points.size();
        out << " with offset ";
        out << (end - aend).length();

        Creature* lastWp = nullptr;
        for (auto i : points)
        {
            lastWp = CreateWps(bot, i.x, i.y, i.z, 0.f, 11144, lastWp);
        }

        botAI->TellMaster(out);

        /*
        PathGenerator path(bot);

        std::ostringstream out;

        out << " area = ";

        out << path.getArea(bot->GetMapId(), x, y, z);

        unsigned short flags = path.getFlags(bot->GetMapId(), x, y, z);

        if (flags & NAV_GROUND)
            out << ", ground";
        if (flags & NAV_MAGMA)
            out << ", magma";
        if (flags & NAV_SLIME)
            out << ", slime";
        if (flags & NAV_WATER)
            out << ", water";
        if (flags & NAV_SLOPE)
            out << ", slope";
        if (flags & NAV_UNUSED1)
            out << ", unused1";
        if (flags & NAV_UNUSED2)
            out << ", unsued2";
        if (flags & NAV_UNUSED3)
            out << ", unsued3";
        if (flags & NAV_UNUSED4)
            out << ", unused4";

        botAI->TellMaster(out);
        */

        /*
        WorldPosition pos = WorldPosition(bot->GetMapId(), x, y, z, 0);

        sTravelNodeMap->m_nMapMtx.lock();
        TravelNode* node = sTravelNodeMap->getNode(&pos,nullptr, 20);

        if (!node)
        {
            node = sTravelNodeMap->addNode(&pos,"User Node", false, true, false);

            if (node)
            {
                botAI->TellMaster("node added");
            }
        }
        else
        {
            if (!node->isImportant())
            {
                sTravelNodeMap->removeNode(node);
                botAI->TellMaster("node removed");
            }
            else
            {
                std::ostringstream out;
                out << "node found" << node->getName();
                botAI->TellMaster(out);
            }
        }

        sTravelNodeMap->m_nMapMtx.unlock();
        */
    }


    if (bot->IsWithinLOS(x, y, z))
        return MoveNear(bot->GetMapId(), x, y, z, 0);
    else
        return MoveTo(bot->GetMapId(), x, y, z, false, false);
}
