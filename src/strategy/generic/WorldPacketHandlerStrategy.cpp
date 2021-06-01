/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WorldPacketHandlerStrategy.h"
#include "Playerbot.h"

void WorldPacketHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("group invite", NextAction::array(0, new NextAction("accept invitation", relevance), nullptr)));
    triggers.push_back(new TriggerNode("group std::set leader", NextAction::array(0, new NextAction("leader", relevance), nullptr)));
    triggers.push_back(new TriggerNode("not enough money", NextAction::array(0, new NextAction("tell not enough money", relevance), nullptr)));
    triggers.push_back(new TriggerNode("not enough reputation", NextAction::array(0, new NextAction("tell not enough reputation", relevance), nullptr)));
    triggers.push_back(new TriggerNode("cannot equip", NextAction::array(0, new NextAction("tell cannot equip", relevance), nullptr)));
    triggers.push_back(new TriggerNode("use game object", NextAction::array(0, new NextAction("add loot", relevance), new NextAction("use meeting stone", relevance), nullptr)));
    triggers.push_back(new TriggerNode("gossip hello", NextAction::array(0, new NextAction("trainer", relevance), nullptr)));
    triggers.push_back(new TriggerNode("activate taxi", NextAction::array(0, new NextAction("remember taxi", relevance), new NextAction("taxi", relevance), nullptr)));
    triggers.push_back(new TriggerNode("taxi done", NextAction::array(0, new NextAction("taxi", relevance), nullptr)));
    triggers.push_back(new TriggerNode("trade status", NextAction::array(0, new NextAction("accept trade", relevance), nullptr)));
    triggers.push_back(new TriggerNode("area trigger", NextAction::array(0, new NextAction("reach area trigger", relevance), nullptr)));
    triggers.push_back(new TriggerNode("within area trigger", NextAction::array(0, new NextAction("area trigger", relevance), nullptr)));
    triggers.push_back(new TriggerNode("loot response", NextAction::array(0, new NextAction("store loot", relevance), nullptr)));
    triggers.push_back(new TriggerNode("item push result", NextAction::array(0, new NextAction("query item usage", relevance), nullptr)));
    triggers.push_back(new TriggerNode("ready check finished", NextAction::array(0, new NextAction("finish ready check", relevance), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("security check", relevance), new NextAction("check mail", relevance), nullptr)));
    triggers.push_back(new TriggerNode("guild invite", NextAction::array(0, new NextAction("guild accept", relevance), nullptr)));
    triggers.push_back(new TriggerNode("seldom", NextAction::array(0, new NextAction("lfg leave", relevance), nullptr)));
    triggers.push_back(new TriggerNode("no non bot players around", NextAction::array(0, new NextAction("delay", relevance), nullptr)));

}

WorldPacketHandlerStrategy::WorldPacketHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
    supported.push_back("loot roll");
    supported.push_back("check mount state");
    supported.push_back("quest objective completed");
    supported.push_back("party command");
    supported.push_back("ready check");
    supported.push_back("uninvite");
    supported.push_back("lfg role check");
    supported.push_back("lfg teleport");
    supported.push_back("random bot update");
    supported.push_back("inventory change failure");
}

void ReadyCheckStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("timer", NextAction::array(0, new NextAction("ready check", relevance), nullptr)));
}
