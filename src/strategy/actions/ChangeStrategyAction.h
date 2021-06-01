/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"

class Event;
class PlayerbotAI;

class ChangeCombatStrategyAction : public Action
{
	public:
		ChangeCombatStrategyAction(PlayerbotAI* botAI, std::string const& name = "co") : Action(botAI, name) { }

        bool Execute(Event event) override;
};

class ChangeNonCombatStrategyAction : public Action
{
    public:
        ChangeNonCombatStrategyAction(PlayerbotAI* botAI) : Action(botAI, "nc") { }

        bool Execute(Event event) override;
};

class ChangeDeadStrategyAction : public Action
{
    public:
        ChangeDeadStrategyAction(PlayerbotAI* botAI) : Action(botAI, "dead") { }

        bool Execute(Event event) override;
};
