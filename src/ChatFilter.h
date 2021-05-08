/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Common.h"
#include "PlayerbotAIAware.h"

class PlayerbotAI;

class ChatFilter : public PlayerbotAIAware
{
    public:
        ChatFilter(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) { }
        virtual std::string Filter(std::string message);
};

class CompositeChatFilter : public ChatFilter
{
    public:
        CompositeChatFilter(PlayerbotAI* botAI);
        virtual ~CompositeChatFilter();
        std::string Filter(std::string message) override;

    private:
        std::vector<ChatFilter*> filters;
};
