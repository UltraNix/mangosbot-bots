/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PartyMemberValue.h"
class PartyMemberToResurrect : public PartyMemberValue
{
	public:
        PartyMemberToResurrect(PlayerbotAI* botAI) : PartyMemberValue(botAI) { }

    protected:
        Unit* Calculate() override;
};
