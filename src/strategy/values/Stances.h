/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Formations.h"
#include "Action.h"
#include "Value.h"

class Event;
class PlayerbotAI;
class Unit;
class WorldLocation;

class Stance : public Formation
{
    public:
        Stance(PlayerbotAI* botAI, std::string const& name) : Formation (botAI, name) { }

        WorldLocation GetLocation() override;
        std::string const& GetTargetName() override;
        float GetMaxDistance() override;

    protected:
        virtual Unit* GetTarget();
        virtual WorldLocation GetLocationInternal() = 0;
        virtual WorldLocation GetNearLocation(float angle, float distance);
};

class MoveStance : public Stance
{
    public:
        MoveStance(PlayerbotAI* botAI, std::string const& name) : Stance(botAI, name) { }

    protected:
        WorldLocation GetLocationInternal();
        virtual float GetAngle() = 0;
};

class StanceValue : public ManualSetValue<Stance*>
{
	public:
        StanceValue(PlayerbotAI* botAI);
        ~StanceValue();

        std::string const& Save() override;
        bool Load(std::string const& value) override;
};

class SetStanceAction : public Action
{
    public:
        SetStanceAction(PlayerbotAI* botAI) : Action(botAI, "std::set Stance") { }

        bool Execute(Event event) override;
};
