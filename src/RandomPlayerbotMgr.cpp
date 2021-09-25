/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomPlayerbotMgr.h"
#include "AccountMgr.h"
#include "AiFactory.h"
#include "ArenaTeamMgr.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "FleeManager.h"
#include "GuildMgr.h"
#include "GuildTaskMgr.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "PerformanceMonitor.h"
#include "Playerbot.h"
#include "PlayerbotCommandServer.h"
#include "PlayerbotFactory.h"
#include "ServerFacade.h"
#include "TravelMgr.h"

#include <iomanip>
#include <boost/thread/thread.hpp>

void PrintStatsThread()
{
    sRandomPlayerbotMgr->PrintStats();
}

void activatePrintStatsThread()
{
    boost::thread t(PrintStatsThread);
    t.detach();
}

void CheckBgQueueThread()
{
    sRandomPlayerbotMgr->CheckBgQueue();
}

void activateCheckBgQueueThread()
{
    boost::thread t(CheckBgQueueThread);
    t.detach();
}

void CheckLfgQueueThread()
{
    sRandomPlayerbotMgr->CheckLfgQueue();
}

void activateCheckLfgQueueThread()
{
    boost::thread t(CheckLfgQueueThread);
    t.detach();
}

void CheckPlayersThread()
{
    sRandomPlayerbotMgr->CheckPlayers();
}

void activateCheckPlayersThread()
{
    boost::thread t(CheckPlayersThread);
    t.detach();
}

RandomPlayerbotMgr::RandomPlayerbotMgr() : PlayerbotHolder(), processTicks(0)
{
    playersLevel = sPlayerbotAIConfig->randombotStartingLevel;

    if (sPlayerbotAIConfig->enabled || sPlayerbotAIConfig->randomBotAutologin)
    {
        sPlayerbotCommandServer->Start();
        PrepareTeleportCache();
    }
}

RandomPlayerbotMgr::~RandomPlayerbotMgr()
{
}

uint32 RandomPlayerbotMgr::GetMaxAllowedBotCount()
{
    return GetEventValue(0, "bot_count");
}

void RandomPlayerbotMgr::UpdateAIInternal(uint32 elapsed)
{
    if (sPlayerbotAIConfig->hasLog("player_location.csv"))
    {
        sPlayerbotAIConfig->openLog("player_location.csv", "w");

        if (sPlayerbotAIConfig->randomBotAutologin)
        {
            for (auto i : GetAllBots())
            {
                Player* bot = i.second;
                if (!bot)
                    continue;

                std::ostringstream out;
                out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
                out << "RND" << ",";
                out << bot->GetName() << ",";
                out << std::fixed << std::setprecision(2);
                WorldPosition(bot).printWKT(out);
                out << bot->GetOrientation() << ",";
                out << std::to_string(bot->getRace()) << ",";
                out << std::to_string(bot->getClass()) << ",";
                out << bot->getLevel() << ",";
                out << bot->GetHealth() << ",";
                out << bot->GetPowerPct(bot->getPowerType()) << ",";
                out << bot->GetMoney() << ",";
                sPlayerbotAIConfig->log("player_location.csv", out.str().c_str());
            }
        }

        for (auto i : GetPlayers())
        {
            Player* bot = i;
            if (!bot)
                continue;

            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
            out << "PLR" << ",";
            out << bot->GetName() << ",";
            out << std::fixed << std::setprecision(2);
            WorldPosition(bot).printWKT(out);
            out << bot->GetOrientation() << ",";
            out << std::to_string(bot->getRace()) << ",";
            out << std::to_string(bot->getClass()) << ",";
            out << bot->getLevel() << ",";
            out << bot->GetHealth() << ",";
            out << bot->GetPowerPct(bot->getPowerType()) << ",";
            out << bot->GetMoney() << ",";
            out << std::fixed << std::setprecision(2);
            sPlayerbotAIConfig->log("player_location.csv", out.str().c_str());
        }
    }

    if (!sPlayerbotAIConfig->randomBotAutologin || !sPlayerbotAIConfig->enabled)
        return;

    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (!maxAllowedBotCount || (maxAllowedBotCount < sPlayerbotAIConfig->minRandomBots || maxAllowedBotCount > sPlayerbotAIConfig->maxRandomBots))
    {
        maxAllowedBotCount = urand(sPlayerbotAIConfig->minRandomBots, sPlayerbotAIConfig->maxRandomBots);
        SetEventValue(0, "bot_count", maxAllowedBotCount, urand(sPlayerbotAIConfig->randomBotCountChangeMinInterval, sPlayerbotAIConfig->randomBotCountChangeMaxInterval));
    }

    // Fix possible divide by zero if maxAllowedBotCount is smaller then sPlayerbotAIConfig->randomBotsPerInterval
    uint32 notDiv = 1;
    if (maxAllowedBotCount > sPlayerbotAIConfig->randomBotsPerInterval)
        notDiv = maxAllowedBotCount / sPlayerbotAIConfig->randomBotsPerInterval;

    //SetNextCheckDelay((uint32)max(1000, int(2000 * notDiv * sPlayerbotAIConfig->randomBotUpdateInterval) / 1000));

    if (playerBots.size() < int(sPlayerbotAIConfig->minRandomBots / 4))
        SetNextCheckDelay((uint32)std::max(500, int(500 * notDiv * sPlayerbotAIConfig->randomBotUpdateInterval) / 1000));
    else if (playerBots.size() < int(sPlayerbotAIConfig->minRandomBots / 2))
        SetNextCheckDelay((uint32)std::max(1000, int(1000 * notDiv * sPlayerbotAIConfig->randomBotUpdateInterval) / 1000));
    else
        SetNextCheckDelay((uint32)std::max(1000, int(2000 * notDiv * sPlayerbotAIConfig->randomBotUpdateInterval) / 1000));

    std::vector<uint32> bots = GetBots();
    uint32 botCount = bots.size();

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_TOTAL, playerBots.size() < maxAllowedBotCount ? "RandomPlayerbotMgr::Login" : "RandomPlayerbotMgr::UpdateAIInternal");

    if (bots.size() < sPlayerbotAIConfig->minRandomBots)
        AddRandomBots();

    if (sPlayerbotAIConfig->syncLevelWithPlayers && !players.empty())
    {
        if (time(nullptr) > (PlayersCheckTimer + 60))
            activateCheckPlayersThread();
    }

    if (sPlayerbotAIConfig->syncLevelWithPlayers && !players.empty())
    {
        if (time(nullptr) > (PlayersCheckTimer + 60))
            activateCheckPlayersThread();
    }

    if (sPlayerbotAIConfig->randomBotJoinBG && !players.empty())
    {
        if (time(nullptr) > (BgCheckTimer + 30))
            activateCheckBgQueueThread();
    }

    uint32 botProcessed = 0;
    for (std::vector<uint32>::iterator i = bots.begin(); i != bots.end(); ++i)
    {
        uint32 bot = *i;
        if (ProcessBot(bot))
            ++botProcessed;

        if (botProcessed >= sPlayerbotAIConfig->randomBotsPerInterval)
            break;
    }

    if (pmo)
        pmo->finish();
}

uint32 RandomPlayerbotMgr::AddRandomBots()
{
    std::set<uint32> bots;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT `bot` FROM playerbot_random_bots WHERE event = 'add'");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            bots.insert(bot);
        } while (results->NextRow());
    }

    std::vector<uint32> guids;
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    int32 maxAllowedNewBotCount = std::max((sPlayerbotAIConfig->randomBotsPerInterval / 4), uint32(maxAllowedBotCount - currentBots.size()));
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin(); i != sPlayerbotAIConfig->randomBotAccounts.end(); i++)
    {
        uint32 accountId = *i;
        if (!AccountMgr::GetCharactersCount(accountId))
            continue;

        if (urand(0, 100) > 10)  // more random selection
            continue;

        QueryResult result = CharacterDatabase.PQuery("SELECT guid, race, name, level FROM characters WHERE account = '%u'", accountId);
        if (!result)
            continue;

        do
        {
            Field* fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            uint8 race = fields[1].GetUInt8();
            std::string name = fields[2].GetString();
            uint32 level = fields[3].GetUInt32();

            if (bots.find(guid) != bots.end())
                continue;

            // Although this code works it cuts the Maximum Bots setting in half.
            // And, also doesn't see to be any reason to do it.
            //bool alliance = guids.size() % 2 == 0;
            //if (bots.find(guid) == bots.end() && ((alliance && IsAlliance(race)) || ((!alliance && !IsAlliance(race)))))
            //{
                guids.push_back(guid);
                uint32 bot = guid;
                SetEventValue(bot, "add", 1, urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime));

                uint32 randomTime = 60 + urand(sPlayerbotAIConfig->randomBotUpdateInterval, sPlayerbotAIConfig->randomBotUpdateInterval * 3);
                ScheduleRandomize(bot, randomTime);

                SetEventValue(bot, "teleport", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
                SetEventValue(bot, "change_strategy", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);

                bots.insert(bot);
                currentBots.push_back(bot);

                LOG_INFO("playerbots", "Bot #%d %s:%d <%s>: log in", guid, IsAlliance(race) ? "A" : "H", level, name.c_str());
                if (guids.size() >= std::min((int)(sPlayerbotAIConfig->randomBotsPerInterval / 4), maxAllowedNewBotCount))
                {
                    return guids.size();
                }
            //}
        } while (result->NextRow());
    }

    return guids.size();
}

