#include "botpch.h"
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "TotemsShamanStrategy.h"

using namespace botAI;

TotemsShamanStrategy::TotemsShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
}

void TotemsShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "grace of air totem",
        NextAction::array(0, new NextAction("grace of air totem", 16.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "mana spring totem",
        NextAction::array(0, new NextAction("mana spring totem", 19.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "strength of earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", 18.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "flametongue totem",
        NextAction::array(0, new NextAction("flametongue totem", 17.0f), nullptr)));
}
