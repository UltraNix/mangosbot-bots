/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Item.h"
#include "../Action.h"

class Event;
class PlayerbotAI;

class InventoryChangeFailureAction : public Action
{
    public:
        InventoryChangeFailureAction(PlayerbotAI* botAI) : Action(botAI, "inventory change failure") { }

        bool Execute(Event event) override;

    private:
        static std::map<InventoryResult, std::string> messages;
};