void RandomPlayerbotMgr::LoadBattleMastersCache()
{
    BattleMastersCache.clear();

    LOG_INFO("playerbots", "---------------------------------------");
    LOG_INFO("playerbots", "          Loading BattleMasters Cache  ");
    LOG_INFO("playerbots", "---------------------------------------");

    QueryResult result = WorldDatabase.Query("SELECT `entry`,`bg_template` FROM `battlemaster_entry`");

    uint32 count = 0;

    if (!result)
    {
        return;
    }

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId = fields[1].GetUInt32();

        CreatureTemplate const* bmaster = sObjectMgr->GetCreatureTemplate(entry);
        if (!bmaster)
            continue;

        FactionTemplateEntry const* bmFaction = sFactionTemplateStore.LookupEntry(bmaster->faction);
        uint32 bmFactionId = bmFaction->faction;
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry(bmFactionId);
        uint32 bmParentTeam = bmParentFaction->team;
        TeamId bmTeam = TEAM_NEUTRAL;
        if (bmParentTeam == 891)
            bmTeam = TEAM_ALLIANCE;

        if (bmFactionId == 189)
            bmTeam = TEAM_ALLIANCE;

        if (bmParentTeam == 892)
            bmTeam = TEAM_HORDE;

        if (bmFactionId == 66)
            bmTeam = TEAM_HORDE;

        BattleMastersCache[bmTeam][BattlegroundTypeId(bgTypeId)].insert(BattleMastersCache[bmTeam][BattlegroundTypeId(bgTypeId)].end(), entry);
        LOG_INFO("playerbots", "Cached Battmemaster #%d for BG Type %d (%s)", entry, bgTypeId, bmTeam == ALLIANCE ? "Alliance" : bmTeam == HORDE ? "Horde" : "Neutral");

    } while (result->NextRow());

    LOG_INFO("playerbots", ">> Loaded %u battlemaster entries", count);
}

void RandomPlayerbotMgr::CheckBgQueue()
{
    if (!BgCheckTimer)
        BgCheckTimer = time(nullptr);

    uint32 count = 0;
    uint32 visual_count = 0;

    uint32 check_time = count > 0 ? 120 : 30;
    if (time(nullptr) < (BgCheckTimer + check_time))
    {
        return;
    }
    else
    {
        BgCheckTimer = time(nullptr);
    }

    LOG_INFO("playerbots", "Checking BG Queue...");

    for (uint32 i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint32 j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BgPlayers[j][i][TEAM_ALLIANCE] = 0;
            BgPlayers[j][i][TEAM_HORDE] = 0;
            BgBots[j][i][TEAM_ALLIANCE] = 0;
            BgBots[j][i][TEAM_HORDE] = 0;
            ArenaBots[j][i][TEAM_ALLIANCE][TEAM_ALLIANCE] = 0;
            ArenaBots[j][i][TEAM_ALLIANCE][TEAM_HORDE] = 0;
            ArenaBots[j][i][TEAM_HORDE][TEAM_ALLIANCE] = 0;
            ArenaBots[j][i][TEAM_HORDE][TEAM_HORDE] = 0;
            NeedBots[j][i][TEAM_ALLIANCE] = false;
            NeedBots[j][i][TEAM_HORDE] = false;
        }
    }

    for (Player* player : players)
    {
        if (!player->InBattlegroundQueue())
            continue;

        if (player->InBattleground() && player->GetBattleground()->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            BattlegroundQueueTypeId queueTypeId = player->GetBattlegroundQueueTypeId(i);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            TeamId teamId = player->GetTeamId();

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, player->getLevel());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();

            if (uint8 arenaType = BattlegroundMgr::BGArenaType(queueTypeId))
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                GroupQueueInfo ginfo;
                TeamId tempT = teamId;

                if (bgQueue.GetPlayerGroupInfoData(player->GetGUID(), &ginfo))
                {
                    if (ginfo.IsRated)
                    {
                        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                        {
                            uint32 arena_team_id = player->GetArenaTeamId(arena_slot);
                            ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamById(arena_team_id);
                            if (!arenateam)
                                continue;

                            if (arenateam->GetType() != arenaType)
                                continue;

                            Rating[queueTypeId][bracketId][1] = arenateam->GetRating();
                        }
                    }

                    teamId = ginfo.IsRated ? TEAM_HORDE : TEAM_ALLIANCE;
                }

                if (player->InArena())
                {
                    if (player->GetBattleground()->isRated() && (ginfo.IsRated && ginfo.ArenaTeamId && ginfo.ArenaTeamRating && ginfo.OpponentsTeamRating))
                        teamId = TEAM_HORDE;
                    else
                        teamId = TEAM_ALLIANCE;
                }

                ArenaBots[queueTypeId][bracketId][teamId][tempT]++;
            }

            if (player->GetPlayerbotAI())
                BgBots[queueTypeId][bracketId][teamId]++;
            else
                BgPlayers[queueTypeId][bracketId][teamId]++;

            if (!player->IsInvitedForBattlegroundInstance() && (!player->InBattleground() || player->GetBattleground()->GetBgTypeID() != BattlegroundMgr::BGTemplateId(queueTypeId)))
                NeedBots[queueTypeId][bracketId][teamId] = true;
        }
    }

    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;

        if (!bot->InBattlegroundQueue())
            continue;

        if (!IsRandomBot(bot->GetGUID().GetCounter()))
            continue;

        if (bot->InBattleground() && bot->GetBattleground()->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(i);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            TeamId teamId = bot->GetTeamId();

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();

            uint8 arenaType = BattlegroundMgr::BGArenaType(queueTypeId);
            if (arenaType)
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                GroupQueueInfo ginfo;
                TeamId tempT = teamId;
                if (bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
                    teamId = ginfo.IsRated ? TEAM_HORDE : TEAM_ALLIANCE;

                if (bot->InArena())
                {
                    if (bot->GetBattleground()->isRated() && (ginfo.IsRated && ginfo.ArenaTeamId && ginfo.ArenaTeamRating && ginfo.OpponentsTeamRating))
                        teamId = TEAM_HORDE;
                    else
                        teamId = TEAM_ALLIANCE;
                }

                ArenaBots[queueTypeId][bracketId][teamId][tempT]++;
            }

            BgBots[queueTypeId][bracketId][teamId]++;
        }
    }

    for (uint8 i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint8 j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(j);

            if ((BgPlayers[j][i][TEAM_ALLIANCE] + BgBots[j][i][TEAM_ALLIANCE] + BgPlayers[j][i][TEAM_HORDE] + BgBots[j][i][TEAM_HORDE]) == 0)
                continue;

            if (uint8 type = BattlegroundMgr::BGArenaType(queueTypeId))
            {
                LOG_INFO("playerbots", "ARENA:%s %s: P (Skirmish:%d, Rated:%d) B (Skirmish:%d, Rated:%d) Total (Skirmish:%d Rated:%d)",
                    type == ARENA_TYPE_2v2 ? "2v2" : type == ARENA_TYPE_3v3 ? "3v3" : "5v5", i == 0 ? "10-19" : i == 1 ? "20-29" : i == 2 ? "30-39" : i == 3 ? "40-49" :
                    i == 4 ? "50-59" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 6) ? "60" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 7) ? "60-69" :
                    i == 6 ? (i == 6 && MAX_BATTLEGROUND_BRACKETS == 16) ? "70-79" : "70" : "80",
                    BgPlayers[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE], BgBots[j][i][TEAM_ALLIANCE], BgBots[j][i][TEAM_HORDE],
                    BgPlayers[j][i][TEAM_ALLIANCE] + BgBots[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE] + BgBots[j][i][TEAM_HORDE]);

                continue;
            }

            BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
            LOG_INFO("playerbots", "BG:%s %s: P (%d:%d) B (%d:%d) Total (A:%d H:%d)",
                bgTypeId == BATTLEGROUND_AV ? "AV" : bgTypeId == BATTLEGROUND_WS ? "WSG" : bgTypeId == BATTLEGROUND_AB ? "AB" : "EoTS", i == 0 ? "10-19" : i == 1 ? "20-29" :
                i == 2 ? "30-39" : i == 3 ? "40-49" : i == 4 ? "50-59" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 6) ? "60" :
                (i == 5 && MAX_BATTLEGROUND_BRACKETS == 7) ? "60-69" : i == 6 ? (i == 6 && MAX_BATTLEGROUND_BRACKETS == 16) ? "70-79" : "70" : "80",
                BgPlayers[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE], BgBots[j][i][TEAM_ALLIANCE], BgBots[j][i][TEAM_HORDE],
                BgPlayers[j][i][TEAM_ALLIANCE] + BgBots[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE] + BgBots[j][i][TEAM_HORDE]);
        }
    }

    LOG_INFO("playerbots", "BG Queue check finished");
}

void RandomPlayerbotMgr::CheckLfgQueue()
{
    if (!LfgCheckTimer || time(nullptr) > (LfgCheckTimer + 30))
        LfgCheckTimer = time(nullptr);

    LOG_INFO("playerbots", "Checking LFG Queue...");

    // Clear LFG list
    LfgDungeons[TEAM_ALLIANCE].clear();
    LfgDungeons[TEAM_HORDE].clear();

    for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player* player = *i;

        Group* group = player->GetGroup();
        ObjectGuid guid = group ? group->GetGUID() : player->GetGUID();

        lfg::LfgState gState = sLFGMgr->GetState(guid);
        if (gState != lfg::LFG_STATE_NONE && gState < lfg::LFG_STATE_DUNGEON)
        {
            lfg::LfgDungeonSet const& dList = sLFGMgr->GetSelectedDungeons(player->GetGUID());
            for (lfg::LfgDungeonSet::const_iterator itr = dList.begin(); itr != dList.end(); ++itr)
            {
                lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(*itr);
                if (!dungeon)
                    continue;

                LfgDungeons[player->GetTeamId()].push_back(dungeon->id);
            }
        }
    }

    LOG_INFO("playerbots", "LFG Queue check finished");
}

