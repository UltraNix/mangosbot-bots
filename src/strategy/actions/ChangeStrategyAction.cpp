/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChangeStrategyAction.h"
#include "Event.h"
#include "Playerbot.h"

bool ChangeCombatStrategyAction::Execute(Event event)
{
    std::string const& text = event.getParam();
    botAI->ChangeStrategy(text.empty() ? getName() : text, BOT_STATE_COMBAT);
    sPlayerbotDbStore.Save(ai);
    return true;
}

bool ChangeNonCombatStrategyAction::Execute(Event event)
{
    std::string const& text = event.getParam();

    uint32 account = sObjectMgr->GetPlayerAccountIdByGUID(bot->GetGUID());
    if (sPlayerbotAIConfig->IsInRandomAccountList(account) && botAI->GetMaster() && botAI->GetMaster()->GetSession()->GetSecurity() < SEC_GAMEMASTER)
    {
        if (text.find("loot") != std::string::npos || text.find("gather") != std::string::npos)
        {
            botAI->TellError("You can change any strategy except loot and gather");
            return false;
        }
    }

    botAI->ChangeStrategy(text, BOT_STATE_NON_COMBAT);
    sPlayerbotDbStore.Save(ai);
    return true;
}

bool ChangeDeadStrategyAction::Execute(Event event)
{
    std::string const& text = event.getParam();
    botAI->ChangeStrategy(text, BOT_STATE_DEAD);
    return true;
}
