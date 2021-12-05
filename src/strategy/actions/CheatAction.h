/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"

class Event;
class PlayerbotAI;

enum class BotCheatMask : uint32;

class CheatAction : public Action
{
    public:
        CheatAction(PlayerbotAI* ai) : Action(ai, "cheat") {}

        bool Execute(Event event) override;

    private:
        static BotCheatMask GetCheatMask(string cheat);
        static string GetCheatName(BotCheatMask cheatMask);
        void ListCheats();
};
