/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DEBUGACTION_H
#define _PLAYERBOT_DEBUGACTION_H

#include "Action.h"

class Event;
class PlayerbotAI;

class DebugAction : public Action
{
    public:
        DebugAction(PlayerbotAI* botAI) : Action(botAI, "Debug") { }

        bool Execute(Event event) override;
};

#endif
