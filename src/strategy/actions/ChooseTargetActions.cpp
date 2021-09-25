/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChooseTargetActions.h"
#include "Event.h"
#include "PossibleRpgTargetsValue.h"
#include "Playerbot.h"
#include "TravelMgr.h"
#include "ServerFacade.h"

bool AttackEnemyPlayerAction::isUseful()
{
    // if carry flag, do not start fight
    if (bot->HasAura(23333) || bot->HasAura(23335))
        return false;

    return !sPlayerbotAIConfig->IsInPvpProhibitedZone(bot->GetAreaId());
}

bool AttackEnemyFlagCarrierAction::isUseful()
{
    Unit* target = context->GetValue<Unit*>("enemy flag carrier")->Get();
    return target && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), 75.0f) && (bot->HasAura(23333) || bot->HasAura(23335));
}

bool DropTargetAction::Execute(Event event)
{
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    ObjectGuid pullTarget = context->GetValue<ObjectGuid>("pull target")->Get();
    GuidVector possible = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets")->Get();

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

    if (!urand(0, 25))
    {
        std::vector<uint32> sounds;
        if (target && target->isDead())
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
            botAI->PlaySound(sounds[urand(0, sounds.size() - 1)]);
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
                context->GetValue<ObjectGuid>("pull target")->Set(grindTarget->GetGUID());
        }
    }

    return result;
}

bool AttackAnythingAction::isUseful()
{
    return GetTarget() && ((botAI->AllowActive(GRIND_ACTIVITY)                                                                             //Bot allowed to be activ
        && AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig->mediumHealth                                                 // Bot has enough health.
        && (!AI_VALUE2(uint8, "mana", "self target") || AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig->mediumMana)       // Bot has no mana or enough mana.
        && !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling()) ||                                                 // Bot is not travelling.
        AI_VALUE2(bool, "combat", "self target"));                                                                                      // Bot is already in combat
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
    if (bot->HasAura(23333) || bot->HasAura(23335))
        return false;

    return true;
}
