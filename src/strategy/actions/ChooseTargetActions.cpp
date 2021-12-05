/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChooseTargetActions.h"
#include "ChooseRpgTargetAction.h"
#include "Event.h"
#include "LootObjectStack.h"
#include "PossibleRpgTargetsValue.h"
#include "Playerbot.h"
#include "TravelMgr.h"
#include "ServerFacade.h"

bool AttackEnemyPlayerAction::isUseful()
{
    // if carry flag, do not start fight
    if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
        return false;

    return !sPlayerbotAIConfig->IsInPvpProhibitedZone(bot->GetAreaId());
}

bool AttackEnemyFlagCarrierAction::isUseful()
{
    Unit* target = context->GetValue<Unit*>("enemy flag carrier")->Get();
    return target && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), 75.0f) && (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976));
}

bool AttackAnythingAction::isUseful()
{
    if (!ai->AllowActivity(GRIND_ACTIVITY))                                              //Bot not allowed to be active
        return false;

    if (!AI_VALUE(bool, "can move around"))
        return false;

    if (context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling() && ChooseRpgTargetAction::isFollowValid(bot, *context->GetValue<TravelTarget*>("travel target")->Get()->getPosition())) //Bot is traveling
        return false;

    Unit* target = GetTarget();

    if (!target)
        return false;

    string name = string(target->GetName());
    if (!name.empty() && name.find("Dummy") != std::string::npos) // Target is not a targetdummy
        return false;

    if (!ChooseRpgTargetAction::isFollowValid(bot, target))                               //Do not grind mobs far away from master.
        return false;

    return true;
}

bool DropTargetAction::Execute(Event event)
{
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    if (target && sServerFacade.UnitIsDead(target))
    {
        ObjectGuid guid = target->GetObjectGuid();
        if (guid)
            context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);
    }

    ObjectGuid pullTarget = context->GetValue<ObjectGuid>("pull target")->Get();
    GuidVector possible = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    if (pullTarget && find(possible.begin(), possible.end(), pullTarget) == possible.end())
    {
        context->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
    }

    context->GetValue<Unit*>("current target")->Set(nullptr);

    bot->SetTarget(ObjectGuid::Empty);
    botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
    botAI->InterruptSpell();
    bot->AttackStop();

    if (Pet* pet = bot->GetPet())
    {
        if (CreatureAI* creatureAI = ((Creature*)pet)->AI())
        {
            pet->SetReactState(REACT_PASSIVE);
            pet->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
            pet->AttackStop();
        }
    }

    if (!urand(0, 50) && ai->HasStrategy("emote", BOT_STATE_NON_COMBAT))
    {
        std::vector<uint32> sounds;
        if (target && sServerFacade.UnitIsDead(target))
        {
            sounds.push_back(TEXT_EMOTE_CHEER);
            sounds.push_back(TEXT_EMOTE_CONGRATULATE);
        }
        else
        {
            sounds.push_back(304); // guard
            sounds.push_back(325); // stay
        }

        if (!sounds.empty())
            botAI->PlayEmote(sounds[urand(0, sounds.size() - 1)]);
    }

    return true;
}

bool AttackAnythingAction::Execute(Event event)
{
    bool result = AttackAction::Execute(event);
    if (result)
    {
        if (Unit* grindTarget = GetTarget())
        {
            if (char const* grindName = grindTarget->GetName().c_str())
            {
                context->GetValue<ObjectGuid>("pull target")->Set(grindTarget->GetObjectGuid());
                bot->GetMotionMaster()->Clear();
                bot->StopMoving();
            }
        }
    }

    return result;
}

bool AttackAnythingAction::isPossible()
{
    return AttackAction::isPossible() && GetTarget();
}

bool AttackAnythingAction::GrindAlone(Player* bot)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return true;

    if (botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get().size() < urand(10, 30))
        return true;

    if (sPlayerbotAIConfig->randomBotGrindAlone <= 0)
        return false;

    uint32 randnum = bot->GetGUID().GetCounter();                   // Semi-random but fixed number for each bot.
    uint32 cycle = floor(getMSTime() / (1000));                     // Semi-random number adds 1 each second.

    cycle = cycle * sPlayerbotAIConfig->randomBotGrindAlone / 6000; // Cycles 0.01 per minute for each 1% of the config. (At 100% this is 1 per minute)
    randnum += cycle;                                               // Random number that increases 0.01 each minute for each % that the bots should be active.
    randnum = (randnum % 100);                                      // Loops the randomnumber at 100. Bassically removes all the numbers above 99.
    randnum = randnum + 1;                                          // Now we have a number unique for each bot between 1 and 100 that increases by 0.01 (per % active each minute).

    return randnum < sPlayerbotAIConfig->randomBotGrindAlone;       // The given percentage of bots should be active and rotate 1% of those active bots each minute.
}

bool DpsAssistAction::isUseful()
{
    // if carry flag, do not start fight
    if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
        return false;

    return true;
}
