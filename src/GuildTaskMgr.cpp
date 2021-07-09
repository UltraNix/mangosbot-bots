/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildTaskMgr.h"
#include "ChatHelper.h"
#include "GuildMgr.h"
#include "Mail.h"
#include "MapManager.h"
#include "Playerbot.h"
#include "RandomItemMgr.h"
#include "ServerFacade.h"

char *strstri(char const* str1, char const* str2);

enum GuildTaskType
{
    GUILD_TASK_TYPE_NONE = 0,
    GUILD_TASK_TYPE_ITEM = 1,
    GUILD_TASK_TYPE_KILL = 2
};

void GuildTaskMgr::Update(Player* player, Player* guildMaster)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
        return;

    if (!GetTaskValue(0, 0, "advert_cleanup"))
    {
        CleanupAdverts();
        RemoveDuplicatedAdverts();
        SetTaskValue(0, 0, "advert_cleanup", 1, sPlayerbotAIConfig->guildTaskAdvertCleanupTime);
    }

    uint32 guildId = guildMaster->GetGuildId();
    if (!guildId || !guildMaster->GetPlayerbotAI() || !guildMaster->GetGuildId())
        return;

    if (!player->IsFriendlyTo(guildMaster))
        return;

	Guild* guild = sGuildMgr->GetGuildById(guildMaster->GetGuildId());

    DenyReason reason = PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel secLevel = guildMaster->GetPlayerbotAI()->GetSecurity()->LevelFor(player, &reason);
    if (secLevel == PLAYERBOT_SECURITY_DENY_ALL || (secLevel == PLAYERBOT_SECURITY_TALK && reason != PLAYERBOT_DENY_FAR))
    {
        sLog->outDebug("%s / %s: skipping guild task update - not enough security level, reason = %u", guild->GetName().c_str(), player->GetName().c_str(), reason);
        return;
    }

    sLog->outDebug("%s: guild task update for player %s", guild->GetName().c_str(), player->GetName().c_str());

    ObjectGuid::LowType owner = player->GetGUID().GetCounter();

    uint32 activeTask = GetTaskValue(owner, guildId, "activeTask");
    if (!activeTask)
    {
        SetTaskValue(owner, guildId, "killTask", 0, 0);
        SetTaskValue(owner, guildId, "itemTask", 0, 0);
        SetTaskValue(owner, guildId, "itemCount", 0, 0);
        SetTaskValue(owner, guildId, "killTask", 0, 0);
        SetTaskValue(owner, guildId, "killCount", 0, 0);
        SetTaskValue(owner, guildId, "payment", 0, 0);
        SetTaskValue(owner, guildId, "thanks", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "reward", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);

        uint32 task = CreateTask(owner, guildId);

        if (task == GUILD_TASK_TYPE_NONE)
        {
            sLog->outError( "%s / %s: error creating guild task", guild->GetName().c_str(), player->GetName().c_str());
        }

        uint32 time = urand(sPlayerbotAIConfig->minGuildTaskChangeTime, sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "activeTask", task, time);
        SetTaskValue(owner, guildId, "advertisement", 1, urand(sPlayerbotAIConfig->minGuildTaskAdvertisementTime, sPlayerbotAIConfig->maxGuildTaskAdvertisementTime));

        sLog->outDebug("%s / %s: guild task %u is set for %u secs", guild->GetName().c_str(), player->GetName().c_str(), task, time);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 advertisement = GetTaskValue(owner, guildId, "advertisement");
    if (!advertisement)
    {
        sLog->outDebug("%s / %s: sending advertisement", guild->GetName().c_str(), player->GetName().c_str());

        if (SendAdvertisement(trans, owner, guildId))
        {
            SetTaskValue(owner, guildId, "advertisement", 1, urand(sPlayerbotAIConfig->minGuildTaskAdvertisementTime, sPlayerbotAIConfig->maxGuildTaskAdvertisementTime));
        }
        else
        {
            sLog->outError( "%s / %s: error sending advertisement", guild->GetName().c_str(), player->GetName().c_str());
        }
    }

    uint32 thanks = GetTaskValue(owner, guildId, "thanks");
    if (!thanks)
    {
        sLog->outDebug("%s / %s: sending thanks", guild->GetName().c_str(), player->GetName().c_str());

        if (SendThanks(trans, owner, guildId, GetTaskValue(owner, guildId, "payment")))
        {
            SetTaskValue(owner, guildId, "thanks", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
            SetTaskValue(owner, guildId, "payment", 0, 0);
        }
        else
        {
            sLog->outError( "%s / %s: error sending thanks", guild->GetName().c_str(), player->GetName().c_str());
        }
    }

    uint32 reward = GetTaskValue(owner, guildId, "reward");
    if (!reward)
    {
        sLog->outDebug("%s / %s: sending reward", guild->GetName().c_str(), player->GetName().c_str());

        if (Reward(trans, owner, guildId))
        {
            SetTaskValue(owner, guildId, "reward", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
            SetTaskValue(owner, guildId, "payment", 0, 0);
        }
        else
        {
            sLog->outError( "%s / %s: error sending reward", guild->GetName().c_str(), player->GetName().c_str());
        }
    }

    CharacterDatabase.CommitTransaction(trans);
}

uint32 GuildTaskMgr::CreateTask(uint32 owner, uint32 guildId)
{
    switch (urand(0, 1))
    {
        case 0:
            CreateItemTask(owner, guildId);
            return GUILD_TASK_TYPE_ITEM;
        default:
            CreateKillTask(owner, guildId);
            return GUILD_TASK_TYPE_KILL;
    }
}

class RandomItemBySkillGuildTaskPredicate : public RandomItemPredicate
{
public:
    RandomItemBySkillGuildTaskPredicate(Player* player) : RandomItemPredicate(), player(player) { }

    bool Apply(ItemTemplate const* proto) override
    {
        for (uint32 skill = SKILL_NONE; skill < MAX_SKILL_TYPE; ++skill)
        {
            if (player->HasSkill(skill) && auctionbot.IsUsedBySkill(proto, skill))
                return true;
        }

        return false;
    }

private:
    Player* player;
};

bool GuildTaskMgr::CreateItemTask(uint32 owner, uint32 guildId)
{
    Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (!player || player->getLevel() < 5)
        return false;

    RandomItemBySkillGuildTaskPredicate predicate(player);
    uint32 itemId = sRandomItemMgr->GetRandomItem(player->getLevel() - 5, RANDOM_ITEM_GUILD_TASK, &predicate);
    if (!itemId)
    {
        sLog->outError( "%s / %s: no items avaible for item task", sGuildMgr->GetGuildById(guildId)->GetName().c_str(), player->GetName().c_str());
        return false;
    }

    uint32 count = GetMaxItemTaskCount(itemId);

    sLog->outDebug("%s / %s: item task %u (x%d)", sGuildMgr->GetGuildById(guildId)->GetName().c_str(), player->GetName().c_str(), itemId, count);

    SetTaskValue(owner, guildId, "itemCount", count, sPlayerbotAIConfig->maxGuildTaskChangeTime);
    SetTaskValue(owner, guildId, "itemTask", itemId, sPlayerbotAIConfig->maxGuildTaskChangeTime);

    return true;
}

bool GuildTaskMgr::CreateKillTask(uint32 owner, uint32 guildId)
{
    Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (!player)
        return false;

    uint32 rank = !urand(0, 2) ? CREATURE_ELITE_RAREELITE : CREATURE_ELITE_RARE;

    std::vector<uint32> ids;

    uint32 level = (uint32)player->getLevel();
    QueryResult results = WorldDatabase.PQuery("SELECT ct.Entry, c.map, c.position_x, c.position_y, ct.Name FROM creature_template ct "
        "JOIN creature c ON ct.Entry = c.id WHERE ct.MaxLevel < %u AND ct.MinLevel > %u AND ct.Rank = %u ", level + 4, level - 3, rank);
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 id = fields[0].GetUInt32();
            uint32 map = fields[1].GetUInt32();
            float x = fields[2].GetFloat();
            float y = fields[3].GetFloat();
            std::string name = fields[4].GetString();

            if (strstr(name.c_str(), "UNUSED"))
                continue;

            float dist = sServerFacade->GetDistance2d(player, x, y);
            if (dist > sPlayerbotAIConfig->guildTaskKillTaskDistance || player->GetMapId() != map)
                continue;

            if (find(ids.begin(), ids.end(), id) == ids.end())
                ids.push_back(id);
        } while (results->NextRow());
    }

    if (ids.empty())
    {
        sLog->outError( "%s / %s: no rare creatures available for kill task", sGuildMgr->GetGuildById(guildId)->GetName().c_str(), player->GetName().c_str());
        return false;
    }

    uint32 index = urand(0, ids.size() - 1);
    uint32 creatureId = ids[index];

    sLog->outDebug("%s / %s: kill task %u", sGuildMgr->GetGuildById(guildId)->GetName().c_str(), player->GetName().c_str(), creatureId);

    SetTaskValue(owner, guildId, "killTask", creatureId, sPlayerbotAIConfig->maxGuildTaskChangeTime);

    return true;
}

