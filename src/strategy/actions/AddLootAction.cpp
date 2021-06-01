/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AddLootAction.h"
#include "Event.h"
#include "LootObjectStack.h"
#include "Playerbot.h"
#include "ServerFacade.h"

bool AddLootAction::Execute(Event event)
{
    ObjectGuid guid = event.getObject();
    if (!guid)
        return false;

    return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
}

bool AddAllLootAction::Execute(Event event)
{
    bool added = false;

    GuidVector gos = context->GetValue<GuidVector>("nearest game objects")->Get();
    for (GuidVector::iterator i = gos.begin(); i != gos.end(); i++)
        added |= AddLoot(*i);

    GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    for (GuidVector::iterator i = corpses.begin(); i != corpses.end(); i++)
        added |= AddLoot(*i);

    return added;
}

bool AddLootAction::isUseful()
{
    return AI_VALUE(uint8, "bag space") < 80;
}

bool AddAllLootAction::isUseful()
{
    return AI_VALUE(uint8, "bag space") < 80;
}

bool AddAllLootAction::AddLoot(ObjectGuid guid)
{
    return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
}

bool AddGatheringLootAction::AddLoot(ObjectGuid guid)
{
    LootObject loot(bot, guid);

    WorldObject* wo = loot.GetWorldObject(bot);
    if (loot.IsEmpty() || !wo)
        return false;

    if (!bot->IsWithinLOSInMap(wo))
        return false;

    if (loot.skillId == SKILL_NONE)
        return false;

    if (!loot.IsLootPossible(bot))
        return false;

    if (sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, wo), INTERACTION_DISTANCE))
    {
        std::list<Unit*> targets;
        acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, sPlayerbotAIConfig->lootDistance);
        acore::UnitListSearcher<acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        wo->VisitNearbyObject(sPlayerbotAIConfig->spellDistance, searcher);
        if (!targets.empty())
        {
            std::ostringstream out;
            out << "Kill that " << targets.front()->GetName() << " so I can loot freely";
            botAI->TellError(out.str());
            return false;
        }
    }

    return AddAllLootAction::AddLoot(guid);
}
