/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PASSLEADERSHIPTOMASTERACTION_H
#define _PLAYERBOT_PASSLEADERSHIPTOMASTERACTION_H

#include "Action.h"

class Event;
class PlayerbotAI;

class PassLeadershipToMasterAction : public Action
{
    public:
        PassLeadershipToMasterAction(PlayerbotAI* botAI) : Action(botAI, "leader") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

#endif
