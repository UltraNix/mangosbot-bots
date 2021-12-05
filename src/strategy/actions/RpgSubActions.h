/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGSUBACTIONS_H
#define _PLAYERBOT_RPGSUBACTIONS_H

#include "Action.h"
#include "AiObject.h"

class GuidPosition;
class Event;
class ObjectGuid;
class PlayerbotAI;

class RpgHelper : public AiObject
{
    public:
        RpgHelper(PlayerbotAI* ai) : AiObject(ai) {}

        void OnExecute(string nextAction = "rpg");
        void BeforeExecute();
        void AfterExecute(bool doDelay = true,  bool waitForGroup = false);

        virtual GuidPosition guidP();
        virtual ObjectGuid guid();
        virtual bool InRange();

    private:
        void setFacingTo(GuidPosition guidPosition);
        void setFacing(GuidPosition guidPosition);
        void setDelay(bool waitForGroup);
};

class RpgEnabled
{
    public:
        RpgEnabled(PlayerbotAI* ai)
        {
            rpg = make_unique<RpgHelper>(ai);
        }

    protected:
        unique_ptr<RpgHelper> rpg;
};

class RpgSubAction : public Action, public RpgEnabled
{
    public:
        RpgSubAction(PlayerbotAI* ai, string name = "rpg sub") : Action(ai, name), RpgEnabled(ai) {}

        //Long range is possible?
        bool isPossible() override;
        //Short range can we do the action now?
        bool isUseful() override;

        bool Execute(Event event) override;

    protected:
        virtual string ActionName();
        virtual Event ActionEvent(Event event);
};

class RpgStayAction : public RpgSubAction
{
    public:
        RpgStayAction(PlayerbotAI* ai, string name = "rpg stay") : RpgSubAction(ai, name) {}

        bool isUseful() override;
        bool Execute(Event event) override;
};

class RpgWorkAction : public RpgSubAction
{
    public:
        RpgWorkAction(PlayerbotAI* ai, string name = "rpg work") : RpgSubAction(ai, name ) {}

        bool isUseful() override;
        bool Execute(Event event) override;
};

class RpgEmoteAction : public RpgSubAction
{
    public:
        RpgEmoteAction(PlayerbotAI* ai, string name = "rpg emote") : RpgSubAction(ai, name) {}

        bool isUseful() override;
        bool Execute(Event event) override;
};

class RpgCancelAction : public RpgSubAction
{
    public:
        RpgCancelAction(PlayerbotAI* ai, string name = "rpg cancel") : RpgSubAction(ai, name) {}

        bool Execute(Event event) override;
};

class RpgTaxiAction : public RpgSubAction
{
    public:
        RpgTaxiAction(PlayerbotAI* ai, string name = "rpg taxi") : RpgSubAction(ai, name) {}

        bool isUseful() override;
        bool Execute(Event event) override;
};

class RpgDiscoverAction : public RpgTaxiAction
{
    public:
        RpgDiscoverAction(PlayerbotAI* ai, string name = "rpg discover") : RpgTaxiAction(ai,name) {}

        bool Execute(Event event) override;
};

class RpgStartQuestAction : public RpgSubAction
{
    public:
        RpgStartQuestAction(PlayerbotAI* ai, string name = "rpg start quest") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgEndQuestAction : public RpgSubAction
{
    public:
        RpgEndQuestAction(PlayerbotAI* ai, string name = "rpg end quest") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgBuyAction : public RpgSubAction
{
    public:
        RpgBuyAction(PlayerbotAI* ai, string name = "rpg buy") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgSellAction : public RpgSubAction
{
    public:
        RpgSellAction(PlayerbotAI* ai, string name = "rpg sell") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgRepairAction : public RpgSubAction
{
    public:
        RpgRepairAction(PlayerbotAI* ai, string name = "rpg repair") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
};

class RpgTrainAction : public RpgSubAction
{
    public:
        RpgTrainAction(PlayerbotAI* ai, string name = "rpg train") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
};

class RpgHealAction : public RpgSubAction
{
    public:
        RpgHealAction(PlayerbotAI* ai, string name = "rpg heal") : RpgSubAction(ai, name) {}

        bool Execute(Event event) override;
};

class RpgHomeBindAction : public RpgSubAction
{
    public:
        RpgHomeBindAction(PlayerbotAI* ai, string name = "rpg home bind") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
};

class RpgQueueBgAction : public RpgSubAction
{
    public:
        RpgQueueBgAction(PlayerbotAI* ai, string name = "rpg queue bg") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
};

class RpgBuyPetitionAction : public RpgSubAction
{
    public:
        RpgBuyPetitionAction(PlayerbotAI* ai, string name = "rpg buy petition") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
};

class RpgUseAction : public RpgSubAction
{
    public:
        RpgUseAction(PlayerbotAI* ai, string name = "rpg use") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgSpellAction : public RpgSubAction
{
    public:
        RpgSpellAction(PlayerbotAI* ai, string name = "rpg spell") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgCraftAction : public RpgSubAction
{
    public:
        RpgCraftAction(PlayerbotAI* ai, string name = "rpg craft") : RpgSubAction(ai, name) {}

    private:
        string ActionName() override;
        Event ActionEvent(Event event) override;
};

class RpgTradeUsefulAction : public RpgSubAction
{
    public:
        RpgTradeUsefulAction(PlayerbotAI* ai, string name = "rpg trade useful") : RpgSubAction(ai, name) {}

        list<Item*> CanGiveItems(GuidPosition guidPosition);

        bool Execute(Event event) override;
};

class RpgDuelAction : public RpgSubAction
{
    public:
        RpgDuelAction(PlayerbotAI* ai, string name = "rpg duel") : RpgSubAction(ai, name) {}

        bool isUseful() override;
        bool Execute(Event event) override;
};

#endif