void RandomPlayerbotMgr::CheckPlayers()
{
    if (!PlayersCheckTimer || time(nullptr) > (PlayersCheckTimer + 60))
        PlayersCheckTimer = time(nullptr);

    LOG_INFO("playerbots", "Checking Players...");

    if (!playersLevel)
        playersLevel = sPlayerbotAIConfig->randombotStartingLevel;

    for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player* player = *i;

        if (player->IsGameMaster())
            continue;

        //if (player->GetSession()->GetSecurity() > SEC_PLAYER)
        //    continue;

        if (player->getLevel() > playersLevel)
            playersLevel = player->getLevel() + 3;
    }

    LOG_INFO("playerbots", "Max player level is %d, max bot level set to %d", playersLevel - 3, playersLevel);
}

void RandomPlayerbotMgr::AddBgBot(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId, bool isRated, bool visual)
{
    BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return;

    uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();
    bool isArena = false;
    std::string _bgType;

    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            _bgType = "AV";
            break;
        case BATTLEGROUND_WS:
            _bgType = "WSG";
            break;
        case BATTLEGROUND_AB:
            _bgType = "AB";
            break;
        case BATTLEGROUND_EY:
            _bgType = "EotS";
            break;
        default:
            break;
    }

    if (uint8 type = BattlegroundMgr::BGArenaType(queueTypeId))
    {
        switch (type)
        {
            case ARENA_TYPE_2v2:
                _bgType = "2v2";
                break;
            case ARENA_TYPE_3v3:
                _bgType = "3v3";
                break;
            case ARENA_TYPE_5v5:
                _bgType = "5v5";
                break;
            default:
                break;
        }
    }

    uint32 ACount = BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE];
    uint32 HCount = BgBots[queueTypeId][bracketId][TEAM_HORDE] + BgPlayers[queueTypeId][bracketId][TEAM_HORDE];

    uint32 BgCount = ACount + HCount;

    uint32 SCount, RCount;

    if (uint8 type = BattlegroundMgr::BGArenaType(queueTypeId))
    {
        isArena = true;
        BracketSize = type * 2;
        TeamSize = type;
        ACount = ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][TEAM_ALLIANCE];
        HCount = ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][TEAM_HORDE];
        BgCount = BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE] + BgPlayers[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE];
        SCount = BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE];
        RCount = BgBots[queueTypeId][bracketId][TEAM_HORDE] + BgPlayers[queueTypeId][bracketId][TEAM_HORDE];
    }

    std::string const& bgType = isArena ? "Arena" : "BG";

    if (BgCount >= BracketSize && !visual && (ACount >= TeamSize) && (HCount >= TeamSize))
    {
        LOG_INFO("playerbots", "Can't add BG Bots to %s %d (%s), it is full", bgType.c_str(), bgTypeId, _bgType.c_str());
        return;
    }

    Player* player = nullptr;
    if (!visual && isArena && ((!isRated && SCount >= BracketSize) || (!isRated && RCount >= BracketSize)))
    {
        LOG_INFO("playerbots", "Can't add bots to %s %s, Arena queue is full", bgType.c_str(), _bgType.c_str());
        return;
    }

    if (!visual)
        LOG_INFO("playerbots", "Searching bots for %s %s", bgType.c_str(), _bgType.c_str());

    if (!isRated)
    {
        for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
        {
            if (urand(0, 100) > 10)
                continue;

            Player* bot = i->second;

            if (bot->IsBeingTeleported())
                continue;

            // check Deserter debuff
            if (!bot->CanJoinToBattleground())
                continue;

            Map* map = bot->GetMap();
            if (map && map->Instanceable())
                continue;

            uint32 bgType = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("bg type")->Get();
            if (bgType && bgType != 20)
                continue;

            if (visual && bgType == 20)
                continue;

            if ((time(nullptr) - bot->GetInGameTime()) < 30)
                continue;

            if (bot->getLevel() < bg->GetMinLevel())
                continue;

            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracket_temp = pvpDiff->GetBracketId();
            if (bracket_temp != bracketId)
                continue;

            if (bot->GetPlayerbotAI()->GetMaster() && !bot->GetPlayerbotAI()->GetMaster()->GetPlayerbotAI())
                continue;

            if (bot->GetGroup())
                continue;

            if (bot->IsInCombat())
                continue;

            if (bot->InBattlegroundQueue())
                continue;

            if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_WAIT_LEAVE)
                continue;

            if (!bot->GetBGAccessByLevel(bgTypeId))
                continue;

            if (bot->GetPlayerbotAI()->IsHeal(bot) && urand(0, 5))
                continue;

            // add only x2 - x9 level
            //if (!visual && bracketId < BG_BRACKET_ID_LAST && (bot->getLevel() < ((bracketId * 10) + 12)))
            //	continue;

            if (player == nullptr)
            {
                player = bot;
                break;
            }
        }
    }

    if (isRated)
    {
        bool found_team = true;
        uint8 type = BattlegroundMgr::BGArenaType(queueTypeId);
        std::vector<uint32> aTeams;
        uint32 rating = Rating[queueTypeId][bracketId][1];
        for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotArenaTeams.begin(); i != sPlayerbotAIConfig->randomBotArenaTeams.end(); ++i)
        {
            ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamById(*i);
            if (!arenateam)
                continue;

            if (arenateam->GetType() != type)
                continue;

            //if ((rating - arenateam->GetRating()) > 150)
            //    continue;

            aTeams.push_back(arenateam->GetId());
        }

        if (aTeams.empty())
        {
            found_team = false;
            LOG_INFO("playerbots", "No teams found for %s match (Rating:%d)", _bgType.c_str(), rating);
        }

        ArenaTeam* arenateam;
        std::vector<uint32> members;
        uint32 maxPlayers = type;
        if (found_team)
        {
            uint32 index = urand(0, aTeams.size() - 1);
            uint32 arenaTeamId = aTeams[index];
            uint32 count = 0;
            arenateam = sArenaTeamMgr->GetArenaTeamById(arenaTeamId);
            if (arenateam)
            {
                for (ArenaTeam::MemberList::iterator itr = arenateam->GetMembers().begin(); itr != arenateam->GetMembers().end(); ++itr)
                {
                    if (count >= maxPlayers)
                        break;

                    bool offline = false;
                    Player* member = ObjectAccessor::FindPlayer(itr->Guid);
                    if (!member)
                    {
                        offline = true;
                    }
                    if (!member && !sObjectMgr->GetPlayerAccountIdByGUID(itr->Guid.GetCounter()))
                        continue;

                    if (offline)
                        sRandomPlayerbotMgr->AddPlayerBot(itr->Guid, 0);

                    //if (member->GetArenaPersonalRating(arenateam->GetSlot()) < (rating - 150))
                    //    continue;

                    if (member->getLevel() < 70)
                        continue;

                    members.push_back(member->GetGUID().GetCounter());
                    count++;
                }
            }

            if (!members.size() || members.size() < maxPlayers)
            {
                found_team = false;
                LOG_INFO("playerbots", "Team #%d <%s> has no members for %s match, skipping", arenaTeamId, arenateam->GetName().c_str(), _bgType.c_str());
            }
        }

        if (found_team && arenateam)
        {
            uint32 count = 0;
            ObjectGuid capGuid = arenateam->GetCaptain();
            Player* cap = ObjectAccessor::FindPlayer(capGuid);
            if (!cap)
                sRandomPlayerbotMgr->AddPlayerBot(capGuid, 0);

            cap = ObjectAccessor::FindPlayer(capGuid);

            bool found_cap = false;
            if (!cap)
            {
                uint32 mem_guid = urand(0, members.size() - 1);
                capGuid = ObjectGuid::Create<HighGuid::Player>(members[mem_guid]);
                cap = ObjectAccessor::FindPlayer(capGuid);

                if (!cap)
                    sRandomPlayerbotMgr->AddPlayerBot(capGuid, 0);

                cap = ObjectAccessor::FindPlayer(capGuid);
            }

            if (cap)
            {
                found_cap = true;

                if (cap->getLevel() < 70)
                    found_cap = false;

                if (cap->GetPlayerbotAI()->GetMaster())
                    found_cap = false;

                if (cap->GetGroup())
                    found_cap = false;

                if (cap->IsInCombat())
                    found_cap = false;

                if (cap->InBattlegroundQueue())
                    found_cap = false;

                uint32 bgType = cap->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("bg type")->Get();
                if (bgType && bgType != 20)
                    found_cap = false;

                if (visual && bgType == 20)
                    found_cap = false;
            }

            if (cap && found_cap)
            {
                LOG_INFO("playerbots", "Bot %s <%s>: captian of <%s> %s",
                    cap->GetGUID().ToString().c_str(), cap->GetName().c_str(), arenateam->GetName().c_str(), _bgType.c_str());

                Group* group = new Group();
                group->Create(cap);
                count = 0;
                for (auto i = begin(members); i != end(members); ++i)
                {
                    if (*i == cap->GetGUID().GetCounter())
                        continue;

                    if (count >= maxPlayers)
                        break;

                    Player* member = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(*i));
                    if (!member)
                        continue;

                    if (member->getLevel() < 70)
                        continue;

                    if (!group->AddMember(member))
                        continue;

                    LOG_INFO("playerbots", "Bot %s <%s>: member of <%s> %s",
                        member->GetGUID().ToString().c_str(), member->GetName().c_str(), arenateam->GetName().c_str(), _bgType.c_str());

                    count++;
                }

                if (cap && cap->getLevel() >= 70)
                    player = cap;
            }
        }
    }

    if (!player)
    {
        if (visual)
            LOG_INFO("playerbots", "No Bots found to simulate waiting for %s (%s)", bgType.c_str(), _bgType.c_str());
        else
            LOG_INFO("playerbots", "No Bots found for %s (%s)", bgType.c_str(), _bgType.c_str());

        return;
    }

    if (!visual)
        LOG_INFO("playerbots", "Found bot %s <%s> for %s %s",
            player->GetGUID().ToString().c_str(), player->GetName().c_str(), bgType.c_str(), _bgType.c_str());

    uint32 bot = player->GetGUID().GetCounter();
    PlayerbotAI* botAI = player->GetPlayerbotAI();
    TeamId team = player->GetTeamId();

    if (!isArena && ((ACount >= TeamSize && team == TEAM_ALLIANCE) || (HCount >= TeamSize && team == TEAM_HORDE)))
    {
        LOG_INFO("playerbots", "Can't add this bot to %s %s, BG queue for this faction is full", bgType.c_str(), _bgType.c_str());
        return;
    }

    if (isArena && (((ACount >= TeamSize && HCount > 0) && team == TEAM_ALLIANCE) || ((HCount >= TeamSize && ACount > 0) && team == TEAM_HORDE)))
    {
        LOG_INFO("playerbots", "Can't add this bot to %s %s, Arena queue for this faction is full", bgType.c_str(), _bgType.c_str());
        return;
    }

    if (isArena && (((ACount > TeamSize && HCount == 0) && team == TEAM_HORDE) || ((HCount > TeamSize && ACount == 0) && team == TEAM_ALLIANCE)))
    {
        LOG_INFO("playerbots", "Can't add this bot to %s %s, Arena queue for this faction is full", bgType.c_str(), _bgType.c_str());
        return;
    }

    if (isArena && ((!isRated && SCount >= BracketSize) || (!isRated && RCount >= BracketSize)))
    {
        LOG_INFO("playerbots", "Can't add this bot to %s %s, Arena queue is full", bgType.c_str(), _bgType.c_str());
        return;
    }

    if (uint8 type = BattlegroundMgr::BGArenaType(queueTypeId))
    {
        isArena = true;
        BracketSize = type * 2;
        TeamSize = type;
        uint32 BgCount = BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE] + BgPlayers[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE];
    }

    if (!visual)
        LOG_INFO("playerbots", "Bot %s <%s> (%d %s) : adding %s (%s) Bot",
            player->GetGUID().ToString().c_str(), player->GetName().c_str(), player->getLevel(), team == TEAM_ALLIANCE ? "A" : "H", bgType.c_str(), _bgType.c_str());

    team == TEAM_ALLIANCE ? ACount++ : HCount++;

    if (!visual)
    {
        LOG_INFO("playerbots", "Changing strategy for bot #%d <%s> to PVP", bot, player->GetName().c_str());
        LOG_INFO("playerbots", "Bot #%d <%s> (%d %s) %s bracket %d sent to BattmeMaster",
            bot, player->GetName().c_str(), player->getLevel(), team == TEAM_ALLIANCE ? "A" : "H", bgType.c_str(), bracketId);
        LOG_INFO("playerbots", "Bot #%d <%s> (%d %s): %s %d (%s), bracket %d (%d/%d) (A:%d H:%d)",
            bot, player->GetName().c_str(), player->getLevel(), team == TEAM_ALLIANCE ? "A" : "H", bgType.c_str(), bgTypeId, _bgType.c_str(), bracketId, BgCount + 1, BracketSize, ACount, HCount);

        // BG Tactics preference
        player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(urand(0, 9));
    }
    else
    {
        LOG_INFO("playerbots", "Bot #%d <%s> (%d %s) simulates waiting %s %d (%s) bracket %d",
            bot, player->GetName().c_str(), player->getLevel(), team == TEAM_ALLIANCE ? "A" : "H", bgType.c_str(), bgTypeId, _bgType.c_str(), bracketId);
    }

    PlayerbotAI* pai = player->GetPlayerbotAI();
    AiObjectContext* context = pai->GetAiObjectContext();
    CreatureData const* bm = AI_VALUE2(CreatureData const*, "bg master", bgTypeId);

    // if found entry
    if (bm)
    {
        if (player->HasUnitState(UNIT_STATE_IN_FLIGHT))
        {
            player->GetMotionMaster()->MovementExpired();
            player->m_taxi.ClearTaxiDestinations();
        }

        //if (BattlegroundMgr::BGArenaType(queueTypeId))
            //player->TeleportTo(bm->GetMapId(), bm->GetPositionX(), bm->GetPositionY(), bm->GetPositionZ(), player->GetOrientation());
        //else
           // RandomTeleportForRpg(player);
        // subject for removal
    }

    if (!bm && isArena)
    {
        if (!visual)
            LOG_ERROR("playerbots", "Bot %s <%s> could not find Battlemaster for %s %d (%s) bracket %d",
                player->GetGUID().ToString().c_str(), player->GetName().c_str(), bgType.c_str(), bgTypeId, _bgType.c_str(), bracketId);

        return;
    }
    else
    {
        //player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<ObjectGuid>("bg type")->Set(0);
        // subject for removal
    }

    player->GetPlayerbotAI()->ChangeStrategy("-travel,-grind,-rpg,-custom::say", BOT_STATE_NON_COMBAT);

    if (urand(0, 100) < 65)
        player->GetPlayerbotAI()->ChangeStrategy("+stay", BOT_STATE_NON_COMBAT);

    player->GetPlayerbotAI()->ChangeStrategy("-mount", BOT_STATE_NON_COMBAT);

    if (visual)
    {
        VisualBots[queueTypeId][bracketId][team]++;

        if (urand(0, 5) < 3)
            player->GetPlayerbotAI()->ChangeStrategy("+rpg,-stay", BOT_STATE_NON_COMBAT);

        player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(10);

        return;
    }

    Refresh(player);
    if (isArena)
    {
        if (isRated)
        {
            BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE] += TeamSize;
            ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][team] += TeamSize;
        }
        else
        {
            BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE]++;
            ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][team]++;
        }
    }
    else
        BgBots[queueTypeId][bracketId][team]++;

    player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(queueTypeId);

    if (isArena)
        player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(isRated);

    if (player->GetPlayerbotAI()->GetMaster() && player->GetPlayerbotAI()->GetMaster()->GetPlayerbotAI())
        player->GetPlayerbotAI()->DoSpecificAction("leave");

    player->GetPlayerbotAI()->ChangeStrategy("+bg", BOT_STATE_NON_COMBAT);
    //player->GetPlayerbotAI()->DoSpecificAction("bg join");
}

