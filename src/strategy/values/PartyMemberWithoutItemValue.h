/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PARTYMEMBERWITHOUTITEMVALUE_H
#define _PLAYERBOT_PARTYMEMBERWITHOUTITEMVALUE_H

#include "PartyMemberValue.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class PartyMemberWithoutItemValue : public PartyMemberValue, public Qualified
{
    public:
        PartyMemberWithoutItemValue(PlayerbotAI* botAI, string name = "party member without item", float range = sPlayerbotAIConfig->farDistance) : PartyMemberValue(botAI, name) { }

    protected:
        Unit* Calculate() override;
        virtual FindPlayerPredicate* CreatePredicate();
};

class PartyMemberWithoutFoodValue : public PartyMemberWithoutItemValue
{
    public:
        PartyMemberWithoutFoodValue(PlayerbotAI* botAI, string name = "party member without food") : PartyMemberWithoutItemValue(ai, name) {}

    protected:
        FindPlayerPredicate* CreatePredicate() override;
};

class PartyMemberWithoutWaterValue : public PartyMemberWithoutItemValue
{
    public:
        PartyMemberWithoutWaterValue(PlayerbotAI* botAI, string name = "party member without water") : PartyMemberWithoutItemValue(ai, name) {}

    protected:
        FindPlayerPredicate* CreatePredicate() override;
};

#endif
