/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"

class LootObject;
class LootObjectStack;

class AvailableLootValue : public ManualSetValue<LootObjectStack*>
{
	public:
        AvailableLootValue(PlayerbotAI* botAI);
        virtual ~AvailableLootValue();
};

class LootTargetValue : public ManualSetValue<LootObject>
{
    public:
        LootTargetValue(PlayerbotAI* botAI);
};

class CanLootValue : public BoolCalculatedValue
{
    public:
        CanLootValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        bool Calculate() override;
};
