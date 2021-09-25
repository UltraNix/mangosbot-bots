/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUESTVALUES_H
#define _PLAYERBOT_QUESTVALUES_H

#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

struct CreatureData;
struct GameObjectData;

enum class QuestRelationType : uint32
{
    objective1 = 0,
    objective2 = 1,
    objective3 = 2,
    objective4 = 3,
    questGiver = 4,
    questTaker = 5
};

typedef std::unordered_map<QuestRelationType, std::vector<GuidPosition>> questGuidps;
typedef std::unordered_map<uint32, questGuidps> questGuidpMap;

typedef std::unordered_map<uint32, std::vector<GuidPosition>> questGivers;

//Cheat class copy to hack into the loot system
class LootTemplateAccess
{
    public:
        class  LootGroup;                                   // A set of loot definitions for items (refs are not allowed inside)
        typedef std::vector<LootGroup> LootGroups;
        LootStoreItemList Entries;                          // not grouped only
        LootGroups        Groups;                           // groups have own (optimized) processing, grouped entries go there
};

//Generic quest object finder
class FindQuestObjectData
{
    public:
        FindQuestObjectData() { GetObjectiveEntries(); }

        void GetObjectiveEntries();
        bool operator()(CreatureData const& dataPair);
        bool operator()(GameObjectData const& dataPair);
        questGuidpMap GetResult() const { return data; };

    private:
        std::unordered_map<int32, std::vector<std::pair<uint32, QuestRelationType>>> entryMap;
        std::unordered_map<uint32, std::vector<std::pair<uint32, QuestRelationType>>> itemMap;

        questGuidpMap data;
};

//All objects to start, do or finish a quest.
class QuestGuidpMapValue : public SingleCalculatedValue<questGuidpMap>
{
    public:
        QuestGuidpMapValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "quest guidp map") { }

        questGuidpMap Calculate() override;
};

//All questgivers and their quests that are Useful for a specific level
class QuestGiversValue : public SingleCalculatedValue<questGivers>, public Qualified
{
	public:
        QuestGiversValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "quest givers") { }

        questGivers Calculate() override;
};

//All questgivers that have a quest for the bot.
class ActiveQuestGiversValue : public CalculatedValue<std::vector<GuidPosition>>
{
    public:
        ActiveQuestGiversValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest givers", 5) { }

        std::vector<GuidPosition> Calculate() override;
};

//All quest takers that the bot has a quest for.
class ActiveQuestTakersValue : public CalculatedValue<std::vector<GuidPosition>>
{
    public:
        ActiveQuestTakersValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest takers", 5) { }

        std::vector<GuidPosition> Calculate() override;
};

//All objectives that the bot still has to complete.
class ActiveQuestObjectivesValue : public CalculatedValue<std::vector<GuidPosition>>
{
    public:
        ActiveQuestObjectivesValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest objectives", 5) { }

        std::vector<GuidPosition> Calculate() override;
};

//Free quest log slots
class FreeQuestLogSlotValue : public Uint8CalculatedValue
{
    public:
        FreeQuestLogSlotValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI, "free quest log slots") { }

        uint8 Calculate() override;
};

#endif
