/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVELMGR_H
#define _PLAYERBOT_TRAVELMGR_H

#include "AiObject.h"
#include "CreatureData.h"
#include "GameObject.h"
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

//Extension of WorldLocation with distance functions.
class WorldPosition
{
    public:
        //Constructors
        WorldPosition();
        WorldPosition(WorldLocation const loc);
        WorldPosition(WorldPosition const& pos);
        WorldPosition(uint32 mapid, float x, float y, float z = 0, float orientation = 0);
        WorldPosition(WorldObject* wo);
        WorldPosition(std::vector<WorldPosition*> list, WorldPositionConst conType);
        WorldPosition(std::vector<WorldPosition> list, WorldPositionConst conType);
        WorldPosition(uint32 mapid, std::pair<uint8, uint8> grid);
        WorldPosition(CreatureData const* cdPair);
        WorldPosition(GameObjectData const* cdPair);
        WorldPosition(ObjectGuid guid);
        WorldPosition(GuidPosition gpos);

        //Setters
        void setX(float x);
        void setY(float y);
        void setZ(float z);
        void setLocation(WorldLocation const loc)
        {
            wLoc = loc;
        }

        void addVisitor()
        {
            ++visitors;
        }

        void remVisitor()
        {
            --visitors;
        }

        //Getters
        operator bool() const;
        bool operator==(WorldPosition const& p1);
        bool operator!=(WorldPosition const& p1);

        WorldLocation getLocation();
        uint32 getMapId();
        float getX();
        float getY();
        float getZ();
        float getO();

        G3D::Vector3 getVector3();
        std::string const& print();

        void printWKT(std::vector<WorldPosition> points, std::ostringstream& out, uint32 dim = 0, bool loop = false);
        void printWKT(std::ostringstream& out) { printWKT({ *this }, out); }

        uint32 getVisitors() { return visitors; }

        bool isOverworld();
        bool isInWater();
        bool isUnderWater();

        WorldPosition relPoint(WorldPosition* center);
        WorldPosition offset(WorldPosition* center);
        float size();

        //Slow distance function using possible map transfers.
        float distance(WorldPosition* center);
        float distance(WorldPosition center)
        {
            return distance(&center);
        };

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

        std::pair<uint8, uint8> getGrid();
        std::vector<std::pair<uint8, uint8>> getGrids(WorldPosition secondPos);
        std::vector<WorldPosition> fromGrid(uint8 x, uint8 y);

        void loadMapAndVMap(uint32 mapId, uint8 x, uint8 y);

        void loadMapAndVMap()
        {
            loadMapAndVMap(getMapId(), getGrid().first, getGrid().second);
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
        std::vector<WorldPosition> getPathFromPath(std::vector<WorldPosition> startPath, Unit* bot);

        std::vector<WorldPosition> getPathFrom(WorldPosition startPos, Unit* bot)
        {
            return getPathFromPath({ startPos }, bot);
        };

        std::vector<WorldPosition> getPathTo(WorldPosition endPos, Unit* bot)
        {
            return endPos.getPathFrom(*this, bot);
        }

        bool isPathTo(std::vector<WorldPosition> path, float maxDistance = sPlayerbotAIConfig->targetPosRecalcDistance) { return !path.empty() && distance(path.back()) < maxDistance; };
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

        //Creatures
        std::vector<CreatureData const*> getCreaturesNear(float radius = 0, uint32 entry = 0);

        //GameObjects
        std::vector<GameObjectData const*> getGameObjectsNear(float radius = 0, uint32 entry = 0);

    private:
        WorldLocation wLoc;
        uint32 visitors = 0;
};

class GuidPosition : public ObjectGuid
{
    public:
        GuidPosition() : ObjectGuid::Empty { }
        GuidPosition(ObjectGuid guid) : ObjectGuid(guid)
        {
            point = WorldPosition(guid);
        }

        template<class T>
        GuidPosition(ObjectGuid guid, T) : ObjectGuid(guid)
        {
            point = WorldPosition(T);
        }

        GuidPosition(GuidPosition const& guidp) : ObjectGuid(guidp)
        {
            point = guidp.point;
        }

        CreatureData const* getCreatureData();
        CreatureTemplate const* GetCreatureTemplate();

        WorldPosition getPosition() { return point; }

        WorldObject* GetWorldObject();
        Unit* getUnit();
        GameObject* getGameObject();

        bool isDead(); //For loaded grids check if the unit/object is unloaded/dead.

    private:
        WorldPosition point = WorldPosition();
};

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
        virtual uint32 getEntry() { return 0; }
        virtual std::string const& getTitle() { return "generic travel destination"; }

        WorldPosition* nearestPoint(WorldPosition* pos);

