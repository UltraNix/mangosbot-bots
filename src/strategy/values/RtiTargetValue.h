/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTITARGETVALUE_H
#define _PLAYERBOT_RTITARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class RtiTargetValue : public TargetValue
{
    public:
        RtiTargetValue(PlayerbotAI* botAI, std::string const& type = "rti") : type(type), TargetValue(botAI) { }

        static int32 GetRtiIndex(std::string const& rti);
        Unit* Calculate() override;

    private:
        std::string type;
};

class RtiCcTargetValue : public RtiTargetValue
{
    public:
        RtiCcTargetValue(PlayerbotAI* botAI) : RtiTargetValue(botAI, "rti cc") { }
};

#endif