bool GuildTaskMgr::SendAdvertisement(SQLTransaction& trans, uint32 owner, uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    uint32 validIn;
    GetTaskValue(owner, guildId, "activeTask", &validIn);

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask)
        return SendItemAdvertisement(trans, itemTask, owner, guildId, validIn);

    uint32 killTask = GetTaskValue(owner, guildId, "killTask");
    if (killTask)
        return SendKillAdvertisement(trans, killTask, owner, guildId, validIn);

    return false;
}

std::string formatTime(uint32 secs)
{
    std::ostringstream out;
    if (secs < 3600)
    {
        out << secs / 60 << " min";
    }
    else if (secs < 7200)
    {
        out << "1 hr " << (secs - 3600) / 60 << " min";
    }
    else if (secs < 3600 * 24)
    {
        out << secs / 3600 << " hr";
    } else
    {
        out << secs / 3600 / 24 << " days";
    }

    return out.str();
}

string formatDateTime(uint32 secs)
{
    time_t rawtime = time(0) + secs;
    tm* timeinfo = localtime (&rawtime);

    char buffer[256];
    strftime(buffer, sizeof(buffer), "%b %d, %H:%M", timeinfo);
    return string(buffer);
}

string GetHelloText(uint32 owner)
{
    std::ostringstream body;
    body << "Hello";

    std::string playerName;
    sObjectMgr->GetPlayerNameByGUID(owner, playerName);
    if (!playerName.empty())
        body << ", " << playerName;

    body << ",\n\n";

    return body.str();
}

