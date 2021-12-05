/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LfgTriggers.h"
#include "Playerbot.h"

bool LfgProposalActiveTrigger::IsActive()
{
    return AI_VALUE(uint32, "lfg proposal");
}

bool UnknownDungeonTrigger::IsActive()
{
    return ai->HasActivePlayerMaster() && ai->GetMaster() && ai->GetMaster()->IsInWorld() && ai->GetMaster()->GetMap()->IsDungeon() && bot->GetMapId() == ai->GetMaster()->GetMapId();
}
