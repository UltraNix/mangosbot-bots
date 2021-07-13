/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Unit.h"
#include "Value.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestUnitsValue : public ObjectGuidListCalculatedValue
{
	public:
        NearestUnitsValue(PlayerbotAI* botAI, std::string const& name, float range = sPlayerbotAIConfig->sightDistance, bool ignoreLos = false) :
            ObjectGuidListCalculatedValue(botAI, name, 2), range(range), ignoreLos(ignoreLos) { }

	public:
        GuidVector Calculate() override;

    protected:
        virtual void FindUnits(std::list<Unit*>& targets) = 0;
        virtual bool AcceptUnit(Unit* unit) = 0;

        float range;
        bool ignoreLos;
};
