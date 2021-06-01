/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PartyMemberValue.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class PartyMemberWithoutItemValue : public PartyMemberValue, public Qualified
{
    public:
        PartyMemberWithoutItemValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance) : PartyMemberValue(botAI) { }

    protected:
        Unit* Calculate() override;
        virtual FindPlayerPredicate* CreatePredicate();
};

class PartyMemberWithoutFoodValue : public PartyMemberWithoutItemValue
{
    public:
        PartyMemberWithoutFoodValue(PlayerbotAI* botAI) : PartyMemberWithoutItemValue(botAI) { }

    protected:
        FindPlayerPredicate* CreatePredicate() override;
};

class PartyMemberWithoutWaterValue : public PartyMemberWithoutItemValue
{
    public:
        PartyMemberWithoutWaterValue(PlayerbotAI* botAI) : PartyMemberWithoutItemValue(botAI) { }

    protected:
        FindPlayerPredicate* CreatePredicate() override;
};
