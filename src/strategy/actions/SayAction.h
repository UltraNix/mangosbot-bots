/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"
#include "NamedObjectContext.h"

class Event;
class PlayerbotAI;

class SayAction : public Action, public Qualified
{
    public:
        SayAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
        bool isUseful() const override;
        std::string const& getName() override { return "say::" + qualifier; }

    private:
        static std::map<std::string, std::vector<std::string>> stringTable;
        static std::map<std::string, uint32> probabilityTable;
};
