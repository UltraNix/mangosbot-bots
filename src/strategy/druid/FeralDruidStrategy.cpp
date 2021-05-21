/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FeralDruidStrategy.h"
#include "../../Playerbot.h"

class FeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        FeralDruidStrategyActionNodeFactory()
        {
            creators["survival instincts"] = &survival_instincts;
            creators["thorns"] = &thorns;
            creators["omen of clarity"] = &omen_of_clarity;
            creators["cure poison"] = &cure_poison;
            creators["cure poison on party"] = &cure_poison_on_party;
            creators["abolish poison"] = &abolish_poison;
            creators["abolish poison on party"] = &abolish_poison_on_party;
        }

    private:
        static ActionNode* survival_instincts(PlayerbotAI* botAI)
        {
            return new ActionNode ("survival instincts",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("barskin"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* thorns(PlayerbotAI* botAI)
        {
            return new ActionNode ("thorns",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* omen_of_clarity(PlayerbotAI* botAI)
        {
            return new ActionNode ("omen of clarity",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* cure_poison(PlayerbotAI* botAI)
        {
            return new ActionNode ("cure poison",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* cure_poison_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("cure poison on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* abolish_poison(PlayerbotAI* botAI)
        {
            return new ActionNode ("abolish poison",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* abolish_poison_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("abolish poison on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
};

FeralDruidStrategy::FeralDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new FeralDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

void FeralDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("not facing target", NextAction::array(0, new NextAction("set facing", ACTION_NORMAL + 7), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("enemy too close for melee", NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("survival instincts", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("omen of clarity", NextAction::array(0, new NextAction("omen of clarity", ACTION_HIGH + 9), nullptr)));
}

