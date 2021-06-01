/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SayAction.h"
#include "Event.h"
#include "Playerbot.h"

std::map<std::string, std::vector<std::string>> SayAction::stringTable;
std::map<std::string, uint32> SayAction::probabilityTable;

SayAction::SayAction(PlayerbotAI* botAI) : Action(botAI, "say"), Qualified()
{
}

void replaceAll(std::string& str, std::string const& from, std::string const& to)
{
    if (from.empty())
        return;

    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

bool SayAction::Execute(Event event)
{
	if (stringTable.empty())
	{
		QueryResult results = CharacterDatabase.PQuery("SELECT name, text, type FROM ai_playerbot_speech");
		if (results)
		{
            do
            {
                Field* fields = results->Fetch();
                std::string const& name = fields[0].GetString();
                std::string text = fields[1].GetString();
                std::string const& type = fields[2].GetString();

                if (type == "yell")
                    text = "/y " + text;

                stringTable[name].push_back(text);
            } while (results->NextRow());
		}
	}

	if (probabilityTable.empty())
	{
        QueryResult results = CharacterDatabase.PQuery("SELECT name, probability FROM ai_playerbot_speech_probability");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string const& name = fields[0].GetString();
                uint32 probability = fields[1].GetUInt32();

                probabilityTable[name] = probability;
            } while (results->NextRow());
		}
	}

    std::vector<std::string>& strings = stringTable[qualifier];
    if (strings.empty())
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    uint32 nextTime = time(0) + urand(1, 30);
    botAI->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    if (Group* group = bot->GetGroup())
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (PlayerbotAI* memberAi = member->GetPlayerbotAI())
                members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                uint32 i1 = urand(0, count - 1);
                uint32 i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        uint32 index = 0;
        for (Player* player : members)
        {
            PlayerbotAI* memberAi = player->GetPlayerbotAI();
            memberAi->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    uint32 probability = probabilityTable[qualifier];
    if (!probability)
        probability = 100;

    if (urand(0, 100) >= probability)
        return false;

    uint32 idx = urand(0, strings.size() - 1);
    std::string text = strings[idx];

    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target)
        target = AI_VALUE(Unit*, "current target");

    if (target)
        replaceAll(text, "<target>", target->GetName());

    replaceAll(text, "<randomfaction>", IsAlliance(bot->getRace()) ? "Alliance" : "Horde");

    if (bot->GetMap())
    {
        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetAreaId()))
            replaceAll(text, "<subzone>", area->area_name[0]);
    }

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), LANG_UNIVERSAL);
    else
        bot->Say(text, LANG_UNIVERSAL);

    return true;
}

bool SayAction::isUseful()
{
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(0) - lastSaid) > 30;
}
