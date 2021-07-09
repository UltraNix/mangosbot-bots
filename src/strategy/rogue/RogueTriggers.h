/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericTriggers.h"

class PlayerbotAI;

class KickInterruptSpellTrigger : public InterruptSpellTrigger
{
    public:
        KickInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "kick") { }
};

class SliceAndDiceTrigger : public BuffTrigger
{
    public:
        SliceAndDiceTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "slice and dice") { }
};

class AdrenalineRushTrigger : public BuffTrigger
{
    public:
        AdrenalineRushTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "adrenaline rush") { }

        bool IsPossible() const override;
};

class RuptureTrigger : public DebuffTrigger
{
    public:
        RuptureTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "rupture") { }
};

class ExposeArmorTrigger : public DebuffTrigger
{
    public:
        ExposeArmorTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "expose armor") { }
};

class KickInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
    public:
        KickInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "kick") { }
};

class InStealthTrigger : public HasAuraTrigger
{
    public:
        InStealthTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "stealth") { }
};

class NoStealthTrigger : public HasNoAuraTrigger
{
    public:
        NoStealthTrigger(PlayerbotAI* botAI) : HasNoAuraTrigger(botAI, "stealth") { }
};

class UnstealthTrigger : public BuffTrigger
{
    public:
        UnstealthTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "stealth", 3) { }

        bool IsActive() override;
};

class StealthTrigger : public Trigger
{
    public:
        StealthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "stealth") { }

        bool IsActive() override;
};

class SapTrigger : public HasCcTargetTrigger
{
    public:
        SapTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "sap") { }

        bool IsPossible() override;
};

class SprintTrigger : public BuffTrigger
{
    public:
        SprintTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "sprint", 3) { }

        bool IsPossible() override;
        bool IsActive() override;
};
