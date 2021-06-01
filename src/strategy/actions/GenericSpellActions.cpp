/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericSpellActions.h"
#include "Event.h"
#include "Playerbot.h"

bool CastSpellAction::Execute(Event event)
{
	return botAI->CastSpell(spell, GetTarget());
}

bool CastSpellAction::isPossible()
{
	return botAI->CanCastSpell(spell, GetTarget());
}

bool CastSpellAction::isUseful()
{
    return GetTarget() && AI_VALUE2(bool, "spell cast useful", spell) && AI_VALUE2(float, "distance", GetTargetName()) <= range;
}

bool CastAuraSpellAction::isUseful()
{
	return CastSpellAction::isUseful() && !botAI->HasAura(spell, GetTarget(), true);
}

bool CastEnchantItemAction::isPossible()
{
    if (!CastSpellAction::isPossible())
        return false;

    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && AI_VALUE2(Item*, "item for spell", spellId);
}

bool CastHealingSpellAction::isUseful()
{
	return CastAuraSpellAction::isUseful();
}

bool CastAoeHealSpellAction::isUseful()
{
	return CastSpellAction::isUseful();
}


Value<Unit*>* CurePartyMemberAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member to dispel", dispelType);
}

Value<Unit*>* BuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

NextAction** CastSpellAction::getPrerequisites()
{
    if (range > botAI->GetRange("spell"))
        return nullptr;
    else if (range > ATTACK_DISTANCE)
        return NextAction::merge(NextAction::array(0, new NextAction("reach spell"), nullptr), Action::getPrerequisites());
    else
        return NextAction::merge(NextAction::array(0, new NextAction("reach melee"), nullptr), Action::getPrerequisites());
}

Value<Unit*>* CastDebuffSpellOnAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("attacker without aura", spell);
}

Value<Unit*>* CastSpellOnEnemyHealerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

Value<Unit*>* CastSnareSpellAction::GetTargetValue()
{
    return context->GetValue<Unit*>("snare target", spell);
}
