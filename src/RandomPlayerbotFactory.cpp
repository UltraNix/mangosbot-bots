/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomPlayerbotFactory.h"
#include "ArenaTeamMgr.h"
#include "AccountMgr.h"
#include "GuildMgr.h"
#include "Playerbot.h"
#include "PlayerbotFactory.h"
#include "SocialMgr.h"

std::map<uint8, std::vector<uint8>> RandomPlayerbotFactory::availableRaces;

RandomPlayerbotFactory::RandomPlayerbotFactory(uint32 accountId) : accountId(accountId)
{
    availableRaces[CLASS_WARRIOR].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_GNOME);
    availableRaces[CLASS_WARRIOR].push_back(RACE_DWARF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_ORC);
    availableRaces[CLASS_WARRIOR].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TAUREN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TROLL);
    availableRaces[CLASS_WARRIOR].push_back(RACE_DRAENEI);

    availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
    availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);
    availableRaces[CLASS_PALADIN].push_back(RACE_DRAENEI);
    availableRaces[CLASS_PALADIN].push_back(RACE_BLOODELF);

    availableRaces[CLASS_ROGUE].push_back(RACE_HUMAN);
    availableRaces[CLASS_ROGUE].push_back(RACE_DWARF);
    availableRaces[CLASS_ROGUE].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_ROGUE].push_back(RACE_GNOME);
    availableRaces[CLASS_ROGUE].push_back(RACE_ORC);
    availableRaces[CLASS_ROGUE].push_back(RACE_TROLL);
    availableRaces[CLASS_ROGUE].push_back(RACE_BLOODELF);

    availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
    availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
    availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
    availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_PRIEST].push_back(RACE_DRAENEI);
    availableRaces[CLASS_PRIEST].push_back(RACE_BLOODELF);

    availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
    availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
    availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_MAGE].push_back(RACE_TROLL);
    availableRaces[CLASS_MAGE].push_back(RACE_DRAENEI);
    availableRaces[CLASS_MAGE].push_back(RACE_BLOODELF);

    availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
    availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);
    availableRaces[CLASS_WARLOCK].push_back(RACE_BLOODELF);

    availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);
    availableRaces[CLASS_SHAMAN].push_back(RACE_DRAENEI);

    availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
    availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
    availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
    availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);
    availableRaces[CLASS_HUNTER].push_back(RACE_DRAENEI);
    availableRaces[CLASS_HUNTER].push_back(RACE_BLOODELF);

    availableRaces[CLASS_DRUID].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DRUID].push_back(RACE_TAUREN);

    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TAUREN);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_HUMAN);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_ORC);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TROLL);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_BLOODELF);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DRAENEI);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_GNOME);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DWARF);
}

