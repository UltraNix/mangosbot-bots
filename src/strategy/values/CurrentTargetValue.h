/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"

class PlayerbotAI;
class Unit;

class CurrentTargetValue : public UnitManualSetValue
{
	public:
        CurrentTargetValue(PlayerbotAI* botAI) : UnitManualSetValue(botAI, nullptr) { }

        Unit* Get() override;
        void Set(Unit* unit) override;

    private:
        ObjectGuid selection;
};
