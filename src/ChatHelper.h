/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATHELPER_H
#define _PLAYERBOT_CHATHELPER_H

#include "Common.h"
#include "ObjectGuid.h"
#include "PlayerbotAIAware.h"
#include "SharedDefines.h"

class GameObject;
class Quest;
class PlayerbotAI;
class WorldObject;

struct ItemTemplate;

typedef std::set<uint32> ItemIds;
typedef std::set<uint32> SpellIds;

class ChatHelper : public PlayerbotAIAware
{
    public:
        ChatHelper(PlayerbotAI* botAI);

        static std::string formatMoney(uint32 copper);
        static uint32 parseMoney(std::string const& text);
        static ItemIds parseItems(std::string const& text);
        uint32 parseSpell(std::string const& text);
        static std::string formatQuest(Quest const* quest);
        static std::string formatItem(ItemTemplate const* proto, uint32 count = 0, uint32 total = 0);
        static string formatQItem(uint32 itemId);
        static std::string formatSpell(SpellInfo const* spellInfo);
        static std::string formatGameobject(GameObject* go);
        static std::string formatWorldobject(WorldObject* wo);
        static std::string formatWorldEntry(int32 entry);
        static std::string formatQuestObjective(std::string const& name, uint32 available, uint32 required);
        static GuidVector parseGameobjects(std::string const& text);

        static ChatMsg parseChat(std::string const& text);
        static std::string formatChat(ChatMsg chat);

        static std::string formatClass(Player* player, int8 spec);
        static std::string formatClass(uint8 cls);
        static std::string formatRace(uint8 race);
        static std::string formatSkill(uint32 skill);
        static std::string formatBoolean(bool flag);

        static uint32 parseItemQuality(std::string const& text);
        static bool parseItemClass(std::string const& text, uint32* itemClass, uint32* itemSubClass);
        static uint32 parseSlot(std::string const& text);
        uint32 parseSkill(std::string const& text);

        static bool parseable(std::string const& text);

        void eraseAllSubStr(std::string& mainStr, std::string const& toErase);

    private:
        static std::map<std::string, uint32> consumableSubClasses;
        static std::map<std::string, uint32> tradeSubClasses;
        static std::map<std::string, uint32> itemQualities;
        static std::map<std::string, uint32> projectileSubClasses;
        static std::map<std::string, uint32> slots;
        static std::map<std::string, uint32> skills;
        static std::map<std::string, ChatMsg> chats;
        static std::map<uint8, std::string> classes;
        static std::map<uint8, std::string> races;
        static std::map<uint8, std::map<uint8, std::string>> specs;
};

#endif
