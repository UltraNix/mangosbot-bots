/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildAcceptAction.h"
#include "../Event.h"
#include "../../Playerbot.h"
#include "../../PlayerbotSecurity.h"

bool GuildAcceptAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    bool accept = true;
    uint32 guildId = master->GetGuildId();
    if (!guildId)
    {
        botAI->TellError("You are not in a guild");
        accept = false;
    }
    else if (bot->GetGuildId())
    {
        botAI->TellError("Sorry, I am in a guild already");
        accept = false;
    }
    else if (!botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, master, true))
    {
        accept = false;
    }

    WorldPacket packet;
    if (accept)
    {
        bot->SetGuildIdInvited(guildId);
        bot->GetSession()->HandleGuildAcceptOpcode(packet);
    }
    else
    {
        bot->GetSession()->HandleGuildDeclineOpcode(packet);
    }

    return true;
}
