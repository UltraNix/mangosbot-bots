/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReachTargetActions.h"
#include "GenericSpellActions.h"

class PlayerbotAI;

class CastFeralChargeCatAction : public CastReachTargetSpellAction
{
	public:
		CastFeralChargeCatAction(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, "feral charge - cat", 1.5f) { }
};

class CastCowerAction : public CastBuffSpellAction
{
	public:
		CastCowerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cower") { }
};

class CastBerserkAction : public CastBuffSpellAction
{
	public:
		CastBerserkAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "berserk") { }
};

class CastTigersFuryAction : public CastBuffSpellAction
{
	public:
		CastTigersFuryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "tiger's fury") { }
};

class CastRakeAction : public CastDebuffSpellAction
{
	public:
        CastRakeAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "rake") { }
};

class CastClawAction : public CastMeleeSpellAction
{
	public:
		CastClawAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "claw") { }
};

class CastMangleCatAction : public CastMeleeSpellAction
{
    public:
		CastMangleCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mangle (cat)") { }
};

class CastSwipeCatAction : public CastMeleeSpellAction
{
	public:
		CastSwipeCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "swipe (cat)") { }
};

class CastFerociousBiteAction : public CastMeleeSpellAction
{
	public:
		CastFerociousBiteAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "ferocious bite") { }
};

class CastRipAction : public CastMeleeSpellAction
{
	public:
		CastRipAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "rip") { }
};
