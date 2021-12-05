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
    if (!sServerFacade.UnitIsDead(bot))
        return false;

    if (bot->InArena())
        return false;

    if (bot->InBattleground())
        return !bot->GetCorpse();

    if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return false;

    if (!bot->GetGroup())
        return true;

    if (!ai->GetGroupMaster())
        return true;

    if (ai->GetGroupMaster() == bot)
        return true;

    if (!ai->HasActivePlayerMaster())
        return true;

    if (ai->HasActivePlayerMaster() && ai->GetGroupMaster()->GetMapId() == bot->GetMapId() && bot->GetMap() && (bot->GetMap()->IsRaid() || bot->GetMap()->IsDungeon()))
        return false;

    if (sServerFacade.UnitIsDead(ai->GetGroupMaster()))
        return true;

    if (sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig.sightDistance))
        return true;

    return false;
}

bool RepopAction::Execute(Event event)
{
    LOG_INFO("playerbots", "Bot %s %s:%d <%s> repops at graveyard", bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str());

    int64 deadTime;

    Corpse* corpse = bot->GetCorpse();
    if (corpse)
        deadTime = time(nullptr) - corpse->GetGhostTime();
    else if (bot->IsDead())
        deadTime = 0;
    else
        deadTime = 60 * MINUTE;

    uint32 dCount = AI_VALUE(uint32, "death count");

    WorldSafeLocsEntry const* ClosestGrave = GetGrave(dCount > 10 || deadTime > 30 * MINUTE);

    if (!ClosestGrave)
        return false;

    bot->TeleportTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, ClosestGrave->o);

    RESET_AI_VALUE(bool, "combat::self target");
    RESET_AI_VALUE(WorldPosition, "current position");

    return true;
}

bool RepopAction::isUseful()
{
    if (bot->InBattleGround())
        return false;

    return true;
}
