/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTSCVALUE_H
#define _PLAYERBOT_RTSCVALUE_H

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

class SeeSpellLocationValue : public LogCalculatedValue<WorldPosition>
{
    public:
        SeeSpellLocationValue(PlayerbotAI* ai, string name = "see spell location") : LogCalculatedValue(ai, name) {};

        bool EqualToLast(WorldPosition value) override;
        WorldPosition Calculate() override;
};

class RTSCSelectedValue : public ManualSetValue<bool>
{
	public:
        RTSCSelectedValue(PlayerbotAI* ai, bool defaultvalue = false, string name = "RTSC selected") : ManualSetValue(ai, defaultvalue,name) {};
};

class RTSCNextSpellActionValue : public ManualSetValue<string>
{
    public:
        RTSCNextSpellActionValue(PlayerbotAI* ai, string defaultvalue = "", string name = "RTSC next spell action") : ManualSetValue(ai, defaultvalue, name) {};
};

class RTSCSavedLocationValue : public ManualSetValue<WorldPosition>, public Qualified
{
    public:
        RTSCSavedLocationValue(PlayerbotAI* ai, WorldPosition defaultvalue = WorldPosition(), string name = "RTSC saved location") : ManualSetValue(ai, defaultvalue, name) {};

        string Save() override;
        bool Load(string text) override;
};

#endif
