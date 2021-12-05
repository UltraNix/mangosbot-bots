/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDCREATEACTION_H
#define _PLAYERBOT_GUILDCREATEACTION_H

#include "InventoryAction.h"
#include "ChooseTravelTargetAction.h"

class Event;
class PlayerbotAI;

class BuyPetitionAction : public InventoryAction
{
    public:
        BuyPetitionAction(PlayerbotAI* ai) : InventoryAction(ai, "buy petition") {}

        bool Execute(Event event) override;
        bool isUseful() override;

        static bool canBuyPetition(Player* bot);
};

class PetitionOfferAction : public Action
{
    public:
        PetitionOfferAction(PlayerbotAI* ai, string name = "petition offer") : Action(ai, name) {}

        bool Execute(Event event) override;
        bool isUseful() override;
};

class PetitionOfferNearbyAction : public PetitionOfferAction
{
    public:
        PetitionOfferNearbyAction(PlayerbotAI* ai) : PetitionOfferAction(ai, "petition offer nearby") {}

        bool Execute(Event event) override;
        bool isUseful() override;
};

class PetitionTurnInAction : public ChooseTravelTargetAction
{
    public:
        PetitionTurnInAction(PlayerbotAI* ai) : ChooseTravelTargetAction(ai, "turn in petitn") {}

        bool Execute(Event event) override;
        bool isUseful() override;
};

class BuyTabardAction : public ChooseTravelTargetAction
{
    public:
        BuyTabardAction(PlayerbotAI* ai) : ChooseTravelTargetAction(ai, "buy tabard") {}

        bool Execute(Event event) override;
        bool isUseful() override;
};

#endif