bool GuildTaskMgr::SendItemAdvertisement(SQLTransaction& trans, uint32 itemId, uint32 owner, uint32 guildId, uint32 validIn)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    std::ostringstream body;
    body << GetHelloText(owner);
    body << "We are in a great need of " << proto->Name1 << ". If you could sell us ";
    uint32 count = GetTaskValue(owner, guildId, "itemCount");
    if (count > 1)
        body << "at least " << count << " of them ";
    else
        body << "some ";
    body << "we'd really appreciate that and pay a high price.\n\n";
    body << "The task will expire at " << formatDateTime(validIn) << "\n";
    body << "\n";
    body << "Best Regards,\n";
    body << guild->GetName() << "\n";
    body << leader->GetName() << "\n";

    std::ostringstream subject;
    subject << "Guild Task: " << proto->Name1;
    if (count > 1)
        subject << " (x" << count << ")";

    MailDraft(subject.str(), body.str()).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    return true;
}

bool GuildTaskMgr::SendKillAdvertisement(SQLTRansaction& trans, uint32 creatureId, uint32 owner, uint32 guildId, uint32 validIn)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(creatureId);
    if (!proto)
        return false;

    QueryResult result = WorldDatabase.PQuery("SELECT map, position_x, position_y, position_z FROM creature WHERE id = '%u'", creatureId);
    if (!result)
        return false;

    std::string location;
    do
    {
        Field* fields = result->Fetch();
        uint32 mapid = fields[0].GetUInt32();
        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        Map* map = sMapMgr->FindMap(mapid, 0);
        if (!map)
            continue;

        AreaTableEntry const* entry = sAreaTableStore.LookupEntry(map->GetAreaId(x, y, z));
        if (!entry)
            continue;

        location = entry->area_name[0];
        break;
    } while (result->NextRow());

    std::ostringstream body;
    body << GetHelloText(owner);
    body << "As you probably know " << proto->Name << " is wanted dead for the crimes it did against our guild. If you should kill it ";
    body << "we'd really appreciate that.\n\n";
    if (!location.empty())
        body << proto->Name << "'s the last known location was " << location << ".\n";
    body << "The task will expire at " << formatDateTime(validIn) << "\n";
    body << "\n";
    body << "Best Regards,\n";
    body << guild->GetName() << "\n";
    body << leader->GetName() << "\n";

    std::ostringstream subject;
    subject << "Guild Task: ";
    if (proto->rank == CREATURE_ELITE_ELITE || proto->rank == CREATURE_ELITE_RAREELITE || proto->rank == CREATURE_ELITE_WORLDBOSS)
        subject << "(Elite) ";
    subject << proto->Name;
    if (!location.empty())
        subject << ", " << location;

    MailDraft(subject.str(), body.str()).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    return true;
}

