/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PRIESTACTIONS_H
#define _PLAYERBOT_PRIESTACTIONS_H

#include "GenericSpellActions.h"
#include "Playerbot.h"

class Event;
class PlayerbotAI;

class CastGreaterHealAction : public CastHealingSpellAction
{
    public:
        CastGreaterHealAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "greater heal") { }
};

class CastGreaterHealOnPartyAction : public HealPartyMemberAction
{
    public:
        CastGreaterHealOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "greater heal") { }

        std::string const& getName() override { return "greater heal on party"; }
};

class CastLesserHealAction : public CastHealingSpellAction
{
    public:
        CastLesserHealAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lesser heal") { }
};

class CastLesserHealOnPartyAction : public HealPartyMemberAction
{
    public:
        CastLesserHealOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "lesser heal") { }

        std::string const& getName() override { return "lesser heal on party"; }
};

class CastFlashHealAction : public CastHealingSpellAction
{
    public:
        CastFlashHealAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "flash heal") { }
};

class CastFlashHealOnPartyAction : public HealPartyMemberAction
{
    public:
        CastFlashHealOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "flash heal") { }

        std::string const& getName() override { return "flash heal on party"; }
};

class CastHealAction : public CastHealingSpellAction
{
    public:
        CastHealAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "heal") { }
};

class CastHealOnPartyAction : public HealPartyMemberAction
{
    public:
        CastHealOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "heal") { }

        std::string const& getName() override { return "heal on party"; }
};

class CastRenewAction : public CastHealingSpellAction
{
    public:
        CastRenewAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "renew") { }
};

class CastRenewOnPartyAction : public HealPartyMemberAction
{
    public:
        CastRenewOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "renew") { }

        std::string const& getName() override { return "renew on party"; }
};

class CastFadeAction : public CastBuffSpellAction
{
    public:
        CastFadeAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fade") { }
};

class CastShadowformAction : public CastBuffSpellAction
{
    public:
        CastShadowformAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shadowform") { }
};

class CastRemoveShadowformAction : public Action
{
    public:
        CastRemoveShadowformAction(PlayerbotAI* botAI) : Action(botAI, "remove shadowform") { }

        bool isUseful() override;
        virtual bool isPossible();
        bool Execute(Event event) override;
};

class CastVampiricEmbraceAction : public CastBuffSpellAction
{
	public:
		CastVampiricEmbraceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "vampiric embrace") { }
};

class CastPowerWordShieldAction : public CastBuffSpellAction
{
	public:
		CastPowerWordShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "power word: shield") { }
};

class CastPowerWordShieldOnPartyAction : public HealPartyMemberAction
{
    public:
        CastPowerWordShieldOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "power word: shield") { }

        std::string const& getName() override { return "power word: shield on party"; }
};

class CastPowerWordFortitudeAction : public CastBuffSpellAction
{
	public:
		CastPowerWordFortitudeAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "power word: fortitude") { }
};

class CastDivineSpiritAction : public CastBuffSpellAction
{
	public:
		CastDivineSpiritAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine spirit") { }
};

class CastInnerFireAction : public CastBuffSpellAction
{
	public:
		CastInnerFireAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "inner fire") { }
};

BEGIN_SPELL_ACTION(CastHolyNovaAction, "holy nova")
bool isUseful() const override
{
    return !botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target"));
}
END_SPELL_ACTION()

BEGIN_RANGED_SPELL_ACTION(CastHolyFireAction, "holy fire")
bool isUseful() const override
{
    return !botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target"));
}
END_SPELL_ACTION()

BEGIN_RANGED_SPELL_ACTION(CastSmiteAction, "smite")
bool isUseful() const override
{
	return !botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target"));
}
END_SPELL_ACTION()

class CastPowerWordFortitudeOnPartyAction : public BuffOnPartyAction
{
	public:
		CastPowerWordFortitudeOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "power word: fortitude") { }
};

