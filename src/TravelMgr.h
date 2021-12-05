/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVELMGR_H
#define _PLAYERBOT_TRAVELMGR_H

#include "AiObject.h"
#include "CreatureData.h"
#include "GameObject.h"
#include "GridDefines.h"
#include "PlayerbotAIConfig.h"

#include <boost/functional/hash.hpp>

class GuidPosition;
class ObjectGuid;
class Quest;
class Player;
class PlayerbotAI;

namespace G3D
{
    class Vector2;
    class Vector3;
    class Vector4;
}

//Constructor types for WorldPosition
enum WorldPositionConst
{
    WP_RANDOM           = 0,
    WP_CENTROID         = 1,
    WP_MEAN_CENTROID    = 2,
    WP_CLOSEST          = 3
};

enum TravelState
{
    TRAVEL_STATE_IDLE                   = 0,
    TRAVEL_STATE_TRAVEL_PICK_UP_QUEST   = 1,
    TRAVEL_STATE_WORK_PICK_UP_QUEST     = 2,
    TRAVEL_STATE_TRAVEL_DO_QUEST        = 3,
    TRAVEL_STATE_WORK_DO_QUEST          = 4,
    TRAVEL_STATE_TRAVEL_HAND_IN_QUEST   = 5,
    TRAVEL_STATE_WORK_HAND_IN_QUEST     = 6,
    TRAVEL_STATE_TRAVEL_RPG             = 7,
    TRAVEL_STATE_TRAVEL_EXPLORE         = 8,
    MAX_TRAVEL_STATE
};

enum TravelStatus
{
    TRAVEL_STATUS_NONE      = 0,
    TRAVEL_STATUS_PREPARE   = 1,
    TRAVEL_STATUS_TRAVEL    = 2,
    TRAVEL_STATUS_WORK      = 3,
    TRAVEL_STATUS_COOLDOWN  = 4,
    TRAVEL_STATUS_EXPIRED   = 5,
    MAX_TRAVEL_STATUS
};

//A quest destination container for quick lookup of all destinations related to a quest.
struct QuestContainer
{
    std::vector<QuestTravelDestination*> questGivers;
    std::vector<QuestTravelDestination*> questTakers;
    std::vector<QuestTravelDestination*> questObjectives;
};

typedef pair<int32, int32> mGridPair;

//Extension of WorldLocation with distance functions.
class WorldPosition : public WorldLocation
{
    public:
        //Constructors
        WorldPosition() : WorldLocation() {};
        WorldPosition(const WorldLocation loc) : WorldLocation(loc) {}
        WorldPosition(const WorldPosition& pos) : WorldLocation(pos), visitors(pos.visitors) {}
        WorldPosition(const string str);
        WorldPosition(uint32 mapid, float x, float y, float z = 0, float orientation = 0) : WorldLocation(mapid, x, y, z, orientation) {}
        WorldPosition(uint32 mapId, const Position& pos);
        WorldPosition(const WorldObject* wo) { if (wo) { set(WorldLocation(wo->GetMapId(), wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(), wo->GetOrientation())); } }
        WorldPosition(CreatureDataPair const* cdPair) { if (cdPair) { set(WorldLocation(cdPair->second.mapid, cdPair->second.posX, cdPair->second.posY, cdPair->second.posZ, cdPair->second.orientation)); } }
        WorldPosition(GameObjectDataPair const* cdPair) { if (cdPair) { set(WorldLocation(cdPair->second.mapid, cdPair->second.posX, cdPair->second.posY, cdPair->second.posZ, cdPair->second.orientation)); } }
        WorldPosition(GuidPosition gpos);
        WorldPosition(vector<WorldPosition*> list, WorldPositionConst conType);
        WorldPosition(vector<WorldPosition> list, WorldPositionConst conType);
        WorldPosition(uint32 mapid, GridPair grid) : WorldLocation(mapid, (int32(grid.x_coord) - CENTER_GRID_ID - 0.5) * SIZE_OF_GRIDS + CENTER_GRID_OFFSET, (int32(grid.y_coord) - CENTER_GRID_ID - 0.5) * SIZE_OF_GRIDS + CENTER_GRID_OFFSET, 0, 0) {}
        WorldPosition(uint32 mapid, CellPair cell) : WorldLocation(mapid, (int32(cell.x_coord) - CENTER_GRID_CELL_ID - 0.5) * SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET, (int32(cell.y_coord) - CENTER_GRID_CELL_ID - 0.5) * SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET, 0, 0) {}
        WorldPosition(uint32 mapid, mGridPair grid) : WorldLocation(mapid, (32 - grid.first) * SIZE_OF_GRIDS, (32 - grid.second) * SIZE_OF_GRIDS, 0, 0) {}