void RandomPlayerbotMgr::ScheduleRandomize(uint32 bot, uint32 time)
{
    SetEventValue(bot, "randomize", 1, time);
    //SetEventValue(bot, "logout", 1, time + 30 + urand(sPlayerbotAIConfig->randomBotUpdateInterval, sPlayerbotAIConfig->randomBotUpdateInterval * 3));
}

void RandomPlayerbotMgr::ScheduleTeleport(uint32 bot, uint32 time)
{
    if (!time)
        time = 60 + urand(sPlayerbotAIConfig->randomBotUpdateInterval, sPlayerbotAIConfig->randomBotUpdateInterval * 3);

    SetEventValue(bot, "teleport", 1, time);
}

void RandomPlayerbotMgr::ScheduleChangeStrategy(uint32 bot, uint32 time)
{
    if (!time)
        time = urand(sPlayerbotAIConfig->minRandomBotChangeStrategyTime, sPlayerbotAIConfig->maxRandomBotChangeStrategyTime);

    SetEventValue(bot, "change_strategy", 1, time);
}

bool RandomPlayerbotMgr::ProcessBot(uint32 bot)
{
    ObjectGuid botGUID = ObjectGuid::Create<HighGuid::Player>(bot);

    Player* player = GetPlayerBot(botGUID);
    PlayerbotAI* botAI = player ? player->GetPlayerbotAI() : nullptr;

    uint32 isValid = GetEventValue(bot, "add");
    if (!isValid)
    {
		if (!player || !player->GetGroup())
		{
            if (player)
                LOG_INFO("playerbots", "Bot #%d %s:%d <%s>: log out", bot, IsAlliance(player->getRace()) ? "A" : "H", player->getLevel(), player->GetName().c_str());
            else
                LOG_INFO("playerbots", "Bot #%d: log out", bot);

			SetEventValue(bot, "add", 0, 0);
			currentBots.erase(std::remove(currentBots.begin(), currentBots.end(), bot), currentBots.end());

			if (player)
                LogoutPlayerBot(botGUID);
		}

        return true;
    }

    uint32 isLogginIn = GetEventValue(bot, "login");
    if (isLogginIn)
        return false;

    if (!player)
    {
        if (urand(0, 100) > 50) // less lag during bots login
        {
            return true;
        }

        AddPlayerBot(botGUID, 0);
        SetEventValue(bot, "login", 1, sPlayerbotAIConfig->randomBotUpdateInterval);

        ChangeStrategy(player);

        uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotReviveTime, sPlayerbotAIConfig->maxRandomBotReviveTime * 5);
        SetEventValue(bot, "update", 1, randomTime);

        return true;
    }

    SetEventValue(bot, "login", 0, 0);

    if (player->GetGroup() || player->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    uint32 update = GetEventValue(bot, "update");
    if (!update)
    {
        if (botAI)
            botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);

        uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotReviveTime, sPlayerbotAIConfig->maxRandomBotReviveTime);
        SetEventValue(bot, "update", 1, randomTime);

        return true;
    }

    uint32 logout = GetEventValue(bot, "logout");
    if (player && !logout && !isValid)
    {
        LOG_INFO("playerbots", "Bot #%d %s:%d <%s>: log out", bot, IsAlliance(player->getRace()) ? "A" : "H", player->getLevel(), player->GetName().c_str());
        LogoutPlayerBot(botGUID);
        SetEventValue(bot, "logout", 1, urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime));
        return true;
    }

    return false;
}

