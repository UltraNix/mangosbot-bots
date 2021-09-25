/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_INVITETOGROUPACTION_H
#define _PLAYERBOT_INVITETOGROUPACTION_H

#include "Action.h"

class Event;
class Player;
class PlayerbotAI;

class InviteToGroupAction : public Action
{
    public:
        InviteToGroupAction(PlayerbotAI* botAI, std::string const& name = "invite") : Action(botAI, name) { }

        bool Execute(Event event) override;

        virtual bool Invite(Player* player);
};

class InviteNearbyToGroupAction : public InviteToGroupAction
{
    public:
        InviteNearbyToGroupAction(PlayerbotAI* botAI, std::string const& name = "invite nearby") : InviteToGroupAction(botAI, name) { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

#endif