        //Setters
        void set(const WorldLocation& pos) { mapid = pos.mapid; coord_x = pos.coord_x; coord_y = pos.coord_y; coord_z = pos.coord_z; orientation = pos.orientation; }
        void set(const WorldPosition& pos) { mapid = pos.mapid; coord_x = pos.coord_x; coord_y = pos.coord_y; coord_z = pos.coord_z; orientation = pos.orientation; visitors= pos.visitors; }
        void set(ObjectGuid guid);
        void setMapId(uint32 id) { mapid = id; }
        void setX(float x) { coord_x = x; }
        void setY(float y) { coord_y = y; }
        void setZ(float z) { coord_z = z; }
        void setO(float o) {orientation = o;}

        void addVisitor()
        {
            ++visitors;
        }

        void remVisitor()
        {
            --visitors;
        }

        //Getters
        operator bool() const { return  mapid != 0 || coord_x != 0 || coord_y != 0 || coord_z != 0; }
        bool operator==(const WorldPosition& p1) { return mapid == p1.mapid && coord_x == p1.coord_x && coord_y == p1.coord_y && coord_z == p1.coord_z && orientation == p1.orientation; }
        bool operator!=(const WorldPosition& p1) { return mapid != p1.mapid || coord_x != p1.coord_x || coord_y != p1.coord_y || coord_z != p1.coord_z || orientation != p1.orientation; }

        WorldPosition& operator+=(const WorldPosition& p1) { coord_x += p1.coord_x; coord_y += p1.coord_y; coord_z += p1.coord_z; return *this; }
        WorldPosition& operator-=(const WorldPosition& p1) { coord_x -= p1.coord_x; coord_y -= p1.coord_y; coord_z -= p1.coord_z; return *this; }

        uint32 getMapId() { return mapid; }
        float getX() { return coord_x; }
        float getY() { return coord_y; }
        float getZ() { return coord_z; }
        float getO() { return orientation; }

        G3D::Vector3 getVector3();
        std::string const& print();

        string to_string() {stringstream out; out << mapid; out << coord_x; out << coord_y; out << coord_z;  out << orientation; return out.str();};

        void printWKT(std::vector<WorldPosition> points, std::ostringstream& out, uint32 dim = 0, bool loop = false);
        void printWKT(std::ostringstream& out) { printWKT({ *this }, out); }

        uint32 getVisitors() { return visitors; }

        bool isOverworld() { return mapid == 0 || mapid == 1 || mapid == 530 || mapid == 571; }
        bool isInWater() { return getTerrain() ? getTerrain()->IsInWater(coord_x, coord_y, coord_z) : false; };
        bool isUnderWater() { return getTerrain() ? getTerrain()->IsUnderWater(coord_x, coord_y, coord_z) : false; };

        WorldPosition relPoint(WorldPosition* center) { return WorldPosition(mapid, coord_x - center->coord_x, coord_y - center->coord_y, coord_z - center->coord_z, orientation); }
        WorldPosition offset(WorldPosition* center) { return WorldPosition(mapid, coord_x + center->coord_x, coord_y + center->coord_y, coord_z + center->coord_z, orientation); }
        float size() { return sqrt(pow(coord_x, 2.0) + pow(coord_y, 2.0) + pow(coord_z, 2.0)); }

        //Slow distance function using possible map transfers.
        float distance(WorldPosition* center);
        float distance(WorldPosition center)
        {
            return distance(&center);
        };

        float fDist(WorldPosition* center);
        float fDist(WorldPosition center)
        {
            return fDist(&center);
        };

        float fDist(WorldPosition start, WorldPosition end)
        {
            return start.fDist(pointFrom) + portalLength + pointTo.fDist(end);
        }

        template<class T>
        std::pair<T, WorldPosition> closest(std::list<std::pair<T, WorldPosition>> list)
        {
            return *std::min_element(list.begin(), list.end(), [this](pair<T, WorldPosition> i, pair<T, WorldPosition> j)
            {
                return this->distance(i.second) < this->distance(j.second);
            });
        }

        template<class T>
        std::pair<T, WorldPosition> closest(std::list<T> list)
        {
            return closest(GetPosList(list));
        }

        template<class T>
        std::pair<T, WorldPosition> closest(std::vector<std::pair<T, WorldPosition>> list)
        {
            return *std::min_element(list.begin(), list.end(), [this](pair<T, WorldPosition> i, pair<T, WorldPosition> j)
            {
                return this->distance(i.second) < this->distance(j.second);
            });
        }

