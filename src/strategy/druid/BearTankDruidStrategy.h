/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FeralDruidStrategy.h"

class BearTankDruidStrategy : public FeralDruidStrategy
{
    public:
        BearTankDruidStrategy();

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        std::string const& getName() override { return "bear"; }
        NextAction** getDefaultActions() override;
		uint32 GetType() const override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
};