bool RandomPlayerbotMgr::ProcessBot(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (player->InBattleground())
        return false;

    if (player->InBattlegroundQueue())
        return false;

    if (player->isDead())
        return false;

    /*if (player->isDead())
    {
        if (!GetEventValue(bot, "dead"))
        {
            uint32 deathcount = GetEventValue(bot, "deathcount");
            if (!deathcount)
            {
                SetEventValue(bot, "deathcount", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
                Revive(player);
                LOG_INFO("playerbots", "Bot #%d <%s>: revived", bot, player->GetName().c_str());
            }
            else
            {
                if (deathcount > 4)
                {
                    SetEventValue(bot, "deathcount", 0, 0);
                    SetEventValue(bot, "dead", 0, 0);
                    SetEventValue(bot, "revive", 0, 0);
                    //Refresh(player);
                    RandomTeleportForRpg(player);
                    uint32 randomChange = urand(240 + sPlayerbotAIConfig->randomBotUpdateInterval, 600 + sPlayerbotAIConfig->randomBotUpdateInterval * 3);
                    ScheduleChangeStrategy(bot, randomChange);
                    SetEventValue(bot, "teleport", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
                    uint32 restTime = int(randomChange / 60);
                    LOG_INFO("playerbots", "Bot #%d <%s>: died %d times, rest %d min", bot, player->GetName().c_str(), deathcount, int(randomChange / 60));
                }
                else
                {
                    SetEventValue(bot, "deathcount", deathcount + 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
                    Revive(player);
                    LOG_INFO("playerbots", "Bot #%d <%s>: revived %d/5", bot, player->GetName().c_str(), deathcount + 1);
                }
            }

            return false; // increase revive rate
        }
    }*/

    if (urand(0, 100) > 25) // move optimisation to the next step
    {
        return true;
    }

    player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(false);

    bool randomiser = true;
    if (player->GetGuildId())
    {
        if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
        {
            if (guild->GetLeaderGUID() == player->GetGUID())
            {
                for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
                    sGuildTaskMgr->Update(*i, player);
            }

            uint32 accountId = sObjectMgr->GetPlayerAccountIdByGUID(guild->GetLeaderGUID().GetCounter());
            if (!sPlayerbotAIConfig->IsInRandomAccountList(accountId))
            {
                uint8 rank = player->GetRank();
                randomiser = rank < 4 ? false : true;
            }
        }
    }

    uint32 randomize = GetEventValue(bot, "randomize");
    if (!randomize)
    {
        if (randomiser)
            Randomize(player);

        if (randomiser)
        {
            LOG_INFO("playerbots", "Bot #%d <%s>: randomized", bot, player->GetName().c_str());
        }
        else
        {
            LOG_INFO("playerbots", "Bot #%d %s <%s>: consumables refreshed", bot, player->GetName().c_str(), sGuildMgr->GetGuildById(player->GetGuildId())->GetName());
        }

        uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
        ScheduleRandomize(bot, randomTime);
        return true;
    }

    uint32 teleport = GetEventValue(bot, "teleport");
    if (!teleport)
    {
        LOG_INFO("playerbots", "Bot #%d <%s>: sent to grind", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
        Refresh(player);
        SetEventValue(bot, "teleport", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
        return true;
    }

    uint32 changeStrategy = GetEventValue(bot, "change_strategy");
    if (!changeStrategy)
    {
        LOG_INFO("playerbots", "Changing strategy for bot  #%d <%s>", bot, player->GetName().c_str());
        ChangeStrategy(player);
        return true;
    }

    return false;
}

void RandomPlayerbotMgr::Revive(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    //LOG_INFO("playerbots", "Bot %d revived", bot);
    SetEventValue(bot, "dead", 0, 0);
    SetEventValue(bot, "revive", 0, 0);

    if (player->getDeathState() == CORPSE)
        RandomTeleport(player);
    else
    {
        RandomTeleportForLevel(player);
        Refresh(player);
    }
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth)
{
    if (bot->IsBeingTeleported())
        return;

    if (bot->InBattleground())
        return;

    if (bot->InBattlegroundQueue())
        return;

    if (bot->getLevel() < 5)
        return;

    if (sPlayerbotAIConfig->randomBotRpgChance < 0)
        return;

    if (locs.empty())
    {
        LOG_ERROR("playerbots", "Cannot teleport bot %s - no locations available", bot->GetName().c_str());
        return;
    }

    std::vector<WorldPosition> tlocs;
    for (auto& loc : locs)
        tlocs.push_back(WorldPosition(loc));

    //Do not teleport to maps disabled in config
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](WorldPosition l)
    {
        std::vector<uint32>::iterator i = find(sPlayerbotAIConfig->randomBotMaps.begin(), sPlayerbotAIConfig->randomBotMaps.end(), l.getMapId());
        return i == sPlayerbotAIConfig->randomBotMaps.end();
    }), tlocs.end());


    //Random shuffle based on distance. Closer distances are more likely (but not exclusivly) to be at the begin of the list.
    tlocs = sTravelMgr->getNextPoint(WorldPosition(bot), tlocs, 0);

    // 5% + 0.1% per level chance node on different map in selection.
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](WorldLocation const& l)
    {
        return l.GetMapId() != bot->GetMapId() && urand(1, 100) > 5 + 0.1 * bot->getLevel();
    }), tlocs.end());


    // Continent is about 20.000 large
    // Bot will travel 0-5000 units + 75-150 units per level.
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](WorldLocation const& l)
    {
        return sServerFacade->GetDistance2d(bot, l.GetPositionX(), l.GetPositionY()) > urand(0, 5000) + bot->getLevel() * 15 * urand(5, 10);
    }), tlocs.end());

    if (tlocs.empty())
    {
        LOG_ERROR("playerbots", "Cannot teleport bot %s - no locations available", bot->GetName().c_str());
        return;
    }

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomTeleportByLocations");

    uint32 index = 0;
    for (uint32 i = 0; i < tlocs.size(); i++)
    {
        for (uint8 attemtps = 0; attemtps < 3; ++attemtps)
        {
            WorldLocation loc = tlocs[i].getLocation();

            float x = loc.GetPositionX() + (attemtps > 0 ? urand(0, sPlayerbotAIConfig->grindDistance) - sPlayerbotAIConfig->grindDistance / 2 : 0);
            float y = loc.GetPositionY() + (attemtps > 0 ? urand(0, sPlayerbotAIConfig->grindDistance) - sPlayerbotAIConfig->grindDistance / 2 : 0);
            float z = loc.GetPositionZ();

            Map* map = sMapMgr->FindMap(loc.GetMapId(), 0);
            if (!map)
                continue;

            AreaTableEntry const* area = sAreaTableStore.LookupEntry(map->GetAreaId(x, y, z));
            if (!area)
                continue;

            // Do not teleport to enemy zones if level is low
            if (area->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE && bot->getLevel() < 40)
                continue;

            if (area->team == 2 && bot->GetTeamId() == TEAM_HORDE && bot->getLevel() < 40)
                continue;

            if (map->IsUnderWater(x, y, z) || map->IsInWater(x, y, z))
                continue;

            float ground = map->GetHeight(x, y, z + 0.5f);
            if (ground <= INVALID_HEIGHT)
                continue;

            z = 0.05f + ground;

            LOG_INFO("playerbots", "Random teleporting bot %s to %s %f,%f,%f (%u/%zu locations)",
                bot->GetName().c_str(), area->area_name[0], x, y, z, attemtps, tlocs.size());

            if (hearth)
            {
                bot->SetHomebind(loc, area->ID);
            }

            bot->GetMotionMaster()->Clear();
            bot->TeleportTo(loc.GetMapId(), x, y, z, 0);
            bot->SendMovementFlagUpdate();
            bot->GetPlayerbotAI()->Reset();

            if (pmo)
                pmo->finish();

            return;
        }
    }

    if (pmo)
        pmo->finish();

    LOG_ERROR("playerbots", "Cannot teleport bot %s - no locations available", bot->GetName().c_str());
}

