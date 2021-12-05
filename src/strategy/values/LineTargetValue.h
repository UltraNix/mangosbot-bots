/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LINETARGETVALUE_H
#define _PLAYERBOT_LINETARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class LineTargetValue : public UnitCalculatedValue
{
	public:
        LineTargetValue(PlayerbotAI* botAI, string name = "line target") : UnitCalculatedValue(ai, name) {}

        Unit* Calculate() override;
};

#endif
