#pragma once
#include "../Value.h"
#include "TargetValue.h"
#include "NearestUnitsValue.h"

namespace botAI
{
    class AttackersValue : public ObjectGuidListCalculatedValue
	{
	public:
        AttackersValue() : ObjectGuidListCalculatedValue(botAI, "attackers", 2) { }
        GuidVector Calculate();

	private:
        void AddAttackersOf(Group* group, set<Unit*>& targets);
        void AddAttackersOf(Player* player, set<Unit*>& targets);
		void RemoveNonThreating(set<Unit*>& targets);

	public:
		static bool IsPossibleTarget(Unit* attacker, Player *bot);
		static bool IsValidTarget(Unit* attacker, Player *bot);
    };

    class PossibleAdsValue : public BoolCalculatedValue
    {
    public:
        PossibleAdsValue(PlayerbotAI* const botAI) : BoolCalculatedValue(botAI) { }
        virtual bool Calculate();
    };
}