bool RandomPlayerbotFactory::CreateRandomBot(uint8 cls, unordered_map<uint8, vector<string>> const& names)
{
    LOG_DEBUG("playerbots",  "Creating new random bot for class %d", cls);

    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;

    uint8 race = availableRaces[cls][urand(0, availableRaces[cls].size() - 1)];

    string name;
    if(names.empty())
        name = CreateRandomBotName(gender);
    else
    {
        if (names[gender].empty())
            return false;

        uint32 i = urand(0, names[gender].size() - 1);
        name = names[gender][i];
        swap(names[gender][i], names[gender].back());
        names[gender].pop_back();
    }

    if (name.empty())
        return false;

    std::vector<uint8> skinColors, facialHairTypes;
    std::vector<std::pair<uint8,uint8>> faces, hairs;
    for (CharSectionsMap::const_iterator itr = sCharSectionMap.begin(); itr != sCharSectionMap.end(); ++itr)
    {
        CharSectionsEntry const* entry = itr->second;
        if (entry->RaceID != race || entry->SexID != gender)
            continue;

        switch (entry->BaseSection)
        {
            case SECTION_TYPE_SKIN:
                skinColors.push_back(entry->ColorIndex);
                break;
            case SECTION_TYPE_FACE:
                faces.push_back(std::pair<uint8,uint8>(entry->VariationIndex, entry->ColorIndex));
                break;
            case SECTION_TYPE_FACIAL_HAIR:
                facialHairTypes.push_back(entry->VariationIndex);
                break;
            case SECTION_TYPE_HAIR:
                hairs.push_back(std::pair<uint8,uint8>(entry->VariationIndex, entry->ColorIndex));
                break;
        }
    }

    uint8 skinColor = skinColors[urand(0, skinColors.size() - 1)];
    std::pair<uint8, uint8> face = faces[urand(0, faces.size() - 1)];
    std::pair<uint8, uint8> hair = hairs[urand(0, hairs.size() - 1)];

	bool excludeCheck = (race == RACE_TAUREN) || (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD_PLAYER);
	uint8 facialHair = excludeCheck ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];

	WorldSession* session = new WorldSession(accountId, nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), LOCALE_enUS, 0, false, false, 0);

    std::unique_ptr<CharacterCreateInfo> characterInfo = std::make_unique<CharacterCreateInfo>(name, race, cls, gender, face.second, face.first, hair.first, hair.second, facialHair, 0, WorldPacket());

    Player* player = new Player(session);
    player->GetMotionMaster()->Initialize();
	if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), characterInfo.get()))
    {
        player->CleanupsBeforeDelete();
        player->DeleteFromDB(player->GetGUID().GetCounter(), accountId, true, true);

        delete session;
        delete player;
        LOG_ERROR("playerbots", "Unable to create random bot for account %d - name: \"%s\"; race: %u; class: %u", accountId, name.c_str(), race, cls);
        return false;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);

    // player->SetSemaphoreTeleportFar(true); //Fake teleport to delay sql save
    // player->SaveToDB();
    // player->SetSemaphoreTeleportFar(false);

    ObjectAccessor::AddObject(player);

    LOG_DEBUG("playerbots", "Random bot created for account %d - name: \"%s\"; race: %u; class: %u", accountId, name.c_str(), race, cls);

    return true;
}

std::string RandomPlayerbotFactory::CreateRandomBotName(uint8 gender)
{
    std::string botName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbot_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random bots");
        return std::move(botName);
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    result = CharacterDatabase.PQuery("SELECT n.name FROM playerbot_names n "
        "LEFT OUTER JOIN characters e ON e.name = n.name WHERE e.guid IS nullptr AND n.gender = '%u' ORDER BY RAND() LIMIT 1", gender);
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random bots");
        return std::move(botName);
    }

	fields = result->Fetch();
    botName = fields[0].GetString();

    return std::move(botName);
}

