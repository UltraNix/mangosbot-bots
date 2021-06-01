/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericSpellActions.h"

class Event;
class PlayerbotAI;

class CastBearFormAction : public CastBuffSpellAction
{
	public:
		CastBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bear form") { }

        bool isPossible() const override;
        bool isUseful() const override;
};

class CastDireBearFormAction : public CastBuffSpellAction
{
	public:
		CastDireBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "dire bear form") { }

        NextAction** getAlternatives() override;
};

class CastCatFormAction : public CastBuffSpellAction
{
	public:
		CastCatFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cat form") { }
};

class CastTreeFormAction : public CastBuffSpellAction
{
    public:
        CastTreeFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "tree of life") { }
};

class CastMoonkinFormAction : public CastBuffSpellAction
{
    public:
        CastMoonkinFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "moonkin form") { }
};

class CastCasterFormAction : public CastBuffSpellAction
{
	public:
		CastCasterFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "caster form") { }

        bool isUseful() const override;
        bool isPossible() const override { return true; }
		bool Execute(Event event) override;
};
