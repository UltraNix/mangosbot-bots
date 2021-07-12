/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"
#include "TravelMgr.h"
class LastMovement
{
    public:
        LastMovement();
        LastMovement(LastMovement& other);

        void clear();

        void Set(Unit* follow);
        void Set(uint32 mapId, float x, float y, float z, float ori);

        void setShort(WorldPosition point);
        void setPath(TravelPath path);

        std::vector<uint32> taxiNodes;
        ObjectGuid taxiMaster;
        Unit* lastFollow;
        uint32 lastAreaTrigger;
        time_t lastFlee;
        uint32 lastMoveToMapId;
        float lastMoveToX;
        float lastMoveToY;
        float lastMoveToZ;
        float lastMoveToOri;
        WorldPosition lastMoveShort;
        TravelPath lastPath;
        time_t nextTeleport;
};

class LastMovementValue : public ManualSetValue<LastMovement&>
{
	public:
        LastMovementValue(PlayerbotAI* botAI) : ManualSetValue<LastMovement&>(botAI, data) { }

    private:
        LastMovement data;
};

class StayTimeValue : public ManualSetValue<time_t>
{
	public:
        StayTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0) { }
};