        template<class T>
        std::pair<T, WorldPosition> closest(std::vector<T> list)
        {
            return closest(GetPosVector(list));
        }

        //Returns the closest point from the list.
        WorldPosition* closest(std::vector<WorldPosition*> list)
        {
            return *std::min_element(list.begin(), list.end(), [this](WorldPosition* i, WorldPosition* j) { return this->distance(i) < this->distance(j); });
        }

        WorldPosition closest(std::vector<WorldPosition> list)
        {
            return *std::min_element(list.begin(), list.end(), [this](WorldPosition i, WorldPosition j) { return this->distance(i) < this->distance(j); });
        }

        //Quick square distance in 2d plane.
        float sqDistance2d(WorldPosition center)
        {
            return (getX() - center.getX()) * (getX() - center.getX()) + (getY() - center.getY()) * (getY() - center.getY());
        };

        //Quick square distance calculation without map check. Used for getting the minimum distant points.
        float sqDistance(WorldPosition center)
        {
            return (getX() - center.getX()) * (getX() - center.getX()) + (getY() - center.getY()) *
                (getY() - center.getY()) + (getZ() - center.getZ()) * (getZ() - center.getZ());
        };

        float sqDistance2d(WorldPosition* center)
        {
            return (getX() - center->getX()) * (getX() - center->getX()) + (getY() - center->getY()) * (getY() - center->getY());
        };

        float sqDistance(WorldPosition* center)
        {
            return (getX() - center->getX()) * (getX() - center->getX()) + (getY() - center->getY()) *
                (getY() - center->getY()) + (getZ() - center->getZ()) * (getZ() - center->getZ());
        };

        //Returns the closest point of the list. Fast but only works for the same map.
        WorldPosition* closestSq(std::vector<WorldPosition*> list)
        {
            return *std::min_element(list.begin(), list.end(), [this](WorldPosition* i, WorldPosition* j) { return this->sqDistance(i) < this->sqDistance(j); });
        }

        WorldPosition closestSq(std::vector<WorldPosition> list)
        {
            return *std::min_element(list.begin(), list.end(), [this](WorldPosition i, WorldPosition j) { return this->sqDistance(i) < this->sqDistance(j); });
        }

        float getAngleTo(WorldPosition endPos)
        {
            float ang = atan2(endPos.getY() - getY(), endPos.getX() - getX());
            return (ang >= 0) ? ang : 2 * static_cast<float>(M_PI) + ang;
        };

       float getAngleBetween(WorldPosition dir1, WorldPosition dir2)
        {
            return abs(getAngleTo(dir1) - getAngleTo(dir2));
        };

        WorldPosition lastInRange(std::vector<WorldPosition> list, float minDist = -1.f, float maxDist = -1.f);
        WorldPosition firstOutRange(std::vector<WorldPosition> list, float minDist = -1.f, float maxDist = -1.f);

        float mSign(WorldPosition* p1, WorldPosition* p2)
        {
            return(getX() - p2->getX()) * (p1->getY() - p2->getY()) - (p1->getX() - p2->getX()) * (getY() - p2->getY());
        }

        bool isInside(WorldPosition* p1, WorldPosition* p2, WorldPosition* p3);

        //Map functions. Player independent.
        MapEntry const* getMapEntry();
        uint32 getInstanceId();
        Map* getMap();
        const float getHeight();

        std::set<Transport*> getTransports(uint32 entry = 0);

        GridPair getGridPair() { return Acore::ComputeGridPair(getX(), getY()); };
        std::vector<GridPair> getGridPairs(WorldPosition secondPos);
        vector<WorldPosition> fromGridPair(GridPair gridPair);

        CellPair getCellPair() { return Acore::ComputeCellPair(getX(), getY()); }
        vector<WorldPosition> fromCellPair(CellPair cellPair);
        vector<WorldPosition> gridFromCellPair(CellPair cellPair);

         mGridPair getmGridPair()
         {
            return make_pair((int32)(CENTER_GRID_ID - getX() / SIZE_OF_GRIDS), (int32)(CENTER_GRID_ID - getY() / SIZE_OF_GRIDS));
         }

        vector<mGridPair> getmGridPairs(WorldPosition secondPos);
        vector<WorldPosition> frommGridPair(mGridPair gridPair);

        void loadMapAndVMap(uint32 mapId, uint8 x, uint8 y);

        void loadMapAndVMap()
        {
            loadMapAndVMap(getMapId(), getmGridPair().first, getmGridPair().second);
        }

        void loadMapAndVMaps(WorldPosition secondPos);

