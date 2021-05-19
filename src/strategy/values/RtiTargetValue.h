#pragma once
#include "../../PlayerbotAIConfig.h"
#include "../../ServerFacade.h"
#include "../Value.h"
#include "Group.h"
#include "TargetValue.h"

namespace ai
{
    class RtiTargetValue : public TargetValue
    {
    public:
        RtiTargetValue(PlayerbotAI* botAI, string type = "rti") : type(type), TargetValue(botAI)
        {}

    public:
        static int GetRtiIndex(string rti)
        {
            int index = -1;
            if(rti == "star") index = 0;
            else if(rti == "circle") index = 1;
            else if(rti == "diamond") index = 2;
            else if(rti == "triangle") index = 3;
            else if(rti == "moon") index = 4;
            else if(rti == "square") index = 5;
            else if(rti == "cross") index = 6;
            else if(rti == "skull") index = 7;
            return index;
        }

        Unit *Calculate()
        {
            Group* group = bot->GetGroup();
            if(!group)
                return nullptr;

            string rti = AI_VALUE(string, type);
            int index = GetRtiIndex(rti);

            if (index == -1)
                return nullptr;

            ObjectGuid guid = group->GetTargetIcon(index);
            if (!guid)
                return nullptr;

            list<ObjectGuid> attackers = context->GetValue<list<ObjectGuid> >("attackers")->Get();
            if (find(attackers.begin(), attackers.end(), guid) == attackers.end()) return nullptr;

            Unit* unit = botAI->GetUnit(guid);
            if (!unit || sServerFacade->UnitIsDead(unit) ||
                    !sServerFacade->IsWithinLOSInMap(bot, unit) ||
                    sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, unit), sPlayerbotAIConfig->sightDistance))
                return nullptr;

            return unit;
        }

    private:
        string type;
    };

    class RtiCcTargetValue : public RtiTargetValue
    {
    public:
        RtiCcTargetValue(PlayerbotAI* botAI) : RtiTargetValue(ai, "rti cc") {}
    };
}
