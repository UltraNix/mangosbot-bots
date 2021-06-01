/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UseFoodStrategy.h"
#include "Playerbot.h"

void UseFoodStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("food", 2.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("drink", 2.0f), nullptr)));
}
