/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotMgr.h"
#include "Playerbot.h"
#include "PlayerbotDbStore.h"
#include "PlayerbotFactory.h"
#include "TravelMgr.h"

PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase()
{
}

void PlayerbotHolder::UpdateSessions(uint32 /*elapsed*/)
{
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->IsBeingTeleported())
        {
            bot->GetPlayerbotAI()->HandleTeleportAck();
        }
        else if (bot->IsInWorld())
        {
            bot->GetSession()->HandleBotPackets();
        }
    }
}

void PlayerbotHolder::LogoutAllBots()
{
    /*
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd())
            break;

        Player* bot= itr->second;
        if (!bot->GetPlayerbotAI()->IsRealPlayer())
            LogoutPlayerBot(bot->GetGUID());
    }
    */

    PlayerBotMap bots = playerBots;
    for (auto& itr : bots)
    {
        Player* bot = itr.second;
        if (!bot)
            continue;

        if (!bot->GetPlayerbotAI() || bot->GetPlayerbotAI()->IsRealPlayer())
            continue;

        LogoutPlayerBot(bot->GetGUID());
    }
}

void PlayerbotMgr::CancelLogout()
{
    Player* master = GetMaster();
    if (!master)
        return;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (!ai || ai->IsRealPlayer())
            continue;

        if (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut())
        {
            WorldPacket p;
            bot->GetSession()->HandleLogoutCancelOpcode(p);
            ai->TellMaster("Logout cancelled!");
        }
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (!ai || ai->IsRealPlayer())
            continue;

        if (bot->GetPlayerbotAI()->GetMaster() != master)
            continue;

        if (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut())
        {
            WorldPacket p;
            bot->GetSession()->HandleLogoutCancelOpcode(p);
        }
    }
}

void PlayerbotHolder::LogoutPlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (!ai)
            return;

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleGround() && !bot->InBattleGroundQueue() && ai->HasActivePlayerMaster())
        {
            sPlayerbotDbStore->Save(ai);
        }

        LOG_INFO("playerbots", "Bot %s logging out", bot->GetName().c_str());
        bot->SaveToDB(false, false);

       WorldSession* botWorldSessionPtr = bot->GetSession();
        WorldSession* masterWorldSessionPtr = nullptr;

        Player* master = ai->GetMaster();
        if (master)
            masterWorldSessionPtr = master->GetSession();

        // check for instant logout
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

        if (masterWorldSessionPtr && masterWorldSessionPtr->ShouldLogOut(time(nullptr)))
            logout = true;

        if (masterWorldSessionPtr && masterWorldSessionPtr->GetState() != WORLD_SESSION_STATE_READY)
            logout = true;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->IsTaxiFlying() ||
            botWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))
        {
            logout = true;
        }

        if (master && (master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->IsTaxiFlying() ||
            (masterWorldSessionPtr && masterWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))))
        {
            logout = true;
        }

        TravelTarget* target = nullptr;
        if (ai->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            target = ai->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        }

        // if no instant logout, request normal logout
        if (!logout)
        {
            if (bot && (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut()))
                return;
            else if (bot)
            {
                ai->TellMaster("I'm logging out!");
                WorldPacket p;
                botWorldSessionPtr->HandleLogoutRequestOpcode(p);
                if (!bot)
                {
                    playerBots.erase(guid);
                    delete botWorldSessionPtr;
                    if (target)
                        delete target;
                }
                return;
            }
            else
            {
                playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap
                delete botWorldSessionPtr;  // finally delete the bot's WorldSession
                if (target)
                    delete target;
            }
            return;
        } // if instant logout possible, do it
        else if (bot && (logout || !botWorldSessionPtr->isLogingOut()))
        {
            ai->TellMaster("Goodbye!");
            playerBots.erase(guid);                 // deletes bot player ptr inside this WorldSession PlayerBotMap
            botWorldSessionPtr->LogoutPlayer(true); // this will delete the bot Player object and PlayerbotAI object
            delete botWorldSessionPtr;              // finally delete the bot's WorldSession
        }
    }
}

