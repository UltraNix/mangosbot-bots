#include "botpch.h"
#include "../../playerbot.h"
#include "WarlockMultipliers.h"
#include "TankWarlockStrategy.h"

using namespace ai;

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
    }
private:
    static ActionNode* summon_voidwalker(PlayerbotAI* botAI)
    {
        return new ActionNode ("summon voidwalker",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("drain soul"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* summon_felguard(PlayerbotAI* botAI)
    {
        return new ActionNode ("summon felguard",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("summon voidwalker"), nullptr),
            /*C*/ nullptr);
    }
};

TankWarlockStrategy::TankWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

NextAction** TankWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("shoot", 10.0f), nullptr);
}

void TankWarlockStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);
}
