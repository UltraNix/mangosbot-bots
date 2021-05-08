#include "botpch.h"
#include "../../playerbot.h"
#include "ChooseRpgTargetAction.h"
#include "../../PlayerbotAIConfig.h"
#include "../values/PossibleRpgTargetsValue.h"

using namespace ai;

bool ChooseRpgTargetAction::Execute(Event event)
{
    list<ObjectGuid> possibleTargets = AI_VALUE(list<ObjectGuid>, "possible rpg targets");
    if (possibleTargets.empty())
        return false;

    vector<Unit*> units;
    for (list<ObjectGuid>::iterator i = possibleTargets.begin(); i != possibleTargets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit) units.push_back(unit);
    }

    if (units.empty())
    {
        sLog->outDetail("%s can't choose RPG target: all %d are not available", bot->GetName(), possibleTargets.size());
        return false;
    }

    Unit* target = units[urand(0, units.size() - 1)];
    if (!target) return false;

    context->GetValue<ObjectGuid>("rpg target")->Set(target->GetGUID());
    return true;
}

bool ChooseRpgTargetAction::isUseful()
{
    return !context->GetValue<ObjectGuid>("rpg target")->Get();
}
