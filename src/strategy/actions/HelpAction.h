/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"
#include "NamedObjectContext.h"

class Event;
class PlayerbotAI;

class HelpAction : public Action
{
    public:
        HelpAction(PlayerbotAI* botAI);

        virtual ~HelpAction();
        bool Execute(Event event) override;

    private:
        void TellChatCommands();
        void TellStrategies();
        std::string const& CombineSupported(std::set<std::string> commands);

        NamedObjectContext<Action>* chatContext;
};
