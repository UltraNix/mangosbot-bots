/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NearestUnitsValue.h"
class PossibleRpgTargetsValue : public NearestUnitsValue
{
	public:
        PossibleRpgTargetsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->rpgDistance);

        static std::vector<uint32> allowedNpcFlags;

    protected:
        void FindUnits(std::list<Unit*>& targets) override;
        bool AcceptUnit(Unit* unit) override;
};
