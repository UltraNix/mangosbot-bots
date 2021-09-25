/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SetHomeAction.h"
#include "Event.h"
#include "Playerbot.h"

bool SetHomeAction::Execute(Event event)
{
    Player* master = GetMaster();

    ObjectGuid selection = bot->GetTarget();
    if (AI_VALUE(ObjectGuid, "rpg target") != bot->GetTarget())
        if (master)
            selection = master->GetTarget();
        else
            return false;

    if (Unit* unit = botAI->GetUnit(selection))
        if (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER))
        {
            float angle = GetFollowAngle();
            float x = unit->GetPositionX() + sPlayerbotAIConfig->followDistance * cos(angle);
            float y = unit->GetPositionY() + sPlayerbotAIConfig->followDistance * sin(angle);
            float z = unit->GetPositionZ();

            WorldLocation loc(unit->GetMapId(), x, y, z);
            bot->SetHomebind(loc, unit->GetAreaId());

            botAI->TellMaster("This inn is my new home");
            return true;
        }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const guid : npcs)
    {
        Creature* unit = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_INNKEEPER);
        if (!unit)
            continue;

        bot->GetSession()->SendBindPoint(unit);
        botAI->TellMaster("This inn is my new home");
        return true;
    }

    botAI->TellError("Can't find any innkeeper around");
    return false;
}
