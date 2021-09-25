/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STATSVALUE_H
#define _PLAYERBOT_STATSVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class HealthValue : public Uint8CalculatedValue, public Qualified
{
    public:
        HealthValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        Unit* GetTarget();
        uint8 Calculate() override;
};

class IsDeadValue : public BoolCalculatedValue, public Qualified
{
    public:
        IsDeadValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        Unit* GetTarget();
        bool Calculate() override;
};

class PetIsDeadValue : public BoolCalculatedValue
{
    public:
        PetIsDeadValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        bool Calculate() override;
};

class PetIsHappyValue : public BoolCalculatedValue
{
    public:
        PetIsHappyValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        bool Calculate() override;
};

class RageValue : public Uint8CalculatedValue, public Qualified
{
    public:
        RageValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        Unit* GetTarget();
        uint8 Calculate() override;
};

class EnergyValue : public Uint8CalculatedValue, public Qualified
{
    public:
        EnergyValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        Unit* GetTarget();
        uint8 Calculate() override;
};

class ManaValue : public Uint8CalculatedValue, public Qualified
{
    public:
        ManaValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        Unit* GetTarget();
        uint8 Calculate() override;
};

class HasManaValue : public BoolCalculatedValue, public Qualified
{
    public:
        HasManaValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        Unit* GetTarget();
        bool Calculate() override;
};

class ComboPointsValue : public Uint8CalculatedValue, public Qualified
{
    public:
        ComboPointsValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        Unit* GetTarget();
        uint8 Calculate() override;
};

class IsMountedValue : public BoolCalculatedValue, public Qualified
{
    public:
        IsMountedValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        Unit* GetTarget();
        bool Calculate() override;
};

class IsInCombatValue : public BoolCalculatedValue, public Qualified
{
    public:
        IsInCombatValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        Unit* GetTarget();
        bool Calculate() override;
};

class BagSpaceValue : public Uint8CalculatedValue
{
    public:
        BagSpaceValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        uint8 Calculate() override;
};

class DurabilityValue : public Uint8CalculatedValue
{
    public:
        DurabilityValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        uint8 Calculate() override;
};

class RepairCostValue : public Uint32CalculatedValue
{
    public:
        RepairCostValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI) { }

        uint32 Calculate() override;
};

class SpeedValue : public Uint8CalculatedValue, public Qualified
{
    public:
        SpeedValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI) { }

        Unit* GetTarget();
        uint8 Calculate() override;
};

class IsInGroupValue : public BoolCalculatedValue
{
    public:
        IsInGroupValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        bool Calculate() override;
};

class DeathCountValue : public ManualSetValue<uint32>
{
    public:
        DeathCountValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "death_count") { }
};

#endif
