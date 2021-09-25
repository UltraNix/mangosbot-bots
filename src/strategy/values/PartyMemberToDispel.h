/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PARTYMEMBERTODISPEL_H
#define _PLAYERBOT_PARTYMEMBERTODISPEL_H

#include "PartyMemberValue.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class PartyMemberToDispel : public PartyMemberValue, public Qualified
{
	public:
        PartyMemberToDispel(PlayerbotAI* botAI) : PartyMemberValue(botAI), Qualified() { }

    protected:
        Unit* Calculate() override;
};

#endif