void RandomPlayerbotMgr::PrepareTeleportCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    QueryResult results = PlayerbotDatabase.PQuery("SELECT map_id, x, y, z, level FROM playerbot_tele_cache");
    if (results)
    {
        LOG_INFO("playerbots", "Loading random teleport caches for %d levels...", maxLevel);

        do
        {
            Field* fields = results->Fetch();
            uint32 mapId = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            uint8 level = fields[4].GetUInt8();

            WorldLocation loc(mapId, x, y, z, 0);
            locsPerLevelCache[level].push_back(std::move(loc));
        } while (results->NextRow());
    }
    else
    {
        LOG_INFO("playerbots", "Preparing random teleport caches for %d levels...", maxLevel);

        for (uint8 level = 1; level <= maxLevel; level++)
        {
            QueryResult results = WorldDatabase.PQuery("SELECT map, position_x, position_y, position_z "
                "FROM (SELECT map, position_x, position_y, position_z, AVG(t.maxlevel), AVG(t.minlevel), %u - (AVG(t.maxlevel) + aAVGvg(t.minlevel)) / 2 delta "
                "FROM creature c INNER JOIN creature_template t ON c.id = t.entry WHERE t.NpcFlags = 0 AND t.lootid != 0 AND t.unitFlags != 768 GROUP BY t.entry HAVING COUNT(*) > 1) q "
                "WHERE delta >= 0 AND delta <= %u AND map IN (%s) AND NOT EXISTS (SELECT map, position_x, position_y, position_z FROM "
                "(SELECT map, c.position_x, c.position_y, c.position_z, AVG(t.maxlevel), AVG(t.minlevel), %u - (AVG(t.maxlevel) + AVG(t.minlevel)) / 2 delta "
                "FROM creature c INNER JOIN creature_template t ON c.id = t.entry WHERE t.NpcFlags = 0 AND t.lootid != 0 GROUP BY t.entry) q1 WHERE abs(delta) > %u and q1.map = q.map AND SQRT("
                "(q1.position_x - q.position_x) * (q1.position_x - q.position_x) + (q1.position_y - q.position_y) * (q1.position_y - q.position_y) + "
                "(q1.position_z - q.position_z) * (q1.position_z - q.position_z)) < %u)", level, sPlayerbotAIConfig->randomBotTeleLevel, sPlayerbotAIConfig->randomBotMapsAsString.c_str(),
                level, sPlayerbotAIConfig->randomBotTeleLevel, (uint32)sPlayerbotAIConfig->sightDistance);
            if (results)
            {
                do
                {
                    Field* fields = results->Fetch();
                    uint16 mapId = fields[0].GetUInt16();
                    float x = fields[1].GetFloat();
                    float y = fields[2].GetFloat();
                    float z = fields[3].GetFloat();
                    WorldLocation loc(mapId, x, y, z, 0);
                    locsPerLevelCache[level].push_back(std::move(loc));

                    PlayerbotDatabase.PExecute("INSERT INTO ai_playerbot_tele_cache (level, map_id, x, y, z) VALUES (%u, %u, %f, %f, %f)",
                            level, mapId, x, y, z);
                } while (results->NextRow());
            }
        }
    }

    LOG_INFO("playerbots", "Preparing RPG teleport caches for %d factions...", sFactionTemplateStore.GetNumRows());

    results = WorldDatabase.PQuery("SELECT map, position_x, position_y, position_z, r.race, r.minl, r.maxl FROM creature c INNER JOIN playerbot_rpg_races r ON c.id = r.entry "
        "WHERE r.race < 15");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].GetUInt16();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            uint32 race = fields[4].GetUInt32();
            uint32 minl = fields[5].GetUInt32();
            uint32 maxl = fields[6].GetUInt32();

            for (uint32 level = 1; level < sPlayerbotAIConfig->randomBotMaxLevel + 1; level++)
            {
                if (level > maxl || level < minl)
                    continue;

                WorldLocation loc(mapId, x, y, z, 0);
                for (uint32 r = 1; r < MAX_RACES; r++)
                {
                    if (race == r || race == 0)
                        rpgLocsCacheLevel[r][level].push_back(loc);
                }
            }
        } while (results->NextRow());
    }
}

void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot)
{
    if (bot->InBattleground())
        return;

    uint32 level = bot->getLevel();
    uint8 race = bot->getRace();
    LOG_INFO("playerbots", "Random teleporting bot %s for RPG (%zu locations available)", bot->GetName().c_str(), rpgLocsCacheLevel[race][level].size());
    RandomTeleport(bot, rpgLocsCacheLevel[race][level]);
    Refresh(bot);
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot)
{
    if (bot->InBattleground())
        return;

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomTeleport");
    std::vector<WorldLocation> locs;

    std::list<Unit*> targets;
    float range = sPlayerbotAIConfig->randomBotTeleportDistance;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    if (!targets.empty())
    {
        for (Unit* unit : targets)
        {
            bot->UpdatePosition(*unit);
            FleeManager manager(bot, sPlayerbotAIConfig->sightDistance, 0, true);
            float rx, ry, rz;
            if (manager.CalculateDestination(&rx, &ry, &rz))
            {
                WorldLocation loc(bot->GetMapId(), rx, ry, rz);
                locs.push_back(loc);
            }
        }
    }
    else
    {
        RandomTeleportForLevel(bot);
    }

    if (pmo)
        pmo->finish();

    Refresh(bot);
}

void RandomPlayerbotMgr::Randomize(Player* bot)
{
    if (bot->InBattleground())
        return;

    if (bot->getLevel() == 1)
        RandomizeFirst(bot);
    else if (bot->getLevel() == 55 && bot->getClass() == CLASS_DEATH_KNIGHT)
        RandomizeFirst(bot);
    else
        IncreaseLevel(bot);

    RandomTeleportForRpg(bot);
}

void RandomPlayerbotMgr::IncreaseLevel(Player* bot)
{
	uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
	if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
		maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

	PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "IncreaseLevel");
	uint32 lastLevel = GetValue(bot, "level");
	uint32 level = bot->getLevel();
	if (lastLevel != level)
	{
        PlayerbotFactory factory(bot, level);
        factory.Randomize(true);
	}

    if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::RandomizeFirst(Player* bot)
{
	uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
	if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
		maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    // if lvl sync is enabled, max level is limited by online players lvl
    if (sPlayerbotAIConfig->syncLevelWithPlayers)
        maxLevel = std::max(sPlayerbotAIConfig->randomBotMinLevel, std::min(playersLevel, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)));

	PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomizeFirst");
    uint32 level = urand(sPlayerbotAIConfig->randomBotMinLevel, maxLevel);

    if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomBotMaxLevelChance)
        level = maxLevel;

    if (bot->getClass() == CLASS_DEATH_KNIGHT)
        level = urand(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL), std::max(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL), maxLevel));

    SetValue(bot, "level", level);

    PlayerbotFactory factory(bot, level);
    factory.Randomize(false);

    uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
    uint32 inworldTime = urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime);
    PlayerbotDatabase.PExecute("UPDATE playerbot_random_bots SET validIn = '%u' WHERE event = 'randomize' AND bot = '%u'", randomTime, bot->GetGUID().GetCounter());
    PlayerbotDatabase.PExecute("UPDATE playerbot_random_bots SET validIn = '%u' WHERE event = 'logout' AND bot = '%u'", inworldTime, bot->GetGUID().GetCounter());

	if (pmo)
        pmo->finish();
}

uint32 RandomPlayerbotMgr::GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ)
{
	uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    uint32 level = 0;
    QueryResult results = WorldDatabase.PQuery("SELECT AVG(t.minlevel) minlevel, AVG(t.maxlevel) maxlevel FROM creature c "
            "INNER JOIN creature_template t ON c.id = t.entry WHERE map = '%u' AND minlevel > 1 AND ABS(position_x - '%f') < '%u' AND ABS(position_y - '%f') < '%u'",
            mapId, teleX, sPlayerbotAIConfig->randomBotTeleportDistance / 2, teleY, sPlayerbotAIConfig->randomBotTeleportDistance / 2);

    if (results)
    {
        Field* fields = results->Fetch();
        uint8 minLevel = fields[0].GetUInt8();
        uint8 maxLevel = fields[1].GetUInt8();
        level = urand(minLevel, maxLevel);
        if (level > maxLevel)
            level = maxLevel;
    }
    else
    {
        level = urand(1, maxLevel);
    }

    return level;
}

void RandomPlayerbotMgr::Refresh(Player* bot)
{
    if (bot->InBattleground())
        return;

    LOG_INFO("playerbots", "Refreshing bot %s <%s>", bot->GetGUID().ToString().c_str(), bot->GetName().c_str());

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "Refresh");

    if (bot->isDead())
    {
        bot->ResurrectPlayer(1.0f);
        bot->SpawnCorpseBones();
        bot->GetPlayerbotAI()->ResetStrategies(false);
    }

    bot->GetPlayerbotAI()->Reset();

    bot->DurabilityRepairAll(false, 1.0f, false);
	bot->SetFullHealth();
	bot->SetPvP(true);

    PlayerbotFactory factory(bot, bot->getLevel());
    factory.Refresh();

    if (bot->GetMaxPower(POWER_MANA) > 0)
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    if (bot->GetMaxPower(POWER_ENERGY) > 0)
        bot->SetPower(POWER_ENERGY, bot->GetMaxPower(POWER_ENERGY));

    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 500 * sqrt(urand(1, bot->getLevel() * 5)));

    if (pmo)
        pmo->finish();
}

bool RandomPlayerbotMgr::IsRandomBot(Player* bot)
{
    if (bot)
        return IsRandomBot(bot->GetGUID().GetCounter());

    return false;
}

bool RandomPlayerbotMgr::IsRandomBot(uint32 bot)
{
    return GetEventValue(bot, "add");
}

std::vector<uint32> RandomPlayerbotMgr::GetBots()
{
    if (!currentBots.empty())
        return currentBots;

    QueryResult results = PlayerbotDatabase.Query("SELECT bot FROM playerbot_random_bots WHERE owner = 0 AND event = 'add'");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            currentBots.push_back(bot);
        } while (results->NextRow());
    }

    return currentBots;
}

std::vector<uint32> RandomPlayerbotMgr::GetBgBots(uint32 bracket)
{
    //if (!currentBgBots.empty()) return currentBgBots;

    std::vector<uint32> BgBots;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT bot FROM playerbot_random_bots WHERE event = 'bg' AND value = '%d'", bracket);
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            BgBots.push_back(bot);
        }
        while (results->NextRow());
    }

    return std::move(BgBots);
}

uint32 RandomPlayerbotMgr::GetEventValue(uint32 bot, std::string const& event)
{
    CachedEvent& e = eventCache[bot][event];
    if (e.IsEmpty())
    {
        QueryResult results = PlayerbotDatabase.PQuery("SELECT `value`, `time`, validIn FROM playerbot_random_bots WHERE owner = 0 AND bot = '%u' AND event = '%s'",
                bot, event.c_str());

        if (results)
        {
            Field* fields = results->Fetch();
            e.value = fields[0].GetUInt32();
            e.lastChangeTime = fields[1].GetUInt32();
            e.validIn = fields[2].GetUInt32();
        }
    }

    if ((time(nullptr) - e.lastChangeTime) >= e.validIn && (event == "add" || IsRandomBot(bot)))
        e.value = 0;

    return e.value;
}