void PlayerbotHolder::DisablePlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = bot->GetPlayerbotAI();
        botAI->TellMaster("Goodbye!");
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleGround() && !bot->InBattleGroundQueue() && bot->GetPlayerbotAI()->HasActivePlayerMaster())
        {
            sPlayerbotDbStore->Save(botAI);
        }

        LOG_DEBUG("playerbots", "Bot %s logged out", bot->GetName().c_str());

        bot->SaveToDB(false, false);

        if (botAI->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
            if (target)
                delete target;
        }

        playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap

        delete botAI;
        bot->SetPlayerbotAI(nullptr);
    }
}

Player* PlayerbotHolder::GetPlayerBot(ObjectGuid playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

void PlayerbotHolder::OnBotLogin(Player* const bot)
{
	PlayerbotAI* botAI = new PlayerbotAI(bot);
	bot->SetPlayerbotAI(botAI);
	OnBotLoginInternal(bot);

    playerBots[bot->GetGUID()] = bot;

    Player* master = botAI->GetMaster();
    if (master)
    {
        ObjectGuid masterGuid = master->GetGUID();
        if (master->GetGroup() && ! master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            if (master)
            {
                if (master->GetObjectGuid() == member)
                {
                    groupValid = true;
                    break;
                }
            }
            else
            {
                uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(member);
                if (!sPlayerbotAIConfig.IsInRandomAccountList(account))
                {
                    groupValid = true;
                    break;
                }
            }
        }

        if (!groupValid)
        {
            WorldPacket p;
            std::string member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
        }
    }

    group = bot->GetGroup();
    if (group)
    {
        botAI->ResetStrategies();
    }
    else
    {
        botAI->ResetStrategies(!sRandomPlayerbotMgr->IsRandomBot(bot->GetGUID().GetCounter()));
    }

    if (master && !master->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        bot->GetMotionMaster()->MovementExpired();
        bot->m_taxi.ClearTaxiDestinations();
    }

    // check activity
    ai->AllowActivity(ALL_ACTIVITY, true);

    // set delay on login
    ai->SetNextCheckDelay(urand(2000, 4000));

    botAI->TellMaster("Hello!");
}

std::string PlayerbotHolder::ProcessBotCommand(std::string const& cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig->enabled || guid.IsEmpty())
        return "bot system is disabled";

    uint32 botAccount = sObjectMgr->GetPlayerAccountIdByGUID(guid.GetCounter());
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(guid.GetCounter());
    bool isRandomAccount = sPlayerbotAIConfig->IsInRandomAccountList(botAccount);
    bool isMasterAccount = (masterAccountId == botAccount);

    if (!isRandomAccount && !isMasterAccount && !admin && masterguid)
    {
        Player* master = ObjectAccessor::FindConnectedPlayer(masterguid);
        if (master && (!sPlayerbotAIConfig->allowGuildBots || !masterGuildId || (masterGuildId && master->GetGuildIdFromStorage(guid.GetCounter()) != masterGuildId)))
            return "not in your guild or account";
    }

    if (cmd == "add" || cmd == "login")
    {
        if (ObjectAccessor::FindPlayer(guid))
            return "player already logged in";

        AddPlayerBot(guid, masterAccountId);
        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        if (!ObjectAccessor::FindPlayer(guid))
            return "player is offline";

        if (!GetPlayerBot(guid))
            return "not your bot";

        LogoutPlayerBot(guid);
        return "ok";
    }

    if (admin)
    {
        Player* bot = GetPlayerBot(guid);
        if (!bot)
            bot = sRandomPlayerbotMgr->GetPlayerBot(guid);

        if (!bot)
            return "bot not found";

        if (Player* master = bot->GetPlayerbotAI()->GetMaster())
        {
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=legendary" || cmd == "init=yellow")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false);
                return "ok";
            }
        }

        if (cmd == "levelup" || cmd == "level")
        {
            PlayerbotFactory factory(bot, bot->getLevel());
            factory.Randomize(true);
            return "ok";
        }
        else if (cmd == "refresh")
        {
            PlayerbotFactory factory(bot, bot->getLevel());
            factory.Refresh();
            return "ok";
        }
        else if (cmd == "random")
        {
            sRandomPlayerbotMgr->Randomize(bot);
            return "ok";
        }
    }

    return "unknown command";
}

bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
	if (!sPlayerbotAIConfig->enabled)
	{
		handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return false;
	}

    WorldSession* m_session = handler->GetSession();
    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = player->GetPlayerbotMgr();
    if (!mgr)
    {
        handler->PSendSysMessage("you cannot control bots yet");
        return false;
    }

    std::vector<std::string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("%s", i->c_str());
    }

    return true;
}

