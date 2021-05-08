#include "botpch.h"
#include "../../playerbot.h"
#include "PaladinTriggers.h"
#include "PaladinActions.h"

using namespace ai;

bool SealTrigger::IsActive()
{
	Unit* target = GetTarget();
	return !botAI->HasAura("seal of justice", target) &&
        !botAI->HasAura("seal of command", target) &&
        !botAI->HasAura("seal of vengeance", target) &&
		!botAI->HasAura("seal of righteousness", target) &&
		!botAI->HasAura("seal of light", target) &&
		!botAI->HasAura("seal of wisdom", target) &&
		AI_VALUE2(bool, "combat", "self target");
}

bool CrusaderAuraTrigger::IsActive()
{
	Unit* target = GetTarget();
	return AI_VALUE2(bool, "mounted", "self target") && !botAI->HasAura("crusader aura", target);
}

bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target,
                    "blessing of might", "blessing of wisdom", "blessing of kings", "blessing of sanctuary", NULL);
}
