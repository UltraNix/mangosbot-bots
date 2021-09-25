/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HEALPRIESTSTRATEGY_H
#define _PLAYERBOT_HEALPRIESTSTRATEGY_H

#include "GenericPriestStrategy.h"

class PlayerbotAI;

class HealPriestStrategy : public GenericPriestStrategy
{
    public:
        HealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI) { }

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        NextAction** getDefaultActions() override;
        std::string const& getName() override { return "heal"; }
		uint32 GetType() const override { return STRATEGY_TYPE_HEAL; }
};

#endif
