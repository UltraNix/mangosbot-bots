/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"
class SummonAction : public MovementAction
{
    public:
        SummonAction(PlayerbotAI* botAI, std::string const& name = "summon") : MovementAction(botAI, name) { }

        bool Execute(Event event) override;

    protected:
        bool Teleport(Player* summoner, Player* player);
        bool SummonUsingGos(Player* summoner, Player* player);
        bool SummonUsingNpcs(Player* summoner, Player* player);
};

class UseMeetingStoneAction : public SummonAction
{
    public:
        UseMeetingStoneAction(PlayerbotAI* botAI) : SummonAction(botAI, "use meeting stone") { }

        bool Execute(Event event) override;
};
