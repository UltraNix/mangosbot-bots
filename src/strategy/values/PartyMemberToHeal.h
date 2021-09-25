/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PARTYMEMBERTOHEAL_H
#define _PLAYERBOT_PARTYMEMBERTOHEAL_H

#include "PartyMemberValue.h"

class Pet;
class PlayerbotAI;

class PartyMemberToHeal : public PartyMemberValue
{
	public:
        PartyMemberToHeal(PlayerbotAI* botAI) : PartyMemberValue(botAI) { }

    protected:
        Unit* Calculate() override;
        bool Check(Unit* player) override;
};

#endif
