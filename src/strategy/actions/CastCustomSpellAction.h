/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CASTCUSTOMSPELLACTION_H
#define _PLAYERBOT_CASTCUSTOMSPELLACTION_H

#include "InventoryAction.h"

class Event;
class PlayerbotAI;
class SpellInfo;
class WorldObject;

class CastCustomSpellAction : public InventoryAction
{
    public:
        CastCustomSpellAction(PlayerbotAI* botAI, std::string const& name = "cast custom spell") : InventoryAction(botAI, name) { }

        bool Execute(Event event) override;
        virtual std::string const& castString(WorldObject* target) { return "cast"; }

protected:
    bool ncCast = false;
};

class CastCustomNcSpellAction : public CastCustomSpellAction
{
    public:
        CastCustomNcSpellAction(PlayerbotAI* botAI, std::string const name = "cast custom nc spell") : CastCustomSpellAction(botAI, name) { }

        bool isUseful() override;
        std::string const& castString(WorldObject* target) override;
};

class CastRandomSpellAction : public CastCustomSpellAction
{
    public:
        CastRandomSpellAction(PlayerbotAI* botAI, std::string const& name = "cast random spell") : CastCustomSpellAction(botAI, name) { }

        virtual bool AcceptSpell(SpellInfo const* pSpellInfo) { return true; }
        bool Execute(Event event) override;

    protected:
        bool MultiCast = false;
};

class CraftRandomItemAction : public CastRandomSpellAction
{
    public:
        CraftRandomItemAction(PlayerbotAI* botAI) : CastRandomSpellAction(botAI, "craft random item")
        {
            MultiCast = true;
        }

        bool AcceptSpell(SpellInfo const* pSpellInfo) override;
};

#endif