        //Display functions
        WorldPosition getDisplayLocation();
        float getDisplayX()
        {
            return getDisplayLocation().getY() * -1.0;
        }

        float getDisplayY()
        {
            return getDisplayLocation().getX();
        }

        uint16 getAreaId();
        AreaTableEntry const* getArea();
        std::string const& getAreaName(bool fullName = true, bool zoneName = false);

        std::vector<WorldPosition> fromPointsArray(std::vector<G3D::Vector3> path);

        //Pathfinding
        std::vector<WorldPosition> getPathStepFrom(WorldPosition startPos, Unit* bot);
        std::vector<WorldPosition> getPathFromPath(std::vector<WorldPosition> startPath, Unit* bot, uint8 maxAttempt = 40);

        std::vector<WorldPosition> getPathFrom(WorldPosition startPos, Unit* bot)
        {
            return getPathFromPath({ startPos }, bot);
        };

        std::vector<WorldPosition> getPathTo(WorldPosition endPos, Unit* bot)
        {
            return endPos.getPathFrom(*this, bot);
        }

        bool isPathTo(std::vector<WorldPosition> path, float maxDistance = sPlayerbotAIConfig->targetPosRecalcDistance) { return !path.empty() && distance(path.back()) < maxDistance; };
        bool cropPathTo(vector<WorldPosition>& path, float maxDistance = sPlayerbotAIConfig.targetPosRecalcDistance);
        bool canPathTo(WorldPosition endPos, Unit* bot) { return endPos.isPathTo(getPathTo(endPos, bot)); }

        float getPathLength(std::vector<WorldPosition> points)
        {
            float dist;
            for (auto& p : points)
                if (&p == &points.front())
                    dist = 0;
                else
                    dist += std::prev(&p, 1)->distance(p);

            return dist;
        }

        bool GetReachableRandomPointOnGround(Player* bot, float radius, bool randomRange = true);

        uint32 getUnitsNear(list<ObjectGuid>& units, float radius);
        uint32 getUnitsAggro(list<ObjectGuid>& units, Player* bot);

        //Creatures
        std::vector<CreatureData const*> getCreaturesNear(float radius = 0, uint32 entry = 0);

        //GameObjects
        std::vector<GameObjectData const*> getGameObjectsNear(float radius = 0, uint32 entry = 0);

    private:
        WorldLocation wLoc;
        uint32 visitors = 0;
};

inline ByteBuffer& operator<<(ByteBuffer& b, WorldPosition& guidP)
{
    b << guidP.getMapId();
    b << guidP.getX();
    b << guidP.getY();
    b << guidP.getZ();
    b << guidP.getO();
    b << guidP.getVisitors();
    return b;
}

inline ByteBuffer& operator>>(ByteBuffer& b, WorldPosition& g)
{
    uint32 mapid;
    float coord_x;
    float coord_y;
    float coord_z;
    float orientation;
    uint32 visitors = 0;
    b >> mapid;
    b >> coord_x;
    b >> coord_y;
    b >> coord_z;
    b >> orientation;
    b >> visitors;

    return b;
}

//Generic creature finder
class FindPointCreatureData
{
    public:
        FindPointCreatureData(WorldPosition point1 = WorldPosition(), float radius1 = 0, uint32 entry1 = 0) { point = point1; radius = radius1; entry = entry1; }

        bool operator()(CreatureDataPair const& dataPair);
        vector<CreatureDataPair const*> GetResult() const { return data; };

    private:
        WorldPosition point;
        float radius;
        uint32 entry;

        vector<CreatureDataPair const*> data;
};

//Generic gameObject finder
class FindPointGameObjectData
{
    public:
        FindPointGameObjectData(WorldPosition point1 = WorldPosition(), float radius1 = 0, uint32 entry1 = 0) { point = point1; radius = radius1; entry = entry1; }

        bool operator()(GameObjectDataPair const& dataPair);
        vector<GameObjectDataPair const*> GetResult() const { return data; };

    private:
        WorldPosition point;
        float radius;
        uint32 entry;

        vector<GameObjectDataPair const*> data;
};

class GuidPosition : public ObjectGuid, public WorldPosition
{
    public:
        GuidPosition() : ObjectGuid(), WorldPosition() {}
        GuidPosition(ObjectGuid guid) { ObjectGuid::Set(guid); WorldPosition::set(guid); };
        GuidPosition(ObjectGuid guid, WorldPosition pos) : ObjectGuid(guid), WorldPosition(pos) {};

