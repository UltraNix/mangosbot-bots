/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AvailableLootValue.h"
#include "LootObjectStack.h"
#include "Playerbot.h"

AvailableLootValue::AvailableLootValue(PlayerbotAI* botAI, string name) : ManualSetValue<LootObjectStack*>(ai, NULL, name)
{
    value = new LootObjectStack(botAI->GetBot());
}

~AvailableLootValue::AvailableLootValue()
{
    delete value;
}

LootTargetValue::LootTargetValue(PlayerbotAI* botAI, string name) : ManualSetValue<LootObject>(ai, LootObject(), name)
{
}

bool CanLootValue::Calculate()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    return !loot.IsEmpty() && loot.GetWorldObject(bot) && loot.IsLootPossible(bot) &&
        sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), INTERACTION_DISTANCE);
}
