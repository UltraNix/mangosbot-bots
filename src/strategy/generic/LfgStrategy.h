/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PassTroughStrategy.h"

class LfgStrategy : public PassTroughStrategy
{
    public:
        LfgStrategy(PlayerbotAI* ai);

	    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
        void InitTriggers(std::vector<TriggerNode*> &triggers) override;
        std::string const& getName() override { return "lfg"; }
};
