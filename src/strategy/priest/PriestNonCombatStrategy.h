/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NonCombatStrategy.h"
class PriestNonCombatStrategy : public NonCombatStrategy
{
    public:
        PriestNonCombatStrategy(PlayerbotAI* botAI);

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        std::string const& getName() override { return "nc"; }
};

class PriestBuffStrategy : public NonCombatStrategy
{
    public:
        PriestBuffStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) { }

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        std::string const& getName() override { return "buff"; }
};

class PriestShadowResistanceStrategy : public NonCombatStrategy
{
    public:
        PriestShadowResistanceStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) { }

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        std::string const& getName() override { return "rshadow"; }
};
