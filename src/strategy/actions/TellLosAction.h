/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"
class TellLosAction : public Action
{
    public:
        TellLosAction(PlayerbotAI* botAI) : Action(botAI, "los") { }

        bool Execute(Event event) override;

    private:
        void ListUnits(std::string const& title, GuidVector units);
        void ListGameObjects(std::string const& title, GuidVector gos);
};
