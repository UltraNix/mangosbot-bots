/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"
#include "NamedObjectContext.h"

class Event;
class Player;
class PlayerbotAI;
class Unit;

class EmoteActionBase : public Action
{
    public:
        EmoteActionBase(PlayerbotAI* botAI, std::string const& name);

    protected:
        bool Emote(Unit* target, uint32 type);
        bool ReceiveEmote(Player* source, uint32 emote);
        Unit* GetTarget();
        void InitEmotes();
        static std::map<std::string, uint32> emotes;
        static std::map<std::string, uint32> textEmotes;
};

class EmoteAction : public EmoteActionBase, public Qualified
{
    public:
        EmoteAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
        bool isUseful() override;
};

class TalkAction : public EmoteActionBase
{
    public:
        TalkAction(PlayerbotAI* botAI) : EmoteActionBase(botAI, "talk") { }

        bool Execute(Event event) override;
        static uint32 GetRandomEmote(Unit* unit);
};