        template<class T>
        GuidPosition(ObjectGuid guid, T) : ObjectGuid(guid) : WorldPosition(T) {};
        GuidPosition(CreatureDataPair const* dataPair) : ObjectGuid(HIGHGUID_UNIT, dataPair->second.id, dataPair->first), WorldPosition(dataPair) {};
        GuidPosition(GameObjectDataPair const* dataPair) : ObjectGuid(HIGHGUID_GAMEOBJECT, dataPair->second.id, dataPair->first), WorldPosition(dataPair) {};
        GuidPosition(WorldObject* wo) : WorldPosition(wo) { ObjectGuid::Set(wo->GetObjectGuid()); };
        //GuidPosition(const GuidPosition& guidp) {this->Set(guidp);  this->setLocation(((WorldPosition)guidp).getLocation()); }

        CreatureData const* getCreatureData();
        CreatureTemplate const* GetCreatureTemplate();

        GameObjectInfo const* GetGameObjectInfo() { return IsGameObject() ? sObjectMgr.GetGameObjectInfo(GetEntry()) : nullptr; };

        WorldObject* GetWorldObject() { return getMap() ? getMap()->GetWorldObject(*this) : nullptr;}
        Creature* GetCreature();
        Unit* GetUnit();
        GameObject* GetGameObject();
        Player* GetPlayer();

        bool HasNpcFlag(NPCFlags flag) { return IsCreature() && GetCreatureTemplate()->NpcFlags & flag; }

        bool isDead(); //For loaded grids check if the unit/object is unloaded/dead.

        operator bool() const { return WorldPosition(*this) && !IsEmpty(); }
        bool operator==(ObjectGuid const& guid) const { return GetRawValue() == guid.GetRawValue(); }
        bool operator!=(ObjectGuid const& guid) const { return GetRawValue() != guid.GetRawValue(); }
        bool operator<(ObjectGuid const& guid) const { return GetRawValue() < guid.GetRawValue(); }
};

inline ByteBuffer& operator<<(ByteBuffer& b, GuidPosition& guidP)
{
    b << (ObjectGuid) guidP;
    b << (WorldPosition) guidP;

    return b;
}

inline ByteBuffer& operator>>(ByteBuffer& b, GuidPosition& g)
{
    ObjectGuid    guid;
    WorldPosition pos;

    b >> guid;
    b >> pos;

    g = GuidPosition(guid, pos);

    return b;
}

template<class T>
std::list<std::pair<T, WorldPosition>> GetPosList(std::list<T> oList)
{
    std::list<std::pair<T, WorldPosition>> retList;
    for (auto& obj : oList)
        retList.push_back(std::make_pair(obj, WorldPosition(obj)));

    return std::move(retList);
};

template<class T>
std::vector<std::pair<T, WorldPosition>> GetPosVector(std::vector<T> oList)
{
    std::vector<std::pair<T, WorldPosition>> retList;
    for (auto& obj : oList)
        retList.push_back(make_pair(obj, WorldPosition(obj)));

    return std::move(retList);
};

class mapTransfer
{
    public:
        mapTransfer(WorldPosition pointFrom1, WorldPosition pointTo1, float portalLength1 = 0.1f)
            : pointFrom(pointFrom1), pointTo(pointTo1), portalLength(portalLength1) { }

        bool isFrom(WorldPosition point)
        {
            return point.getMapId() == pointFrom.getMapId();
        }

        bool isTo(WorldPosition point)
        {
            return point.getMapId() == pointTo.getMapId();
        }

        WorldPosition* getPointFrom()
        {
            return &pointFrom;
        }

        WorldPosition* getPointTo()
        {
            return &pointTo;
        }

        bool isUseful(WorldPosition point)
        {
            return isFrom(point) || isTo(point);
        }

        float distance(WorldPosition point)
        {
            return isUseful(point) ? (isFrom(point) ? point.distance(pointFrom) : point.distance(pointTo)) : 200000;
        }

        bool isUseful(WorldPosition start, WorldPosition end)
        {
            return isFrom(start) && isTo(end);
        }

        float distance(WorldPosition start, WorldPosition end)
        {
            return (isUseful(start, end) ? (start.distance(pointFrom) + portalLength + pointTo.distance(end)) : 200000);
        }

    private:
        WorldPosition pointFrom;
        WorldPosition pointTo;
        float portalLength = 0.1f;
};

//A destination for a bot to travel to and do something.
class TravelDestination
{
    public:
        TravelDestination() { }
        TravelDestination(float radiusMin1, float radiusMax1)
        {
            radiusMin = radiusMin1; radiusMax = radiusMax1;
        }
        TravelDestination(std::vector<WorldPosition*> points1, float radiusMin1, float radiusMax1)
        {
            points = points1;  radiusMin = radiusMin1; radiusMax = radiusMax1;
        }

