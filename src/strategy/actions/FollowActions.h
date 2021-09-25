/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FOLLOWACTIONS_H
#define _PLAYERBOT_FOLLOWACTIONS_H

#include "MovementActions.h"

class Event;
class PlayerbotAI;

class FollowAction : public MovementAction
{
	public:
		FollowAction(PlayerbotAI* botAI) : MovementAction(botAI, "follow") { }

		bool Execute(Event event) override;
        bool isUseful() override;
};

class FleeToMasterAction : public MovementAction
{
    public:
        FleeToMasterAction(PlayerbotAI* botAI) : MovementAction(botAI, "flee to master") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

#endif
