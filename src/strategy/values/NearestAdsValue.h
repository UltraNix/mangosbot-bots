/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PossibleTargetsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestAdsValue : public PossibleTargetsValue
{
	public:
        NearestAdsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->tooCloseDistance) : PossibleTargetsValue(botAI, "nearest ads", range) { }

    protected:
        bool AcceptUnit(Unit* unit) override;
};