void RandomPlayerbotFactory::CreateRandomBots()
{
    // check if scheduled for delete
    bool delAccs = false;
    bool delFriends = false;
    QueryResult results = PlayerbotDatabase.Query("SELECT value FROM playerbot_random_bots WHERE event = 'bot_delete'");

    if (results)
    {
        delAccs = true;

        Field* fields = results->Fetch();
        uint32 deleteType = fields[0].GetUInt32();

        if (deleteType > 1)
            delFriends = true;

        PlayerbotDatabase.PExecute("DELETE FROM playerbot_random_bots WHERE event = 'bot_delete'");
    }

    if (sPlayerbotAIConfig->deleteRandomBotAccounts || delAccs)
    {
        std::vector<uint32> botAccounts;
        std::vector<uint32> botFriends;

        for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig->randomBotAccountCount; ++accountNumber)
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
            std::string const& accountName = out.str();

            QueryResult results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
            if (!results)
                continue;

            Field* fields = results->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            botAccounts.push_back(accountId);
        }

        if (!delFriends)
            LOG_INFO("playerbots", "Deleting random bot characters without friends/guild...");
        else
            LOG_INFO("playerbots", "Deleting all random bot characters...");

        // load list of friends
        if (!delFriends)
        {
            QueryResult result = CharacterDatabase.PQuery("SELECT friend FROM character_social WHERE flags='%u'", SOCIAL_FLAG_FRIEND);
            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();
                    uint32 guidlow = fields[0].GetUInt32();
                    botFriends.push_back(guidlow);

                } while (result->NextRow());
            }
        }

        vector<std::future<void>> dels;
        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account WHERE username LIKE '%s%%'", sPlayerbotAIConfig->randomBotAccountPrefix.c_str());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint32 accId = fields[0].GetUInt32();

                if (!delFriends)
                {
                    // existing characters list
                    QueryResult result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account='%u'", accId);
                    if (result)
                    {
                        do
                        {
                            Field* fields = result->Fetch();
                            uint32 guidlow = fields[0].GetUInt32();
                            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidlow);

                            // if bot is someone's friend - don't delete it
                            if ((find(botFriends.begin(), botFriends.end(), guidlow) != botFriends.end()) && !delFriends)
                                continue;
                            // if bot is in someone's guild - don't delete it
                            uint32 guildId = Player::GetGuildIdFromStorage(guidlow);
                            if (guildId && !delFriends)
                            {
                                Guild* guild = sGuildMgr->GetGuildById(guildId);
                                uint32 accountId = sObjectMgr->GetPlayerAccountIdByGUID(guild->GetLeaderGUID().GetCounter());

                                if (find(botAccounts.begin(), botAccounts.end(), accountId) == botAccounts.end())
                                    continue;
                            }

                            Player::DeleteFromDB(guidlow, accId, false, true);       // no need to update realm characters
                        } while (result->NextRow());
                    }
                }
                else
                    dels.push_back(std::async([accId]
                    {
                        sAccountMgr.DeleteAccount(accId);
                    }));

            } while (results->NextRow());
        }

        for (uint32 i = 0; i < dels.size(); i++)
        {
            dels[i].wait();
        }

        PlayerbotDatabase.Execute("DELETE FROM playerbot_random_bots");

        LOG_INFO("playerbots", "Random bot characters deleted");
    }

    uint32 totalAccCount = sPlayerbotAIConfig->randomBotAccountCount;

    LOG_INFO("playerbots", "Creating random bot accounts...");

    vector<std::future<void>> account_creations;
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();

        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", accountName.c_str());
        if (results)
        {
            continue;
        }

        std::string password = "";
        if (sPlayerbotAIConfig.randomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
            {
                password += (char) urand('!', 'z');
            }
        }
        else
            password = accountName;

        account_creations.push_back(std::async([accountName, password]
        {
            AccountMgr::CreateAccount(accountName, password);
        }));

        LOG_DEBUG("playerbots", "Account %s created for random bots", accountName.c_str());
    }

    for (uint32 i = 0; i < account_creations.size(); i++)
    {
        account_creations[i].wait();
    }

    //LoginDatabase.PExecute("UPDATE account SET expansion = '%u' WHERE username LIKE '%s%%'", EXPANSION_WRATH_OF_THE_LICH_KING, sPlayerbotAIConfig->randomBotAccountPrefix.c_str());

    sLog.outString("Loading available names...");
    unordered_map<uint8,vector<string>> names;
    QueryResult* result = CharacterDatabase.PQuery("SELECT n.gender, n.name FROM playerbot_names n LEFT OUTER JOIN characters e ON e.name = n.name WHERE e.guid IS NULL");
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint8 gender = fields[0].GetUInt8();
        string bname = fields[1].GetString();
        names[gender].push_back(bname);
    } while (result->NextRow());

    LOG_INFO("playerbots", "Creating random bot characters...");
    uint32 totalRandomBotChars = 0;
    uint32 totalCharCount = sPlayerbotAIConfig->randomBotAccountCount * 10;
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig->randomBotAccountCount; ++accountNumber)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();

        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", accountName.c_str());
        if (!results)
            continue;

        Field* fields = results->Fetch();
        uint32 accountId = fields[0].GetUInt32();

        sPlayerbotAIConfig->randomBotAccounts.push_back(accountId);

        uint32 count = AccountMgr::GetCharactersCount(accountId);
        if (count >= 10)
        {
            totalRandomBotChars += count;
            continue;
        }

        RandomPlayerbotFactory factory(accountId);
        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES - count; ++cls)
        {
           if (cls != 10)
               factory.CreateRandomBot(cls, names);
        }

        totalRandomBotChars += AccountMgr::GetCharactersCount(accountId);
    }

    vector<std::future<void>> bot_creations;
    for (auto pl : sObjectAccessor.GetPlayers())
    {
        Player* player = pl.second;
        account_creations.push_back(std::async([player]
        {
            player->SaveToDB();
        }));
    }

    for (uint32 i = 0; i < account_creations.size(); i++)
    {
        account_creations[i].wait();
    }

    LOG_INFO("playerbots", "%zu random bot accounts with %d characters available", sPlayerbotAIConfig->randomBotAccounts.size(), totalRandomBotChars);
}