bool GuildTaskMgr::SendThanks(SQLTransaction& trans, uint32 owner, uint32 guildId, uint32 payment)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTask);
        if (!proto)
            return false;

        std::ostringstream body;
        body << GetHelloText(owner);
        body << "One of our guild members wishes to thank you for the " << proto->Name1 << "!";
        uint32 count = GetTaskValue(owner, guildId, "itemCount");
        if (count)
        {
            body << " If we have another ";
            body << count << " of them that would help us tremendously.\n";
        }
        body << "\n";
        body << "Thanks again,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";

        MailDraft("Thank You", body.str()).
                AddMoney(payment).
                SendMailTo(trans, MailReceiver(owner), MailSender(leader));

        Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
        if (player)
            SendCompletionMessage(player, "payed for");

        return true;
    }

    return false;
}

uint32 GuildTaskMgr::GetMaxItemTaskCount(uint32 itemId)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return 0;

    if (!proto->Stackable || proto->GetMaxStackSize() == 1)
        return 1;

    if (proto->Quality == ITEM_QUALITY_NORMAL)
    {
        switch (proto->GetMaxStackSize())
        {
            case 5:
                return urand(1, 3) * proto->GetMaxStackSize();
            case 10:
                return urand(2, 6) * proto->GetMaxStackSize() / 2;
            case 20:
                return urand(4, 12) * proto->GetMaxStackSize() / 4;
            default:
                return proto->GetMaxStackSize();
        }
    }

    if (proto->Quality < ITEM_QUALITY_RARE)
    {
        switch (proto->GetMaxStackSize())
        {
            case 5:
                return proto->GetMaxStackSize();
            case 10:
                return urand(1, 2) * proto->GetMaxStackSize() / 2;
            case 20:
                return urand(1, 4) * proto->GetMaxStackSize() / 4;
            default:
                return proto->GetMaxStackSize();
        }
    }

    return 1;
}

bool GuildTaskMgr::IsGuildTaskItem(uint32 itemId, uint32 guildId)
{
    uint32 value = 0;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT `value`, `time`, validIn FROM ai_playerbot_guild_tasks WHERE `value` = '%u' AND guildid = '%u' AND `type` = 'itemTask'",
        itemId, guildId);

    if (results)
    {
        Field* fields = results->Fetch();
        value = fields[0].GetUInt32();
        uint32 lastChangeTime = fields[1].GetUInt32();
        uint32 validIn = fields[2].GetUInt32();
        if ((time(0) - lastChangeTime) >= validIn)
            value = 0;
    }

    return value;
}

std::map<uint32, uint32> GuildTaskMgr::GetTaskValues(uint32 owner, std::string type, uint32* validIn /* = nullptr */)
{
    std::map<uint32, uint32> result;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT `value`, `time`, validIn, guildid FROM ai_playerbot_guild_tasks WHERE owner = '%u' AND `type` = '%s'",
        owner, type.c_str());

    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 value = fields[0].GetUInt32();
            uint32 lastChangeTime = fields[1].GetUInt32();
            uint32 secs = fields[2].GetUInt32();
            uint32 guildId = fields[3].GetUInt32();
            if ((time(0) - lastChangeTime) >= secs)
                value = 0;

            result[guildId] = value;

        } while (results->NextRow());
    }

	return std::move(result);
}

uint32 GuildTaskMgr::GetTaskValue(uint32 owner, uint32 guildId, std::string type, uint32* validIn /* = nullptr */)
{
    uint32 value = 0;

    QueryResult results = PlayerbotDatabase.PQuery("SELECT `value`, `time`, validIn FROM ai_playerbot_guild_tasks WHERE owner = '%u' AND guildid = '%u' AND `type` = '%s'",
        owner, guildId, type.c_str());

    if (results)
    {
        Field* fields = results->Fetch();
        value = fields[0].GetUInt32();
        uint32 lastChangeTime = fields[1].GetUInt32();
        uint32 secs = fields[2].GetUInt32();
        if ((time(0) - lastChangeTime) >= secs)
            value = 0;

        if (validIn)
            *validIn = secs;
    }

	return value;
}

