/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NearestNpcsValue.h"
#include "GridNotifiers.h"

void NearestNpcsValue::FindUnits(std::list<Unit*> &targets)
{
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    bot->VisiNearbyObject(range, searcher);
}

bool NearestNpcsValue::AcceptUnit(Unit* unit)
{
    return !unit->IsHostileTo(bot) && !unit->IsPlayer();
}
