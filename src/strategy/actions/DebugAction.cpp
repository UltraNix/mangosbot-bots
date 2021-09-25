/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DebugAction.h"
#include "Event.h"
#include "Playerbot.h"
#include "TravelMgr.h"

bool DebugAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const& text = event.getParam();

    if (text == "scan")
    {
        std::vector<WorldPosition> apos;
        std::vector<WorldPosition> ipos;
        for (auto p : WorldPosition().getCreaturesNear())
        {
            Unit* unit = botAI->GetUnit(p);
            if (unit)
                apos.push_back(WorldPosition(p));
            else
                ipos.push_back(WorldPosition(p));
        }

        std::ostringstream out;
        out << "1,";
        WorldPosition().printWKT(apos, out);
        out << "\n0,";
        WorldPosition().printWKT(ipos, out);

        sPlayerbotAIConfig->log("active.csv", out.str().c_str());

    }

    std::string const& response = botAI->HandleRemoteCommand(text);
    botAI->TellMaster(response);
    return true;
}