void RandomPlayerbotFactory::CreateRandomGuilds()
{
    std::vector<uint32> randomBots;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT `bot` FROM playerbot_random_bots WHERE event = 'add'");

    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            randomBots.push_back(bot);
        } while (results->NextRow());
    }

    if (sPlayerbotAIConfig->deleteRandomBotGuilds)
    {
        LOG_INFO("playerbots", "Deleting random bot guilds...");
        for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            if (Guild* guild = sGuildMgr->GetGuildByLeader(ObjectGuid::Create<HighGuid::Player>(*i)))
                guild->Disband();
        }

        LOG_INFO("playerbots", "Random bot guilds deleted");
    }

    uint32 guildNumber = 0;
    GuidVector availableLeaders;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid leader = ObjectGuid::Create<HighGuid::Player>(*i);
        if (Guild* guild = sGuildMgr->GetGuildByLeader(leader))
        {
            ++guildNumber;
            sPlayerbotAIConfig->randomBotGuilds.push_back(guild->GetId());
        }
        else
        {
            Player* player = ObjectAccessor::FindPlayer(leader);
            if (player && !player->GetGuildId())
                availableLeaders.push_back(leader);
        }
    }

    for (; guildNumber < sPlayerbotAIConfig->randomBotGuildCount; ++guildNumber)
    {
        std::string const& guildName = CreateRandomGuildName();
        if (guildName.empty())
            continue;

        if (availableLeaders.empty())
        {
            LOG_ERROR("playerbots", "No leaders for random guilds available");
            continue;
        }

        uint32 index = urand(0, availableLeaders.size() - 1);
        ObjectGuid leader = availableLeaders[index];
        Player* player = ObjectAccessor::FindPlayer(leader);
        if (!player)
        {
            LOG_ERROR("playerbots", "Cannot find player for leader %s", player->GetName().c_str());
            continue;
        }

        Guild* guild = new Guild();
        if (!guild->Create(player, guildName))
        {
            LOG_ERROR("playerbots", "Error creating guild %s", guildName.c_str());
            delete guild;
            continue;
        }

        sGuildMgr->AddGuild(guild);

        // create random emblem
        uint32 st, cl, br, bc, bg;
        bg = urand(0, 51);
        bc = urand(0, 17);
        cl = urand(0, 17);
        br = urand(0, 7);
        st = urand(0, 180);
        guild->SetEmblem(st, cl, br, bc, bg);

        sPlayerbotAIConfig->randomBotGuilds.push_back(guild->GetId());
    }

    LOG_INFO("playerbots", "%d random bot guilds available", guildNumber);
}

std::string const& RandomPlayerbotFactory::CreateRandomGuildName()
{
    std::string guildName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbot_guild_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return std::move(guildName);
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM playerbot_guild_names n "
            "LEFT OUTER JOIN guild e ON e.name = n.name WHERE e.guildid IS nullptr AND n.name_id >= '%u' LIMIT 1", id);
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return std::move(guildName);
    }

    fields = result->Fetch();
    guildName = fields[0].GetString();

    return std::move(guildName);
}

