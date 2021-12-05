/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H
#define _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H

#include "Common.h"

class RandomPlayerbotFactory
{
    public:
        RandomPlayerbotFactory(uint32 accountId);
		virtual ~RandomPlayerbotFactory() { }

        bool CreateRandomBot(uint8 cls, unordered_map<uint8, vector<string>> const& names);
        static void CreateRandomBots();
        static void CreateRandomGuilds();
        static void CreateRandomArenaTeams();
        static string CreateRandomGuildName();

	private:
        std::string CreateRandomBotName(uint8 gender);
        static std::string const& CreateRandomArenaTeamName();

        uint32 accountId;
        static std::map<uint8, std::vector<uint8>> availableRaces;
};

#endif
