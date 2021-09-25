/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARRIORTRIGGERS_H
#define _PLAYERBOT_WARRIORTRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;

BUFF_TRIGGER(BattleShoutTrigger, "battle shout", "battle shout")
BUFF_TRIGGER(BattleStanceTrigger, "battle stance", "battle stance")
BUFF_TRIGGER(DefensiveStanceTrigger, "defensive stance", "defensive stance")
BUFF_TRIGGER(ShieldBlockTrigger, "shield block", "shield block")

DEBUFF_TRIGGER(RendDebuffTrigger, "rend", "rend")
DEBUFF_TRIGGER(DisarmDebuffTrigger, "disarm", "disarm")
DEBUFF_TRIGGER(SunderArmorDebuffTrigger, "sunder armor", "sunder armor")

class RendDebuffOnAttackerTrigger : public DebuffOnAttackerTrigger
{
    public:
        RendDebuffOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "rend") { }
};

class RevengeAvailableTrigger : public SpellCanBeCastTrigger
{
	public:
		RevengeAvailableTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "revenge") { }
};

class BloodrageDebuffTrigger : public DebuffTrigger
{
    public:
        BloodrageDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "bloodrage") { }

        bool IsActive() override;
};

class ShieldBashInterruptSpellTrigger : public InterruptSpellTrigger
{
    public:
        ShieldBashInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "shield bash") { }
};

class VictoryRushTrigger : public HasAuraTrigger
{
    public:
        VictoryRushTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "victory rush") { }
};

class SwordAndBoardTrigger : public HasAuraTrigger
{
    public:
        SwordAndBoardTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "sword and board") { }
};

class ConcussionBlowTrigger : public SnareTargetTrigger
{
    public:
        ConcussionBlowTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "concussion blow") { }
};

class HamstringTrigger : public SnareTargetTrigger
{
    public:
        HamstringTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "hamstring") { }
};

class DeathWishTrigger : public BoostTrigger
{
    public:
        DeathWishTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "death wish") { }
};

class ShieldBashInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
    public:
        ShieldBashInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "shield bash") { }
};

#endif
