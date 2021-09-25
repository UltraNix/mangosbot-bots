/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHOOSETRAVELTARGETACTION_H
#define _PLAYERBOT_CHOOSETRAVELTARGETACTION_H

#include "MovementActions.h"
#include "TravelMgr.h"

class Event;
class Quest;
class PlayerbotAI;
class Unit;

struct QuestStatusData;

class ChooseTravelTargetAction : public MovementAction
{
    public:
        ChooseTravelTargetAction(PlayerbotAI* botAI, std::string const& name = "choose travel target") : MovementAction(botAI, name) { }

        bool Execute(Event event) override;
        bool isUseful() override;

    private:
        bool SetTarget(TravelTarget* target, TravelTarget* oldTarget);

        bool getBestDestination(std::vector<TravelDestination*>* activeDestinations, std::vector<WorldPosition*>* activePoints);

        bool SetGroupTarget(TravelTarget* target);
        bool SetCurrentTarget(TravelTarget* target, TravelTarget* oldTarget);
        bool SetQuestTarget(TravelTarget* target, bool onlyCompleted = false);
        bool SetNewQuestTarget(TravelTarget* target);
        bool SetRpgTarget(TravelTarget* target);
        bool SetGrindTarget(TravelTarget* target);
        bool SetExploreTarget(TravelTarget* target);
        bool SetNullTarget(TravelTarget* target);

        void ReportTravelTarget(TravelTarget* newTarget, TravelTarget* oldTarget);

        TravelTarget ChooseTarget(TravelTarget* oldTarget);

        TravelTarget getBestTarget(std::vector<TravelDestination*> activeDestinations, std::vector<WorldPosition*> activePoints, bool groupCopy = false);

        TravelTarget GetGroupTarget();
        TravelTarget GetCurrentTarget(TravelTarget* oldTarget);
        TravelTarget GetQuestTarget();
        TravelTarget GetNewQuestTarget();
        TravelTarget GetNullTarget();

        void ReportTravelTarget(TravelTarget* newTarget, TravelTarget* oldTarget);

        virtual bool needForQuest(Unit* target);
        virtual bool needItemForQuest(uint32 itemId, Quest const* questTemplate, QuestStatusData const* questStatus);
};

#endif