std::vector<std::string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    std::vector<std::string> messages;

    if (!*args)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return std::move(messages);
    }

    char* cmd = strtok ((char*)args, " ");
    char* charname = strtok (nullptr, " ");
    if (!cmd)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return std::move(messages);
    }

    if (!strcmp(cmd, "list"))
    {
        messages.push_back(ListBots(master));
        return std::move(messages);
    }

    if (!strcmp(cmd, "reload"))
    {
        messages.push_back("Reloading config");
        sPlayerbotAIConfig->Initialize();
        return messages;
    }

    if (!strcmp(cmd, "tweak"))
    {
        sPlayerbotAIConfig.tweakValue = sPlayerbotAIConfig.tweakValue++;
        if (sPlayerbotAIConfig.tweakValue > 2)
            sPlayerbotAIConfig.tweakValue = 0;

        messages.push_back("Set tweakvalue to " + to_string(sPlayerbotAIConfig.tweakValue));
        return messages;
    }

    if (!strcmp(cmd, "self"))
    {
        if (master->GetPlayerbotAI())
        {
            messages.push_back("Disable player botAI");
            DisablePlayerBot(master->GetGUID());
        }
        else if (sPlayerbotAIConfig->selfBotLevel == 0)
            messages.push_back("Self-bot is disabled");
        else if (sPlayerbotAIConfig->selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            messages.push_back("You do not have permission to enable player botAI");
        else
        {
            messages.push_back("Enable player botAI");
            OnBotLogin(master);
        }

        return messages;
    }

    if (!charname)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return std::move(messages);
    }

    std::string cmdStr = cmd;
    std::string charnameStr = charname;

    std::set<std::string> bots;
    if (charnameStr == "*" && master)
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            messages.push_back("you must be in group");
            return std::move(messages);
        }

        Group::MemberSlotList slots = group->GetMemberSlots();
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
			ObjectGuid member = i->guid;

			if (member == master->GetGUID())
				continue;

			std::string bot;
			if (sObjectMgr->GetPlayerNameByGUID(member.GetCounter(), bot))
			    bots.insert(bot);
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER)
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            if (Player* bot = i->second)
                if (bot->IsInWorld())
                    bots.insert(bot->GetName());
        }
    }

    std::vector<std::string> chars = split(charnameStr, ',');
    for (std::vector<std::string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        std::string s = *i;

        uint32 accountId = GetAccountId(s);
        if (!accountId)
        {
            bots.insert(s);
            continue;
        }

        QueryResult results = CharacterDatabase.PQuery("SELECT name FROM characters WHERE account = '%u'", accountId);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string charName = fields[0].GetString();
                bots.insert(charName);
            } while (results->NextRow());
        }
	}

    for (std::set<std::string>::iterator i = bots.begin(); i != bots.end(); ++i)
    {
        std::string bot = *i;

        std::ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sObjectMgr->GetPlayerGUIDByName(bot);
        if (!member)
        {
            out << "character not found";
        }
        else if (master && member != master->GetGUID())
        {
            out << ProcessBotCommand(cmdStr, member, master->GetGUID(), master->GetSession()->GetSecurity() >= SEC_GAMEMASTER, master->GetSession()->GetAccountId(), master->GetGuildId());
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, ObjectGuid::Empty, true, -1, -1);
        }

        messages.push_back(out.str());
    }

    return std::move(messages);
}

