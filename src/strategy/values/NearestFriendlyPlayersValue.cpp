/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NearestFriendlyPlayersValue.h"
#include "Playerbot.h"

void NearestFriendlyPlayersValue::FindUnits(std::list<Unit*> &targets)
{
    acore::AnyFriendlyUnitInObjectRangeCheck u_check(bot, range);
    acore::UnitListSearcher<acore::AnyFriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    bot->VisitNearbyObject(range, searcher);
}

bool NearestFriendlyPlayersValue::AcceptUnit(Unit* unit)
{
    ObjectGuid guid = unit->GetGUID();
    return guid.IsPlayer() && guid != botAI->GetBot()->GetGUID();
}
