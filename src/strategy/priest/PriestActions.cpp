/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestActions.h"
#include "../Event.h"

NextAction** CastAbolishDiseaseAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("cure disease"), nullptr), CastSpellAction::getAlternatives());
}

NextAction** CastAbolishDiseaseOnPartyAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("cure disease on party"), nullptr), CastSpellAction::getAlternatives());
}

bool CastRemoveShadowformAction::isUseful() const
{
    return botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target"));
}

bool CastRemoveShadowformAction::isPossible()
{
    return true;
}

bool CastRemoveShadowformAction::Execute(Event event)
{
    botAI->RemoveAura("shadowform");
    return true;
}

Value<Unit*>* CastShackleUndeadAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", getName());
}
