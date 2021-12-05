/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ISMOVINGVALUE_H
#define _PLAYERBOT_ISMOVINGVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class IsMovingValue : public BoolCalculatedValue, public Qualified
{
	public:
        IsMovingValue(PlayerbotAI* botAI, string name = "is moving") : BoolCalculatedValue(ai, name) {}

        bool Calculate() override;
};

class IsSwimmingValue : public BoolCalculatedValue, public Qualified
{
	public:
        IsSwimmingValue(PlayerbotAI* botAI, string name = "is swimming") : BoolCalculatedValue(ai, name) {}

        bool Calculate() override;
};

#endif
