/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDTRIGGER_H
#define _PLAYERBOT_GUILDTRIGGER_H

#include "Trigger.h"

class PlayerbotAI;

class PetitionTurnInTrigger : public Trigger
{
    public:
        PetitionTurnInTrigger(PlayerbotAI* ai) : Trigger(ai) {}

        bool IsActive() override;
};

class BuyTabardTrigger : public Trigger
{
    public:
        BuyTabardTrigger(PlayerbotAI* ai) : Trigger(ai) {}

        bool IsActive() override;
};

class LeaveLargeGuildTrigger : public Trigger
{
    public:
        LeaveLargeGuildTrigger(PlayerbotAI* ai) : Trigger(ai) {}

        bool IsActive();
};

#endif
