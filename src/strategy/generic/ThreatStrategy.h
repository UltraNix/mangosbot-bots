/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "../Strategy.h"

class PlayerbotAI;

class ThreatMultiplier : public Multiplier
{
    public:
        ThreatMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "threat") { }

        float GetValue(Action* action) override;
};

class ThreatStrategy : public Strategy
{
    public:
        ThreatStrategy(PlayerbotAI* botAI) : Strategy(botAI) { }

        void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
        std::string const& getName() override { return "threat"; }
};
