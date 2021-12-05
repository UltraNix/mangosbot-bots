/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomBotUpdateAction.h"
#include "Event.h"
#include "Playerbot.h"

bool RandomBotUpdateAction::Execute(Event event)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if (bot->GetGroup() && ai->GetGroupMaster() && (!ai->GetGroupMaster()->GetPlayerbotAI() || ai->GetGroupMaster()->GetPlayerbotAI()->IsRealPlayer()))
        return true;

    if (botAI->HasPlayerNearby(sPlayerbotAIConfig->grindDistance))
        return true;

    return sRandomPlayerbotMgr->ProcessBot(bot);
}

bool RandomBotUpdateAction::isUseful()
{
    return AI_VALUE(bool, "random bot update");
}