uint32 GuildTaskMgr::SetTaskValue(uint32 owner, uint32 guildId, std::string type, uint32 value, uint32 validIn)
{
    PlayerbotDatabase.DirectPExecute("DELETE FROM ai_playerbot_guild_tasks WHERE owner = '%u' AND guildid = '%u' AND `type` = '%s'", owner, guildId, type.c_str());
    if (value)
    {
        PlayerbotDatabase.DirectPExecute("INSERT INTO ai_playerbot_guild_tasks (owner, guildid, `time`, validIn, `type`, `value`) VALUES ('%u', '%u', '%u', '%u', '%s', '%u')",
            owner, guildId, (uint32)time(0), validIn, type.c_str(), value);
    }

    return value;
}

bool GuildTaskMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
    {
        sLog->outError( "Guild task system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        sLog->outError( "Usage: gtask stats/reset");
        return false;
    }

    std::string cmd = args;

    if (cmd == "reset")
    {
        PlayerbotDatabase.PExecute("DELETE FROM ai_playerbot_guild_tasks");
        sLog->outString("Guild tasks were reset for all players");
        return true;
    }

    if (cmd == "stats")
    {
        sLog->outString("Usage: gtask stats <player name>");
        return true;
    }

    if (cmd.find("stats ") != std::string::npos)
    {
        std::string charName = cmd.substr(cmd.find("stats ") + 6);
        ObjectGuid guid = sObjectMgr->GetPlayerGUIDByName(charName);
        if (!guid)
        {
            sLog->outError( "Player %s not found", charName.c_str());
            return false;
        }

        uint32 owner = guid.GetCounter();

        QueryResult result = PlayerbotDatabase.PQuery("SELECT `value`, `time`, validIn, guildid FROM ai_playerbot_guild_tasks WHERE owner = '%u' AND type='activeTask' ORDER BY guildid",
            owner);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 value = fields[0].GetUInt32();
                uint32 lastChangeTime = fields[1].GetUInt32();
                uint32 validIn = fields[2].GetUInt32();
                if ((time(0) - lastChangeTime) >= validIn)
                    value = 0;

                uint32 guildId = fields[3].GetUInt32();

                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (!guild)
                    continue;

                std::ostringstream name;
                if (value == GUILD_TASK_TYPE_ITEM)
                {
                    name << "ItemTask";
                    uint32 itemId = sGuildTaskMgr->GetTaskValue(owner, guildId, "itemTask");
                    uint32 itemCount = sGuildTaskMgr->GetTaskValue(owner, guildId, "itemCount");

                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
                    {
                        name << " (" << proto->Name1 << " x" << itemCount << ",";

                        switch (proto->Quality)
                        {
                            case ITEM_QUALITY_UNCOMMON:
                                name << "green";
                                break;
                            case ITEM_QUALITY_NORMAL:
                                name << "white";
                                break;
                            case ITEM_QUALITY_RARE:
                                name << "blue";
                                break;
                            case ITEM_QUALITY_EPIC:
                                name << "epic";
                                break;
                            case ITEM_QUALITY_LEGENDARY:
                                name << "yellow";
                                break;
                        }

                        name << ")";
                    }
                }
                else if (value == GUILD_TASK_TYPE_KILL)
                {
                    name << "KillTask";
                    uint32 creatureId = sGuildTaskMgr->GetTaskValue(owner, guildId, "killTask");

                    if (CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(creatureId))
                    {
                        name << " (" << proto->Name << ",";

                        switch (proto->rank)
                        {
                            case CREATURE_ELITE_RARE:
                                name << "rare";
                                break;
                            case CREATURE_ELITE_RAREELITE:
                                name << "rare elite";
                                break;
                        }

                        name << ")";
                    }
                }
                else
                    continue;

                uint32 advertValidIn = 0;
                uint32 advert = sGuildTaskMgr->GetTaskValue(owner, guildId, "advertisement", &advertValidIn);
                if (advert && advertValidIn < validIn)
                    name << " advert in " << formatTime(advertValidIn);

                uint32 thanksValidIn = 0;
                uint32 thanks = sGuildTaskMgr->GetTaskValue(owner, guildId, "thanks", &thanksValidIn);
                if (thanks && thanksValidIn < validIn)
                    name << " thanks in " << formatTime(thanksValidIn);

                uint32 rewardValidIn = 0;
                uint32 reward = sGuildTaskMgr->GetTaskValue(owner, guildId, "reward", &rewardValidIn);
                if (reward && rewardValidIn < validIn)
                    name << " reward in " << formatTime(rewardValidIn);

                uint32 paymentValidIn = 0;
                uint32 payment = sGuildTaskMgr->GetTaskValue(owner, guildId, "payment", &paymentValidIn);
                if (payment && paymentValidIn < validIn)
                    name << " payment " << ChatHelper::formatMoney(payment) << " in " << formatTime(paymentValidIn);

                sLog->outString("%s: %s valid in %s ['%s']",
                        charName.c_str(), name.str().c_str(), formatTime(validIn).c_str(), guild->GetName().c_str());

            } while (result->NextRow());
        }

        return true;
    }

    if (cmd == "cleanup")
    {
        sGuildTaskMgr->CleanupAdverts();
        sGuildTaskMgr->RemoveDuplicatedAdverts();
        return true;
    }

    if (cmd == "reward")
    {
        sLog->outString("Usage: gtask reward <player name>");
        return true;
    }

    if (cmd == "advert")
    {
        sLog->outString("Usage: gtask advert <player name>");
        return true;
    }

    bool reward = cmd.find("reward ") != string::npos;
    bool advert = cmd.find("advert ") != string::npos;
    if (reward || advert)
    {
        std::string charName;
        if (reward)
            charName = cmd.substr(cmd.find("reward ") + 7);

        if (advert)
            charName = cmd.substr(cmd.find("advert ") + 7);

        ObjectGuid guid = sObjectMgr->GetPlayerGUIDByName(charName);
        if (!guid)
        {
            sLog->outError( "Player %s not found", charName.c_str());
            return false;
        }

        uint32 owner = guid.GetCounter();
        QueryResult result = PlayerbotDatabase.PQuery("SELECT DISTINCT guildid FROM ai_playerbot_guild_tasks WHERE owner = '%u'", owner);

        if (result)
        {
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();
                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (!guild)
                    continue;

                if (reward)
                    sGuildTaskMgr->Reward(trans, owner, guildId);

                if (advert)
                    sGuildTaskMgr->SendAdvertisement(trans, owner, guildId);
            } while (result->NextRow());

            CharacterDatabase.CommitTransaction(trans);
            return true;
        }
    }

    return false;
}

