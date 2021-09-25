/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PRIESTTRIGGERS_H
#define _PLAYERBOT_PRIESTTRIGGERS_H

#include "CureTriggers.h"

class PlayerbotAI;

class PowerWordFortitudeOnPartyTrigger : public BuffOnPartyTrigger
{
    public:
        PowerWordFortitudeOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "power word: fortitude", 2) { }

        bool IsActive() override;
};

class PowerWordFortitudeTrigger : public BuffTrigger
{
    public:
        PowerWordFortitudeTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "power word: fortitude", 2) { }

        bool IsActive() override;
};

class DivineSpiritOnPartyTrigger : public BuffOnPartyTrigger
{
    public:
        DivineSpiritOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "divine spirit", 2) { }

        bool IsActive() override;
};

class DivineSpiritTrigger : public BuffTrigger
{
    public:
        DivineSpiritTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "divine spirit", 2) { }

        bool IsActive() override;
};

class PrayerOfFortitudeTrigger : public BuffOnPartyTrigger
{
    public:
        PrayerOfFortitudeTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "prayer of fortitude", 2) { }

        bool IsActive() override;
};

class PrayerOfSpiritTrigger : public BuffOnPartyTrigger
{
    public:
        PrayerOfSpiritTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "prayer of spirit", 2) { }

        bool IsActive() override;
};

class InnerFireTrigger : public BuffTrigger
{
    public:
        InnerFireTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "inner fire", 2) { }

        bool IsActive() override;
};

BUFF_TRIGGER(VampiricEmbraceTrigger, "vampiric embrace", "vampiric embrace")

class PowerWordPainOnAttackerTrigger : public DebuffOnAttackerTrigger
{
    public:
        PowerWordPainOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "shadow word: pain") { }
};

DEBUFF_TRIGGER(PowerWordPainTrigger, "shadow word: pain", "shadow word: pain")
DEBUFF_TRIGGER(DevouringPlagueTrigger, "devouring plague", "devouring plague")
DEBUFF_TRIGGER(VampiricTouchTrigger, "vampiric touch", "vampiric touch")

class DispelMagicTrigger : public NeedCureTrigger
{
    public:
        DispelMagicTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "dispel magic", DISPEL_MAGIC) { }
};

class DispelMagicPartyMemberTrigger : public PartyMemberNeedCureTrigger
{
    public:
        DispelMagicPartyMemberTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "dispel magic", DISPEL_MAGIC) { }
};

class CureDiseaseTrigger : public NeedCureTrigger
{
    public:
        CureDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cure disease", DISPEL_DISEASE) { }
};

class PartyMemberCureDiseaseTrigger : public PartyMemberNeedCureTrigger
{
    public:
        PartyMemberCureDiseaseTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure disease", DISPEL_DISEASE) { }
};

class ShadowformTrigger : public BuffTrigger
{
    public:
        ShadowformTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "shadowform") { }

        bool IsActive() override;
};

class PowerInfusionTrigger : public BuffTrigger
{
    public:
        PowerInfusionTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "power infusion") { }
};

class InnerFocusTrigger : public BuffTrigger
{
    public:
        InnerFocusTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "inner focus") { }
};

class ShadowProtectionOnPartyTrigger : public BuffOnPartyTrigger
{
    public:
        ShadowProtectionOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "shadow protection", 2) { }
};

class ShadowProtectionTrigger : public BuffTrigger
{
    public:
        ShadowProtectionTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "shadow protection", 2) { }
};

class ShackleUndeadTrigger : public HasCcTargetTrigger
{
    public:
        ShackleUndeadTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "shackle undead") { }
};

#endif
