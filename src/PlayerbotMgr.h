/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTMGR_H
#define _PLAYERBOT_PLAYERBOTMGR_H

#include "Common.h"
#include "QueryHolder.h"
#include "QueryResult.h"
#include "Player.h"
#include "PlayerbotAIBase.h"

class ChatHandler;
class WorldPacket;

typedef std::map<ObjectGuid, Player*> PlayerBotMap;
typedef std::map<std::string, std::set<std::string> > PlayerBotErrorMap;

class PlayerbotHolder : public PlayerbotAIBase
{
    public:
        PlayerbotHolder();
        virtual ~PlayerbotHolder() { };

        void AddPlayerBot(ObjectGuid guid, uint32 masterAccountId);

        void LogoutPlayerBot(ObjectGuid guid);
        void DisablePlayerBot(ObjectGuid guid);
        Player* GetPlayerBot (ObjectGuid guid) const;
        PlayerBotMap::const_iterator GetPlayerBotsBegin() const { return playerBots.begin(); }
        PlayerBotMap::const_iterator GetPlayerBotsEnd()   const { return playerBots.end();   }

        void UpdateAIInternal(uint32 elapsed) override { };
        void UpdateSessions(uint32 elapsed);

        void LogoutAllBots();
        void OnBotLogin(Player* const bot);

        std::vector<std::string> HandlePlayerbotCommand(char const* args, Player* master = nullptr);
        std::string ProcessBotCommand(std::string const& cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId);
        uint32 GetAccountId(std::string const& name);
        std::string ListBots(Player* master);

    protected:
        virtual void OnBotLoginInternal(Player* const bot) = 0;

        PlayerBotMap playerBots;
};

class PlayerbotMgr : public PlayerbotHolder
{
    public:
        PlayerbotMgr(Player* const master);
        virtual ~PlayerbotMgr() { };

        static bool HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args);
        void HandleMasterIncomingPacket(WorldPacket const& packet);
        void HandleMasterOutgoingPacket(WorldPacket const& packet);
        void HandleCommand(uint32 type, std::string const& text);
        void OnPlayerLogin(Player* player);

        void UpdateAIInternal(uint32 elapsed) override;
        void TellError(std::string const& botName, std::string const& text);

        Player* GetMaster() const { return master; };

        void SaveToDB();

    protected:
        void OnBotLoginInternal(Player* const bot) override;
        void CheckTellErrors(uint32 elapsed);

    private:
        Player* const master;
        PlayerBotErrorMap errors;
        time_t lastErrorTell;
};

#endif
