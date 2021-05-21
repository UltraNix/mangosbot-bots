#pragma once
#include "../Value.h"

namespace botAI
{
    class IsBehindValue : public BoolCalculatedValue, public Qualified
	{
	public:
        IsBehindValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) { }

        virtual bool Calculate() 
        {
            Unit* target = AI_VALUE(Unit*, qualifier);
            if (!target)
                return false;

            
            float targetOrientation = target->GetOrientation();
            float orientation = bot->GetOrientation();
            float distance = bot->GetDistance(target);

            return distance <= ATTACK_DISTANCE && abs(targetOrientation - orientation) < M_PI / 2;
        }
    };
}