        float distanceTo(WorldPosition* pos)
        {
            return nearestPoint(pos)->distance(pos);
        }

        virtual bool isIn(WorldPosition* pos) { return distanceTo(pos) <= radiusMin; }
        virtual bool isOut(WorldPosition* pos) { return distanceTo(pos) > radiusMax; }

        float getRadiusMin()
        {
            return radiusMin;
        }

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

//Generic creature finder
class FindPointCreatureData
{
    public:
        FindPointCreatureData(WorldPosition point1 = WorldPosition(), float radius1 = 0, uint32 entry1 = 0)
        {
            point = point1;
            radius = radius1;
            entry = entry1;
        }

        bool operator()(CreatureData const& data);
        std::vector<CreatureData const*> GetResult() const { return data; };

    private:
        WorldPosition point;
        float radius;
        uint32 entry;

        std::vector<CreatureData const*> data;
};

//Generic gameObject finder
class FindPointGameObjectData
{
    public:
        FindPointGameObjectData(WorldPosition point1 = WorldPosition(), float radius1 = 0, uint32 entry1 = 0)
        {
            point = point1;
            radius = radius1;
            entry = entry1;
        }

        bool operator()(GameObjectData const& data);
        std::vector<GameObjectData const*> GetResult() const { return data; };

    private:
        WorldPosition point;
        float radius;
        uint32 entry;

        std::vector<GameObjectData const*> data;
};

//A travel target that is always inactive and jumps to cooldown.
class NullTravelDestination : public TravelDestination
{
    public:
        NullTravelDestination(uint32 coodownDelay1 = 5 * 60 * 1000) : TravelDestination()
        {
            cooldownDelay = coodownDelay1;
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
        uint32 getEntry() override { return 0; }
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
        uint32 getEntry() override { return entry; }
        std::string const& getTitle() override;
        virtual uint32 getRelation() { return relation; }

    private:
        uint32 relation;
        uint32 entry;
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
        uint32 getEntry() override { return entry; }
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
        uint32 getEntry() override { return 0; }
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
        uint32 getEntry() override { return 0; }
        std::string const& getTitle() override;
        virtual uint32 getAreaId() { return areaId; }

    protected:
        uint32 areaId;
};

//A location with zone exploration target(s)
class GrindTravelDestination : public TravelDestination
{
    public:
        GrindTravelDestination(uint32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
        {
            entry = entry1;
        }

        static uint32 moneyNeeded(Player* bot);

        bool isActive(Player* bot) override;
        virtual CreatureTemplate const* GetCreatureTemplate();
        std::string const& getName() override { return "GrindTravelDestination"; }
        uint32 getEntry() override { return entry; }
        std::string const& getTitle() override;

    protected:
        uint32 entry;
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

        void copyTarget(TravelTarget* target);
        void addVisitors();
        void releaseVisitors();

        float distance(Player* bot);

        WorldLocation getLocation()
        {
            return wPosition->getLocation();
        };

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

        std::vector<WorldPosition*> getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points, uint32 amount = 1);
        std::vector<WorldPosition> getNextPoint(WorldPosition center, std::vector<WorldPosition> points, uint32 amount = 1);
        QuestStatusData* getQuestStatus(Player* bot, uint32 questId);
        bool getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective);
        uint32 getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest);
        std::vector<TravelDestination*> getQuestTravelDestinations(Player* bot, int32 questId = -1, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000, bool ignoreObjectives = false);
        std::vector<TravelDestination*> getRpgTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000);
        std::vector<TravelDestination*> getExploreTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false);
        std::vector<TravelDestination*> getGrindTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000);

        void setNullTravelTarget(Player* player);

        void addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance = 0.1f, bool makeShortcuts = true);
        void loadMapTransfers();
        float mapTransDistance(WorldPosition start, WorldPosition end);

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

    protected:
        void logQuestError(uint32 errorNr, Quest* quest, uint32 objective = 0, uint32 unitId = 0, uint32 itemId = 0);

        std::vector<QuestTravelDestination*> questGivers;
        std::vector<RpgTravelDestination*> rpgNpcs;
        std::vector<GrindTravelDestination*> grindMobs;

        std::unordered_map<uint32, ExploreTravelDestination*> exploreLocs;
        std::unordered_map<uint32, QuestContainer*> quests;
        std::unordered_map<int32, WorldPosition> pointsMap;

        std::vector<std::tuple<uint32, uint8, uint8>> badVmap, badMmap;

        std::unordered_map<std::pair<uint32, uint32>, std::vector<mapTransfer>, boost::hash<std::pair<uint32, uint32>>> mapTransfersMap;
};

#define sTravelMgr TravelMgr::instance()

#endif
