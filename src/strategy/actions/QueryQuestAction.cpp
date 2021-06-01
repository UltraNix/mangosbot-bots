/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "QueryQuestAction.h"
#include "Event.h"
#include "ChatHelper.h"
#include "Playerbot.h"

void QueryQuestAction::TellObjective(std::string const& name, uint32 available, uint32 required)
{
    botAI->TellMaster(chat->formatQuestObjective(name, available, required));
}

bool QueryQuestAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    std::string const& text = event.getParam();

    PlayerbotChatHandler ch(bot);
    uint32 questId = ch.extractQuestId(text);
    if (!questId)
        return false;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        if (questId != bot->GetQuestSlotQuestId(slot))
            continue;

        std::ostringstream out;
        out << "--- " << chat->formatQuest(sObjectMgr->GetQuestTemplate(questId)) << " ";

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        {
            out << "|c0000FF00completed|r ---";
            botAI->TellMaster(out);
        }
        else
        {
            out << "|c00FF0000not completed|r ---";
            botAI->TellMaster(out);
            TellObjectives(questId);
        }

        return true;
    }

    return false;
}

void QueryQuestAction::TellObjectives(uint32 questId)
{
    Quest const* questTemplate = sObjectMgr->GetQuestTemplate(questId);
    QuestStatusData questStatus = bot->getQuestStatusMap()[questId];

    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (!questTemplate->ObjectiveText[i].empty())
            botAI->TellMaster(questTemplate->ObjectiveText[i]);

        if (questTemplate->RequiredItemId[i])
        {
            uint32 required = questTemplate->RequiredItemCount[i];
            uint32 available = questStatus.ItemCount[i];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(questTemplate->RequiredItemId[i]);
            TellObjective(chat->formatItem(proto), available, required);
        }

        if (questTemplate->RequiredNpcOrGo[i])
        {
            uint32 required = questTemplate->RequiredNpcOrGoCount[i];
            uint32 available = questStatus.CreatureOrGOCount[i];

            if (questTemplate->RequiredNpcOrGo[i] < 0)
            {
                if (GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(-questTemplate->RequiredNpcOrGo[i]))
                    TellObjective(info->name, available, required);
            }
            else
            {

                if (CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(questTemplate->RequiredNpcOrGo[i]))
                    TellObjective(info->Name, available, required);
            }
        }
    }
}
