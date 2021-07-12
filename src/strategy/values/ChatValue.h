/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"
enum ChatMsg : uint32;

class ChatValue : public ManualSetValue<ChatMsg>
{
	public:
        ChatValue(PlayerbotAI* botAI) : ManualSetValue<ChatMsg>(botAI, CHAT_MSG_WHISPER) { }
};
