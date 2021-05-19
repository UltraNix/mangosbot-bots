/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Strategy.h"
#include "../Playerbot.h"

class ActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
    public:
        ActionNodeFactoryInternal()
        {
            creators["melee"] = &melee;
            creators["healthstone"] = &healthstone;
            creators["be near"] = &follow_master_random;
            creators["attack anything"] = &attack_anything;
            creators["move random"] = &move_random;
            creators["move to loot"] = &move_to_loot;
            creators["food"] = &food;
            creators["drink"] = &drink;
            creators["mana potion"] = &mana_potion;
            creators["healing potion"] = &healing_potion;
            creators["flee"] = &flee;
        }

    private:
        static ActionNode* melee()
        {
            return new ActionNode ("melee",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* healthstone()
        {
            return new ActionNode ("healthstone",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("healing potion"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* follow_master_random()
        {
            return new ActionNode ("be near",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("follow"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* attack_anything()
        {
            return new ActionNode ("attack anything",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* move_random()
        {
            return new ActionNode ("move random",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("stay line"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* move_to_loot()
        {
            return new ActionNode ("move to loot",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* food()
        {
            return new ActionNode ("food",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* drink()
        {
            return new ActionNode ("drink",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* mana_potion()
        {
            return new ActionNode ("mana potion",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("drink"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* healing_potion()
        {
            return new ActionNode ("healing potion",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("food"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* flee()
        {
            return new ActionNode ("flee",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
};

Strategy::Strategy(PlayerbotAI* botAI) : PlayerbotAIAware(botAI)
{
    actionNodeFactories.Add(new ActionNodeFactoryInternal());
}

ActionNode* Strategy::GetAction(std::string const& name)
{
    return actionNodeFactories.GetObject(name);
}

