/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InventoryAction.h"

class Event;
class PlayerbotAI;

class TellItemCountAction : public InventoryAction
{
    public:
        TellItemCountAction(PlayerbotAI* botAI) : InventoryAction(botAI, "c") { }

        bool Execute(Event event) override;
};
