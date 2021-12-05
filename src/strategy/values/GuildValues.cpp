/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildValues.h"
#include "Playerbot.h"

uint8 PetitionSignsValue::Calculate()
{
    if (bot->GetGuildId())
       return 0;

    list<Item*> petitions = AI_VALUE2(list<Item*>, "inventory items", chat->formatQItem(5863));

    if (petitions.empty())
        return 0;

    QueryResult* result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitions.front()->GetObjectGuid().GetCounter());

    return result ? (uint8)result->GetRowCount() : 0;
};
