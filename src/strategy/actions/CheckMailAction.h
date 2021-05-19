/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "../Action.h"

class Event;
class PlayerbotAI;

struct Mail;

class CheckMailAction : public Action
{
    public:
        CheckMailAction(PlayerbotAI* botAI) : Action(botAI, "check mail") { }

        bool Execute(Event event) override;

    private:
        void ProcessMail(Mail* mail, Player* owner);
};
