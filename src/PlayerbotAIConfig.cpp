/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotAIConfig.h"
#include "Config.h"
#include "Playerbot.h"
#include "PlayerbotFactory.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "TravelMgr.h"

#include <iostream>

template <class T>
void LoadList(std::string const& value, T& list)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        if (!id)
            continue;

        list.push_back(id);
    }
}

template <class T>
void LoadListString(std::string const& value, T& list)
{
    std::vector<string> strings = split(value, ',');
    for (std::vector<string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        std::string const& string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

bool PlayerbotAIConfig::Initialize()
{
    LOG_INFO("playerbots", "Initializing AI Playerbot by ike3, based on the original Playerbot by blueboy");

    if (!sConfigMgr->SetSource(SYSCONFDIR"aiplayerbot.conf"))
    {
        LOG_INFO("playerbots", "AI Playerbot is Disabled. Unable to open configuration file aiplayerbot.conf");
        return false;
    }

    enabled = sConfigMgr->GetBoolDefault("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        LOG_INFO("playerbots", "AI Playerbot is Disabled in aiplayerbot.conf");
        return false;
    }

    globalCoolDown = sConfigMgr->GetIntDefault("AiPlayerbot.GlobalCooldown", 500);
    maxWaitForMove = sConfigMgr->GetIntDefault("AiPlayerbot.MaxWaitForMove", 3000);
    expireActionTime = sConfigMgr->GetIntDefault("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = sConfigMgr->GetIntDefault("AiPlayerbot.DispelAuraDuration", 7000);
    reactDelay = sConfigMgr->GetIntDefault("AiPlayerbot.ReactDelay", 100);
    passiveDelay = sConfigMgr->GetIntDefault("AiPlayerbot.PassiveDelay", 4000);
    repeatDelay = sConfigMgr->GetIntDefault("AiPlayerbot.RepeatDelay", 5000);
    errorDelay = sConfigMgr->GetIntDefault("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = sConfigMgr->GetIntDefault("AiPlayerbot.RpgDelay", 3000);
    sitDelay = sConfigMgr->GetIntDefault("AiPlayerbot.SitDelay", 30000);
    returnDelay = sConfigMgr->GetIntDefault("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = sConfigMgr->GetIntDefault("AiPlayerbot.LootDelayDelay", 750);

    farDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.SightDistance", 75.0f);
    spellDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.SpellDistance", 22.0f);
    shootDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.healDistance", 20.0f);
    lootDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.FleeDistance", 7.5f);
    aggroDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.AggroDistance", 22.0f);
    tooCloseDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.MeleeDistance", 1.5f);
    followDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.FollowDistance", 1.5f);
    whisperDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = sConfigMgr->GetFloatDefault("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.RpgDistance", 80.0f);
    grindDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.GrindDistance", 75.0f);
    reactDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.ReactDistance", 150.0f);

    criticalHealth = sConfigMgr->GetIntDefault("AiPlayerbot.CriticalHealth", 20);
    lowHealth = sConfigMgr->GetIntDefault("AiPlayerbot.LowHealth", 50);
    mediumHealth = sConfigMgr->GetIntDefault("AiPlayerbot.MediumHealth", 70);
    almostFullHealth = sConfigMgr->GetIntDefault("AiPlayerbot.AlmostFullHealth", 85);
    lowMana = sConfigMgr->GetIntDefault("AiPlayerbot.LowMana", 15);
    mediumMana = sConfigMgr->GetIntDefault("AiPlayerbot.MediumMana", 40);

    randomGearLoweringChance = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomGearLoweringChance", 0.15f);
    randomBotMaxLevelChance = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomBotMaxLevelChance", 0.15f);
    randomBotRpgChance = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomBotRpgChance", 0.35f);

    iterationsPerTick = sConfigMgr->GetIntDefault("AiPlayerbot.IterationsPerTick", 100);

    allowGuildBots = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowGuildBots", true);

    randomBotMapsAsString = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<std::vector<uint32> >(randomBotMapsAsString, randomBotMaps);
    LoadList<std::vector<uint32> >(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotQuestItems", "6948,5175,5176,5177,5178,16309,12382,13704,11000"), randomBotQuestItems);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotSpellIds", "1"), randomBotSpellIds);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.PvpProhibitedZoneIds", "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,3539,3623,4152,3988,4658,4284,4418,4436,4275,4323"), pvpProhibitedZoneIds);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotQuestIds", "7848,3802,5505,6502,7761"), randomBotQuestIds);

    botAutologin = sConfigMgr->GetBoolDefault("AiPlayerbot.BotAutologin", false);
    randomBotAutologin = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotAutologin", true);
    minRandomBots = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBots", 200);
    randomBotUpdateInterval = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotUpdateInterval", 60);
    randomBotCountChangeMinInterval = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotCountChangeMinInterval", 24 * 3600);
    randomBotCountChangeMaxInterval = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotCountChangeMaxInterval", 3 * 24 * 3600);
    minRandomBotInWorldTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotInWorldTime", 24 * 3600);
    maxRandomBotInWorldTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotInWorldTime", 14 * 24 * 3600);
    minRandomBotRandomizeTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotRandomizeTime", 2 * 3600);
    maxRandomBotRandomizeTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomRandomizeTime", 14 * 24 * 3600);
    minRandomBotChangeStrategyTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotChangeStrategyTime", 1800);
    maxRandomBotChangeStrategyTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * 3600);
    minRandomBotReviveTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotReviveTime", 60);
    maxRandomBotReviveTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomReviveTime", 300);
    randomBotTeleportDistance = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotTeleportDistance", 1000);
    randomBotsPerInterval = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotsPerInterval", 50);
    minRandomBotsPriceChangeInterval = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * 3600);
    maxRandomBotsPriceChangeInterval = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * 3600);
    randomBotJoinLfg = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotJoinLfg", true); // Ultranix
    randomBotJoinBG = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotJoinBG", true);
    randomBotBracketCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotBracketCount", 3);
    logInGroupOnly = sConfigMgr->GetBoolDefault("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = sConfigMgr->GetBoolDefault("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotMaxLevel", 255);
    randomBotLoginAtStartup = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotLoginAtStartup", true);
    randomBotTeleLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotTeleLevel", 5);
    openGoSpell = sConfigMgr->GetIntDefault("AiPlayerbot.OpenGoSpell", 6477);

    randomChangeMultiplier = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotCombatStrategies", "+dps assist,-threat,+custom::say");
    randomBotNonCombatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotNonCombatStrategies", "+grind,+loot,+rpg,+custom::say");
    combatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.CombatStrategies", "+custom::say");
    nonCombatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.NonCombatStrategies", "+custom::say,+return");

    commandPrefix = sConfigMgr->GetStringDefault("AiPlayerbot.CommandPrefix", "");
    commandSeparator = sConfigMgr->GetStringDefault("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = sConfigMgr->GetIntDefault("AiPlayerbot.CommandServerPort", 0);
    perfMonEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.PerfMonEnabled", false);

    LOG_INFO("playerbots", "---------------------------------------");
    LOG_INFO("playerbots", "          Loading TalentSpecs          ");
    LOG_INFO("playerbots", "---------------------------------------");
    LOG_INFO("playerbots", );

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        classSpecs[cls] = ClassSpecs(1 << (cls - 1));

        for (uint32 spec = 0; spec < MAX_LEVEL; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;

            std::string const& specName = sConfigMgr->GetStringDefault(os.str().c_str(), "");
            if (!specName.empty())
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeSpecProb." << cls << "." << spec;
                uint32 probability = sConfigMgr->GetIntDefault(os.str().c_str(), 100);

                TalentPath talentPath(spec, specName, probability);

                for (uint32 level = 10; level <= 100; level++)
                {
                    std::ostringstream os;
                    os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;

                    std::string specLink = sConfigMgr->GetStringDefault(os.str().c_str(), "");
                    specLink = specLink.substr(0, specLink.find("#", 0));;
                    specLink = specLink.substr(0, specLink.find(" ", 0));;

                    if (!specLink.empty())
                    {
                        std::ostringstream out;

                        //Ignore bad specs.
                        if (!classSpecs[cls].baseSpec.CheckTalentLink(specLink, &out))
                        {
                            LOG_ERROR("playerbots", "Error with premade spec link: %s", specLink.c_str());
                            LOG_ERROR("playerbots", "%s", out.str().c_str());
                            continue;
                        }

                        TalentSpec linkSpec(&classSpecs[cls].baseSpec, specLink);

                        if (!linkSpec.CheckTalents(level, &out))
                        {
                            LOG_ERROR("playerbots", "Error with premade spec: %s", specLink.c_str());
                            LOG_ERROR("playerbots", "%s", out.str().c_str());
                            continue;
                        }


                        talentPath.talentSpec.push_back(linkSpec);
                    }
                }

                // Only add paths that have atleast 1 spec.
                if (talentPath.talentSpec.size() > 0)
                    classSpecs[cls].talentPath.push_back(talentPath);
            }
        }
    }

    LoadListString<std::vector<std::string>>(sConfigMgr->GetStringDefault("AiPlayerbot.AllowedLogFiles", ""), allowedLogFiles);

    worldBuffs.clear();

    for (uint32 factionId = 0; factionId < 3; factionId++)
    {
        for (uint32 classId = 0; classId < MAX_CLASSES; classId++)
        {
            for (uint32 minLevel = 0; minLevel < MAX_LEVEL; minLevel++)
            {
                for (uint32 maxLevel = 0; maxLevel < MAX_LEVEL; maxLevel++)
                {
                    loadWorldBuf(factionId, classId, minLevel, maxLevel);
                }
            }
        }
    }

    randomBotAccountPrefix = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAccountCount", 50);
    deleteRandomBotAccounts = sConfigMgr->GetBoolDefault("AiPlayerbot.DeleteRandomBotAccounts", false);
    randomBotGuildCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotGuildCount", 20);
    deleteRandomBotGuilds = sConfigMgr->GetBoolDefault("AiPlayerbot.DeleteRandomBotGuilds", false);

    guildTaskEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableGuildTasks", true);
    minGuildTaskChangeTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinGuildTaskChangeTime", 3 * 24 * 3600);
    maxGuildTaskChangeTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxGuildTaskChangeTime", 4 * 24 * 3600);
    minGuildTaskAdvertisementTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinGuildTaskAdvertisementTime", 60);
    maxGuildTaskAdvertisementTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxGuildTaskAdvertisementTime", 12 * 3600);
    minGuildTaskRewardTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinGuildTaskRewardTime", 30);
    maxGuildTaskRewardTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxGuildTaskRewardTime", 120);
    guildTaskAdvertCleanupTime = sConfigMgr->GetIntDefault("AiPlayerbot.GuildTaskAdvertCleanupTime", 300);
    guildTaskKillTaskDistance = sConfigMgr->GetIntDefault("AiPlayerbot.GuildTaskKillTaskDistance", 2000);
    targetPosRecalcDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.TargetPosRecalcDistance", 0.1f);

    //cosmetics (by lidocain)
    randomBotShowCloak = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotShowCloak", false);
    randomBotShowHelmet = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotShowHelmet", false);

    //SPP switches
    enableGreet = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableGreet", false);
    disableRandomLevels = sConfigMgr->GetBoolDefault("AiPlayerbot.DisableRandomLevels", false);
    playerbotsXPrate = sConfigMgr->GetIntDefault("AiPlayerbot.KillXPRate", 1);
    botActiveAlone = sConfigMgr->GetIntDefault("AiPlayerbot.BotActiveAlone", 10); //hidden config
    randombotsWalkingRPG = sConfigMgr->GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG", false);
    randombotsWalkingRPGInDoors = sConfigMgr->GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = sConfigMgr->GetIntDefault("AiPlayerbot.mMinEnchantingBotLevel", 60);
    randombotStartingLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandombotStartingLevel", 5);
    gearscorecheck = sConfigMgr->GetBoolDefault("AiPlayerbot.GearScoreCheck", false);
    randomBotPreQuests = sConfigMgr->GetBoolDefault("AiPlayerbot.PreQuests", true);

    //SPP automation
    autoPickReward = sConfigMgr->GetStringDefault("AiPlayerbot.autoPickReward", "no");
    autoEquipUpgradeLoot = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoEquipUpgradeLoot", false);
    syncQuestWithPlayer = sConfigMgr->GetBoolDefault("AiPlayerbot.SyncQuestWithPlayer", false);
    syncQuestForPlayer = sConfigMgr->GetBoolDefault("AiPlayerbot.SyncQuestForPlayer", false);
    autoTrainSpells = sConfigMgr->GetStringDefault("AiPlayerbot.AutoTrainSpells", "no");
    autoPickTalents = sConfigMgr->GetStringDefault("AiPlayerbot.AutoPickTalents", "no");
    autoLearnTrainerSpells = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoLearnTrainerSpells", false);
    autoLearnQuestSpells = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoLearnQuestSpells", false);
    autoDoQuests = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoDoQuests", false);
    syncLevelWithPlayers = sConfigMgr->GetBoolDefault("AiPlayerbot.SyncLevelWithPlayers", false);
    randomBotSayWithoutMaster = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotSayWithoutMaster", false);
    randomBotGroupNearby = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotGroupNearby", false);

    //arena
    randomBotArenaTeamCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotArenaTeamCount", 20);
    deleteRandomBotArenaTeams = sConfigMgr->GetBoolDefault("AiPlayerbot.DeleteRandomBotArenaTeams", false);

    selfBotLevel = sConfigMgr->GetIntDefault("AiPlayerbot.SelfBotLevel", 1);

    RandomPlayerbotFactory::CreateRandomBots();
    PlayerbotFactory::Init();
    sRandomItemMgr->Init();
    sRandomItemMgr->InitAfterAhBot();

    if (!sPlayerbotAIConfig.autoDoQuests)
    {
        LOG_INFO("playerbots", "Loading Quest Detail Data...");
        sTravelMgr->LoadQuestTravelTable();
    }

    if (sPlayerbotAIConfig.randomBotJoinBG)
        sRandomPlayerbotMgr.LoadBattleMastersCache();

    LOG_INFO("playerbots", "---------------------------------------");
    LOG_INFO("playerbots", "        AI Playerbot initialized       ");
    LOG_INFO("playerbots", "---------------------------------------");
    LOG_INFO("playerbots", );

    return true;
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(randomBotQuestItems.begin(), randomBotQuestItems.end(), id) != randomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
    return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

