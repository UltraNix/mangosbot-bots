#include "botpch.h"
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "GenericMageStrategy.h"

using namespace botAI;

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["fire blast"] = &fire_blast;
        creators["scorch"] = &scorch;
        creators["frost nova"] = &frost_nova;
        creators["icy veins"] = &icy_veins;
        creators["combustion"] = &combustion;
        creators["evocation"] = &evocation;
        creators["dragon's breath"] = &dragons_breath;
        creators["blast wave"] = &blast_wave;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
    }
private:
    static ActionNode* frostbolt(PlayerbotAI* botAI)
    {
        return new ActionNode ("frostbolt",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* fire_blast(PlayerbotAI* botAI)
    {
        return new ActionNode ("fire blast",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("scorch"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* scorch(PlayerbotAI* botAI)
    {
        return new ActionNode ("scorch",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* frost_nova(PlayerbotAI* botAI)
    {
        return new ActionNode ("frost nova",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* icy_veins(PlayerbotAI* botAI)
    {
        return new ActionNode ("icy veins",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* combustion(PlayerbotAI* botAI)
    {
        return new ActionNode ("combustion",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* evocation(PlayerbotAI* botAI)
    {
        return new ActionNode ("evocation",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* dragons_breath(PlayerbotAI* botAI)
    {
        return new ActionNode ("dragon's breath",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("blast wave"), nullptr),
            /*C*/ NextAction::array(0, new NextAction("flamestrike", 71.0f), nullptr));
    }
    static ActionNode* blast_wave(PlayerbotAI* botAI)
    {
        return new ActionNode ("blast wave",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("frost nova"), nullptr),
            /*C*/ NextAction::array(0, new NextAction("flamestrike", 71.0f), nullptr));
    }
    static ActionNode* remove_curse(PlayerbotAI* botAI)
    {
        return new ActionNode ("remove curse",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("remove lesser curse"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* remove_curse_on_party(PlayerbotAI* botAI)
    {
        return new ActionNode ("remove curse on party",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("remove lesser curse on party"), nullptr),
            /*C*/ nullptr);
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("frost nova", 50.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "counterspell on enemy healer",
        NextAction::array(0, new NextAction("counterspell on enemy healer", 40.0f), nullptr)));

	triggers.push_back(new TriggerNode(
		"critical health",
		NextAction::array(0, new NextAction("ice block", 80.0f), nullptr)));

	triggers.push_back(new TriggerNode(
		"spellsteal",
		NextAction::array(0, new NextAction("spellsteal", 40.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("invisibility", 60.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("evocation", ACTION_EMERGENCY + 5), nullptr)));
}

void MageCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", 41.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", 40.0f), nullptr)));

}

void MageBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "arcane power",
        NextAction::array(0, new NextAction("arcane power", 41.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "presence of mind",
        NextAction::array(0, new NextAction("presence of mind", 42.0f), nullptr)));

}

void MageCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "polymorph",
        NextAction::array(0, new NextAction("polymorph", 30.0f), nullptr)));
}