/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericSpellActions.h"
#include "Event.h"
#include "Playerbot.h"

bool CastSpellAction::Execute(Event event)
{
    if (spell == "conjure food" || spell == "conjure water")
    {
        //uint32 id = AI_VALUE2(uint32, "spell id", spell);
        //if (!id)
        //    return false;

        uint32 castId = 0;

        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            uint32 spellId = itr->first;

            SpellInfo const* pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!pSpellInfo)
                continue;

            std::string namepart = pSpellInfo->SpellName[0];
            std::wstring wnamepart;
            if (!Utf8toWStr(namepart, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (!Utf8FitTo(spell, wnamepart))
                continue;

            if (pSpellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
                continue;

            uint32 itemId = pSpellInfo->Effects[0].ItemType;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
                continue;

            if (pSpellInfo->Id > castId)
                castId = pSpellInfo->Id;
        }

        return botAI->CastSpell(castId, bot);
    }

    return botAI->CastSpell(spell, GetTarget());
}

bool CastSpellAction::isPossible()
{
    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        bot->Dismount();
        return false;
    }

    Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return ai->CanCastSpell(spell, GetTarget(), true);
}

bool CastSpellAction::isUseful()
{
    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        bot->Dismount();
        return false;
    }

    Unit* spellTarget = GetTarget();
    if (!spellTarget)
        return false;

    if (!spellTarget->IsInWorld() || spellTarget->GetMapId() != bot->GetMapId())
        return false;

    float combatReach = bot->GetCombinedCombatReach(spellTarget, true);
    if (ai->IsRanged(bot))
        combatReach = bot->GetCombinedCombatReach(spellTarget, false);

    return spellTarget && AI_VALUE2(bool, "spell cast useful", spell) && sServerFacade.GetDistance2d(bot, spellTarget) <= (range + combatReach);
}

CastMeleeSpellAction::CastMeleeSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell)
{
    range = ATTACK_DISTANCE;
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target)
        range = max(5.0f, bot->GetCombinedCombatReach(target, true));

    // range = target->GetCombinedCombatReach();
}

bool CastAuraSpellAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && (GetTarget() != NULL) && CastSpellAction::isUseful() && !ai->HasAura(spell, GetTarget(), true);
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

CastShootAction::CastShootAction(PlayerbotAI* ai) : CastSpellAction(ai, "shoot")
{
    if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        spell = "shoot";

        switch (pItem->GetProto()->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_GUN:
                spell += " gun";
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
                spell += " bow";
                break;
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                spell += " crossbow";
                break;
        }
    }
}

NextAction** CastSpellAction::getPrerequisites()
{
    if (spell == "mount")
        return nullptr;

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

Value<Unit*>* CastCrowdControlSpellAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", getName());
}

bool CastCrowdControlSpellAction::Execute(Event event)
{
    return ai->CastSpell(getName(), GetTarget());
}

bool CastCrowdControlSpellAction::isPossible()
{
    return ai->CanCastSpell(getName(), GetTarget(), true);
}

bool CastCrowdControlSpellAction::isUseful()
{
    return true;
}

std::string const& CastProtectSpellAction::GetTargetName()
{
    return "party member to protect";
}

bool CastProtectSpellAction::isUseful()
{
    return GetTarget() && !ai->HasAura(spell, GetTarget());
}
