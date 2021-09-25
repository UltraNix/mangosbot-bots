/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReleaseSpiritAction.h"
#include "Event.h"
#include "Playerbot.h"

bool ReleaseSpiritAction::Execute(Event event)
{
    if (bot->IsAlive())
    {
        botAI->TellMasterNoFacing("I am not dead, will wait here");
        botAI->ChangeStrategy("-follow,+stay", BOT_STATE_NON_COMBAT);
        return false;
    }

    if (bot->GetCorpse() && bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
    {
        botAI->TellMasterNoFacing("I am already a spirit");
        return false;
    }

    WorldPacket& p = event.getPacket();
    if (!p.empty() && p.GetOpcode() == CMSG_REPOP_REQUEST)
        botAI->TellMasterNoFacing("Releasing...");
    else
        botAI->TellMasterNoFacing("Meet me at the graveyard");

    // Death Count to prevent skeleton piles
    Player* master = GetMaster();
    if (!master || (master && master->GetPlayerbotAI()))
    {
        uint32 dCount = AI_VALUE(uint32, "death count");
        context->GetValue<uint32>("death count")->Set(dCount + 1);
    }

    LOG_INFO("playerbots", "Bot %s %s:%d <%s> released", bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    WorldPacket packet(CMSG_REPOP_REQUEST);
    packet << uint8(0);
    bot->GetSession()->HandleRepopRequestOpcode(packet);

    // add waiting for ress aura
    if (bot->InBattleground() && !botAI->HasAura(2584, bot))
    {
        // cast Waiting for Resurrect
        bot->CastSpell(bot, 2584, true);
    }

    // add waiting for ress aura
    if (bot->InBattleground())
        bot->CastSpell(bot, 2584, true);

    return true;
}

bool AutoReleaseSpiritAction::Execute(Event event)
{
    //Death Count to prevent skeleton piles
    Player* master = GetMaster();
    if (!master || (master && master->GetPlayerbotAI()))
    {
        uint32 dCount = AI_VALUE(uint32, "death count");
        context->GetValue<uint32>("death count")->Set(dCount + 1);
    }

    LOG_INFO("playerbots", "Bot %s %s:%d <%s> auto released", bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    WorldPacket packet(CMSG_REPOP_REQUEST);
    packet << uint8(0);
    bot->GetSession()->HandleRepopRequestOpcode(packet);

    LOG_INFO("playerbots", "Bot %s %s:%d <%s> releases spirit", bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    if (bot->InBattleground() && !botAI->HasAura(2584, bot))
    {
        // cast Waiting for Resurrect
        bot->CastSpell(bot, 2584, true);
    }

    return true;
}

bool AutoReleaseSpiritAction::isUseful()
{
    if (bot->InBattleground())
        return bot->isDead() && !bot->GetCorpse();

    return ((!bot->GetGroup()) || (bot->GetGroup() && botAI->GetGroupMaster() == bot) || (botAI->GetGroupMaster() && botAI->GetGroupMaster() != bot &&
        botAI->GetGroupMaster()->isDead() &&
        bot->getDeathState() != botAI->GetGroupMaster()->getDeathState()))
        && bot->isDead() && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
}

bool RepopAction::Execute(Event event)
{
    LOG_INFO("playerbots", "Bot %s %s:%d <%s> repops at graveyard", bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    bot->RepopAtGraveyard();

    return true;
}

bool RepopAction::isUseful()
{
    return true;
}
