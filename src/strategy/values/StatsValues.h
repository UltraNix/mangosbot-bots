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
        HealthValue(PlayerbotAI* botAI, string name = "health") : Uint8CalculatedValue(ai, name) {}

        Unit* GetTarget() override;
        uint8 Calculate() override;
};

class IsDeadValue : public BoolCalculatedValue, public Qualified
{
    public:
        IsDeadValue(PlayerbotAI* botAI, string name = "dead") : BoolCalculatedValue(ai, name) {}

        Unit* GetTarget()override;
        bool Calculate() override;
};

class PetIsDeadValue : public BoolCalculatedValue
{
    public:
        PetIsDeadValue(PlayerbotAI* botAI, string name = "pet dead") : BoolCalculatedValue(ai, name) {}

        bool Calculate() override;
};

class PetIsHappyValue : public BoolCalculatedValue
{
    public:
        PetIsHappyValue(PlayerbotAI* botAI, string name = "pet happy") : BoolCalculatedValue(ai, name) {}

        bool Calculate() override;
};

class RageValue : public Uint8CalculatedValue, public Qualified
{
    public:
        RageValue(PlayerbotAI* botAI, string name = "rage") : Uint8CalculatedValue(ai, name) {}

        Unit* GetTarget() override;
        uint8 Calculate() override;
};

class EnergyValue : public Uint8CalculatedValue, public Qualified
{
    public:
        EnergyValue(PlayerbotAI* botAI, string name = "energy") : Uint8CalculatedValue(ai, name) {}

        Unit* GetTarget() override;
        uint8 Calculate() override;
};

class ManaValue : public Uint8CalculatedValue, public Qualified
{
    public:
        ManaValue(PlayerbotAI* botAI, string name = "mana") : Uint8CalculatedValue(ai, name) {}

        Unit* GetTarget()override;
        uint8 Calculate() override;
};

class HasManaValue : public BoolCalculatedValue, public Qualified
{
    public:
        HasManaValue(PlayerbotAI* botAI, string name = "has mana") : BoolCalculatedValue(ai, name) {}

        Unit* GetTarget()override;
        bool Calculate() override;
};

class ComboPointsValue : public Uint8CalculatedValue, public Qualified
{
    public:
        ComboPointsValue(PlayerbotAI* botAI, string name = "combo points") : Uint8CalculatedValue(ai, name) {}

        Unit* GetTarget() override;
        uint8 Calculate() override;
};

class IsMountedValue : public BoolCalculatedValue, public Qualified
{
    public:
        IsMountedValue(PlayerbotAI* botAI, string name = "mounted") : BoolCalculatedValue(ai, name) {}

        Unit* GetTarget() override;
        bool Calculate() override;
};

class IsInCombatValue : public MemoryCalculatedValue<bool>, public Qualified
{
    public:
        IsInCombatValue(PlayerbotAI* botAI, string name = "combat") : MemoryCalculatedValue(ai, name) {}

        Unit* GetTarget() override;
        bool Calculate() override;
        bool EqualToLast(bool value) override;
};

class BagSpaceValue : public Uint8CalculatedValue
{
    public:
        BagSpaceValue(PlayerbotAI* botAI, string name = "bag space") : Uint8CalculatedValue(ai, name) {}

        uint8 Calculate() override;
};

class DurabilityValue : public Uint8CalculatedValue
{
    public:
        DurabilityValue(PlayerbotAI* botAI, string name = "durability") : Uint8CalculatedValue(ai, name) {}

        uint8 Calculate() override;
};

class SpeedValue : public Uint8CalculatedValue, public Qualified
{
    public:
        SpeedValue(PlayerbotAI* botAI, string name = "speed") : Uint8CalculatedValue(ai, name) {}

        Unit* GetTarget()override;
        uint8 Calculate() override;
};

class IsInGroupValue : public BoolCalculatedValue
{
    public:
        IsInGroupValue(PlayerbotAI* botAI, string name = "in group") : BoolCalculatedValue(ai, name) {}

        bool Calculate() override;
};

class DeathCountValue : public ManualSetValue<uint32>
{
    public:
        DeathCountValue(PlayerbotAI* botAI, string name = "death count") : ManualSetValue<uint32>(ai, 0, name) {}
};

class ExperienceValue : public MemoryCalculatedValue<uint32>
{
    public:
        ExperienceValue(PlayerbotAI* ai, string name = "experience", uint32 checkInterval = 60) : MemoryCalculatedValue<uint32>(ai, name, checkInterval) {}

        bool EqualToLast(uint32 value) override
        {
            return value != lastValue;
        }

        uint32 Calculate() override
        {
            return bot->GetUInt32Value(PLAYER_XP);
        }
};

#endif
