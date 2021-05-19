#include "botpch.h"
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "MeleeDruidStrategy.h"

using namespace ai;

MeleeDruidStrategy::MeleeDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
}

NextAction** MeleeDruidStrategy::getDefaultActions()
{
    return NextAction::array(0,
            new NextAction("faerie fire", ACTION_NORMAL + 1),
            new NextAction("melee", ACTION_NORMAL),
            nullptr);
}

void MeleeDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "omen of clarity",
        NextAction::array(0, new NextAction("omen of clarity", ACTION_HIGH + 9), nullptr)));

    CombatStrategy::InitTriggers(triggers);
}