class CastDivineSpiritOnPartyAction : public BuffOnPartyAction
{
	public:
		CastDivineSpiritOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "divine spirit") { }
};

class CastPrayerOfFortitudeOnPartyAction : public BuffOnPartyAction
{
    public:
        CastPrayerOfFortitudeOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "prayer of fortitude") { }
};

class CastPrayerOfSpiritOnPartyAction : public BuffOnPartyAction
{
    public:
        CastPrayerOfSpiritOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "prayer of spirit") { }
};

class CastPowerWordPainAction : public CastDebuffSpellAction
{
    public:
	    CastPowerWordPainAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "shadow word: pain") { }
};

class CastPowerWordPainOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
    public:
	    CastPowerWordPainOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "shadow word: pain") { }
};

BEGIN_DEBUFF_ACTION(CastDevouringPlagueAction, "devouring plague")
END_SPELL_ACTION()

BEGIN_DEBUFF_ACTION(CastVampiricTouchAction, "vampiric touch")
END_SPELL_ACTION()

BEGIN_RANGED_SPELL_ACTION(CastMindBlastAction, "mind blast")
END_SPELL_ACTION()

BEGIN_RANGED_SPELL_ACTION(CastMindFlayAction, "mind flay")
END_SPELL_ACTION()

class CastCureDiseaseAction : public CastCureSpellAction
{
	public:
		CastCureDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cure disease") { }
};

class CastCureDiseaseOnPartyAction : public CurePartyMemberAction
{
    public:
        CastCureDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cure disease", DISPEL_DISEASE) { }

        std::string const& getName() override { return "cure disease on party"; }
};

class CastAbolishDiseaseAction : public CastCureSpellAction
{
	public:
		CastAbolishDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "abolish disease") { }

		NextAction** getAlternatives() override;
};

class CastAbolishDiseaseOnPartyAction : public CurePartyMemberAction
{
    public:
        CastAbolishDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "abolish disease", DISPEL_DISEASE) { }

        std::string const& getName() override { return "abolish disease on party"; }
        NextAction** getAlternatives() override;
};

class CastDispelMagicAction : public CastCureSpellAction
{
	public:
		CastDispelMagicAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "dispel magic") { }
};

class CastDispelMagicOnTargetAction : public CastSpellAction
{
    public:
        CastDispelMagicOnTargetAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dispel magic") { }
};

class CastDispelMagicOnPartyAction : public CurePartyMemberAction
{
    public:
        CastDispelMagicOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "dispel magic", DISPEL_MAGIC) { }

        std::string const& getName() override { return "dispel magic on party"; }
};

class CastResurrectionAction : public ResurrectPartyMemberAction
{
	public:
		CastResurrectionAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "resurrection") { }
};

class CastCircleOfHealingAction : public CastAoeHealSpellAction
{
	public:
		CastCircleOfHealingAction(PlayerbotAI* botAI) : CastAoeHealSpellAction(botAI, "circle of healing") { }
};

class CastPsychicScreamAction : public CastSpellAction
{
	public:
	    CastPsychicScreamAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "psychic scream") { }
};

class CastDispersionAction : public CastSpellAction
{
	public:
	    CastDispersionAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dispersion") { }

	    std::string const& GetTargetName() override { return "self target"; }
};

class CastInnerFocusAction : public CastBuffSpellAction
{
    public:
	    CastInnerFocusAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "inner focus") { }
};

class CastPowerInfusionAction : public CastBuffSpellAction
{
    public:
        CastPowerInfusionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "power infusion") { }
};

class CastShadowProtectionAction : public CastBuffSpellAction
{
    public:
        CastShadowProtectionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shadow protection") { }
};

class CastShadowProtectionOnPartyAction : public BuffOnPartyAction
{
    public:
        CastShadowProtectionOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "shadow protection") { }
};

class CastShackleUndeadAction : public CastBuffSpellAction
{
    public:
        CastShackleUndeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shackle undead") { }

        Value<Unit*>* GetTargetValue() override;
};

#endif
