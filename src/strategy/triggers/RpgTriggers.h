/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGTRIGGERS_H
#define _PLAYERBOT_RPGTRIGGERS_H

#include "Trigger.h"

class Event;
class GuidPosition;
class PlayerbotAI;

class NoRpgTargetTrigger : public Trigger
{
    public:
        NoRpgTargetTrigger(PlayerbotAI* ai, string name = "no rpg target", int checkInterval = 1) : Trigger(ai, name, checkInterval) {}

        bool IsActive() override;
};

class HasRpgTargetTrigger : public NoRpgTargetTrigger
{
    public:
        HasRpgTargetTrigger(PlayerbotAI* ai, string name = "has rpg target", int checkInterval = 1) : NoRpgTargetTrigger(ai, name, checkInterval) {}

        bool IsActive() override;
};

class FarFromRpgTargetTrigger : public NoRpgTargetTrigger
{
    public:
        FarFromRpgTargetTrigger(PlayerbotAI* ai, string name = "far from rpg target", int checkInterval = 1) : NoRpgTargetTrigger(ai, name, checkInterval) {}

        bool IsActive() override;
};

class NearRpgTargetTrigger : public FarFromRpgTargetTrigger
{
    public:
        NearRpgTargetTrigger(PlayerbotAI* ai, string name = "near rpg target", int checkInterval = 1) : FarFromRpgTargetTrigger(ai, name, checkInterval) {}

        bool IsActive() override;
};

//Sub actions:
class RpgTrigger : public FarFromRpgTargetTrigger
{
    public:
        RpgTrigger(PlayerbotAI* ai, string name = "sub rpg", int checkInterval = 2) : FarFromRpgTargetTrigger(ai, name, checkInterval) {}

        GuidPosition getGuidP() { return AI_VALUE(GuidPosition, "rpg target"); }

        bool IsActive() override;
        Event Check() override;
};

class RpgTaxiTrigger : public RpgTrigger
{
    public:
        RpgTaxiTrigger(PlayerbotAI* ai, string name = "rpg taxi") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgDiscoverTrigger : public RpgTrigger
{
    public:
        RpgDiscoverTrigger(PlayerbotAI* ai, string name = "rpg discover") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgStartQuestTrigger : public RpgTrigger
{
    public:
        RpgStartQuestTrigger(PlayerbotAI* ai, string name = "rpg start quest") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgEndQuestTrigger : public RpgTrigger
{
    public:
        RpgEndQuestTrigger(PlayerbotAI* ai, string name = "rpg end quest") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgBuyTrigger : public RpgTrigger
{
    public:
        RpgBuyTrigger(PlayerbotAI* ai, string name = "rpg buy") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgSellTrigger : public RpgTrigger
{
    public:
        RpgSellTrigger(PlayerbotAI* ai, string name = "rpg sell") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgRepairTrigger : public RpgTrigger
{
    public:
        RpgRepairTrigger(PlayerbotAI* ai, string name = "rpg repair") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgTrainTrigger : public RpgTrigger
{
    public:
        RpgTrainTrigger(PlayerbotAI* ai, string name = "rpg train") : RpgTrigger(ai, name) {}

        static bool IsTrainerOf(CreatureInfo const* cInfo, Player* pPlayer);

        bool IsActive() override;
};

class RpgHealTrigger : public RpgTrigger
{
    public:
        RpgHealTrigger(PlayerbotAI* ai, string name = "rpg heal") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgHomeBindTrigger : public RpgTrigger
{
    public:
        RpgHomeBindTrigger(PlayerbotAI* ai, string name = "rpg home bind") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgQueueBGTrigger : public RpgTrigger
{
    public:
        RpgQueueBGTrigger(PlayerbotAI* ai, string name = "rpg queue bg") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgBuyPetitionTrigger : public RpgTrigger
{
    public:
        RpgBuyPetitionTrigger(PlayerbotAI* ai, string name = "rpg buy petition") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgUseTrigger : public RpgTrigger
{
    public:
        RpgUseTrigger(PlayerbotAI* ai, string name = "rpg use") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgSpellTrigger : public RpgTrigger
{
    public:
        RpgSpellTrigger(PlayerbotAI* ai, string name = "rpg spell") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgCraftTrigger : public RpgTrigger
{
    public:
        RpgCraftTrigger(PlayerbotAI* ai, string name = "rpg craft") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgTradeUsefulTrigger : public RpgTrigger
{
    public:
        RpgTradeUsefulTrigger(PlayerbotAI* ai, string name = "rpg trade useful") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

class RpgDuelTrigger : public RpgTrigger
{
    public:
        RpgDuelTrigger(PlayerbotAI* ai, string name = "rpg duel") : RpgTrigger(ai, name) {}

        bool IsActive() override;
};

#endif
