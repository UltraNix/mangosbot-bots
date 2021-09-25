/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SEESPELLACTION_H
#define _PLAYERBOT_SEESPELLACTION_H

#include "MovementActions.h"

class Creature;
class Event;
class PlayerbotAI;

class SeeSpellAction : public MovementAction
{
    public:
        SeeSpellAction(PlayerbotAI* botAI) : MovementAction(botAI, "see spell") { }

        bool Execute(Event event) override;

    private:
        Creature* CreateWps(Player* wpOwner, float x, float y, float z, float o, uint32 entry, Creature* lastWp, bool important = false);
};

#endif
