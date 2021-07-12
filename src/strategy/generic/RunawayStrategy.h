/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NonCombatStrategy.h"
class RunawayStrategy : public NonCombatStrategy
{
    public:
        RunawayStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) { }

        std::string const& getName() override { return "runaway"; }
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};
