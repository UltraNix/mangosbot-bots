/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InventoryAction.h"

class Event;
class PlayerbotAI;
class Object;

class RewardAction : public InventoryAction
{
    public:
        RewardAction(PlayerbotAI* botAI) : InventoryAction(botAI, "reward") { }

        bool Execute(Event event) override;

    private:
        bool Reward(uint32 itemId, Object* pNpc);
};