        void addPoint(WorldPosition* pos)
        {
            points.push_back(pos);
        }

        void setExpireDelay(uint32 delay)
        {
            expireDelay = delay;
        }

        void setCooldownDelay(uint32 delay)
        {
            cooldownDelay = delay;
        }

        void setMaxVisitors(uint32 maxVisitors1 = 0, uint32 maxVisitorsPerPoint1 = 0)
        {
            maxVisitors = maxVisitors1;
            maxVisitorsPerPoint = maxVisitorsPerPoint1;
        }

        std::vector<WorldPosition*> getPoints(bool ignoreFull = false);
        uint32 getExpireDelay() { return expireDelay; }
        uint32 getCooldownDelay() { return cooldownDelay; }

        void addVisitor()
        {
            ++visitors;
        }

        void remVisitor()
        {
            --visitors;
        }

        uint32 getVisitors()
        {
            return visitors;
        }

        virtual Quest const* GetQuestTemplate() { return nullptr; }
        virtual bool isActive(Player* bot) { return false; }

        bool isFull(bool ignoreFull = false);

        virtual std::string const& getName() { return "TravelDestination"; }
        virtual int32 getEntry() { return 0; }
        virtual std::string const& getTitle() { return "generic travel destination"; }

        WorldPosition* nearestPoint(WorldPosition* pos);

        float distanceTo(WorldPosition* pos) { return nearestPoint(pos)->distance(pos); }
        bool onMap(WorldPosition* pos) { return nearestPoint(pos)->getMapId() == pos->getMapId(); }
        virtual bool isIn(WorldPosition* pos, float radius = 0) { return onMap(pos) && distanceTo(pos) <= (radius? radius : radiusMin); }
        virtual bool isOut(WorldPosition* pos, float radius = 0) { return !onMap(pos) || distanceTo(pos) > (radius? radius : radiusMax); }
        float getRadiusMin() { return radiusMin; }

        std::vector<WorldPosition*> touchingPoints(WorldPosition* pos);
        std::vector<WorldPosition*> sortedPoints(WorldPosition* pos);
        std::vector<WorldPosition*> nextPoint(WorldPosition* pos, bool ignoreFull = true);

    protected:
        std::vector<WorldPosition*> points;
        float radiusMin = 0;
        float radiusMax = 0;

        uint32 visitors = 0;
        uint32 maxVisitors = 0;
        uint32 maxVisitorsPerPoint = 0;
        uint32 expireDelay = 5 * 1000;
        uint32 cooldownDelay = 60 * 1000;
};

//A travel target that is always inactive and jumps to cooldown.
class NullTravelDestination : public TravelDestination
{
    public:
        NullTravelDestination(uint32 cooldownDelay1 = 5 * 60 * 1000) : TravelDestination()
        {
            cooldownDelay = cooldownDelay1;
        };

        Quest const* GetQuestTemplate() override { return nullptr; }

        bool isActive(Player* bot) override { return false; }

        std::string const& getName() override { return "NullTravelDestination"; }
        std::string const& getTitle() override { return "no destination"; }

        bool isIn(WorldPosition* pos) override { return true; }
        bool isOut(WorldPosition* pos) override { return false; }

};

//A travel target specifically related to a quest.
class QuestTravelDestination : public TravelDestination
{
    public:
        QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1);

        Quest const* GetQuestTemplate() override { return questTemplate;  }

        bool isActive(Player* bot) override;

        std::string const& getName() override { return "QuestTravelDestination"; }
        int32 getEntry() override { return 0; }
        std::string const& getTitle() override;

    protected:
        uint32 questId;
        Quest const* questTemplate;
};

//A quest giver or taker.
class QuestRelationTravelDestination : public QuestTravelDestination
{
    public:
        QuestRelationTravelDestination(uint32 quest_id1, uint32 entry1, uint32 relation1, float radiusMin1, float radiusMax1) :
            QuestTravelDestination(quest_id1, radiusMin1, radiusMax1)
        {
            entry = entry1;
            relation = relation1;
        }

        bool isActive(Player* bot) override;
        std::string const& getName() override { return "QuestRelationTravelDestination"; }
        int32 getEntry() override { return entry; }
        std::string const& getTitle() override;
        virtual uint32 getRelation() { return relation; }

    private:
        uint32 relation;
        int32 entry;
};

