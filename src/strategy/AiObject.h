/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AIOBJECT_H
#define _PLAYERBOT_AIOBJECT_H

#include "Common.h"
#include "PlayerbotAIAware.h"

class AiObjectContext;
class ChatHelper;
class Player;
class PlayerbotAI;

class AiObject : public PlayerbotAIAware
{
	public:
        AiObject(PlayerbotAI* botAI);

    protected:
        Player* bot;
        Player* GetMaster();
        AiObjectContext* context;
        ChatHelper* chat;
};

class AiNamedObject : public AiObject
{
    public:
        AiNamedObject(PlayerbotAI* botAI, std::string const& name) : AiObject(botAI), name(name) { }

    public:
        virtual std::string const& getName() { return name; }

    protected:
        std::string name;
};

#endif
