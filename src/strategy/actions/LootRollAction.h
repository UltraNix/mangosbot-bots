/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTROLLACTION_H
#define _PLAYERBOT_LOOTROLLACTION_H

#include "QueryItemUsageAction.h"

class Event;
class PlayerbotAI;

struct ItemTemplate;

class LootRollAction : public QueryItemUsageAction
{
    public:
        LootRollAction(PlayerbotAI* botAI) : QueryItemUsageAction(botAI, "loot roll") { }

        bool Execute(Event event) override;

    private:
        RollVote CalculateRollVote(ItemTemplate const* proto);
};

#endif