bool GuildTaskMgr::CheckItemTask(uint32 itemId, uint32 obtained, Player* ownerPlayer, Player* bot, bool byMail)
{
    if (!bot)
        return false;

    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    uint32 owner = ownerPlayer->GetGUID().GetCounter();
	Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
	if (!guild)
		return false;

	if (!sRandomPlayerbotMgr->IsRandomBot(bot))
	    return false;

    sLog->outDebug("%s / %s: checking guild task", guild->GetName().c_str(), ownerPlayer->GetName().c_str());

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask != itemId)
    {
        sLog->outDebug("%s / %s: item %u is not guild task item (%u)", guild->GetName().c_str(), ownerPlayer->GetName().c_str(), itemId, itemTask);

        if (byMail)
            SendCompletionMessage(ownerPlayer, "made a mistake with");

        return false;
    }

    uint32 count = GetTaskValue(owner, guildId, "itemCount");
    if (!count)
    {
        return false;
    }

    uint32 rewardTime = urand(sPlayerbotAIConfig->minGuildTaskRewardTime, sPlayerbotAIConfig->maxGuildTaskRewardTime);
    if (byMail)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            return false;

        uint32 money = GetTaskValue(owner, guildId, "payment");
        SetTaskValue(owner, guildId, "payment", money + auctionbot.GetBuyPrice(proto) * obtained, rewardTime + 300);
    }

    if (obtained >= count)
    {
        sLog->outDebug("%s / %s: guild task complete", guild->GetName().c_str(), ownerPlayer->GetName().c_str());
        SetTaskValue(owner, guildId, "reward", 1, rewardTime - 15);
        SetTaskValue(owner, guildId, "itemCount", 0, 0);
        SetTaskValue(owner, guildId, "thanks", 0, 0);
        SendCompletionMessage(ownerPlayer, "completed");
    }
    else
    {
        sLog->outDebug("%s / %s: guild task progress %u/%u", guild->GetName().c_str(), ownerPlayer->GetName().c_str(), obtained, count);
        SetTaskValue(owner, guildId, "itemCount", count - obtained, sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "thanks", 1, rewardTime - 30);
        SendCompletionMessage(ownerPlayer, "made a progress with");
    }
    return true;
}

