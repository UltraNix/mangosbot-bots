/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOGLEVELVALUE_H
#define _PLAYERBOT_LOGLEVELVALUE_H

#include "Value.h"

class PlayerbotAI;

class LogLevelValue : public ManualSetValue<LogLevel>
{
	public:
        LogLevelValue(PlayerbotAI* botAI) : ManualSetValue<LogLevel>(botAI, LOG_LEVEL_DEBUG) { }
};

#endif