std::string PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(std::string const& fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;

    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(std::make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen) //close log file
        fclose(file);

    std::string m_logsDir = sConfigMgr->GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }


    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;

    return true;
}

void PlayerbotAIConfig::log(std::string const& fileName, char const* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName) && !openLog(fileName, "a"))
        return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::loadWorldBuf(uint32 factionId1, uint32 classId1, uint32 minLevel1, uint32 maxLevel1)
{
    std::vector<uint32> buffs;

    std::ostringstream os;
    os << "AiPlayerbot.WorldBuff." << factionId1 << "." << classId1 << "." << minLevel1 << "." << maxLevel1;

    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault(os.str().c_str(), ""), buffs);

    for (auto buff : buffs)
    {
        worldBuff wb = { buff, factionId1, classId1, minLevel1, maxLevel1 };
        worldBuffs.push_back(wb);
    }

    if (maxLevel1 == 0)
    {
        std::ostringstream os;
        os << "AiPlayerbot.WorldBuff." << factionId1 << "." << classId1 << "." << minLevel1;

        LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (maxLevel1 == 0 && minLevel1 == 0)
    {
        std::ostringstream os;
        os << "AiPlayerbot.WorldBuff." << factionId1 << "." << factionId1 << "." << classId1;

        LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (classId1 == 0 && maxLevel1 == 0 && minLevel1 == 0)
    {
        std::ostringstream os;
        os << "AiPlayerbot.WorldBuff." << factionId1;

        LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (factionId1 == 0 && classId1 == 0 && maxLevel1 == 0 && minLevel1 == 0)
    {
        std::ostringstream os;
        os << "AiPlayerbot.WorldBuff";

        LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }
}