bool GuildTaskMgr::Reward(SQLTransaction& trans, uint32 owner, uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (!player)
        return false;

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    uint32 killTask = GetTaskValue(owner, guildId, "killTask");
    if (!itemTask && !killTask)
        return false;

    std::ostringstream body;
    body << GetHelloText(owner);

    RandomItemType rewardType;
    uint32 itemId = 0;
    uint32 itemCount = 1;
    if (itemTask)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTask);
        if (!proto)
            return false;

        body << "We wish to thank you for the " << proto->Name1 << " you provided so kindly. We really appreciate this and may this small gift bring you our thanks!\n";
        body << "\n";
        body << "Many thanks,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";
        rewardType = proto->Quality > ITEM_QUALITY_NORMAL ? RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE : RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN;
        itemId = sRandomItemMgr->GetRandomItem(player->getLevel() - 5, rewardType);
    }
    else if (killTask)
    {
        CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(killTask);
        if (!proto)
            return false;

        body << "We wish to thank you for the " << proto->Name << " you've killed recently. We really appreciate this and may this small gift bring you our thanks!\n";
        body << "\n";
        body << "Many thanks,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";
        rewardType = proto->rank == CREATURE_ELITE_RARE ? RANDOM_ITEM_GUILD_TASK_REWARD_TRADE : RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE;
        itemId = sRandomItemMgr->GetRandomItem(player->getLevel(), rewardType);
        if (itemId)
        {
            ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(itemId);
            if (proto)
            {
                if (itemProto->Quality == ITEM_QUALITY_NORMAL)
                    itemCount = itemProto->GetMaxStackSize();

                if (proto->rank != CREATURE_ELITE_RARE && itemProto->Quality > ITEM_QUALITY_NORMAL)
                    itemCount = urand(1, itemProto->GetMaxStackSize());
            }
        }
    }

    uint32 payment = GetTaskValue(owner, guildId, "payment");
    if (payment)
        SendThanks(trans, owner, guildId, payment);

    MailDraft draft("Thank You", body.str());

    if (itemId)
    {
        Item* item = Item::CreateItem(itemId, itemCount);
        if (item)
        {
            item->SaveToDB(trans);
            draft.AddItem(item);
        }
    }

    draft.SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (player)
        SendCompletionMessage(player, "rewarded for");

    SetTaskValue(owner, guildId, "activeTask", 0, 0);
    SetTaskValue(owner, guildId, "payment", 0, 0);

    return true;
}

void GuildTaskMgr::CheckKillTask(Player* player, Unit* victim)
{
    if (!player)
        return;

    if (Group* group = player->GetGroup())
    {
        for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
        {
            CheckKillTaskInternal(gr->GetSource(), victim);
        }
    }
    else
    {
        CheckKillTaskInternal(player, victim);
    }
}

void GuildTaskMgr::SendCompletionMessage(Player* player, std::string verb)
{
    std::ostringstream out;
    out << player->GetName() << " has " << verb << " a guild task";

    if (Group* group = player->GetGroup())
    {
        for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
        {
            Player* member = gr->GetSource();
            if (member != player)
                ChatHandler(member->GetSession()).PSendSysMessage(out.str().c_str());
        }
    }
    else
    {
        if (PlayerbotAI* botAI = player->GetPlayerbotAI())
            if (Player* master = botAI->GetMaster())
                ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }

    std::ostringstream self;
    self << "You have " << verb << " a guild task";
    ChatHandler(player->GetSession()).PSendSysMessage(self.str().c_str());
}

void GuildTaskMgr::CheckKillTaskInternal(Player* player, Unit* victim)
{
    ObjectGuid::LowType owner = player->GetGUID().GetCounter();
    if (victim->GetTypeId() != TYPEID_UNIT)
        return;

    Creature* creature = reinterpret_cast<Creature*>(victim);
    if (!creature)
        return;

    std::map<uint32, uint32> tasks = GetTaskValues(owner, "killTask");
    for (std::map<uint32, uint32>::iterator i = tasks.begin(); i != tasks.end(); ++i)
    {
        uint32 guildId = i->first;
        uint32 value = i->second;
        Guild* guild = sGuildMgr->GetGuildById(guildId);

        if (value != creature->GetEntry())
            continue;

        sLog->outDebug("%s / %s: guild task complete", guild->GetName().c_str(), player->GetName().c_str());
        SetTaskValue(owner, guildId, "reward", 1, urand(sPlayerbotAIConfig->minGuildTaskRewardTime, sPlayerbotAIConfig->maxGuildTaskRewardTime));

        SendCompletionMessage(player, "completed");
    }
}