uint32 RandomPlayerbotMgr::SetEventValue(uint32 bot, std::string const& event, uint32 value, uint32 validIn)
{
    PlayerbotDatabase.PExecute("DELETE FROM playerbot_random_bots WHERE owner = 0 AND bot = '%u' AND event = '%s'", bot, event.c_str());
    if (value)
    {
        PlayerbotDatabase.PExecute("INSERT INTO playerbot_random_bots (owner, bot, `time`, validIn, event, `value`) VALUES ('%u', '%u', '%u', '%u', '%s', '%u')",
                0, bot, (uint32)time(nullptr), validIn, event.c_str(), value);
    }

    CachedEvent e(value, (uint32)time(nullptr), validIn);
    eventCache[bot][event] = std::move(e);
    return value;
}

uint32 RandomPlayerbotMgr::GetValue(uint32 bot, std::string const& type)
{
    return GetEventValue(bot, type);
}

uint32 RandomPlayerbotMgr::GetValue(Player* bot, std::string const& type)
{
    return GetValue(bot->GetGUID().GetCounter(), type);
}

void RandomPlayerbotMgr::SetValue(uint32 bot, std::string const& type, uint32 value)
{
    SetEventValue(bot, type, value, sPlayerbotAIConfig->maxRandomBotInWorldTime);
}

void RandomPlayerbotMgr::SetValue(Player* bot, std::string const& type, uint32 value)
{
    SetValue(bot->GetGUID().GetCounter(), type, value);
}

bool RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->enabled)
    {
        LOG_ERROR("playerbots", "Playerbot system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        LOG_ERROR("playerbots", "Usage: rndbot stats/update/reset/init/refresh/add/remove");
        return false;
    }

    std::string const& cmd = args;

    if (cmd == "reset")
    {
        PlayerbotDatabase.PExecute("DELETE FROM playerbot_random_bots");
        sRandomPlayerbotMgr->eventCache.clear();
        LOG_INFO("playerbots", "Random bots were reset for all players. Please restart the Server.");
        return true;
    }

    if (cmd == "stats")
    {
        activatePrintStatsThread();
        return true;
    }

    if (cmd == "update")
    {
        sRandomPlayerbotMgr->UpdateAIInternal(0);
        return true;
    }

    std::map<std::string, ConsoleCommandHandler> handlers;
    handlers["init"] = &RandomPlayerbotMgr::RandomizeFirst;
    handlers["levelup"] = handlers["level"] = &RandomPlayerbotMgr::IncreaseLevel;
    handlers["refresh"] = &RandomPlayerbotMgr::Refresh;
    handlers["teleport"] = &RandomPlayerbotMgr::RandomTeleportForLevel;
    handlers["rpg"] = &RandomPlayerbotMgr::RandomTeleportForRpg;
    handlers["revive"] = &RandomPlayerbotMgr::Revive;
    handlers["grind"] = &RandomPlayerbotMgr::RandomTeleport;
    handlers["change_strategy"] = &RandomPlayerbotMgr::ChangeStrategy;

    for (std::map<std::string, ConsoleCommandHandler>::iterator j = handlers.begin(); j != handlers.end(); ++j)
    {
        std::string const& prefix = j->first;
        if (cmd.find(prefix) != 0)
            continue;

        std::string const& name = cmd.size() > prefix.size() + 1 ? cmd.substr(1 + prefix.size()) : "%";

        std::vector<uint32> botIds;
        for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin(); i != sPlayerbotAIConfig->randomBotAccounts.end(); ++i)
        {
            uint32 account = *i;
            if (QueryResult results = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account = '%u' AND name like '%s'", account, name.c_str()))
            {
                do
                {
                    Field* fields = results->Fetch();

                    uint32 botId = fields[0].GetUInt32();
                    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(botId);
                    Player* bot = ObjectAccessor::FindPlayer(guid);
                    if (!bot)
                        continue;

                    botIds.push_back(botId);
                } while (results->NextRow());
			}
        }

        if (botIds.empty())
        {
            LOG_INFO("playerbots", "Nothing to do");
            return false;
        }

        uint32 processed = 0;
        for (std::vector<uint32>::iterator i = botIds.begin(); i != botIds.end(); ++i)
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(*i);
            Player* bot = ObjectAccessor::FindPlayer(guid);
            if (!bot)
                continue;

            LOG_INFO("playerbots", "[%u/%zu] Processing command '%s' for bot '%s'", processed++, botIds.size(), cmd.c_str(), bot->GetName().c_str());

            ConsoleCommandHandler handler = j->second;
            (sRandomPlayerbotMgr->*handler)(bot);
        }

        return true;
    }

    std::vector<std::string> messages = sRandomPlayerbotMgr->HandlePlayerbotCommand(args);
    for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        LOG_INFO("playerbots", "%s", i->c_str());
    }
    return true;
}

void RandomPlayerbotMgr::HandleCommand(uint32 type, std::string const& text, Player* fromPlayer)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleCommand(type, text, fromPlayer);
    }
}

void RandomPlayerbotMgr::OnPlayerLogout(Player* player)
{
    DisablePlayerBot(player->GetGUID());

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = bot->GetPlayerbotAI();
        if (player == botAI->GetMaster())
        {
            botAI->SetMaster(nullptr);
            if (!bot->InBattleground())
            {
                botAI->ResetStrategies();
                botAI->ChangeStrategy("-rpg", BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy("-grind", BOT_STATE_NON_COMBAT);
            }
        }
    }

    std::vector<Player*>::iterator i = std::find(players.begin(), players.end(), player);
    if (i != players.end())
        players.erase(i);
}

void RandomPlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    LOG_INFO("playerbots", "%zu/%d Bot %s logged in", playerBots.size(), sRandomPlayerbotMgr->GetMaxAllowedBotCount(), bot->GetName().c_str());
}

void RandomPlayerbotMgr::OnPlayerLogin(Player* player)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (player == bot/* || player->GetPlayerbotAI()*/) // TEST
            continue;

        Group* group = bot->GetGroup();
        if (!group)
            continue;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            PlayerbotAI* botAI = bot->GetPlayerbotAI();
            if (member == player && (!botAI->GetMaster() || botAI->GetMaster()->GetPlayerbotAI()))
            {
                if (!bot->InBattleground())
                {
                    botAI->SetMaster(player);
                    botAI->ResetStrategies();

                    if (!player->GetPlayerbotAI())
                    {
                        botAI->ChangeStrategy("-rpg", BOT_STATE_NON_COMBAT);
                        botAI->ChangeStrategy("-grind", BOT_STATE_NON_COMBAT);
                    }

                    botAI->TellMaster("Hello");
                }

                break;
            }
        }
    }

    if (IsRandomBot(player))
    {
        ObjectGuid::LowType guid = player->GetGUID().GetCounter();
        SetEventValue(guid, "login", 0, 0);
    }
    else
    {
        players.push_back(player);
        LOG_DEBUG("playerbots", "Including non-random bot player %s into random bot update", player->GetName().c_str());
    }
}

void RandomPlayerbotMgr::OnPlayerLoginError(uint32 bot)
{
    SetEventValue(bot, "add", 0, 0);
    currentBots.erase(std::remove(currentBots.begin(), currentBots.end(), bot), currentBots.end());
}

Player* RandomPlayerbotMgr::GetRandomPlayer()
{
    if (players.empty())
        return nullptr;

    uint32 index = urand(0, players.size() - 1);
    return players[index];
}

