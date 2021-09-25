/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TARGETVALUE_H
#define _PLAYERBOT_TARGETVALUE_H

#include "Unit.h"
#include "Value.h"

class PlayerbotAI;
class ThreatManager;
class TravelTarget;

class FindTargetStrategy
{
    public:
        FindTargetStrategy(PlayerbotAI* botAI) : result(nullptr), botAI(botAI) { }

        Unit* GetResult();
        virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) = 0;
        void GetPlayerCount(Unit* creature, uint32* tankCount, uint32* dpsCount);

    protected:
        Unit* result;
        PlayerbotAI* botAI;
        std::map<Unit*, uint32> tankCountCache;
        std::map<Unit*, uint32> dpsCountCache;
};

class FindNonCcTargetStrategy : public FindTargetStrategy
{
    public:
        FindNonCcTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) { }

    protected:
        virtual bool IsCcTarget(Unit* attacker);
};

class TargetValue : public UnitCalculatedValue
{
	public:
        TargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI) { }

    protected:
        Unit* FindTarget(FindTargetStrategy* strategy);
};

class RpgTargetValue : public ManualSetValue<ObjectGuid>
{
    public:
        RpgTargetValue(PlayerbotAI* botAI) : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty) { }
};

class TravelTargetValue : public ManualSetValue<TravelTarget*>
{
    public:
        TravelTargetValue(PlayerbotAI* botAI) : ManualSetValue<TravelTarget*>(botAI, new TravelTarget(botAI)) { }

        virtual ~TravelTargetValue() { delete value; }
};

class LastLongMoveValue : public CalculatedValue<WorldPosition>
{
    public:
        LastLongMoveValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "last long move", 30) { }

        WorldPosition Calculate() override;
};

class HomeBindValue : public CalculatedValue<WorldPosition>
{
    public:
        HomeBindValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "home bind", 30) { }

        WorldPosition Calculate() override;
};

class IgnoreRpgTargetValue : public ManualSetValue<GuidSet&>
{
    public:
        IgnoreRpgTargetValue(PlayerbotAI* botAI) : ManualSetValue<GuidSet&>(botAI, data, "ignore rpg targets") { }

    private:
        GuidSet data;
};

class TalkTargetValue : public ManualSetValue<ObjectGuid>
{
    public:
        TalkTargetValue(PlayerbotAI* botAI) : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty) { }
};

class PullTargetValue : public ManualSetValue<ObjectGuid>
{
    public:
        PullTargetValue(PlayerbotAI* botAI) : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty) { }
};

#endif
