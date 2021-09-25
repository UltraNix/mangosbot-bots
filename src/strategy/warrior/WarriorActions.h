/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARRIORACTIONS_H
#define _PLAYERBOT_WARRIORACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

// all
class CastHeroicStrikeAction : public CastMeleeSpellAction
{
    public:
        CastHeroicStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "heroic strike") { }
};

// all
class CastCleaveAction : public CastMeleeSpellAction
{
    public:
        CastCleaveAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "cleave") { }
};

// battle, berserker
class CastMockingBlowAction : public CastMeleeSpellAction
{
    public:
        CastMockingBlowAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mocking blow") { }
};

class CastBloodthirstAction : public CastMeleeSpellAction
{
    public:
        CastBloodthirstAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "bloodthirst") { }
};

// battle, berserker
class CastExecuteAction : public CastMeleeSpellAction
{
    public:
        CastExecuteAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "execute") { }
};

// battle
class CastOverpowerAction : public CastMeleeSpellAction
{
    public:
        CastOverpowerAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "overpower") { }
};

// battle, berserker
class CastHamstringAction : public CastSnareSpellAction
{
    public:
        CastHamstringAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "hamstring") { }
};

// defensive
class CastTauntAction : public CastMeleeSpellAction
{
    public:
        CastTauntAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "taunt") { }
};

// defensive
class CastShieldBlockAction : public CastBuffSpellAction
{
    public:
        CastShieldBlockAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shield block") { }
};

// defensive
class CastShieldWallAction : public CastMeleeSpellAction
{
    public:
        CastShieldWallAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "shield wall") { }
};

class CastBloodrageAction : public CastBuffSpellAction
{
    public:
        CastBloodrageAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bloodrage") { }
};

// defensive
class CastDevastateAction : public CastMeleeSpellAction
{
    public:
        CastDevastateAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "devastate") { }
};

// all
class CastSlamAction : public CastMeleeSpellAction
{
    public:
        CastSlamAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "slam") { }
};

// all
class CastShieldSlamAction : public CastMeleeSpellAction
{
	public:
		CastShieldSlamAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "shield slam") { }
};

// after dodge
BEGIN_MELEE_SPELL_ACTION(CastRevengeAction, "revenge")
END_SPELL_ACTION()

//debuffs
BEGIN_DEBUFF_ACTION(CastRendAction, "rend")
END_SPELL_ACTION()

class CastRendOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
    public:
        CastRendOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "rend") { }
};

BEGIN_DEBUFF_ACTION(CastDisarmAction, "disarm")
END_SPELL_ACTION()

class CastSunderArmorAction : public CastDebuffSpellAction
{
    public:
        CastSunderArmorAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "sunder armor"), range(ATTACK_DISTANCE) { }

        bool isUseful() override;
};

class CastDemoralizingShoutAction : public CastDebuffSpellAction
{
    public:
        CastDemoralizingShoutAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "demoralizing shout"), range(ATTACK_DISTANCE) { }
};

BEGIN_MELEE_SPELL_ACTION(CastChallengingShoutAction, "challenging shout")
END_SPELL_ACTION()

// stuns
BEGIN_MELEE_SPELL_ACTION(CastShieldBashAction, "shield bash")
END_SPELL_ACTION()

BEGIN_MELEE_SPELL_ACTION(CastIntimidatingShoutAction, "intimidating shout")
END_SPELL_ACTION()

BEGIN_MELEE_SPELL_ACTION(CastThunderClapAction, "thunder clap")
END_SPELL_ACTION()

// buffs
class CastBattleShoutAction : public CastBuffSpellAction
{
	public:
		CastBattleShoutAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "battle shout"), range(ATTACK_DISTANCE) { }

        bool isUseful() override;
};

class CastDefensiveStanceAction : public CastBuffSpellAction
{
	public:
		CastDefensiveStanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "defensive stance") { }
};

class CastBattleStanceAction : public CastBuffSpellAction
{
	public:
		CastBattleStanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "battle stance") { }
};

class CastChargeAction : public CastReachTargetSpellAction
{
    public:
        CastChargeAction(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, "charge", 1.5f) { }
};

class CastDeathWishAction : public CastBuffSpellAction
{
	public:
		CastDeathWishAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "death wish") { }
};

class CastBerserkerRageAction : public CastBuffSpellAction
{
	public:
		CastBerserkerRageAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "berserker rage") { }
};

class CastLastStandAction : public CastBuffSpellAction
{
	public:
		CastLastStandAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "last stand") { }
};

// defensive
class CastShockwaveAction : public CastMeleeSpellAction
{
	public:
		CastShockwaveAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "shockwave") { }
};

// defensive
class CastConcussionBlowAction : public CastSnareSpellAction
{
	public:
		CastConcussionBlowAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "concussion blow") { }
};

BEGIN_MELEE_SPELL_ACTION(CastVictoryRushAction, "victory rush")
END_SPELL_ACTION()

class CastShieldBashOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
    public:
        CastShieldBashOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "shield bash") { }
};

class CastBattleShoutTauntAction : public CastMeleeSpellAction
{
    public:
	    CastBattleShoutTauntAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "battle shout") { }

        bool isUseful() override;
};

#endif