void RandomPlayerbotMgr::PrintStats()
{
    LOG_INFO("playerbots", "%zu Random Bots online", playerBots.size());

    std::map<uint8, uint32> alliance, horde;
    for (uint32 i = 0; i < 10; ++i)
    {
        alliance[i] = 0;
        horde[i] = 0;
    }

    std::map<uint8, uint32> perRace;
    std::map<uint8, uint32> perClass;
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        perRace[race] = 0;
    }

    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        perClass[cls] = 0;
    }

    uint32 dps = 0;
    uint32 heal = 0;
    uint32 tank = 0;
    uint32 active = 0;
    uint32 update = 0;
    uint32 randomize = 0;
    uint32 teleport = 0;
    uint32 changeStrategy = 0;
    uint32 dead = 0;
    uint32 revive = 0;
    uint32 stateCount[MAX_TRAVEL_STATE + 1] = { 0 };
    std::vector<std::pair<Quest const*, int32>> questCount;
    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;
        if (IsAlliance(bot->getRace()))
            ++alliance[bot->getLevel() / 10];
        else
            ++horde[bot->getLevel() / 10];

        ++perRace[bot->getRace()];
        ++perClass[bot->getClass()];

        if (bot->GetPlayerbotAI()->IsActive())
            ++active;

        if (bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<bool>("random bot update")->Get())
            ++update;

        uint32 botId = bot->GetGUID().GetCounter();
        if (!GetEventValue(botId, "randomize"))
            ++randomize;

        if (!GetEventValue(botId, "teleport"))
            ++teleport;

        if (!GetEventValue(botId, "change_strategy"))
            ++changeStrategy;

        if (bot->isDead())
        {
            ++dead;
            if (!GetEventValue(botId, "dead"))
                ++revive;
        }

        uint8 spec = AiFactory::GetPlayerSpecTab(bot);
        switch (bot->getClass())
        {
            case CLASS_DRUID:
                if (spec == 2)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_PALADIN:
                if (spec == 1)
                    ++tank;
                else if (spec == 0)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_PRIEST:
                if (spec != 2)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_SHAMAN:
                if (spec == 2)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_WARRIOR:
                if (spec == 2)
                    ++tank;
                else
                    ++dps;
                break;
            case CLASS_DEATH_KNIGHT:
                if (spec == 0)
                    tank++;
                else
                    dps++;
                break;
            default:
                ++dps;
                break;
        }

        if (TravelTarget* target = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get())
        {
            TravelState state = target->getTravelState();
            stateCount[state]++;

            Quest const* quest;
            if (target->getDestination())
                quest = target->getDestination()->GetQuestTemplate();

            if (quest)
            {
                bool found = false;

                for (auto& q : questCount)
                {
                    if (q.first != quest)
                        continue;

                    q.second++;
                    found = true;
                }

                if (!found)
                    questCount.push_back(std::make_pair(quest, 1));
            }
        }
    }

    LOG_INFO("playerbots", "Per level:");
	uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
	for (uint8 i = 0; i < 10; ++i)
    {
        if (!alliance[i] && !horde[i])
            continue;

        uint32 from = i*10;
        uint32 to = std::min(from + 9, maxLevel);
        if (!from)
            from = 1;

        LOG_INFO("playerbots", "    %d..%d: %d alliance, %d horde", from, to, alliance[i], horde[i]);
    }

    LOG_INFO("playerbots", "Per race:");
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        if (perRace[race])
            LOG_INFO("playerbots", "    %s: %d", ChatHelper::formatRace(race).c_str(), perRace[race]);
    }

    LOG_INFO("playerbots", "Per class:");
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        if (perClass[cls])
            LOG_INFO("playerbots", "    %s: %d", ChatHelper::formatClass(cls).c_str(), perClass[cls]);
    }

    LOG_INFO("playerbots", "Per role:");
    LOG_INFO("playerbots", "    tank: %d", tank);
    LOG_INFO("playerbots", "    heal: %d", heal);
    LOG_INFO("playerbots", "    dps: %d", dps);

    LOG_INFO("playerbots", "Active bots: %d", active);
    LOG_INFO("playerbots", "Dead bots: %d", dead);
    LOG_INFO("playerbots", "Bots to:");
    LOG_INFO("playerbots", "    update: %d", update);
    LOG_INFO("playerbots", "    randomize: %d", randomize);
    LOG_INFO("playerbots", "    teleport: %d", teleport);
    LOG_INFO("playerbots", "    change_strategy: %d", changeStrategy);
    LOG_INFO("playerbots", "    revive: %d", revive);

    LOG_INFO("playerbots", "Bots travel:");
    LOG_INFO("playerbots", "    travel to pick up quest: %d", stateCount[TRAVEL_STATE_TRAVEL_PICK_UP_QUEST]);
    LOG_INFO("playerbots", "    try to pick up quest: %d", stateCount[TRAVEL_STATE_WORK_PICK_UP_QUEST]);
    LOG_INFO("playerbots", "    travel to do quest: %d", stateCount[TRAVEL_STATE_TRAVEL_DO_QUEST]);
    LOG_INFO("playerbots", "    try to do quest: %d", stateCount[TRAVEL_STATE_WORK_DO_QUEST]);
    LOG_INFO("playerbots", "    travel to hand in quest: %d", stateCount[TRAVEL_STATE_TRAVEL_HAND_IN_QUEST]);
    LOG_INFO("playerbots", "    try to hand in quest: %d", stateCount[TRAVEL_STATE_WORK_HAND_IN_QUEST]);
    LOG_INFO("playerbots", "    idling: %d", stateCount[TRAVEL_STATE_IDLE]);

    sort(questCount.begin(), questCount.end(), [](std::pair<Quest const*, int32> i, std::pair<Quest const*, int32> j) { return i.second > j.second; });

    LOG_INFO("playerbots", "Bots top quests:");

    uint32 cnt = 0;
    for (auto& quest : questCount)
    {
        LOG_INFO("playerbots", "    [%d]: %s (%d)", quest.second, quest.first->GetTitle().c_str(), quest.first->GetQuestLevel());
        cnt++;
        if (cnt > 25)
            break;
    }
}

double RandomPlayerbotMgr::GetBuyMultiplier(Player* bot)
{
    uint32 id = bot->GetGUID().GetCounter();
    uint32 value = GetEventValue(id, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        uint32 validIn = urand(sPlayerbotAIConfig->minRandomBotsPriceChangeInterval, sPlayerbotAIConfig->maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "buymultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

double RandomPlayerbotMgr::GetSellMultiplier(Player* bot)
{
    uint32 id = bot->GetGUID().GetCounter();
    uint32 value = GetEventValue(id, "sellmultiplier");
    if (!value)
    {
        value = urand(80, 250);
        uint32 validIn = urand(sPlayerbotAIConfig->minRandomBotsPriceChangeInterval, sPlayerbotAIConfig->maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "sellmultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

void RandomPlayerbotMgr::AddTradeDiscount(Player* bot, Player* master, int32 value)
{
    if (!master)
        return;

    uint32 discount = GetTradeDiscount(bot, master);
    int32 result = (int32)discount + value;
    discount = (result < 0 ? 0 : result);

    SetTradeDiscount(bot, master, discount);
}

void RandomPlayerbotMgr::SetTradeDiscount(Player* bot, Player* master, uint32 value)
{
    if (!master)
        return;

    uint32 botId =  bot->GetGUID().GetCounter();
    uint32 masterId =  master->GetGUID().GetCounter();

    std::ostringstream name;
    name << "trade_discount_" << masterId;
    SetEventValue(botId, name.str(), value, sPlayerbotAIConfig->maxRandomBotInWorldTime);
}

uint32 RandomPlayerbotMgr::GetTradeDiscount(Player* bot, Player* master)
{
    if (!master)
        return 0;

    uint32 botId =  bot->GetGUID().GetCounter();
    uint32 masterId = master->GetGUID().GetCounter();

    std::ostringstream name;
    name << "trade_discount_" << masterId;
    return GetEventValue(botId, name.str());
}

std::string RandomPlayerbotMgr::HandleRemoteCommand(std::string const& request)
{
    std::string::const_iterator pos = std::find(request.begin(), request.end(), ',');
    if (pos == request.end())
    {
        std::ostringstream out;
        out << "invalid request: " << request;
        return out.str();
    }

    std::string command = std::string(request.begin(), pos);
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(atoi(std::string(pos + 1, request.end()).c_str()));
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        return "invalid guid";

    PlayerbotAI* botAI = bot->GetPlayerbotAI();
    if (!botAI)
        return "invalid guid";

    return botAI->HandleRemoteCommand(command);
}

void RandomPlayerbotMgr::ChangeStrategy(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (frand(0.f, 100.f) > sPlayerbotAIConfig->randomBotRpgChance)
    {
        LOG_INFO("playerbots", "Changing strategy for #%d <%s> to grinding", bot, player->GetName().c_str());
        ScheduleTeleport(bot, 30);
    }
    else
    {
        LOG_INFO("playerbots", "Changing strategy for bot #%d <%s> to RPG", bot, player->GetName().c_str());
        LOG_INFO("playerbots", "Bot #%d <%s>: sent to inn", bot, player->GetName().c_str());
        RandomTeleportForRpg(player);
        SetEventValue(bot, "teleport", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
    }

    ScheduleChangeStrategy(bot);
}

void RandomPlayerbotMgr::RandomTeleportForRpg(Player* bot)
{
    uint32 race = bot->getRace();
    LOG_INFO("playerbots", "Random teleporting bot %s for RPG (%zu locations available)", bot->GetName().c_str(), rpgLocsCache[race].size());
    RandomTeleport(bot, rpgLocsCache[race], true);
}

void RandomPlayerbotMgr::Remove(Player* bot)
{
    ObjectGuid owner = bot->GetGUID();
    PlayerbotDatabase.PExecute("DELETE FROM playerbot_random_bots WHERE owner = 0 AND bot = '%u'", owner.GetCounter());
    eventCache[owner.GetCounter()].clear();

    LogoutPlayerBot(owner);
}

CreatureData const* RandomPlayerbotMgr::GetCreatureDataByEntry(uint32 entry)
{
    if (CreatureDataByEntrySet const* creatureData = sObjectMgr->GetCreatureDataByEntry(entry))
        return &*creatureData->begin();

    return nullptr;
}

uint32 RandomPlayerbotMgr::GetCreatureGuidByEntry(uint32 entry)
{
    uint32 guid = 0;

    if (CreatureData const* data= sRandomPlayerbotMgr->GetCreatureDataByEntry(entry))
        guid = data->id;

    return guid;
}

uint32 RandomPlayerbotMgr::GetBattleMasterEntry(Player* bot, BattlegroundTypeId bgTypeId, bool fake)
{
    TeamId team = bot->GetTeamId();
    uint32 entry = 0;
    std::vector<uint32> Bms;

    for (auto i = std::begin(BattleMastersCache[team][bgTypeId]); i != std::end(BattleMastersCache[team][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    for (auto i = std::begin(BattleMastersCache[TEAM_NEUTRAL][bgTypeId]); i != std::end(BattleMastersCache[TEAM_NEUTRAL][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    if (Bms.empty())
        return entry;

    float dist1 = FLT_MAX;

    for (auto i = begin(Bms); i != end(Bms); ++i)
    {
        CreatureData const* data = sRandomPlayerbotMgr->GetCreatureDataByEntry(*i);
        if (!data)
            continue;

        Unit* Bm = PlayerbotAI::GetUnit(data);
        if (!Bm)
            continue;

        if (bot->GetMapId() != Bm->GetMapId())
            continue;

        // return first available guid on map if queue from anywhere
        if (fake)
        {
            entry = *i;
            break;
        }

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(Bm->GetAreaId());
        if (!area)
            continue;

        if (area->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
            continue;

        if (area->team == 2 && bot->GetTeamId() == TEAM_HORDE)
            continue;

        if (Bm->getDeathState() == DEAD)
            continue;

        float dist2 = sServerFacade->GetDistance2d(bot, data->posX, data->posY);
        if (dist2 < dist1)
        {
            dist1 = dist2;
            entry = *i;
        }
    }

    return entry;
}
