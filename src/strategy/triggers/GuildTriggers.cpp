/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildTriggers.h"
#include "Playerbot.h"

bool PetitionTurnInTrigger::IsActive()
{
    return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) && AI_VALUE(uint8, "petition signs") >= sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS);
}

bool BuyTabardTrigger::IsActive()
{
    return bot->GetGuildId() && !AI_VALUE2(uint32, "item count", chat->formatQItem(5976));
}

bool LeaveLargeGuildTrigger::IsActive()
{
	if (!bot->GetGuildId())
		return false;

	if (ai->IsRealPlayer())
		return false;

	if (ai->IsAlt())
		return false;

	GuilderType type = ai->GetGuilderType();

	Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());

	Player* leader = sObjectMgr.GetPlayer(guild->GetLeaderGuid());

	//Only leave the guild if we know the leader is not a real player.
	if (!leader || !leader->GetPlayerbotAI() || leader->GetPlayerbotAI()->IsRealPlayer())
		return false;

	if (type == GuilderType::SOLO && guild->GetLeaderGuid() != bot->GetObjectGuid())
		return true;

	uint32 members = guild->GetMemberSize();
	uint32 maxMembers = uint8(type);

	return members > maxMembers;
}