//A quest objective (creature/gameobject to grind/loot)
class QuestObjectiveTravelDestination : public QuestTravelDestination
{
    public:
        QuestObjectiveTravelDestination(uint32 quest_id1, uint32 entry1, uint32 objective1, float radiusMin1, float radiusMax1, uint32 itemId1 = 0) :
            QuestTravelDestination(quest_id1, radiusMin1, radiusMax1)
        {
            objective = objective1;
            entry = entry1;
            itemId = itemId1;
        }

        bool isCreature();
        uint32 ReqCreature();
        uint32 ReqGOId();
        uint32 ReqCount();

        bool isActive(Player* bot) override;
        std::string const& getName() override { return "QuestObjectiveTravelDestination"; }
        int32 getEntry() override { return entry; }
        std::string const& getTitle() override;

    private:
        uint32 objective;
        uint32 entry;
        uint32 itemId = 0;
};

//A location with rpg target(s) based on race and level
class RpgTravelDestination : public TravelDestination
{
    public:
        RpgTravelDestination(uint32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
        {
            entry = entry1;
        }

        bool isActive(Player* bot) override;
        virtual CreatureTemplate const* GetCreatureTemplate();
        std::string const& getName() override { return "RpgTravelDestination"; }
        int32 getEntry() override { return 0; }
        std::string const& getTitle() override;

    protected:
        uint32 entry;
};

//A location with zone exploration target(s)
class ExploreTravelDestination : public TravelDestination
{
    public:
        ExploreTravelDestination(uint32 areaId1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
        {
            areaId = areaId1;
        }

        bool isActive(Player* bot) override;
        std::string const& getName() override { return "ExploreTravelDestination"; }
        int32 getEntry() override { return 0; }
        std::string const& getTitle() override;
        virtual uint32 getAreaId() { return areaId; }

    protected:
        uint32 areaId;
};

//A location with zone exploration target(s)
class GrindTravelDestination : public TravelDestination
{
    public:
        GrindTravelDestination(int32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
        {
            entry = entry1;
        }

        bool isActive(Player* bot) override;
        virtual CreatureTemplate const* GetCreatureTemplate();
        std::string const& getName() override { return "GrindTravelDestination"; }
        int32 getEntry() override { return entry; }
        std::string const& getTitle() override;

    protected:
        int32 entry;
};

//A location with a boss
class BossTravelDestination : public TravelDestination
{
    public:
        BossTravelDestination(int32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
        {
            entry = entry1;
            cooldownDelay = 1000;
        }

        bool isActive(Player* bot) override;
        CreatureInfo const* getCreatureInfo() override { return ObjectMgr::GetCreatureTemplate(entry); }
        string getName() override { return "BossTravelDestination"; }
        int32 getEntry() override { return entry; }
        string getTitle() override;

    protected:
        int32 entry;
};

//Current target and location for the bot to travel to.
//The flow is as follows:
//PREPARE   (wait until no loot is near)
//TRAVEL    (move towards target until close enough) (rpg and grind is disabled)
//WORK      (grind/rpg until the target is no longer active) (rpg and grind is enabled on quest mobs)
//COOLDOWN  (wait some time free to do what the bot wants)
//EXPIRE    (if any of the above actions take too long pick a new target)
class TravelTarget : AiObject
{
    public:
        TravelTarget(PlayerbotAI* botAI) : AiObject(botAI), m_status(TRAVEL_STATUS_NONE), startTime(getMSTime()) { };
        TravelTarget(PlayerbotAI* botAI, TravelDestination* tDestination1, WorldPosition* wPosition1) : AiObject(botAI), m_status(TRAVEL_STATUS_NONE), startTime(getMSTime())
        {
            setTarget(tDestination1, wPosition1);
        }

        ~TravelTarget();

        void setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1 = false);
        void setStatus(TravelStatus status);
        void setExpireIn(uint32 expireMs)
        {
            statusTime = getExpiredTime() + expireMs;
        }

        void incRetry(bool isMove)
        {
            if (isMove)
                ++moveRetryCount;
            else
                ++extendRetryCount;
        }

        void setRetry(bool isMove, uint32 newCount = 0)
        {
            if (isMove)
                moveRetryCount = newCount;
            else
                extendRetryCount = newCount;
        }

        void setForced(bool forced1)
        {
            forced = forced1;
        }

        void setRadius(float radius1)
        {
            radius = radius1;
        }

        void copyTarget(TravelTarget* target);
        void addVisitors();
        void releaseVisitors();

        float distance(Player* bot);

        WorldPosition* getPosition()
        {
            return wPosition;
        };

        TravelDestination* getDestination()
        {
            return tDestination;
        };

        uint32 getEntry()
        {
            if (!tDestination)
                return 0;

            return tDestination->getEntry();
        }

