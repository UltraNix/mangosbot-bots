/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"
class ReviveFromCorpseAction : public MovementAction
{
	public:
		ReviveFromCorpseAction(PlayerbotAI* botAI) : MovementAction(botAI, "revive from corpse") { }

        bool Execute(Event event) override;
};

class FindCorpseAction : public MovementAction
{
    public:
        FindCorpseAction(PlayerbotAI* botAI) : MovementAction(botAI, "find corpse") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

class SpiritHealerAction : public Action
{
	public:
	    SpiritHealerAction(PlayerbotAI* botAI) : Action(botAI, "spirit healer") { }

        bool Execute(Event event) override;
};
