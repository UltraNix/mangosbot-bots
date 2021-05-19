/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DBCStructure.h"
#include "../Action.h"
#include "../values/CraftValue.h"

class Event;
class PlayerbotAI;

class SetCraftAction : public Action
{
    public:
        SetCraftAction(PlayerbotAI* botAI) : Action(botAI, "craft") { }

        bool Execute(Event event) override;

        static uint32 GetCraftFee(CraftData& craftData);

    private:
        void TellCraft();

        static std::map<uint32, SkillLineAbilityEntry const*> skillSpells;

};