        PlayerbotAI* getAi()
        {
            return botAI;
        }

        uint32 getExpiredTime()
        {
            return getMSTime() - startTime;
        }

        uint32 getTimeLeft()
        {
            return statusTime - getExpiredTime();
        }

        uint32 getMaxTravelTime();
        uint32 getRetryCount(bool isMove)
        {
            return isMove ? moveRetryCount: extendRetryCount;
        }

        bool isTraveling();
        bool isActive();
        bool isWorking();
        bool isPreparing();
        bool isMaxRetry(bool isMove)
        {
            return isMove ? (moveRetryCount > 5) : (extendRetryCount > 5);
        }

        TravelStatus getStatus()
        {
            return m_status;
        }

        TravelState getTravelState();

        bool isGroupCopy()
        {
            return groupCopy;
        }

        bool isForced()
        {
            return forced;
        }

    protected:
        TravelStatus m_status;

        uint32 startTime;
        uint32 statusTime = 0;

        bool forced = false;
        float radius = 0.f;
        bool groupCopy = false;
        bool visitor = true;

        uint32 extendRetryCount = 0;
        uint32 moveRetryCount = 0;

        TravelDestination* tDestination = nullptr;
        WorldPosition* wPosition = nullptr;
};

//General container for all travel destinations.
class TravelMgr
{
    public:
        TravelMgr() { };

        static TravelMgr* instance()
        {
            static TravelMgr instance;
            return &instance;
        }

        void Clear();
        void LoadQuestTravelTable();

        static void SetMobAvoidArea();
        static void SetMobAvoidAreaMap(uint32 mapId);

        std::vector<WorldPosition*> getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points, uint32 amount = 1);
        std::vector<WorldPosition> getNextPoint(WorldPosition center, std::vector<WorldPosition> points, uint32 amount = 1);
        QuestStatusData* getQuestStatus(Player* bot, uint32 questId);
        bool getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective);
        uint32 getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest);
        std::vector<TravelDestination*> getQuestTravelDestinations(Player* bot, int32 questId = -1, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000, bool ignoreObjectives = false);
        std::vector<TravelDestination*> getRpgTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000);
        std::vector<TravelDestination*> getExploreTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false);
        std::vector<TravelDestination*> getGrindTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000);
        vector<TravelDestination*> getBossTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 25000);

        void setNullTravelTarget(Player* player);

        void addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance = 0.1f, bool makeShortcuts = true);
        void loadMapTransfers();
        float mapTransDistance(WorldPosition start, WorldPosition end);
        float fastMapTransDistance(WorldPosition start, WorldPosition end);

        NullTravelDestination* nullTravelDestination = new NullTravelDestination();
        WorldPosition* nullWorldPosition = new WorldPosition();

        void addBadVmap(uint32 mapId, uint8 x, uint8 y)
        {
            badVmap.push_back(std::make_tuple(mapId, x, y));
        }

        void addBadMmap(uint32 mapId, uint8 x, uint8 y)
        {
            badMmap.push_back(std::make_tuple(mapId, x, y));
        }

        bool isBadVmap(uint32 mapId, uint8 x, uint8 y)
        {
            return std::find(badVmap.begin(), badVmap.end(), std::make_tuple(mapId, x, y)) != badVmap.end();
        }

        bool isBadMmap(uint32 mapId, uint8 x, uint8 y)
        {
            return std::find(badMmap.begin(), badMmap.end(), std::make_tuple(mapId, x, y)) != badMmap.end();
        }

        void printGrid(uint32 mapId, int x, int y, string type);
        void printObj(WorldObject* obj, string type);

    //protected:
        void logQuestError(uint32 errorNr, Quest* quest, uint32 objective = 0, uint32 unitId = 0, uint32 itemId = 0);

        vector<uint32> avoidLoaded;

        std::vector<QuestTravelDestination*> questGivers;
        std::vector<RpgTravelDestination*> rpgNpcs;
        std::vector<GrindTravelDestination*> grindMobs;
        vector<BossTravelDestination*> bossMobs;

        std::unordered_map<uint32, ExploreTravelDestination*> exploreLocs;
        std::unordered_map<uint32, QuestContainer*> quests;
        std::unordered_map<uint64, WorldPosition> pointsMap;

        std::vector<std::tuple<uint32, uint8, uint8>> badVmap, badMmap;

        std::unordered_map<std::pair<uint32, uint32>, std::vector<mapTransfer>, boost::hash<std::pair<uint32, uint32>>> mapTransfersMap;
};

#define sTravelMgr TravelMgr::instance()

#endif
