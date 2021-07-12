/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"
class ReachTargetAction : public MovementAction
{
    public:
        ReachTargetAction(PlayerbotAI* botAI, std::string const& name, float distance) : MovementAction(botAI, name) : distance(distance) { }

        bool Execute(Event event) override;
        bool isUseful() override;
        std::string const& GetTargetName() override;

    protected:
        float distance;
};

class CastReachTargetSpellAction : public CastSpellAction
{
    public:
        CastReachTargetSpellAction(PlayerbotAI* botAI, std::string const& spell, float distance) : CastSpellAction(botAI, spell) : distance(distance) { }

        bool isUseful() override;

    protected:
        float distance;
};

class ReachMeleeAction : public ReachTargetAction
{
    public:
        ReachMeleeAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach melee", sPlayerbotAIConfig->meleeDistance) { }
};

class ReachSpellAction : public ReachTargetAction
{
    public:
        ReachSpellAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach spell", botAI->GetRange("spell")) { }
};

class ReachPartyMemberToHealAction : public ReachTargetAction
{
    public:
        ReachPartyMemberToHealAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach party member to heal", botAI->GetRange("heal")) { }
        std::string const& GetTargetName() override;
};
