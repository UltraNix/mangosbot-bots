/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericHunterNonCombatStrategy.h"
#include "../../Playerbot.h"

class GenericHunterNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        GenericHunterNonCombatStrategyActionNodeFactory()
        {
            creators["rapid fire"] = &rapid_fire;
            creators["boost"] = &rapid_fire;
            creators["aspect of the pack"] = &aspect_of_the_pack;
        }

    private:
        static ActionNode* rapid_fire(PlayerbotAI* botAI)
        {
            return new ActionNode ("rapid fire",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("readiness"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* aspect_of_the_pack(PlayerbotAI* botAI)
        {
            return new ActionNode ("aspect of the pack",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("aspect of the cheetah"), nullptr),
                /*C*/ nullptr);
        }
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterNonCombatStrategyActionNodeFactory());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("trueshot aura", NextAction::array(0, new NextAction("trueshot aura", 2.0f), nullptr)));
}

void HunterPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("call pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("pet not happy", NextAction::array(0, new NextAction("feed pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet low health", NextAction::array(0, new NextAction("mend pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet dead", NextAction::array(0, new NextAction("revive pet", 60.0f), nullptr)));}
