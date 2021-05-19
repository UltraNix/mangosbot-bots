/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GreetAction.h"
#include "../Event.h"
#include "../../Playerbot.h"

GreetAction::GreetAction(PlayerbotAI* botAI) : Action(botAI, "greet")
{
}

bool GreetAction::Execute(Event event)
{
    ObjectGuid guid = AI_VALUE(ObjectGuid, "new player nearby");
    if (!guid || !guid.IsPlayer())
        return false;

    Player* player = dynamic_cast<Player*>(botAI->GetUnit(guid));
    if (!player)
        return false;

    if (!bot->IsInFront(player, sPlayerbotAIConfig->sightDistance, CAST_ANGLE_IN_FRONT))
        bot->SetFacingTo(player);

    ObjectGuid oldSel = bot->GetTarget();
    bot->SetTarget(guid);
    bot->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
    botAI->PlaySound(TEXT_EMOTE_HELLO);
    bot->SetTarget(oldSel);

    GuidSet& alreadySeenPlayers = botAI->GetAiObjectContext()->GetValue<GuidSet&>("already seen players")->Get();
    alreadySeenPlayers.insert(guid);

    GuidVector nearestPlayers = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (ObjectGuid const guid : nearestPlayers)
    {
        alreadySeenPlayers.insert(guid);
    }

    return true;
}
