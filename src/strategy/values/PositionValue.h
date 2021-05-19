#pragma once
#include "../Value.h"

namespace ai
{
    class PositionInfo
    {
    public:
        PositionInfo() : valueSet(false), x(0), y(0), z(0), mapId(0) {}
        PositionInfo(const PositionInfo&other) : valueSet(other.valueSet), x(other.x), y(other.y), z(other.z), mapId(other.mapId) {}
        void Set(double x, double y, double z, uint32 mapId) { this->x = x; this->y = y; this->z = z; this->mapId = mapId; this->valueSet = true; }
        void Reset() { valueSet = false; }
        bool isSet() { return valueSet; }

        double x, y, z;
        bool valueSet;
        uint32 mapId;
    };

    typedef map<string, PositionInfo> PositionMap;

    class PositionValue : public ManualSetValue<PositionMap&>
	{
	public:
        PositionValue(PlayerbotAI* botAI);

        virtual string Save();
        virtual bool Load(string value);

	private:
        PositionMap positions;
    };
}
