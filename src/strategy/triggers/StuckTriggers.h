/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STUCKTRIGGERS_H
#define _PLAYERBOT_STUCKTRIGGERS_H

#include "Trigger.h"


class MoveStuckTrigger : public Trigger
{
    public:
        MoveStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "move stuck", 5) {}

        bool IsActive() override;
};

class MoveLongStuckTrigger : public Trigger
{
    public:
        MoveLongStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "move long stuck", 5) {}

        bool IsActive() override;
};

class CombatStuckTrigger : public Trigger
{
    public:
        CombatStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "combat stuck", 5) {}

        bool IsActive() override;
};

class CombatLongStuckTrigger : public Trigger
{
    public:
        CombatLongStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "combat long stuck", 5) {}

        bool IsActive() override;
};