void GuildTaskMgr::CleanupAdverts()
{
    uint32 deliverTime = time(0) - sPlayerbotAIConfig->minGuildTaskChangeTime;
    QueryResult result = CharacterDatabase.PQuery("SELECT id, receiver FROM mail WHERE subject LIKE 'Guild Task%%' AND deliver_time <= '%u'", deliverTime);
    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();

        if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt32())))
            player->RemoveMail(id);

        ++count;
    } while (result->NextRow());

    if (count > 0)
    {
        CharacterDatabase.PExecute("DELETE FROM mail WHERE subject LIKE 'Guild Task%%' AND deliver_time <= '%u'", deliverTime);
        sLog->outBasic("%d old gtask adverts removed", count);
    }
}

void GuildTaskMgr::RemoveDuplicatedAdverts()
{
    uint32 deliverTime = time(0);
    QueryResult result = CharacterDatabase.PQuery("SELECT m.id, m.receiver FROM (SELECT MAX(id) AS id, subject, receiver FROM mail WHERE subject LIKE 'Guild Task%%' "
            "AND deliver_time <= '%u' GROUP BY subject, receiver) q JOIN mail m ON m.subject = q.subject WHERE m.id <> q.id AND m.deliver_time <= '%u'", deliverTime, deliverTime);

    if (!result)
        return;

    std::vector<uint32> ids;
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();

        if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt32())))
            player->RemoveMail(id);

        ++count;
        ids.push_back(id);
    } while (result->NextRow());

    if (count > 0)
    {
        std::vector<uint32> buffer;
        for (std::vector<uint32>::iterator i = ids.begin(); i != ids.end(); ++i)
        {
            buffer.push_back(*i);
            if (buffer.size() > 50)
            {
                DeleteMail(buffer);
                buffer.clear();
            }
        }

        DeleteMail(buffer);
        sLog->outBasic("%d duplicated gtask adverts removed", count);
    }

}

void GuildTaskMgr::DeleteMail(std::vector<uint32> buffer)
{
    std::ostringstream sql;
    sql << "DELETE FROM mail WHERE id IN (";

    bool first = true;
    for (std::vector<uint32>::iterator j = buffer.begin(); j != buffer.end(); ++j)
    {
        if (first)
            first = false;
        else
            sql << ",";

        sql << *j;
    }

    sql << ")";

    CharacterDatabase.PExecute(sql.str().c_str());
}

bool GuildTaskMgr::CheckTaskTransfer(std::string text, Player* ownerPlayer, Player* bot)
{
    if (!bot)
        return false;

    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    ObjectGuid::LowType owner = ownerPlayer->GetGUID().GetCounter();
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if (text.empty())
        return false;

    strToLower(text);

    sLog->outDebug("%s / %s: checking guild task transfer", guild->GetName().c_str(), ownerPlayer->GetName().c_str());

    uint32 account = sObjectMgr->GetPlayerAccountIdByGUID(owner);

    if (QueryResult results = CharacterDatabase.PQuery("SELECT guid, name FROM characters WHERE account = '%u'", account))
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 newOwner = fields[0].GetUInt32();
            string name = fields[1].GetString();
            if (newOwner == bot->GetGUID().GetCounter())
                continue;

            strToLower(name);

            if (text.find(name) != string::npos)
            {
                uint32 validIn;
                uint32 activeTask = GetTaskValue(owner, guildId, "activeTask", &validIn);
                uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
                uint32 killTask = GetTaskValue(owner, guildId, "killTask");

                if (itemTask || killTask)
                {
                    SetTaskValue(newOwner, guildId, "activeTask", activeTask, validIn);
                    SetTaskValue(newOwner, guildId, "advertisement", 1, 15);

                    if (itemTask)
                        SetTaskValue(newOwner, guildId, "itemTask", itemTask, validIn);

                    if (killTask)
                        SetTaskValue(newOwner, guildId, "killTask", killTask, validIn);

                    SetTaskValue(owner, guildId, "activeTask", 0, 0);
                    SetTaskValue(owner, guildId, "payment", 0, 0);

                    SendCompletionMessage(ownerPlayer, "transfered");
                }
            }
        } while (results->NextRow());
    }
}
