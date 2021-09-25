/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACTION_H
#define _PLAYERBOT_ACTION_H

#include "AiObject.h"
#include "Common.h"
#include "Event.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class NextAction
{
    public:
        NextAction(std::string name, float relevance = 0.0f) : name(name), relevance(relevance) { }
        NextAction(NextAction const& o) : name(o.name), relevance(o.relevance) { }

        std::string const& getName() { return name; }
        float getRelevance() {return relevance;}

        static uint32 size(NextAction** actions);
        static NextAction** clone(NextAction** actions);
        static NextAction** merge(NextAction** what, NextAction** with);
        static NextAction** array(uint32 nil,...);
        static void destroy(NextAction** actions);

    private:
        float relevance;
        std::string name;
};

class Action : public AiNamedObject
{
	public:
        enum ActionThreatType
        {
            ACTION_THREAT_NONE      = 0,
            ACTION_THREAT_SINGLE    = 1,
            ACTION_THREAT_AOE       = 2
        };

        Action(PlayerbotAI* botAI, std::string name = "action") : verbose(false), AiNamedObject(botAI, name) { }
        virtual ~Action(void) { }

        virtual bool Execute(Event event) { return true; }
        virtual bool isPossible() { return true; }
        virtual bool isUseful() { return true; }
        virtual NextAction** getPrerequisites() { return nullptr; }
        virtual NextAction** getAlternatives() { return nullptr; }
        virtual NextAction** getContinuers() { return nullptr; }
        virtual ActionThreatType getThreatType() { return ACTION_THREAT_NONE; }
        void Update() { }
        void Reset() { }
        virtual Unit* GetTarget();
        virtual Value<Unit*>* GetTargetValue();
        virtual std::string const& GetTargetName() { return "self target"; }
        void MakeVerbose() { verbose = true; }
        void setRelevance(uint32 relevance1) { relevance = relevance1; };
        virtual float getRelevance() { return relevance; }

    protected:
        bool verbose;
        float relevance = 0;
};

class ActionNode
{
    public:
        ActionNode(std::string name, NextAction** prerequisites = nullptr, NextAction** alternatives = nullptr, NextAction** continuers = nullptr) :
            action(nullptr), name(name), prerequisites(prerequisites), alternatives(alternatives), continuers(continuers) { }

        virtual ~ActionNode()
        {
            NextAction::destroy(prerequisites);
            NextAction::destroy(alternatives);
            NextAction::destroy(continuers);
        }

        Action* getAction() { return action; }
        void setAction(Action* action) { this->action = action; }
        std::string const& getName() { return name; }

        NextAction** getContinuers() { return NextAction::merge(NextAction::clone(continuers), action->getContinuers()); }
        NextAction** getAlternatives() { return NextAction::merge(NextAction::clone(alternatives), action->getAlternatives()); }
        NextAction** getPrerequisites() { return NextAction::merge(NextAction::clone(prerequisites), action->getPrerequisites()); }

    private:
        std::string name;
        Action* action;
        NextAction** continuers;
        NextAction** alternatives;
        NextAction** prerequisites;
};

class ActionBasket
{
	public:
        ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event) :
            action(action), relevance(relevance), skipPrerequisites(skipPrerequisites), event(event), created(time(nullptr)) { }

        virtual ~ActionBasket(void) { }

		float getRelevance() {return relevance;}
		ActionNode* getAction() {return action;}
        Event getEvent() { return event; }
        bool isSkipPrerequisites() { return skipPrerequisites; }
        void AmendRelevance(float k) {relevance *= k; }
        void setRelevance(float relevance) { this->relevance = relevance; }
        bool isExpired(time_t secs) { return time(nullptr) - created >= secs; }

	private:
		ActionNode* action;
		float relevance;
        bool skipPrerequisites;
        Event event;
        time_t created;
};

#endif
