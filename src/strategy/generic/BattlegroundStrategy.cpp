/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BattlegroundStrategy.h"
#include "Playerbot.h"

void BGStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("bg join", relevance), nullptr)));
    triggers.push_back(new TriggerNode("bg invite active", NextAction::array(0, new NextAction("bg status check", relevance), nullptr)));
}

BGStrategy::BGStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
}

void WarsongStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", 70.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", NextAction::array(0, new NextAction("attack enemy flag carrier", 80.0f), nullptr)));
    triggers.push_back(new TriggerNode("player has flag", NextAction::array(0, new NextAction("bg move to objective", 90.0f), nullptr)));
}

void AlteracStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    /* placeholder */
}

void ArathiStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", 70.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", 30.0f), nullptr)));
}

void BattlegroundStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("bg waiting", NextAction::array(0, new NextAction("bg move to start", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("bg active", NextAction::array(0, new NextAction("bg move to objective", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("bg check objective", 10.0f), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", NextAction::array(0, new NextAction("attack enemy flag carrier", 80.0f), nullptr)));
    /*triggers.push_back(new TriggerNode("team flagcarrier near", NextAction::array(0, new NextAction("bg protect fc", 40.0f), nullptr)));*/
    triggers.push_back(new TriggerNode("player has flag", NextAction::array(0, new NextAction("bg move to objective", 90.0f), nullptr)));
}

void ArenaStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("no possible targets", NextAction::array(0, new NextAction("arena tactics", 1.0f), nullptr)));
}

void EyeStrategy::InitTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", 70.0f), NULL)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", 30.0f), NULL)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", NextAction::array(0, new NextAction("attack enemy flag carrier", 80.0f), NULL)));
    triggers.push_back(new TriggerNode("player has flag", NextAction::array(0, new NextAction("bg move to objective", 90.0f), NULL)));
}
