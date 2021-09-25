/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LEASTHPTARGETVALUE_H
#define _PLAYERBOT_LEASTHPTARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;

class LeastHpTargetValue : public TargetValue
{
	public:
        LeastHpTargetValue(PlayerbotAI* botAI) : TargetValue(botAI) { }

        Unit* Calculate() override;
};

#endif