uint32 PlayerbotHolder::GetAccountId(std::string const& name)
{
    uint32 accountId = 0;

    if (QueryResult results = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", name.c_str()))
    {
        Field* fields = results->Fetch();
        accountId = fields[0].GetUInt32();
    }

    return accountId;
}

std::string PlayerbotHolder::ListBots(Player* master)
{
    std::set<std::string> bots;
    std::map<uint8, std::string> classNames;

    classNames[CLASS_DEATH_KNIGHT] = "Death Knight";
    classNames[CLASS_DRUID] = "Druid";
    classNames[CLASS_HUNTER] = "Hunter";
    classNames[CLASS_MAGE] = "Mage";
    classNames[CLASS_PALADIN] = "Paladin";
    classNames[CLASS_PRIEST] = "Priest";
    classNames[CLASS_ROGUE] = "Rogue";
    classNames[CLASS_SHAMAN] = "Shaman";
    classNames[CLASS_WARLOCK] = "Warlock";
    classNames[CLASS_WARRIOR] = "Warrior";
    classNames[CLASS_DEATH_KNIGHT] = "DeathKnight";

    std::map<std::string, std::string> online;
    std::list<std::string> names;
    std::map<std::string, std::string> classes;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        std::string name = bot->GetName();
        bots.insert(name);

        names.push_back(name);
        online[name] = "+";
        classes[name] = classNames[bot->getClass()];
    }

    if (master)
    {
        QueryResult results = CharacterDatabase.PQuery("SELECT class, name FROM characters WHERE account = '%u'", master->GetSession()->GetAccountId());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint8 cls = fields[0].GetUInt8();
                std::string name = fields[1].GetString();
                if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
                {
                    names.push_back(name);
                    online[name] = "-";
                    classes[name] = classNames[cls];
                }
            } while (results->NextRow());
        }
    }

    names.sort();

    if (Group* group = master->GetGroup())
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player *member = ObjectAccessor::FindPlayer(itr->guid);
            if (member && sRandomPlayerbotMgr->IsRandomBot(member))
            {
                std::string name = member->GetName();

                names.push_back(name);
                online[name] = "+";
                classes[name] = classNames[member->getClass()];
            }
        }
    }

    std::ostringstream out;
    bool first = true;
    out << "Bot roster: ";
    for (std::list<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        if (first)
            first = false;
        else
            out << ", ";

        std::string name = *i;
        out << online[name] << name << " " << classes[name];
    }

    return out.str();
}

PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(),  master(master), lastErrorTell(0)
{
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool /*minimal*/)
{
    SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
    CheckTellErrors(elapsed);
}

void PlayerbotMgr::HandleCommand(uint32 type, std::string const& text)
{
    Player* master = GetMaster();
    if (!master)
        return;

    if (text.find(sPlayerbotAIConfig->commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, text, sPlayerbotAIConfig->commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i);
        }

        return;
    }

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleCommand(type, text, master);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == master)
            bot->GetPlayerbotAI()->HandleCommand(type, text, master);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            break;
        }
        // if master cancelled logout, cancel too
        case CMSG_LOGOUT_CANCEL:
        {
            CancelLogout();
            break;
        }
    }
}

void PlayerbotMgr::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB(false, false);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->SaveToDB(false, false);
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    bot->GetPlayerbotAI()->SetMaster(master);
    bot->GetPlayerbotAI()->ResetStrategies();

    LOG_INFO("playerbots", "Bot %s logged in", bot->GetName().c_str());
}

void PlayerbotMgr::OnPlayerLogin(Player* player)
{
    if (sPlayerbotAIConfig->selfBotLevel > 2)
        HandlePlayerbotCommand("self", player);

    if (!sPlayerbotAIConfig->botAutologin)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.PQuery("SELECT name FROM characters WHERE account = '%u'", accountId);
    if (results)
    {
        std::ostringstream out;
        out << "add ";
        bool first = true;
        do
        {
            Field* fields = results->Fetch();

            if (first)
                first = false;
            else
                out << ",";

            out << fields[0].GetString();
        } while (results->NextRow());

        HandlePlayerbotCommand(out.str().c_str(), player);
    }
}

void PlayerbotMgr::TellError(std::string const& botName, std::string const& text)
{
    std::set<std::string> names = errors[text];
    if (names.find(botName) == names.end())
    {
        names.insert(botName);
    }

    errors[text] = names;
}

void PlayerbotMgr::CheckTellErrors(uint32 elapsed)
{
    time_t now = time(nullptr);
    if ((now - lastErrorTell) < sPlayerbotAIConfig->errorDelay / 1000)
        return;

    lastErrorTell = now;

    for (PlayerBotErrorMap::iterator i = errors.begin(); i != errors.end(); ++i)
    {
        std::string text = i->first;
        std::set<std::string> names = i->second;

        std::ostringstream out;
        bool first = true;
        for (std::set<std::string>::iterator j = names.begin(); j != names.end(); ++j)
        {
            if (!first)
                out << ", ";
            else
                first = false;

            out << *j;
        }

        out << "|cfff00000: " << text;

        ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }

    errors.clear();
}
