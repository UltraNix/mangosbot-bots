/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DESTROYITEMACTION_H
#define _PLAYERBOT_DESTROYITEMACTION_H

#include "InventoryAction.h"

class Event;
class FindItemVisitor;
class PlayerbotAI;

class DestroyItemAction : public InventoryAction
{
    public:
        DestroyItemAction(PlayerbotAI* botAI) : InventoryAction(botAI, "destroy") { }

        bool Execute(Event event) override;

    private:
        void DestroyItem(FindItemVisitor* visitor);
};

#endif