void RandomPlayerbotFactory::CreateRandomArenaTeams()
{
    std::vector<uint32> randomBots;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT `bot` FROM playerbot_random_bots WHERE event = 'add'");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            randomBots.push_back(bot);
        } while (results->NextRow());
    }

    if (sPlayerbotAIConfig->deleteRandomBotArenaTeams)
    {
        LOG_INFO("playerbots", "Deleting random bot arena teams...");

        for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid captain = ObjectGuid::Create<HighGuid::Player>(*i);
            ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamByCaptain(captain);
            if (arenateam)
                //sObjectMgr->RemoveArenaTeam(arenateam->GetId());
                arenateam->Disband(nullptr);
        }

        LOG_INFO("playerbots", "Random bot arena teams deleted");
    }

    uint32 arenaTeamNumber = 0;
    GuidVector availableCaptains;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid captain = ObjectGuid::Create<HighGuid::Player>(*i);
        ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamByCaptain(captain);
        if (arenateam)
        {
            ++arenaTeamNumber;
            sPlayerbotAIConfig->randomBotArenaTeams.push_back(arenateam->GetId());
        }
        else
        {
            Player* player = ObjectAccessor::FindConnectedPlayer(captain);

            if (!arenateam && player && player->getLevel() >= 70)
                availableCaptains.push_back(captain);
        }
    }

    for (; arenaTeamNumber < sPlayerbotAIConfig->randomBotArenaTeamCount; ++arenaTeamNumber)
    {
        std::string const& arenaTeamName = CreateRandomArenaTeamName();
        if (arenaTeamName.empty())
            continue;

        if (availableCaptains.empty())
        {
            LOG_ERROR("playerbots", "No captains for random arena teams available");
            continue;
        }

        uint32 index = urand(0, availableCaptains.size() - 1);
        ObjectGuid captain = availableCaptains[index];
        Player* player = ObjectAccessor::FindConnectedPlayer(captain);
        if (!player)
        {
            LOG_ERROR("playerbots", "Cannot find player for captain %d", captain);
            continue;
        }

        if (player->getLevel() < 70)
        {
            LOG_ERROR("playerbots", "Bot %d must be level 70 to create an arena team", captain);
            continue;
        }

        QueryResult results = CharacterDatabase.PQuery("SELECT `type` FROM playerbot_arena_team_names WHERE name = '%s'", arenaTeamName.c_str());
        if (!results)
        {
            LOG_ERROR("playerbots", "No valid types for arena teams");
            return;
        }

        Field* fields = results->Fetch();
        uint8 slot = fields[0].GetUInt8();

        ArenaType type;
        switch (slot)
        {
            case 2:
                type = ARENA_TYPE_2v2;
                break;
            case 3:
                type = ARENA_TYPE_3v3;
                break;
            case 5:
                type = ARENA_TYPE_5v5;
                break;
        }

        ArenaTeam* arenateam = new ArenaTeam();
        if (!arenateam->Create(player->GetGUID(), type, arenaTeamName, 0, 0, 0, 0, 0))
        {
            LOG_ERROR("playerbots", "Error creating arena team %s", arenaTeamName.c_str());
            continue;
        }

        arenateam->SetCaptain(player->GetGUID());
        // set random rating
        arenateam->SetRatingForAll(urand(1500, 2300));
        // set random emblem
        uint32 backgroundColor = urand(0xFF000000, 0xFFFFFFFF), emblemStyle = urand(0, 5), emblemColor = urand(0xFF000000, 0xFFFFFFFF), borderStyle = urand(0, 5), borderColor = urand(0xFF000000, 0xFFFFFFFF);
        arenateam->SetEmblem(backgroundColor, emblemStyle, emblemColor, borderStyle, borderColor);
        // set random kills (wip)
        //arenateam->SetStats(STAT_TYPE_GAMES_WEEK, urand(0, 30));
        //arenateam->SetStats(STAT_TYPE_WINS_WEEK, urand(0, arenateam->GetStats().games_week));
        //arenateam->SetStats(STAT_TYPE_GAMES_SEASON, urand(arenateam->GetStats().games_week, arenateam->GetStats().games_week * 5));
        //arenateam->SetStats(STAT_TYPE_WINS_SEASON, urand(arenateam->GetStats().wins_week, arenateam->GetStats().games
        arenateam->SaveToDB();

        sArenaTeamMgr->AddArenaTeam(arenateam);
        sPlayerbotAIConfig->randomBotArenaTeams.push_back(arenateam->GetId());
    }

    LOG_INFO("playerbots", "%d random bot arena teams available", arenaTeamNumber);
}

std::string const& RandomPlayerbotFactory::CreateRandomArenaTeamName()
{
    std::string arenaTeamName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbot_arena_team_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random arena teams");
        return std::move(arenaTeamName);
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM playerbot_arena_team_names n LEFT OUTER JOIN arena_team e ON e.name = n.name "
        "WHERE e.arenateamid IS nullptr AND n.name_id >= '%u' LIMIT 1", id);

    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random arena teams");
        return std::move(arenaTeamName);
    }

    fields = result->Fetch();
    arenaTeamName = fields[0].GetString();

    return std::move(arenaTeamName);
}
