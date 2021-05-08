#include "CombatStrategy.h"
#pragma once

namespace ai
{
    class RangedCombatStrategy : public CombatStrategy
    {
    public:
        RangedCombatStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}
        virtual void InitTriggers(std::list<TriggerNode*> &triggers);
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_RANGED; }
        virtual string getName() { return "ranged"; }
    };


}
