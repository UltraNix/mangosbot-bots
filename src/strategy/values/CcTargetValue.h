/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TargetValue.h"
#include "../NamedObjectContext.h"

class PlayerbotAI;

class CcTargetValue : public TargetValue, public Qualified
{
	public:
        CcTargetValue(PlayerbotAI* botAI) : TargetValue(botAI) { }

        Unit* Calculate() override;
};
