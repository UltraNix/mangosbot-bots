/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TravelMgr.h"
#include "CellImpl.h"
#include "ChatHelper.h"
#include "MapManager.h"
#include "MMapFactory.h"
#include "PathGenerator.h"
#include "Playerbot.h"
#include "SharedValueContext.h"
#include "StrategyContext.h"
#include "TransportMgr.h"
#include "TravelNode.h"
#include "VMapFactory.h"
#include "VMapManager2.h"

#include <numeric>
#include <iomanip>

WorldPosition::WorldPosition()
{
    wLoc = WorldLocation();
}

WorldPosition::WorldPosition(WorldLocation const loc)
{
    wLoc = loc;
}

WorldPosition::WorldPosition(WorldPosition const& pos)
{
    wLoc = pos.wLoc;
    visitors = pos.visitors;
}

WorldPosition::WorldPosition(uint32 mapid, float x, float y, float z = 0, float orientation = 0)
{
    wLoc = WorldLocation(mapid, x, y, z, orientation);
}

WorldPosition::WorldPosition(const string str)
{
    stringstream out(str);
    out >> mapid;
    out >> coord_x;
    out >> coord_y;
    out >> coord_z;
    out >> orientation;
}

WorldPosition::WorldPosition(uint32 mapId, const Position& pos) : WorldLocation(mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetPositionO())
{
}

WorldPosition::WorldPosition(WorldObject* wo)
{
    if (wo)
    {
        wLoc = WorldLocation(wo->GetMapId(), wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(), wo->GetOrientation());
    }
}

WorldPosition::WorldPosition(uint32 mapid, std::pair<uint8, uint8> grid)
{
    wLoc = WorldLocation(mapid, (32 - grid.first) * SIZE_OF_GRIDS, (32 - grid.second) * SIZE_OF_GRIDS, 0, 0);
}

WorldPosition::WorldPosition(CreatureData const* creatureData)
{
    if (creatureData)
    {
        wLoc = WorldLocation(creatureData->mapid, creatureData->posX, creatureData->posY, creatureData->posZ, creatureData->orientation);
    }
}

WorldPosition::WorldPosition(GameObjectData const* gameobjectData)
{
    if (gameobjectData)
    {
        wLoc = WorldLocation(gameobjectData->mapid, gameobjectData->posX, gameobjectData->posY, gameobjectData->posZ, gameobjectData->orientation);
    }
}

WorldPosition::WorldPosition(GuidPosition guid)
{
    if (guid.mapid !=0 || guid.coord_x != 0 || guid.coord_y != 0 || guid.coord_z !=0)
    {
        set(WorldPosition(guid.mapid, guid.coord_x, guid.coord_y, guid.coord_z, guid.orientation));
        return;
    }

    set(ObjectGuid(guid));
 }

void WorldPosition::set(ObjectGuid guid)
{
    switch (guid.GetHigh())
    {
        case HighGuid::Player:
        {
            Player* player = ObjectAccessor::FindPlayer(guid);
            if (player)
                set(WorldLocation(player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()));
            break;
        }
        case HighGuid::GameObject:
        {
            GameObjectData const* gameobjectData = &sObjectMgr->NewGOData(guid.GetCounter());
            if (gameobjectData && gameobjectData->id)
                set(WorldLocation(gpair->second.mapid, gpair->second.posX, gpair->second.posY, gpair->second.posZ, gpair->second.orientation));
            break;
        }
        case HighGuid::Unit:
        {
            CreatureData const* creatureData = &sObjectMgr->NewOrExistCreatureData(guid.GetCounter());
            if (creatureData && creatureData->id)
                set(WorldLocation(cpair->second.mapid, cpair->second.posX, cpair->second.posY, cpair->second.posZ, cpair->second.orientation));
            break;
        }
    default:
        break;
    }
}

WorldPosition::WorldPosition(std::vector<WorldPosition*> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (!size)
        return;

    if (size == 1)
        set(*list.front());
    else if (conType == WP_RANDOM)
        set(*list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
    {
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0]->getMapId(), 0, 0, 0, 0), [size](WorldLocation i, WorldPosition* j)
        {
            i.m_positionX += j->getX() / size;
            i.m_positionY += j->getY() / size;
            i.m_positionZ += j->getZ() / size;
            i.NormalizeOrientation(i.m_orientation += j->getO() / size);
            return i;
        }));
    }
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(*pos.closestSq(list));
    }
}

WorldPosition::WorldPosition(std::vector<WorldPosition> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (!size)
        return;

    if (size == 1)
        set(list.front());
    else if (conType == WP_RANDOM)
        set(list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
    {
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0].getMapId(), 0, 0, 0, 0), [size](WorldLocation i, WorldPosition* j)
        {
            i.m_positionX += j->getX() / size;
            i.m_positionY += j->getY() / size;
            i.m_positionZ += j->getZ() / size;
            i.NormalizeOrientation(i.m_orientation += j->getO() / size);
            return i;
        }));
    }
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(pos.closestSq(list));
    }
}

void WorldPosition::setX(float x)
{
    wLoc.m_positionX = x;
}

void WorldPosition::setY(float y)
{
    wLoc.m_positionY = y;
}

void WorldPosition::setZ(float z)
{
    wLoc.m_positionZ = z;
}

WorldPosition::operator bool() const
{
    return  wLoc.GetMapId() != 0 || wLoc.GetPositionX() != 0 || wLoc.GetPositionY() != 0 || wLoc.GetPositionZ() != 0;
}

bool WorldPosition::operator==(WorldPosition const& p1)
{
    return wLoc.GetMapId() == wLoc.GetMapId() && wLoc.GetPositionX() == p1.wLoc.GetPositionX() &&
        wLoc.GetPositionY() == p1.wLoc.GetPositionY() && wLoc.GetPositionZ() == p1.wLoc.GetPositionZ() && wLoc.GetOrientation() == p1.wLoc.GetOrientation();
}

bool WorldPosition::operator!=(WorldPosition const& p1)
{
    return wLoc.GetMapId() != wLoc.GetMapId() || wLoc.GetPositionX() != p1.wLoc.GetPositionX() || wLoc.GetPositionY() != p1.wLoc.GetPositionY() ||
        wLoc.GetPositionZ() != p1.wLoc.GetPositionZ() || wLoc.GetOrientation() != p1.wLoc.GetOrientation();
}

WorldLocation WorldPosition::getLocation()
{
    return wLoc;
}

uint32 WorldPosition::getMapId()
{
    return wLoc.GetMapId();
}

float WorldPosition::getX()
{
    return wLoc.GetPositionX();
}

float WorldPosition::getY()
{
    return wLoc.GetPositionY();
}

float WorldPosition::getZ()
{
    return wLoc.GetPositionZ();
}

float WorldPosition::getO()
{
    return wLoc.GetOrientation();
}

bool WorldPosition::isOverworld()
{
    return wLoc.GetMapId() == 0 || wLoc.GetMapId() == 1 || wLoc.GetMapId() == 530 || wLoc.GetMapId() == 571;
}

bool WorldPosition::isInWater()
{
    return getMap() ? getMap()->IsInWater(wLoc.GetPositionX(), wLoc.GetPositionY(), wLoc.GetPositionZ()) : false;
};

bool WorldPosition::isUnderWater()
{
    return getMap() ? getMap()->IsUnderWater(wLoc.GetPositionX(), wLoc.GetPositionY(), wLoc.GetPositionZ()) : false;
};

WorldPosition WorldPosition::relPoint(WorldPosition* center)
{
    return WorldPosition(wLoc.GetMapId(), wLoc.GetPositionX() - center->wLoc.GetPositionX(),
        wLoc.GetPositionY() - center->wLoc.GetPositionY(), wLoc.GetPositionZ() - center->wLoc.GetPositionZ(), wLoc.GetOrientation());
}

WorldPosition WorldPosition::offset(WorldPosition* center)
{
    return WorldPosition(wLoc.GetMapId(), wLoc.GetPositionX() + center->wLoc.GetPositionX(),
        wLoc.GetPositionY() + center->wLoc.GetPositionY(), wLoc.GetPositionZ() + center->wLoc.GetPositionZ(), wLoc.GetOrientation());
}

float WorldPosition::size()
{
    return sqrt(pow(wLoc.GetPositionX(), 2.0) + pow(wLoc.GetPositionY(), 2.0) + pow(wLoc.GetPositionZ(), 2.0));
}

float WorldPosition::distance(WorldPosition* center)
{
    if(wLoc.GetMapId() == center->getMapId())
        return relPoint(center).size();

    //this -> mapTransfer | mapTransfer -> center
    return sTravelMgr->mapTransDistance(*this, *center);
};

float WorldPosition::fDist(WorldPosition* center)
{
    if (mapid == center->getMapId())
        return sqrt(sqDistance2d(center));

    // this -> mapTransfer | mapTransfer -> center
    return sTravelMgr.fastMapTransDistance(*this, *center);
};

//When moving from this along list return last point that falls within range.
//Distance is move distance along path.
WorldPosition WorldPosition::lastInRange(std::vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    float startDist = 0.0f;

    //Enter the path at the closest point.
    for (auto& p : list)
    {
        float curDist = distance(p);
        if (startDist < curDist || p == list.front())
            startDist = curDist + 0.1f;
    }

    float totalDist = 0.0f;

    //Follow the path from the last nearest point
    //Return last point in range.
    for (auto& p : list)
    {
        float curDist = distance(p);

        if (totalDist > 0) //We have started the path. Keep counting.
            totalDist += p.distance(std::prev(&p, 1));

        if (curDist == startDist) //Start the path here.
            totalDist = startDist;

        if (minDist > 0 && totalDist < minDist)
            continue;

        if (maxDist > 0 && totalDist > maxDist)
            continue; //We do not break here because the path may loop back and have a second startDist point.

        rPoint = p;
    }

    return rPoint;
};

//Todo: remove or adjust to above standard.
WorldPosition WorldPosition::firstOutRange(std::vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    for (auto& p : list)
    {
        if (minDist > 0 && distance(p) < minDist)
            return p;

        if (maxDist > 0 && distance(p) > maxDist)
            return p;

        rPoint = p;
    }

    return rPoint;
}

//Returns true if (on the x-y plane) the position is inside the three points.
bool WorldPosition::isInside(WorldPosition* p1, WorldPosition* p2, WorldPosition* p3)
{
    if (getMapId() != p1->getMapId() != p2->getMapId() != p3->getMapId())
        return false;

    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = mSign(p1, p2);
    d2 = mSign(p2, p3);
    d3 = mSign(p3, p1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

MapEntry const* WorldPosition::getMapEntry()
{
    return sMapStore.LookupEntry(wLoc.GetMapId());
};

uint32 WorldPosition::getInstanceId()
{
    if (Map* map = sMapMgr->FindBaseMap(getMapId()))
        return map->GetInstanceId();

    return 0;
}

Map* WorldPosition::getMap()
{
    return sMapMgr->FindMap(wLoc.GetMapId(), getMapEntry()->Instanceable() ? getInstanceId() : 0);
}

const float WorldPosition::getHeight()
{
    return getMap()->GetHeight(getX(), getY(), getZ());
}

G3D::Vector3 WorldPosition::getVector3()
{
    return G3D::Vector3(coord_x, coord_y, coord_z);
}

std::string const& WorldPosition::print()
{
    std::ostringstream out;
    out << mapid << std::fixed << std::setprecision(2);
    out << ';' << coord_x;
    out << ';' << coord_y;
    out << ';' << coord_z;
    out << ';' << orientation;

    return out.str();
}

void WorldPosition::printWKT(std::vector<WorldPosition> points, std::ostringstream& out, uint32 dim, bool loop)
{
    switch (dim)
    {
        case 0:
            if(points.size() == 1)
                out << "\"POINT(";
            else
                out << "\"MULTIPOINT(";
            break;
        case 1:
            out << "\"LINESTRING(";
            break;
        case 2:
            out << "\"POLYGON((";
    }

    for (auto& p : points)
        out << p.getDisplayX() << " " << p.getDisplayY() << (!loop && &p == &points.back() ? "" : ",");

    if (loop)
        out << points.front().getDisplayX() << " " << points.front().getDisplayY();

    out << (dim == 2 ? "))\"," : ")\",");
}

WorldPosition WorldPosition::getDisplayLocation()
{
    return offset(&sTravelNodeMap->getMapOffset(getMapId()));
};

uint16 WorldPosition::getAreaId()
{
    return sMapMgr->GetAreaId(getMapId(), getX(), getY(), getZ());
};

AreaTableEntry const* WorldPosition::getArea()
{
    uint16 areaId = getAreaId();
    if (!areaId)
        return nullptr;

    return sAreaTableStore.LookupEntry(areaId);
}

std::string const& WorldPosition::getAreaName(bool fullName, bool zoneName)
{
    if (!isOverworld())
    {
        MapEntry const* map = sMapStore.LookupEntry(getMapId());
        if (map)
            return map->name[0];
    }

    AreaTableEntry const* area = getArea();
    if (!area)
        return "";

    std::string areaName = area->area_name[0];

    if (fullName)
    {
        uint16 zoneId = area->zone;

        while (zoneId > 0)
        {
            AreaTableEntry const* parentArea = sAreaTableStore.LookupEntry(zoneId);
            if (!parentArea)
                break;

            std::string const& subAreaName = parentArea->area_name[0];

            if (zoneName)
                areaName = subAreaName;
            else
                areaName = subAreaName + " " + areaName;

            zoneId = parentArea->zone;
        }
    }

    return areaName;
}

std::set<Transport*> WorldPosition::getTransports(uint32 entry)
{
    /*
    if(!entry)
        return getMap()->m_transports;
    else
    {
    */
        std::set<Transport*> transports;
        /*
        for (auto transport : getMap()->m_transports)
            if(transport->GetEntry() == entry)
                transports.insert(transport);

        return transports;
    }
    */
    return transports;
}

std::vector<GridPair> WorldPosition::getGridPairs(WorldPosition secondPos)
{
    std::vector<GridPair> retVec;

    int lx = std::min(getGridPair().x_coord, secondPos.getGridPair().x_coord);
    int ly = std::min(getGridPair().y_coord, secondPos.getGridPair().y_coord);
    int ux = std::max(getGridPair().x_coord, secondPos.getGridPair().x_coord);
    int uy = std::max(getGridPair().y_coord, secondPos.getGridPair().y_coord);

    int32 border = 1;

    lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(GridPair(x, y));
        }
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromGridPair(GridPair gridPair)
{
    std::vector<WorldPosition> retVec;
    GridPair g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridPair;

        if (d == 1 || d == 2)
            g >> 1;

        if (d == 2 || d == 3)
            g += 1;

        retVec.push_back(WorldPosition(getMapId(), g));
    }

    return retVec;
}

vector<WorldPosition> WorldPosition::fromCellPair(CellPair cellPair)
{
    vector<WorldPosition> retVec;
    CellPair p;

    for (uint32 d = 0; d < 4; d++)
    {
        p = cellPair;

        if (d == 1 || d == 2)
            p >> 1;

        if (d == 2 || d == 3)
            p += 1;

        retVec.push_back(WorldPosition(getMapId(), p));
    }
    return retVec;
}

vector<WorldPosition> WorldPosition::gridFromCellPair(CellPair cellPair)
{
    Cell c(cellPair);

    return fromGridPair(GridPair(c.GridX(), c.GridY()));
}

vector<pair<int,int>> WorldPosition::getmGridPairs(WorldPosition secondPos)
{
    std::vector<mGridPair> retVec;

    int lx = std::min(getmGridPair().first, secondPos.getmGridPair().first);
    int ly = std::min(getmGridPair().second, secondPos.getmGridPair().second);
    int ux = std::max(getmGridPair().first, secondPos.getmGridPair().first);
    int uy = std::max(getmGridPair().second, secondPos.getmGridPair().second);
    int border = 1;

    //lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    //ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    //ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    //uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(make_pair(x, y));
        }
    }

    return retVec;
}

vector<WorldPosition> WorldPosition::frommGridPair(mGridPair gridPair)
{
    vector<WorldPosition> retVec;
    mGridPair g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridPair;

        if (d == 1 || d == 2)
            g.second++;
        if (d == 2 || d == 3)
            g.first++;

        retVec.push_back(WorldPosition(getMapId(), g));
    }

    return retVec;
}

void WorldPosition::loadMapAndVMap(uint32 mapId, uint8 x, uint8 y)
{
    std::string const& fileName = "load_map_grid.csv";

    if (isOverworld() && false || false)
    {
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y))
            if (sPlayerbotAIConfig.hasLog(fileName))
            {
                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr();
                out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr.isBadMmap(mapId, x, y) ? "0" : "1") << ",";
                printWKT(fromGridPair(GridPair(x, y)), out, 1, true);
                sPlayerbotAIConfig.log(fileName, out.str().c_str());
            }

        int px = (float) (32 - x) * SIZE_OF_GRIDS;
        int py = (float) (32 - y) * SIZE_OF_GRIDS;

        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y))
            if (getTerrain())
                getTerrain()->GetTerrainType(px, py);
    }
    else
    {
        //This needs to be disabled or maps will not load.
        //Needs more testing to check for impact on movement.
        if (false)
            if (!VMAP::VMapFactory::createOrGetVMapManager()->IsTileLoaded(mapId, x, y) && !sTravelMgr.isBadVmap(mapId, x, y))
            {
                // load VMAPs for current map/grid...
                const MapEntry* i_mapEntry = sMapStore.LookupEntry(mapId);
                const char* mapName = i_mapEntry ? i_mapEntry->name[sWorld.GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";

                int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld.GetDataPath() + "vmaps").c_str(), mapId, x, y);
                switch (vmapLoadResult)
                {
                case VMAP::VMAP_LOAD_RESULT_OK:
                    //sLog.outError("VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, mapId, x, y, x, y);
                    break;
                case VMAP::VMAP_LOAD_RESULT_ERROR:
                    //sLog.outError("Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, mapId, x, y, x, y);
                    sTravelMgr.addBadVmap(mapId, x, y);
                    break;
                case VMAP::VMAP_LOAD_RESULT_IGNORED:
                    sTravelMgr.addBadVmap(mapId, x, y);
                    //sLog.outError("Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, mapId, x, y, x, y);
                    break;
                }

                if (sPlayerbotAIConfig.hasLog(fileName))
                {
                    ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr();
                    out << "+00,\"vmap\", " << x << "," << y << ", " << (sTravelMgr.isBadVmap(mapId, x, y) ? "0" : "1") << ",";
                    printWKT(frommGridPair(GridPair(x, y)), out, 1, true);
                    sPlayerbotAIConfig.log(fileName, out.str().c_str());
                }
            }

        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y) && !sTravelMgr.isBadMmap(mapId, x, y))
        {
            // load navmesh
            if (!MMAP::MMapFactory::createOrGetMMapManager()->loadMap(mapId, x, y))
                sTravelMgr.addBadMmap(mapId, x, y);

            if (sPlayerbotAIConfig.hasLog(fileName))
            {
                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr();
                out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr.isBadMmap(mapId, x, y) ? "0" : "1") << ",";
                printWKT(fromGridPair(GridPair(x, y)), out, 1, true);
                sPlayerbotAIConfig.log(fileName, out.str().c_str());
            }
        }
    }
}

void WorldPosition::loadMapAndVMaps(WorldPosition secondPos)
{
    for (auto& grid : getmGridPairs(secondPos))
    {
        loadMapAndVMap(getMapId(), grid.first, grid.second);
    }
}

std::vector<WorldPosition> WorldPosition::fromPointsArray(std::vector<G3D::Vector3> path)
{
    std::vector<WorldPosition> retVec;
    for (auto p : path)
        retVec.push_back(WorldPosition(getMapId(), p.x, p.y, p.z, getO()));

    return retVec;
}

//A single pathfinding attempt from one position to another. Returns pathfinding status and path.
std::vector<WorldPosition> WorldPosition::getPathStepFrom(WorldPosition startPos, Unit* bot)
{
    if (!bot)
        return { };

    //Load mmaps and vmaps between the two points.
    loadMapAndVMaps(startPos);

    PathGenerator path(bot);
    path.CalculatePath(startPos.getX(), startPos.getY(), startPos.getZ());

    Movement::PointsArray points = path.GetPath();
    PathType type = path.GetPathType();


    if (sPlayerbotAIConfig->hasLog("pathfind_attempt_point.csv"))
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(1);
        printWKT({ startPos, *this }, out);
        sPlayerbotAIConfig->log("pathfind_attempt_point.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig->hasLog("pathfind_attempt.csv") && (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL))
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
        out << std::fixed << std::setprecision(1) << type << ",";
        printWKT(fromPointsArray(points), out, 1);
        sPlayerbotAIConfig->log("pathfind_attempt.csv", out.str().c_str());
    }

    if (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL)
        return fromPointsArray(points);

    return { };
}

bool WorldPosition::cropPathTo(vector<WorldPosition>& path, float maxDistance)
{
    if (path.empty())
        return false;

    auto bestPos = std::min_element(path.begin(), path.end(), [this](WorldPosition i, WorldPosition j) {return this->sqDistance(i) < this->sqDistance(j); });

    bool insRange = this->sqDistance(*bestPos) <= maxDistance * maxDistance;

    if (bestPos == path.end())
        return insRange;

    path.erase(std::next(bestPos), path.end());

    return insRange;
}

//A sequential series of pathfinding attempts. Returns the complete path and if the patfinder eventually found a way to the destination.
std::vector<WorldPosition> WorldPosition::getPathFromPath(std::vector<WorldPosition> startPath, Unit* bot, uint8 maxAttempt)
{
    //We start at the end of the last path.
    WorldPosition currentPos = startPath.back();

    //No pathfinding across maps.
    if (getMapId() != currentPos.getMapId())
        return { };

    std::vector<WorldPosition> subPath, fullPath = startPath;

    // Limit the pathfinding attempts
    for (uint32 i = 0; i < maxAttempt; i++)
    {
        //Try to pathfind to this position.
        subPath = getPathStepFrom(currentPos, bot);

        //If we could not find a path return what we have now.
        if (subPath.empty() || currentPos.distance(&subPath.back()) < sPlayerbotAIConfig->targetPosRecalcDistance)
            break;

        //Append the path excluding the start (this should be the same as the end of the startPath)
        fullPath.insert(fullPath.end(), std::next(subPath.begin(),1), subPath.end());

        //Are we there yet?
        if (isPathTo(subPath))
            break;

        //Continue pathfinding.
        currentPos = subPath.back();
    }

    return fullPath;
}

bool WorldPosition::GetReachableRandomPointOnGround(Player* bot, float radius, bool randomRange = true)
{
    return getMap()->GetReachableRandomPointOnGround(bot->GetPhaseMask(), wLoc.coord_x, wLoc.coord_y, wLoc.coord_z, radius, randomRange);
}

uint32 WorldPosition::getUnitsNear(list<ObjectGuid>& units, float radius)
{
    units.remove_if([this, radius](ObjectGuid guid)
    {
        return sqDistance(WorldPosition(guid)) > radius * radius;
    });

    return units.size();
};

uint32 WorldPosition::getUnitsAggro(list<ObjectGuid>& units, Player* bot)
{
    units.remove_if([this, bot](ObjectGuid guid)
    {
        Unit* unit = GuidPosition(guid).GetUnit();
        if (!unit)
            return true;

        return sqDistance(WorldPosition(guid)) > unit->GetAttackDistance(bot) * unit->GetAttackDistance(bot);
    });

    return units.size();
};

bool FindPointCreatureData::operator()(CreatureData const& creatureData)
{
    if (!entry || creatureData.id == entry)
        if ((!point || creatureData.mapid == point.getMapId()) &&
            (!radius || point.sqDistance(WorldPosition(creatureData.mapid, creatureData.posX, creatureData.posY, creatureData.posZ)) < radius * radius))
        {
            data.push_back(&creatureData);
        }

    return false;
}

bool FindPointGameObjectData::operator()(GameObjectData const& gameobjectData)
{
    if (!entry || gameobjectData.id == entry)
        if ((!point || gameobjectData.mapid == point.getMapId()) &&
            (!radius || point.sqDistance(WorldPosition(gameobjectData.mapid, gameobjectData.posX, gameobjectData.posY, gameobjectData.posZ)) < radius * radius))
        {
            data.push_back(&gameobjectData);
        }

    return false;
}

std::vector<CreatureData const*> WorldPosition::getCreaturesNear(float radius, uint32 entry)
{
    FindPointCreatureData worker(*this, radius, entry);
    sObjectMgr->DoCreatureData(worker);
    return worker.GetResult();
}

std::vector<GameObjectData const*> WorldPosition::getGameObjectsNear(float radius, uint32 entry)
{
    FindPointGameObjectData worker(*this, radius, entry);
    sObjectMgr->DoGOData(worker);
    return worker.GetResult();
}

Creature* GuidPosition::GetCreature()
{
    if (!*this)
        return nullptr;

    return getMap()->GetAnyTypeCreature(*this);
}

Unit* GuidPosition::GetUnit()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return ObjectAccessor::FindPlayer(*this);

    if (IsPet())
        return point.getMap()->GetPet(*this);

    return GetCreature();
}

GameObject* GuidPosition::GetGameObject()
{
    if (!*this)
        return nullptr;

    return getMap()->GetGameObject(*this);
}

Player* GuidPosition::GetPlayer()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return sObjectAccessor.FindPlayer(*this);

    return nullptr;
}

bool GuidPosition::isDead()
{
    if (!getMap())
        return false;

    if (!getMap()->IsGridLoaded(getX(), getY()))
        return false;

    if (IsUnit() && GetUnit() && GetUnit()->IsInWorld() && GetUnit()->IsAlive())
        return false;

    if (IsGameObject() && GetGameObject() && GetGameObject()->IsInWorld())
        return false;

    return true;
}

CreatureData const* GuidPosition::getCreatureData()
{
    return IsCreature() ? sObjectMgr->GetCreatureData(GetCounter()) : nullptr;
}

CreatureTemplate const* GuidPosition::GetCreatureTemplate()
{
    return IsCreature() ? sObjectMgr->GetCreatureTemplate(GetEntry()) : nullptr;
};

WorldObject* GuidPosition::GetWorldObject()
{
    switch (GetHigh())
    {
        case HighGuid::Player:
            return ObjectAccessor::FindPlayer(*this);
        case HighGuid::Transport:
        case HighGuid::Mo_Transport:
        case HighGuid::GameObject:
            return point.getMap()->GetGameObject(*this);
        case HighGuid::Vehicle:
        case HighGuid::Unit:
            return point.getMap()->GetCreature(*this);
        case HighGuid::Pet:
            return point.getMap()->GetPet(*this);
        case HighGuid::DynamicObject:
            return point.getMap()->GetDynamicObject(*this);
        case HighGuid::Corpse:
            return point.getMap()->GetCorpse(*this);
        default:
            return nullptr;
    }

    return nullptr;
}

std::vector<WorldPosition*> TravelDestination::getPoints(bool ignoreFull)
{
    if (ignoreFull)
        return points;

    uint32 max = maxVisitorsPerPoint;
    if (!max)
        return points;

    std::vector<WorldPosition*> retVec;
    std::copy_if(points.begin(), points.end(), std::back_inserter(retVec), [max](WorldPosition* p) { return p->getVisitors() < max; });
    return retVec;
}

WorldPosition* TravelDestination::nearestPoint(WorldPosition* pos)
{
    return *std::min_element(points.begin(), points.end(), [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
}

std::vector<WorldPosition*> TravelDestination::touchingPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points;
    for (auto& point : points)
    {
        float dist = pos->distance(point);
        if (!dist)
            continue;

        if (dist > radiusMax * 2)
            continue;

        ret_points.push_back(point);
    }

    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::sortedPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points = points;
    std::sort(ret_points.begin(), ret_points.end(), [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::nextPoint(WorldPosition* pos, bool ignoreFull)
{
    return sTravelMgr->getNextPoint(pos, ignoreFull ? points : getPoints());
}

bool TravelDestination::isFull(bool ignoreFull)
{
    if (!ignoreFull && maxVisitors > 0 && visitors >= maxVisitors)
        return true;

    if (maxVisitorsPerPoint > 0)
        if (getPoints(ignoreFull).empty())
            return true;

    return false;
}

std::string const& QuestTravelDestination::getTitle()
{
    return ChatHelper::formatQuest(questTemplate);
}

bool QuestRelationTravelDestination::isActive(Player* bot)
{
    if (relation == 0)
    {
        if (questTemplate->GetQuestLevel() >= bot->getLevel() + 5)
            return false;

        //if (questTemplate->XPValue(bot) == 0)
        //    return false;

        if (!bot->GetMap()->IsContinent() || !bot->CanTakeQuest(questTemplate, false))
            return false;

        PlayerbotAI* ai = bot->GetPlayerbotAI();
        AiObjectContext* context = ai->GetAiObjectContext();

        uint32 dialogStatus = sTravelMgr.getDialogStatus(bot, entry, questTemplate);

        if (AI_VALUE(bool, "can fight equal"))
        {
            if (dialogStatus != DIALOG_STATUS_AVAILABLE)
                return false;
        }
        else
        {
            if (dialogStatus != DIALOG_STATUS_LOW_LEVEL_AVAILABLE)
                return false;
        }

        // Do not try to pick up dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) && !AI_VALUE(bool, "can fight boss"))
            return false;
    }
    else
    {
        if (!bot->IsActiveQuest(questId))
            return false;

        if (!bot->CanRewardQuest(questTemplate, false))
            return false;

        uint32 dialogStatus = sTravelMgr.getDialogStatus(bot, entry, questTemplate);

        if (dialogStatus != DIALOG_STATUS_REWARD2 && dialogStatus != DIALOG_STATUS_REWARD && dialogStatus != DIALOG_STATUS_REWARD_REP)
            return false;

        PlayerbotAI* ai = bot->GetPlayerbotAI();
        AiObjectContext* context = ai->GetAiObjectContext();

        // Do not try to hand-in dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) && !AI_VALUE(bool, "can fight boss"))
        {
            if (!this->nearestPoint(&WorldPosition(bot))->isOverworld())
                return false;
        }
    }

    return true;
}

std::string const& QuestRelationTravelDestination::getTitle()
{
    std::ostringstream out;

    if (relation == 0)
        out << "questgiver";
    else
        out << "questtaker";

    out << " " << ChatHelper::formatWorldEntry(entry);
    return out.str();
}

bool QuestObjectiveTravelDestination::isActive(Player* bot)
{
    if (questTemplate->GetQuestLevel() > bot->getLevel() + 1)
        return false;

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();
    if (questTemplate->GetQuestLevel() + 5 > bot->getLevel() && !AI_VALUE(bool, "can fight equal"))
        return false;

    //Check mob level
    if (getEntry() > 0)
    {
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(getEntry());
        if (cInfo && (int)cInfo->maxlevel - (int)bot->getLevel() > 4)
            return false;

         // Do not try to hand-in dungeon/elite quests in instances without a group.
        if (cInfo->Rank > CREATURE_ELITE_NORMAL)
        {
            if (!this->nearestPoint(&WorldPosition(bot))->isOverworld() && !AI_VALUE(bool, "can fight boss"))
                return false;
            else if (!AI_VALUE(bool, "can fight elite"))
                return false;
        }
    }

    if (questTemplate->GetType() == QUEST_TYPE_ELITE && !AI_VALUE(bool, "can fight elite"))
        return false;

    if (!sTravelMgr.getObjectiveStatus(bot, questTemplate, objective))
        return false;

    WorldPosition botPos(bot);

    if (getEntry() > 0 && !isOut(&botPos))
    {
        list<ObjectGuid> targets = AI_VALUE(list<ObjectGuid>, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == getEntry() && target.IsCreature() && ai->GetCreature(target) && ai->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    return true;
}

std::string const& QuestObjectiveTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "objective " << objective;

    if (itemId)
        out << " loot " << ChatHelper::formatItem(sObjectMgr->GetItemTemplate(itemId), 0, 0) << " from";
    else
        out << " to kill";

    out << " " << ChatHelper::formatWorldEntry(entry);
    return out.str();
}

bool RpgTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = bot->GetPlayerbotAI();
    AiObjectContext* context = botAI->GetAiObjectContext();

    CreatureInfo const* cInfo = this->getCreatureInfo();

    if (!cInfo)
        return false;

    bool isUsefull = false;

    if (cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR)
        if (AI_VALUE2_LAZY(bool, "group or", "should sell,can sell,following party,near leader"))
            isUsefull = true;

    if (cInfo->NpcFlags & UNIT_NPC_FLAG_REPAIR)
        if (AI_VALUE2_LAZY(bool, "group or", "should repair,can repair,following party,near leader"))
            isUsefull = true;

    if (!isUsefull)
        return false;

    //Once the target rpged with it is added to the ignore list. We can now move on.
    GuidSet& ignoreList = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<GuidSet&>("ignore rpg target")->Get();

    for (ObjectGuid const guid : ignoreList)
    {
        if (guid.GetEntry() == getEntry())
        {
            return false;
        }
    }

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction > REP_NEUTRAL;
}

CreatureTemplate const* RpgTravelDestination::GetCreatureTemplate()
{
    return sObjectMgr->GetCreatureTemplate(entry);
}

std::string const& RpgTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "rpg npc ";

    out << " " << ChatHelper::formatWorldEntry(entry);

    return out.str();
}

bool ExploreTravelDestination::isActive(Player* bot)
{
    AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId);

    if (area->area_level && (uint32)area->area_level > bot->getLevel() && bot->getLevel() < DEFAULT_MAX_LEVEL)
        return false;

    if (area->exploreFlag == 0xffff)
        return false;

    int offset = area->exploreFlag / 32;

    uint32 val = (uint32)(1 << (area->exploreFlag % 32));
    uint32 currFields = bot->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    return !(currFields & val);
}

std::string const& ExploreTravelDestination::getTitle()
{
    return points[0]->getAreaName();
};

bool GrindTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = bot->GetPlayerbotAI();
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "should get money"))
        return false;

    if (AI_VALUE(bool, "should sell"))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    int32 botLevel = bot->getLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = std::max(botLevel * (0.5f + levelMod), botLevel - 5.0f + levelBoost);

    if ((int32)cInfo->maxlevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = std::max(botLevel * (0.4f + levelMod), botLevel - 12.0f + levelBoost);

    if ((int32)cInfo->maxlevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;

    if (!cInfo->mingold)
        return false;

    if (cInfo->Rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite"))
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction < REP_NEUTRAL;
}

CreatureTemplate const* GrindTravelDestination::GetCreatureTemplate()
{
    return sObjectMgr->GetCreatureTemplate(entry);
}

std::string const& GrindTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "grind mob ";

    out << " " << ChatHelper::formatWorldEntry(entry);

    return out.str();
}

bool BossTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!AI_VALUE(bool, "can fight boss"))
        return false;

    CreatureInfo const* cInfo = getCreatureInfo();

    /*
    int32 botLevel = bot->getLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = botLevel + 3.0;

    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = botLevel - 10;

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;
    */

    if ((int32)cInfo->MaxLevel > bot->getLevel() + 3)
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
    ReputationRank reaction = ai->getReaction(factionEntry);

    if (reaction >= REP_NEUTRAL)
        return false;

    WorldPosition botPos(bot);
    if (!isOut(&botPos))
    {
        list<ObjectGuid> targets = AI_VALUE(list<ObjectGuid>, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == getEntry() && target.IsCreature() && ai->GetCreature(target) && ai->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    if (!AI_VALUE2(bool, "has upgrade", getEntry()))
        return false;

    return true;
}

string BossTravelDestination::getTitle()
{
    ostringstream out;
    out << "boss mob ";
    out << " " << ChatHelper::formatWorldEntry(entry);

    return out.str();
}

TravelTarget::~TravelTarget()
{
    if (!tDestination)
        return;

    releaseVisitors();
    //sTravelMgr->botTargets.erase(std::remove(sTravelMgr->botTargets.begin(), sTravelMgr->botTargets.end(), this), sTravelMgr->botTargets.end());
}

void TravelTarget::setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1)
{
    releaseVisitors();

    wPosition = wPosition1;
    tDestination = tDestination1;
    groupCopy = groupCopy1;
    forced = false;
    radius = 0;

    addVisitors();

    setStatus(TRAVEL_STATUS_TRAVEL);
}

void TravelTarget::copyTarget(TravelTarget* target)
{
    setTarget(target->tDestination, target->wPosition);
    groupCopy = target->isGroupCopy();
    forced = target->forced;
    extendRetryCount = target->extendRetryCount;
}

void TravelTarget::addVisitors()
{
    if (!visitor)
    {
        wPosition->addVisitor();
        tDestination->addVisitor();
    }

    visitor = true;
}

void TravelTarget::releaseVisitors()
{
    if (visitor)
    {
        if (tDestination)
            tDestination->remVisitor();
        if (wPosition)
            wPosition->remVisitor();
    }

    visitor = false;
}

float TravelTarget::distance(Player* bot)
{
    WorldPosition pos(bot);
    return wPosition->distance(&pos);
};

void TravelTarget::setStatus(TravelStatus status)
{
    m_status = status;
    startTime = getMSTime();

    switch (m_status)
    {
        case TRAVEL_STATUS_NONE:
        case TRAVEL_STATUS_PREPARE:
        case TRAVEL_STATUS_EXPIRED:
            statusTime = 1;
            break;
        case TRAVEL_STATUS_TRAVEL:
            statusTime = getMaxTravelTime() * 2 + sPlayerbotAIConfig->maxWaitForMove;
            break;
        case TRAVEL_STATUS_WORK:
            statusTime = tDestination->getExpireDelay();
            break;
        case TRAVEL_STATUS_COOLDOWN:
            statusTime = tDestination->getCooldownDelay();
    }
}

bool TravelTarget::isActive()
{
    if (m_status == TRAVEL_STATUS_NONE || m_status == TRAVEL_STATUS_EXPIRED || m_status == TRAVEL_STATUS_PREPARE)
        return false;

    if (forced && isTraveling())
        return true;

    if ((statusTime > 0 && startTime + statusTime < getMSTime()))
    {
        setStatus(TRAVEL_STATUS_EXPIRED);
        return false;
    }

    if (m_status == TRAVEL_STATUS_COOLDOWN)
        return true;

    if (isTraveling())
        return true;

    if (isWorking())
        return true;

    if (!tDestination->isActive(bot)) //Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return true;
    }

    return true;
};

uint32 TravelTarget::getMaxTravelTime()
{
    return (1000.0 * distance(bot)) / bot->GetSpeed(MOVE_RUN);
}

bool TravelTarget::isTraveling()
{
    if (m_status != TRAVEL_STATUS_TRAVEL)
        return false;

    if (!tDestination->isActive(bot) && !forced) //Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    bool HasArrived = tDestination->isIn(&pos, radius);

    if (HasArrived)
    {
        setStatus(TRAVEL_STATUS_WORK);
        return false;
    }

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isWorking()
{
    if (m_status != TRAVEL_STATUS_WORK)
        return false;

    if (!tDestination->isActive(bot)) //Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    /*
    bool HasLeft = tDestination->isOut(&pos);

    if (HasLeft)
    {
        setStatus(TRAVEL_STATUS_TRAVEL);
        return false;
    }
    */

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isPreparing()
{
    if (m_status != TRAVEL_STATUS_PREPARE)
        return false;

    return true;
}

TravelState TravelTarget::getTravelState()
{
    if (!tDestination || tDestination->getName() == "NullTravelDestination")
        return TRAVEL_STATE_IDLE;

    if (tDestination->getName() == "QuestRelationTravelDestination")
    {
        if (((QuestRelationTravelDestination*)tDestination)->getRelation() == 0)
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_PICK_UP_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_PICK_UP_QUEST;
        }
        else
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_HAND_IN_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_HAND_IN_QUEST;
        }
    }
    else if (tDestination->getName() == "QuestObjectiveTravelDestination")
    {
        if (isTraveling() || isPreparing())
            return TRAVEL_STATE_TRAVEL_DO_QUEST;

        if (isWorking())
            return TRAVEL_STATE_WORK_DO_QUEST;
    }
    else if (tDestination->getName() == "RpgTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_RPG;
    }
    else if (tDestination->getName() == "ExploreTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_EXPLORE;
    }

    return TRAVEL_STATE_IDLE;
}

void TravelMgr::Clear()
{
    std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        TravelMgr::setNullTravelTarget(itr->second);

    for (auto& quest : quests)
    {
        for (auto& dest : quest.second->questGivers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questTakers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questObjectives)
        {
            delete dest;
        }
    }

    questGivers.clear();
    quests.clear();
    pointsMap.clear();
}

void TravelMgr::logQuestError(uint32 errorNr, Quest* quest, uint32 objective, uint32 unitId, uint32 itemId)
{
    bool logQuestErrors = false; //For debugging.

    if (!logQuestErrors)
        return;

    std::string unitName = "<unknown>";
    CreatureTemplate const* cInfo = nullptr;
    GameObjectTemplate const* gInfo = nullptr;

    if (unitId > 0)
        cInfo = sObjectMgr->GetCreatureTemplate(unitId);
    else
        gInfo = sObjectMgr->GetGameObjectTemplate(unitId * -1);

    if (cInfo)
        unitName = cInfo->Name;
    else if (gInfo)
        unitName = gInfo->name;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);

    if (errorNr == 1)
    {
        LOG_INFO("playerbots", "Quest %s [%d] has %s %s [%d] but none is found in the world.",
            quest->GetTitle().c_str(), quest->GetQuestId(), objective == 0 ? "quest giver" : "quest taker", unitName.c_str(), unitId);
    }
    else if (errorNr == 2)
    {
        LOG_ERROR("playerbots", "Quest %s [%d] needs %s [%d] for objective %d but none is found in the world.",
            quest->GetTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, objective);
    }
    else if (errorNr == 3)
    {
        LOG_ERROR("playerbots", "Quest %s [%d] needs itemId %d but no such item exists.", quest->GetTitle().c_str(), quest->GetQuestId(), itemId);
    }
    else if (errorNr == 4)
    {
        LOG_INFO("playerbots", "Quest %s [%d] needs %s [%d] for loot of item %s [%d] for objective %d but none is found in the world.",
            quest->GetTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, proto->Name1.c_str(), itemId, objective);
    }
    else if (errorNr == 5)
    {
        LOG_INFO("playerbots", "Quest %s [%d] needs item %s [%d] for objective %d but none is found in the world.",
            quest->GetTitle().c_str(), quest->GetQuestId(), proto->Name1.c_str(), itemId, objective);
    }
    else if (errorNr == 6)
    {
        LOG_ERROR("playerbots", "Quest %s [%d] has no quest giver.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
    else if (errorNr == 7)
    {
        LOG_ERROR("playerbots", "Quest %s [%d] has no quest taker.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
    else if (errorNr == 8)
    {
        LOG_ERROR("playerbots", "Quest %s [%d] has no quest viable quest objective.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
}

void TravelMgr::SetMobAvoidArea()
{
    sLog.outString("start mob avoidance maps");
    PathFinder path;
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        CreatureData const cData = creaturePair->second;
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

        if (!cInfo)
            continue;

        WorldPosition point = WorldPosition(cData.mapid, cData.posX, cData.posY, cData.posZ, cData.orientation);

        if (cInfo->NpcFlags > 0)
            continue;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, factionEntry);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, factionEntry);

        if (reactionHum >= REP_NEUTRAL || reactionOrc >= REP_NEUTRAL)
            continue;

        if (!point.getTerrain())
            continue;

        point.loadMapAndVMap();

        path.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 11, 50.0f);
        path.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 12, 20.0f);
    }

    sLog.outString("end mob avoidance maps");
}

void TravelMgr::LoadQuestTravelTable()
{
    if (!sTravelMgr->quests.empty())
        return;

    // Clearing store (for reloading case)
    Clear();

    /* remove this
    questGuidMap cQuestMap = GAI_VALUE(questGuidMap,"quest objects");

    for (auto cQuest : cQuestMap)
    {
        LOG_ERROR("playerbots", "[Quest id: %d]", cQuest.first);

        for (auto cObj : cQuest.second)
        {
            LOG_ERROR("playerbots", " [Objective type: %d]", cObj.first);

            for (auto cCre : cObj.second)
            {
                LOG_ERROR("playerbots", " %s %d", cCre.GetTypeName().c_str(), cCre.GetEntry());
            }
        }
    }
    */

    struct unit
    {
        uint64 guid;
        uint32 type;
        uint32 entry;
        uint32 map;
        float  x;
        float  y;
        float  z;
        float  o;
        uint32 c;
    } t_unit;
    std::vector<unit> units;

    /*struct relation
    {
        uint32 type;
        uint32 role;
        uint32 entry;
        uint32 questId;
    } t_rel;
    std::vector<relation> relations;

    struct loot
    {
        uint32 type;
        uint32 entry;
        uint32 item;
    } t_loot;
    std::vector<loot> loots;*/

    ObjectMgr::QuestMap const& questMap = sObjectMgr->GetQuestTemplates();
    std::vector<uint32> questIds;
    unordered_map<uint32, uint32> entryCount;

    for (auto& quest : questMap)
        questIds.push_back(quest.first);

    sort(questIds.begin(), questIds.end());

    sLog.outErrorDb("Loading units locations.");
    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        t_unit.type = 0;
        t_unit.guid = ObjectGuid(HIGHGUID_UNIT, creaturePair->second.id, creaturePair->first).GetRawValue();
        t_unit.entry = creaturePair->second.id;
        t_unit.map = creaturePair->second.mapid;
        t_unit.x = creaturePair->second.posX;
        t_unit.y = creaturePair->second.posY;
        t_unit.z = creaturePair->second.posZ;
        t_unit.o = creaturePair->second.orientation;

        entryCount[creaturePair->second.id]++;

        units.push_back(t_unit);
    }

    for (auto& unit : units)
    {
        unit.c = entryCount[unit.entry];
    }

    sLog.outErrorDb("Loading game object locations.");
    for (auto& goPair : WorldPosition().getGameObjectsNear())
    {
        t_unit.type = 1;
        t_unit.guid = ObjectGuid(HIGHGUID_GAMEOBJECT, goPair->second.id, goPair->first).GetRawValue();
        t_unit.entry = goPair->second.id;
        t_unit.map = goPair->second.mapid;
        t_unit.x = goPair->second.posX;
        t_unit.y = goPair->second.posY;
        t_unit.z = goPair->second.posZ;
        t_unit.o = goPair->second.orientation;
        t_unit.c = 1;

        units.push_back(t_unit);
    }

    /*
    //                          0    1  2   3          4          5          6           7     8
    std::string query = "SELECT 0,guid,id,map,position_x,position_y,position_z,orientation, (SELECT COUNT(*) FROM creature k WHERE c.id = k.id) FROM creature c UNION ALL SELECT 1,guid,id,map,position_x,position_y,position_z,orientation, (SELECT COUNT(*) FROM gameobject h WHERE h.id = g.id)  FROM gameobject g";

    QueryResult result = WorldDatabase.PQuery(query.c_str());
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            t_unit.type = fields[0].GetUInt32();
            t_unit.guid = fields[1].GetUInt32();
            t_unit.entry = fields[2].GetUInt32();
            t_unit.map = fields[3].GetUInt32();
            t_unit.x = fields[4].GetFloat();
            t_unit.y = fields[5].GetFloat();
            t_unit.z = fields[6].GetFloat();
            t_unit.o = fields[7].GetFloat();
            t_unit.c = uint32(fields[8].GetUInt64());

            units.push_back(t_unit);

        } while (result->NextRow());

        LOG_INFO("playerbots", ">> Loaded %zu units locations.", units.size());
    }
    else
    {
        LOG_ERROR("playerbots", ">> Error loading units locations.");
    }

    query = "SELECT 0, 0, id, quest FROM creature_queststarter UNION ALL SELECT 0, 1, id, quest FROM creature_questender UNION ALL SELECT 1, 0, id, quest FROM gameobject_queststarter UNION ALL SELECT 1, 1, id, quest FROM gameobject_questender";
    result = WorldDatabase.PQuery(query.c_str());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            t_rel.type = fields[0].GetUInt32();
            t_rel.role = fields[1].GetUInt32();
            t_rel.entry = fields[2].GetUInt32();
            t_rel.questId = fields[3].GetUInt32();

            relations.push_back(t_rel);

        } while (result->NextRow());

        LOG_INFO("playerbots", ">> Loaded %zu relations.", relations.size());
    }
    else
    {
        LOG_ERROR("playerbots", ">> Error loading relations.");
    }

    query = "SELECT 0, ct.entry, item FROM creature_template ct JOIN creature_loot_template clt ON (ct.lootid = clt.entry) UNION ALL SELECT 0, entry, item FROM npc_vendor UNION ALL SELECT 1, gt.entry, item FROM gameobject_template gt JOIN gameobject_loot_template glt ON (gt.TYPE = 3 AND gt.DATA1 = glt.entry)";
    result = WorldDatabase.PQuery(query.c_str());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            t_loot.type = fields[0].GetUInt32();
            t_loot.entry = fields[1].GetUInt32();
            t_loot.item = fields[2].GetUInt32();

            loots.push_back(t_loot);

        } while (result->NextRow());

        LOG_INFO("playerbots", ">> Loaded %zu loot lists.", loots.size());
    }
    else
    {
        LOG_ERROR("playerbots", ">> Error loading loot lists.");
    }
    */

    sLog.outErrorDb("Loading quest data.");

    bool loadQuestData = true;
    if (loadQuestData)
    {
        questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

        for (auto& q : questMap)
        {
            uint32 questId = q.first;

            QuestContainer* container = new QuestContainer;

            for (auto& r : q.second)
            {
                uint32 flag = r.first;

                for (auto& e : r.second)
                {
                    int32 entry = e.first;

                    QuestTravelDestination* loc;
                    vector<QuestTravelDestination*> locs;

                    if (flag & (uint32)QuestRelationFlag::questGiver)
                    {
                        loc = new QuestRelationTravelDestination(questId, entry, 0, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questGivers.push_back(loc);
                        locs.push_back(loc);
                    }
                    if (flag & (uint32)QuestRelationFlag::questTaker)
                    {
                        loc = new QuestRelationTravelDestination(questId, entry, 1, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questTakers.push_back(loc);
                        locs.push_back(loc);
                    }
                    else
                    {
                        uint32 objective;
                        if (flag & (uint32)QuestRelationFlag::objective1)
                            objective = 0;
                        else if (flag & (uint32)QuestRelationFlag::objective2)
                            objective = 1;
                        else if (flag & (uint32)QuestRelationFlag::objective3)
                            objective = 2;
                        else if (flag & (uint32)QuestRelationFlag::objective4)
                            objective = 3;

                        loc = new QuestObjectiveTravelDestination(questId, entry, objective, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                        loc->setExpireDelay(1 * 60 * 1000);
                        loc->setMaxVisitors(100, 1);
                        container->questObjectives.push_back(loc);
                        locs.push_back(loc);
                    }

                    for (auto& guidP : e.second)
                    {
                        WorldPosition point = guidP;
                        pointsMap.insert(make_pair(guidP.GetRawValue(), point));

                        for (auto tLoc : locs)
                        {
                            tLoc->addPoint(&pointsMap.find(guidP.GetRawValue())->second);
                        }
                    }
                }
            }

            if (!container->questTakers.empty())
            {
                quests.insert(make_pair(questId, container));

                for (auto loc : container->questGivers)
                    questGivers.push_back(loc);
            }
        }
    }

    /*
    if (loadQuestData && false)
    {
        for (auto& questId : questIds)
        {
            Quest* quest = questMap.find(questId)->second;

            QuestContainer* container = new QuestContainer;
            QuestTravelDestination* loc = nullptr;
            WorldPosition point;

            bool hasError = false;

            //Relations
            for (auto& r : relations)
            {
                if (questId != r.questId)
                    continue;

                int32 entry = r.type == 0 ? r.entry : r.entry * -1;

                loc = new QuestRelationTravelDestination(r.questId, entry, r.role, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
                loc->setExpireDelay(5 * 60 * 1000);
                loc->setMaxVisitors(15, 0);

                for (auto& u : units)
                {
                    if (r.type != u.type || r.entry != u.entry)
                        continue;

                    int32 guid = u.type == 0 ? u.guid : u.guid * -1;

                    point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
                    pointsMap.insert(std::make_pair(guid, point));

                    loc->addPoint(&pointsMap.find(guid)->second);
                }

                if (loc->getPoints(0).empty())
                {
                    logQuestError(1, quest, r.role, entry);
                    delete loc;
                    continue;
                }


                if (r.role == 0)
                {
                    container->questGivers.push_back(loc);
                }
                else
                    container->questTakers.push_back(loc);

            }

            //Mobs
            for (uint32 i = 0; i < 4; i++)
            {
                if (quest->RequiredNpcOrGoCount[i] == 0)
                    continue;

                uint32 reqEntry = quest->RequiredNpcOrGo[i];

                loc = new QuestObjectiveTravelDestination(questId, reqEntry, i, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
                loc->setExpireDelay(1 * 60 * 1000);
                loc->setMaxVisitors(100, 1);

                for (auto& u : units)
                {
                    int32 entry = u.type == 0 ? u.entry : u.entry * -1;

                    if (entry != reqEntry)
                        continue;

                    int32 guid = u.type == 0 ? u.guid : u.guid * -1;

                    point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
                    pointsMap.insert(std::make_pair(u.guid, point));

                    loc->addPoint(&pointsMap.find(u.guid)->second);
                }

                if (loc->getPoints(0).empty())
                {
                    logQuestError(2, quest, i, reqEntry);

                    delete loc;
                    hasError = true;
                    continue;
                }

                container->questObjectives.push_back(loc);
            }

            //Loot
            for (uint32 i = 0; i < 4; i++)
            {
                if (quest->RequiredItemCount[i] == 0)
                    continue;

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(quest->RequiredItemId[i]);
                if (!proto)
                {
                    logQuestError(3, quest, i, 0, quest->RequiredItemId[i]);
                    hasError = true;
                    continue;
                }

                uint32 foundLoot = 0;

                for (auto& l : loots)
                {
                    if (l.item != quest->RequiredItemId[i])
                        continue;

                    int32 entry = l.type == 0 ? l.entry : l.entry * -1;

                    loc = new QuestObjectiveTravelDestination(questId, entry, i, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance, l.item);
                    loc->setExpireDelay(1 * 60 * 1000);
                    loc->setMaxVisitors(100, 1);

                    for (auto& u : units)
                    {
                        if (l.type != u.type || l.entry != u.entry)
                            continue;

                        int32 guid = u.type == 0 ? u.guid : u.guid * -1;

                        point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
                        pointsMap.insert(std::make_pair(guid, point));

                        loc->addPoint(&pointsMap.find(guid)->second);
                    }

                    if (loc->getPoints(0).empty())
                    {
                        logQuestError(4, quest, i, entry, quest->RequiredItemId[i]);
                        delete loc;
                        continue;
                    }

                    container->questObjectives.push_back(loc);

                    foundLoot++;
                }

                if (foundLoot == 0)
                {
                    hasError = true;
                    logQuestError(5, quest, i, 0, quest->RequiredItemId[i]);
                }
            }

            if (container->questTakers.empty())
                logQuestError(7, quest);

            if (!container->questGivers.empty() || !container->questTakers.empty() || hasError)
            {
                quests.insert(std::make_pair(questId, container));

                for (auto loc : container->questGivers)
                    questGivers.push_back(loc);
            }
        }

        LOG_INFO("playerbots", ">> Loaded %zu quest details.", questIds.size());
    }
    */

    WorldPosition point;

    sLog.outErrorDb("Loading Rpg, Grind and Boss locations.");

    //Rpg locations
    for (auto& u : units)
    {
        RpgTravelDestination* rLoc;
        GrindTravelDestination* gLoc;
        BossTravelDestination* bLoc;

        if (u.type != 0)
            continue;

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
        if (!cInfo)
            continue;

        if (cInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        std::vector<uint32> allowedNpcFlags;

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

        point = WorldPosition(u.map, u.x, u.y, u.z, u.o);

        for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
        {
            if ((cInfo->npcflag & *i) != 0)
            {
                rLoc = new RpgTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
                rLoc->setExpireDelay(5 * 60 * 1000);
                rLoc->setMaxVisitors(15, 0);

                pointsMap.insert_or_assign(u.guid, point);
                rLoc->addPoint(&pointsMap.find(u.guid)->second);
                rpgNpcs.push_back(rLoc);
                break;
            }
        }

        if (cInfo->mingold > 0)
        {
            gLoc = new GrindTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
            gLoc->setExpireDelay(5 * 60 * 1000);
            gLoc->setMaxVisitors(100, 0);

            point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            pointsMap.insert_or_assign(u.guid, point);
            gLoc->addPoint(&pointsMap.find(u.guid)->second);
            grindMobs.push_back(gLoc);
        }

        if (cInfo->Rank == 3 || (cInfo->Rank == 1 && !point.isOverworld() && u.c == 1))
        {
            string nodeName = cInfo->Name;

            bLoc = new BossTravelDestination(u.entry, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
            bLoc->setExpireDelay(5 * 60 * 1000);
            bLoc->setMaxVisitors(0, 0);

            pointsMap.insert_or_assign(u.guid, point);
            bLoc->addPoint(&pointsMap.find(u.guid)->second);
            bossMobs.push_back(bLoc);
        }
    }

    sLog.outErrorDb("Loading Explore locations.");

    //Explore points
    for (auto& u : units)
    {
        ExploreTravelDestination* loc;

        WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
        AreaTableEntry const* area = point.getArea();

        if (!area)
            continue;

        if (!area->exploreFlag)
            continue;

        if (u.type == 1)
            continue;

        auto iloc = exploreLocs.find(area->ID);

        int32 guid = u.type == 0 ? u.guid : u.guid * -1;

        pointsMap.insert_or_assign(guid, point);

        if (iloc == exploreLocs.end())
        {
            loc = new ExploreTravelDestination(area->ID, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
            loc->setMaxVisitors(1000, 0);
            loc->setCooldownDelay(1000);
            loc->setExpireDelay(1000);
            exploreLocs.insert_or_assign(area->ID, loc);
        }
        else
        {
            loc = iloc->second;
        }

        loc->addPoint(&pointsMap.find(guid)->second);
    }

    //Clear these logs files
    sPlayerbotAIConfig.openLog("zones.csv", "w");
    sPlayerbotAIConfig.openLog("creatures.csv", "w");
    sPlayerbotAIConfig.openLog("gos.csv", "w");
    sPlayerbotAIConfig.openLog("bot_movement.csv", "w");
    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_attempt.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_attempt_point.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_result.csv", "w");
    sPlayerbotAIConfig.openLog("load_map_grid.csv", "w");
    sPlayerbotAIConfig.openLog("strategy.csv", "w");

    sPlayerbotAIConfig.openLog("unload_grid.csv", "w");
    sPlayerbotAIConfig.openLog("unload_obj.csv", "w");

#ifdef IKE_PATHFINDER
    bool mmapAvoidMobMod = true;

    if (mmapAvoidMobMod)
    {
        //Mob avoidance
        SetMobAvoidArea();
    }
#endif

    sTravelNodeMap.loadNodeStore();

    sTravelNodeMap.generateAll();

    /*
    bool fullNavPointReload = false;
    bool storeNavPointReload = true;

   if (!fullNavPointReload && true)
        TravelNodeStore::loadNodes();

    //sTravelNodeMap.loadNodeStore();

    for (auto node : sTravelNodeMap->getNodes())
    {
        node->setLinked(true);
    }

    bool reloadNavigationPoints = false || fullNavPointReload || storeNavPointReload;

    if (reloadNavigationPoints)
    {
        LOG_INFO("playerbots", "Loading navigation points");

        //Npc nodes

        WorldPosition pos;

        for (auto& u : units)
        {
            if (u.type != 0)
                continue;

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
            if (!cInfo)
                continue;

            std::vector<uint32> allowedNpcFlags;

            allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
            allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
            //allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);

            for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
            {
                if ((cInfo->npcflag & *i) != 0)
                {
                    pos = WorldPosition(u.map, u.x, u.y, u.z, u.o);

                    std::string nodeName = pos.getAreaName(false);
                    if ((cInfo->npcflag & UNIT_NPC_FLAG_INNKEEPER) != 0)
                        nodeName += " innkeeper";
                    else
                        nodeName += " flightMaster";

                    sTravelNodeMap->addNode(&pos, nodeName, true, true);

                    break;
                }
            }
        }

        //Build flight paths
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);

            if (!taxiPath)
                continue;

            TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
            if (!startTaxiNode)
                continue;

            TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);
            if (!endTaxiNode)
                continue;

            TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[taxiPath->ID];
            if (nodes.empty())
                continue;

            WorldPosition startPos(startTaxiNode->map_id, startTaxiNode->x, startTaxiNode->y, startTaxiNode->z);
            WorldPosition endPos(endTaxiNode->map_id, endTaxiNode->x, endTaxiNode->y, endTaxiNode->z);

            TravelNode* startNode = sTravelNodeMap.getNode(&startPos, nullptr, 15.0f);
            TravelNode* endNode = sTravelNodeMap.getNode(&endPos, nullptr, 15.0f);

            if (!startNode || !endNode)
                continue;

            vector<WorldPosition> ppath;

            for (auto& n : nodes)
                ppath.push_back(WorldPosition(n->mapid, n->x, n->y, n->z, 0.0));

            float totalTime = startPos.getPathLength(ppath) / (450 * 8.0f);

            TravelNodePath travelPath(0.1f, totalTime, (uint8) TravelNodePathType::flightPath, i, true);
            travelPath.setPath(ppath);

            startNode->setPathTo(endNode, travelPath);
        }

        //Unique bosses
        for (auto& u : units)
        {
            if (u.type != 0)
                continue;

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
            if (!cInfo)
                continue;

            pos = WorldPosition(u.map, u.x, u.y, u.z, u.o);

            if (cInfo->rank == 3 || (cInfo->rank == 1 && !pos.isOverworld() && u.c == 1))
            {
                std::string nodeName = cInfo->Name;
                sTravelNodeMap->addNode(&pos, nodeName, true, true);
            }
        }

        std::map<uint8, std::string> startNames;
        startNames[RACE_HUMAN] = "Human";
        startNames[RACE_ORC] = "Orc and Troll";
        startNames[RACE_DWARF] = "Dwarf and Gnome";
        startNames[RACE_NIGHTELF] = "Night Elf";
        startNames[RACE_UNDEAD_PLAYER] = "Undead";
        startNames[RACE_TAUREN] = "Tauren";
        startNames[RACE_GNOME] = "Dwarf and Gnome";
        startNames[RACE_TROLL] = "Orc and Troll";
        startNames[RACE_DRAENEI] = "Draenei";
        startNames[RACE_BLOODELF] = "Blood Elf";

        for (uint32 i = 0; i < MAX_RACES; i++)
        {
            for (uint32 j = 0; j < MAX_CLASSES; j++)
            {
                PlayerInfo const* info = sObjectMgr->GetPlayerInfo(i, j);
                if (!info)
                    continue;

                pos = WorldPosition(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

                std::string nodeName = startNames[i] + " start";
                sTravelNodeMap->addNode(&pos, nodeName, true, true);
            }
        }

        //Entrance nodes

        for (int i = 0; i < 6000; i++)
        {
            AreaTrigger const* at = sObjectMgr->GetAreaTrigger(i);
            if (!at)
                continue;

            AreaTriggerTeleport const* atEntry = sObjectMgr->GetAreaTriggerTeleport(i);
            if (!atEntry)
                continue;

            WorldPosition inPos = WorldPosition(at->map, at->x, at->y, at->z - 4.0f, 0);

            WorldPosition outPos = WorldPosition(atEntry->target_mapId, atEntry->target_X, atEntry->target_Y, atEntry->target_Z, atEntry->target_Orientation);

            std::string nodeName;

            if (!outPos.isOverworld())
                nodeName = outPos.getAreaName(false) + " entrance";
            else if (!inPos.isOverworld())
                nodeName = inPos.getAreaName(false) + " exit";
            else
                nodeName = inPos.getAreaName(false) + " portal";

            sTravelNodeMap->addNode(&inPos, nodeName, true, true);
        }

        //Exit nodes

        for (int i = 0; i < 6000; i++)
        {
            AreaTrigger const* at = sObjectMgr->GetAreaTrigger(i);
            if (!at)
                continue;

            AreaTriggerTeleport const* atEntry = sObjectMgr->GetAreaTriggerTeleport(i);
            if (!atEntry)
                continue;

            WorldPosition inPos = WorldPosition(at->map, at->x, at->y, at->z - 4.0f, 0);

            WorldPosition outPos = WorldPosition(atEntry->target_mapId, atEntry->target_X, atEntry->target_Y, atEntry->target_Z, atEntry->target_Orientation);

            std::string nodeName;

            if (!outPos.isOverworld())
                nodeName = outPos.getAreaName(false) + " entrance";
            else if (!inPos.isOverworld())
                nodeName = inPos.getAreaName(false) + " exit";
            else
                nodeName = inPos.getAreaName(false) + " portal";

            TravelNode* entryNode = sTravelNodeMap->getNode(&outPos, nullptr, 20.0f); //Entry side, portal exit.

            TravelNode* outNode = sTravelNodeMap->addNode(&outPos, nodeName, true, true); //Exit size, portal exit.

            TravelNode* inNode = sTravelNodeMap->getNode(&inPos, nullptr, 5.0f); //Entry side, portal center.

            //Portal link from area trigger to area trigger destination.
            if (outNode && inNode)
            {
                TravelNodePath travelPath(0.1f, 3.0f, (uint8) TravelNodePathType::portal, i, true);
                travelPath.setPath({*inNode->getPosition(), *outNode->getPosition()});
                inNode->setPathTo(outNode, travelPath);
            }
        }

        //Transports
        GameObjectTemplateContainer const* goTemplates = sObjectMgr->GetGameObjectTemplates();
        for (auto const& iter : *goTemplates)
        {
            GameObjectTemplate const* data = &iter.second;
            if (data && (data->type == GAMEOBJECT_TYPE_TRANSPORT || data->type == GAMEOBJECT_TYPE_MO_TRANSPORT))
            {
                TransportAnimation const* animation = sTransportMgr->GetTransportAnimInfo(iter.first);

                uint32 pathId = data->moTransport.taxiPathId;
                float moveSpeed = data->moTransport.moveSpeed;
                if (pathId >= sTaxiPathNodesByPath.size())
                    continue;

                TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

                std::vector<WorldPosition> ppath;
                TravelNode* prevNode = nullptr;

                //Elevators/Trams
                if (path.empty())
                {
                    if (animation)
                    {
                        TransportPathContainer aPath = animation->Path;
                        float timeStart;

                        for (auto& u : units)
                        {
                            if (u.type != 1)
                                continue;

                            if (u.entry != iter.first)
                                continue;

                            prevNode = nullptr;
                            WorldPosition lPos = WorldPosition(u.map, 0, 0, 0, 0);

                            for (auto& p : aPath)
                            {
                                float dx = cos(u.o) * p.second->X - sin(u.o) * p.second->Y;
                                float dy = sin(u.o) * p.second->X + cos(u.o) * p.second->Y;
                                WorldPosition pos = WorldPosition(u.map, u.x + dx, u.y + dy, u.z + p.second->Z, u.o);

                                if (prevNode)
                                {
                                    ppath.push_back(pos);
                                }

                                if (pos.distance(&lPos) == 0)
                                {
                                    TravelNode* node = sTravelNodeMap->addNode(&pos, data->name, true, true, true, iter.first);

                                    if (!prevNode)
                                    {
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }
                                    else
                                    {
                                        float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;
                                        TravelNodePath travelPath(0.1f, totalTime, (uint8) TravelNodePathType::transport, entry, true);
                                        node->setPathTo(prevNode, travelPath);
                                        ppath.clear();
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }

                                    prevNode = node;
                                }

                                lPos = pos;
                            }

                            if (prevNode)
                            {
                                for (auto& p : aPath)
                                {
                                    float dx = cos(u.o) * p.second->X - sin(u.o) * p.second->Y;
                                    float dy = sin(u.o) * p.second->X + cos(u.o) * p.second->Y;
                                    WorldPosition pos = WorldPosition(u.map, u.x + dx, u.y + dy, u.z + p.second->Z, u.o);

                                    ppath.push_back(pos);

                                    if (pos.distance(&lPos) == 0)
                                    {
                                        TravelNode* node = sTravelNodeMap->addNode(&pos, data->name, true, true, true, iter.first);
                                        if (node != prevNode)
                                        {
                                            float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;
                                            TravelNodePath travelPath(0.1f, totalTime, (uint8) TravelNodePathType::transport, entry, true);
                                            travelPath.setPath(ppath);
                                            node->setPathTo(prevNode, travelPath);
                                            ppath.clear();
                                            ppath.push_back(pos);
                                            timeStart = p.second->TimeSeg;
                                        }
                                    }

                                    lPos = pos;
                                }
                            }

                            ppath.clear();
                        }
                    }
                }
                else //Boats/Zepelins
                {
                    //Loop over the path and connect stop locations.
                    for (auto& p : path)
                    {
                        WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                        //if (data->displayId == 3015)
                        //    pos.setZ(pos.getZ() + 6.0f);
                        //else if(data->displayId == 3031)
                       //     pos.setZ(pos.getZ() - 17.0f);

                        if (prevNode)
                        {
                            ppath.push_back(pos);
                        }

                        if (p->delay > 0)
                        {
                            TravelNode* node = sTravelNodeMap->addNode(&pos, data->name, true, true, true, iter.first);

                            if (!prevNode)
                            {
                                ppath.push_back(pos);
                            }
                            else
                            {
                                TravelNodePath travelPath(0.1f, 0.0, (uint8) TravelNodePathType::transport, entry, true);
                                travelPath.setPathAndCost(ppath, moveSpeed);
                                node->setPathTo(prevNode, travelPath);
                                ppath.clear();
                                ppath.push_back(pos);
                            }

                            prevNode = node;
                        }
                    }

                    if (prevNode)
                    {
                        //Continue from start until first stop and connect to end.
                        for (auto& p : path)
                        {
                            WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                            //if (data->displayId == 3015)
                            //    pos.setZ(pos.getZ() + 6.0f);
                            //else if (data->displayId == 3031)
                            //    pos.setZ(pos.getZ() - 17.0f);

                            ppath.push_back(pos);

                            if (p->delay > 0)
                            {
                                TravelNode* node = sTravelNodeMap->getNode(&pos, nullptr, 5.0f);
                                if (node != prevNode)
                                {
                                    TravelNodePath travelPath(0.1f, 0.0, (uint8) TravelNodePathType::transport, entry, true);
                                    travelPath.setPathAndCost(ppath, moveSpeed);
                                    node->setPathTo(prevNode, travelPath);
                                }
                            }
                        }
                    }

                    ppath.clear();
                }
            }
        }

        //Zone means
        for (auto& loc : exploreLocs)
        {
            std::vector<WorldPosition*> points;

            for (auto p : loc.second->getPoints(true))
                if (!p->isUnderWater())
                    points.push_back(p);

            if (points.empty())
                points = loc.second->getPoints(true);

            WorldPosition  pos = WorldPosition(points, WP_MEAN_CENTROID);

            TravelNode* node = sTravelNodeMap->addNode(&pos, pos.getAreaName(), true, true, false);
        }

        LOG_INFO("playerbots", ">> Loaded %zu navigation points.", sTravelNodeMap->getNodes().size());
    }

    sTravelNodeMap->calcMapOffset();
    loadMapTransfers();
    */

    /*
    bool preloadNodePaths = false || fullNavPointReload || storeNavPointReload;             //Calculate paths using PathGenerator.
    bool preloadReLinkFullyLinked = false || fullNavPointReload || storeNavPointReload;      //Retry nodes that are fully linked.
    bool preloadUnlinkedPaths = false || fullNavPointReload;        //Try to connect points currently unlinked.
    bool preloadWorldPaths = true;            //Try to load paths in overworld.
    bool preloadInstancePaths = true;         //Try to load paths in instances.
    bool preloadSubPrint = false;             //Print output every 2%.

    if (preloadNodePaths)
    {
        std::unordered_map<uint32, Map*> instances;

        //PathGenerator
        std::vector<WorldPosition> ppath;

        uint32 cur = 0, max = sTravelNodeMap->getNodes().size();

        for (auto& startNode : sTravelNodeMap->getNodes())
        {
            if (!preloadReLinkFullyLinked && startNode->isLinked())
                continue;

            for (auto& endNode : sTravelNodeMap->getNodes())
            {
                if (startNode == endNode)
                    continue;

                if (startNode->getPosition()->isOverworld() && !preloadWorldPaths)
                    continue;

                if (!startNode->getPosition()->isOverworld() && !preloadInstancePaths)
                    continue;

                if (startNode->hasCompletePathTo(endNode))
                    continue;

                if (!preloadUnlinkedPaths && !startNode->hasLinkTo(endNode))
                    continue;

                if (startNode->getMapId() != endNode->getMapId())
                    continue;

                //if (preloadUnlinkedPaths && !startNode->hasLinkTo(endNode) && startNode->isUselessLink(endNode))
                //    continue;

                startNode->buildPath(endNode, nullptr, false);

                //if (startNode->hasLinkTo(endNode) && !startNode->getPathTo(endNode)->getComplete())
                    //startNode->removeLinkTo(endNode);
            }

            startNode->setLinked(true);

            cur++;

            if (preloadSubPrint && (cur * 50) / max > ((cur - 1) * 50) / max)
            {
                sTravelNodeMap->printMap();
                sTravelNodeMap->printNodeStore();
            }
        }

        if (!preloadSubPrint)
        {
            sTravelNodeMap->printNodeStore();
            sTravelNodeMap->printMap();
        }

        LOG_INFO("playerbots", ">> Loaded paths for %zu nodes.", sTravelNodeMap->getNodes().size());
    }

    bool removeLowLinkNodes = false || fullNavPointReload || storeNavPointReload;

    if (removeLowLinkNodes)
    {
        std::vector<TravelNode*> goodNodes;
        std::vector<TravelNode*> remNodes;
        for (auto& node : sTravelNodeMap->getNodes())
        {
            if (!node->getPosition()->isOverworld())
                continue;

            if (std::find(goodNodes.begin(), goodNodes.end(), node) != goodNodes.end())
                continue;

            if (std::find(remNodes.begin(), remNodes.end(), node) != remNodes.end())
                continue;

            std::vector<TravelNode*> nodes = node->getNodeMap(true);

            if (nodes.size() < 5)
                remNodes.insert(remNodes.end(), nodes.begin(), nodes.end());
            else
                goodNodes.insert(goodNodes.end(), nodes.begin(), nodes.end());
        }

        for (auto& node : remNodes)
            sTravelNodeMap->removeNode(node);

        LOG_INFO("playerbots", ">> Checked %zu nodes.", sTravelNodeMap->getNodes().size());
    }

    bool cleanUpNodeLinks = false || fullNavPointReload || storeNavPointReload;
    bool cleanUpSubPrint = false;             //Print output every 2%.

    if (cleanUpNodeLinks)
    {
        //Routes
        uint32 cur = 0;
        uint32 max = sTravelNodeMap->getNodes().size();

        //Clean up node links
        for (auto& startNode : sTravelNodeMap->getNodes())
        {
             startNode->cropUselessLinks();

             cur++;
             if (cleanUpSubPrint && (cur * 10) / max > ((cur - 1) * 10) / max)
             {
                 sTravelNodeMap->printMap();
                 sTravelNodeMap->printNodeStore();
             }
        }

        LOG_INFO("playerbots", ">> Cleaned paths for %zu nodes.", sTravelNodeMap->getNodes().size());
    }

    bool reCalculateCost = false || fullNavPointReload || storeNavPointReload;
    bool forceReCalculate = false;

    if (reCalculateCost)
    {
        for (auto& startNode : sTravelNodeMap->getNodes())
        {
            for (auto& path : *startNode->getLinks())
            {
                TravelNodePath* nodePath = path.second;

                if (path.second->getPathType() != TravelNodePathType::walk)
                    continue;

                if (nodePath->getCalculated() && !forceReCalculate)
                    continue;

                nodePath->calculateCost();
            }
        }

        LOG_INFO("playerbots", ">> Calculated pathcost for %zu nodes.", sTravelNodeMap->getNodes().size());
    }

    bool mirrorMissingPaths = true || fullNavPointReload || storeNavPointReload;

    if (mirrorMissingPaths)
    {
        for (auto& startNode : sTravelNodeMap.getNodes())
        {
            for (auto& path : *startNode->getLinks())
            {
                TravelNode* endNode = path.first;

                if (endNode->hasLinkTo(startNode))
                    continue;

                if (path.second->getPathType() != TravelNodePathType::walk)
                    continue;

                TravelNodePath nodePath = *path.second;

                vector<WorldPosition> pPath = nodePath.getPath();
                std::reverse(pPath.begin(), pPath.end());

                nodePath.setPath(pPath);

                endNode->setPathTo(startNode, nodePath, true);
            }
        }

        sLog.outString(">> Reversed missing paths for " SIZEFMTD " nodes.", sTravelNodeMap.getNodes().size());
    }
    */

    sTravelNodeMap->printMap();
    sTravelNodeMap->printNodeStore();
    sTravelNodeMap.saveNodeStore();

    //Creature/gos/zone export.
    if (sPlayerbotAIConfig->hasLog("creatures.csv"))
    {
        for (CreatureData const* cData : WorldPosition().getCreaturesNear())
        {
            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cData->id);
            if (!cInfo)
                continue;

            WorldPosition point = WorldPosition(cData->mapid, cData->posX, cData->posY, cData->posZ, cData->orientation);

            std::string name = cInfo->Name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << cInfo->maxlevel << ",";
            out << cInfo->rank << ",";
            out << cInfo->faction << ",";
            out << cInfo->npcflag << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig->log("creatures.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig.hasLog("vmangoslines.csv"))
    {
        uint32 mapId = 0;
        vector<WorldPosition> pos;

        static float const topNorthSouthLimit[] =
        {
            2032.048340f, -6927.750000f,
            1634.863403f, -6157.505371f,
            1109.519775f, -5181.036133f,
            1315.204712f, -4096.020508f,
            1073.089233f, -3372.571533f,
            825.8331910f, -3125.778809f,
            657.3439940f, -2314.813232f,
            424.7361450f, -1888.283691f,
            744.3958130f, -1647.935425f,
            1424.160645f, -654.9481810f,
            1447.065308f, -169.7513580f,
            1208.715454f, 189.74870300f,
            1596.240356f, 998.61669900f,
            1577.923706f, 1293.4199220f,
            1458.520264f, 1727.3732910f,
            1591.916138f, 3728.1394040f
        };

        pos.clear();

# define my_sizeof(type) ((char *)(&type+1)-(char*)(&type))

        int size = my_sizeof(topNorthSouthLimit) / my_sizeof(topNorthSouthLimit[0]);
        for (uint32 i = 0; i < size-1; i=i+2)
        {
            if (topNorthSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, topNorthSouthLimit[i], topNorthSouthLimit[i + 1], 0));
        }

        ostringstream out;
        out << "topNorthSouthLimit" << ",";
        WorldPosition().printWKT(pos,out,1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const ironforgeAreaSouthLimit[] =
        {
            -7491.33f, 3093.740f,
            -7472.04f, -391.880f,
            -6366.68f, -730.100f,
            -6063.96f, -1411.76f,
            -6087.62f, -2190.21f,
            -6349.54f, -2533.66f,
            -6308.63f, -3049.32f,
            -6107.82f, -3345.30f,
            -6008.49f, -3590.52f,
            -5989.37f, -4312.29f,
            -5806.26f, -5864.11f
        };

        pos.clear();

        size = my_sizeof(ironforgeAreaSouthLimit) / my_sizeof(ironforgeAreaSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (ironforgeAreaSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, ironforgeAreaSouthLimit[i], ironforgeAreaSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "ironforgeAreaSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const stormwindAreaNorthLimit[] =
        {
            -8004.250f, 3714.110f,
            -8075.000f, -179.000f,
            -8638.000f, 169.0000f,
            -9044.000f, 35.00000f,
            -9068.000f, -125.000f,
            -9094.000f, -147.000f,
            -9206.000f, -290.000f,
            -9097.000f, -510.000f,
            -8739.000f, -501.000f,
            -8725.500f, -1618.45f,
            -9810.400f, -1698.41f,
            -10049.60f, -1740.40f,
            -10670.61f, -1692.51f,
            -10908.48f, -1563.87f,
            -13006.40f, -1622.80f,
            -12863.23f, -4798.42f
        };

        pos.clear();

        size = my_sizeof(stormwindAreaNorthLimit) / my_sizeof(stormwindAreaNorthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (stormwindAreaNorthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, stormwindAreaNorthLimit[i], stormwindAreaNorthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "stormwindAreaNorthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const stormwindAreaSouthLimit[] =
        {
            -8725.3378910f, 3535.62402300f,
            -9525.6992190f, 910.13256800f,
            -9796.9531250f, 839.06958000f,
            -9946.3417970f, 743.10284400f,
            -10287.361328f, 760.07647700f,
            -10083.828125f, 380.38989300f,
            -10148.072266f, 80.056450000f,
            -10014.583984f, -161.6385190f,
            -9978.1464840f, -361.6380310f,
            -9877.4892580f, -563.3048710f,
            -9980.9677730f, -1128.510498f,
            -9991.7177730f, -1428.793213f,
            -9887.5791020f, -1618.514038f,
            -10169.600586f, -1801.582031f,
            -9966.2744140f, -2227.197754f,
            -9861.3095700f, -2989.841064f,
            -9944.0263670f, -3205.886963f,
            -9610.2099610f, -3648.369385f,
            -7949.3295900f, -4081.389404f,
            -7910.8593750f, -5855.578125f
        };

        pos.clear();

        size = my_sizeof(stormwindAreaSouthLimit) / my_sizeof(stormwindAreaSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (stormwindAreaSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, stormwindAreaSouthLimit[i], stormwindAreaSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "stormwindAreaSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        mapId = 1;

        static float const northMiddleLimit[] =
        {
            -2280.00f, 4054.000f,
            -2401.00f, 2365.000f,
            -2432.00f, 1338.000f,
            -2286.00f, 769.0000f,
            -2137.00f, 662.0000f,
            -2044.54f, 489.8600f,
            -1808.52f, 436.3900f,
            -1754.85f, 504.5500f,
            -1094.55f, 651.7500f,
            -747.460f, 647.7300f,
            -685.550f, 408.4300f,
            -311.380f, 114.4300f,
            -358.400f, -587.420f,
            -377.920f, -748.700f,
            -512.570f, -919.490f,
            -280.650f, -1008.87f,
            -81.2900f, -930.890f,
            284.3100f, -1105.39f,
            568.8600f, -892.280f,
            1211.090f, -1135.55f,
            879.6000f, -2110.18f,
            788.9600f, -2276.02f,
            899.6800f, -2625.56f,
            1281.540f, -2689.42f,
            1521.820f, -3047.85f,
            1424.220f, -3365.69f,
            1694.110f, -3615.20f,
            2373.780f, -4019.96f,
            2388.130f, -5124.35f,
            2193.790f, -5484.38f,
            1703.570f, -5510.53f,
            1497.590f, -6376.56f,
            1368.000f, -8530.00f
        };

        pos.clear();

        size = my_sizeof(northMiddleLimit) / my_sizeof(northMiddleLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (northMiddleLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, northMiddleLimit[i], northMiddleLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "northMiddleLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const durotarSouthLimit[] =
        {
            2755.0f, -3766.f,
            2225.0f, -3596.f,
            1762.0f, -3746.f,
            1564.0f, -3943.f,
            1184.0f, -3915.f,
            737.00f, -3782.f,
            -75.00f, -3742.f,
            -263.0f, -3836.f,
            -173.0f, -4064.f,
            -81.00f, -4091.f,
            -49.00f, -4089.f,
            -16.00f, -4187.f,
            -5.000f, -4192.f,
            -14.00f, -4551.f,
            -397.0f, -4601.f,
            -522.0f, -4583.f,
            -668.0f, -4539.f,
            -790.0f, -4502.f,
            -1176.f, -4213.f,
            -1387.f, -4674.f,
            -2243.f, -6046.f
        };

        pos.clear();

        size = my_sizeof(durotarSouthLimit) / my_sizeof(durotarSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (durotarSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, durotarSouthLimit[i], durotarSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "durotarSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const valleyoftrialsSouthLimit[] =
        {
            -324.f, -3869.f,
            -774.f, -3992.f,
            -965.f, -4290.f,
            -932.f, -4349.f,
            -828.f, -4414.f,
            -661.f, -4541.f,
            -521.f, -4582.f
        };

        pos.clear();

        size = my_sizeof(valleyoftrialsSouthLimit) / my_sizeof(valleyoftrialsSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (valleyoftrialsSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, valleyoftrialsSouthLimit[i], valleyoftrialsSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "valleyoftrialsSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const middleToSouthLimit[] =
        {
            -2402.010000f, 4255.7000000f,
            -2475.933105f, 3199.5683590f, // Desolace
            -2344.124023f, 1756.1643070f,
            -2826.438965f, 403.82473800f, // Mulgore
            -3472.819580f, 182.52247600f, // Feralas
            -4365.006836f, -1602.575439f, // the Barrens
            -4515.219727f, -1681.356079f,
            -4543.093750f, -1882.869385f, // Thousand Needles
            -4824.160000f, -2310.110000f,
            -5102.913574f, -2647.062744f,
            -5248.286621f, -3034.536377f,
            -5246.920898f, -3339.139893f,
            -5459.449707f, -4920.155273f, // Tanaris
            -5437.000000f, -5863.000000f
        };

        pos.clear();

        size = my_sizeof(middleToSouthLimit) / my_sizeof(middleToSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (middleToSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, middleToSouthLimit[i], middleToSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "middleToSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const orgrimmarSouthLimit[] =
        {
            2132.5076f, -3912.2478f,
            1944.4298f, -3855.2583f,
            1735.6906f, -3834.2417f,
            1654.3671f, -3380.9902f,
            1593.9861f, -3975.5413f,
            1439.2548f, -4249.6923f,
            1436.3106f, -4007.8950f,
            1393.3199f, -4196.0625f,
            1445.2428f, -4373.9052f,
            1407.2349f, -4429.4145f,
            1464.7142f, -4545.2875f,
            1584.1331f, -4596.8764f,
            1716.8065f, -4601.1323f,
            1875.8312f, -4788.7187f,
            1979.7647f, -4883.4585f,
            2219.1562f, -4854.3330f
        };

        pos.clear();

        size = my_sizeof(orgrimmarSouthLimit) / my_sizeof(orgrimmarSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (orgrimmarSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, orgrimmarSouthLimit[i], orgrimmarSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "orgrimmarSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

        static float const feralasThousandNeedlesSouthLimit[] =
        {
            -6495.4995f, -4711.9810f,
            -6674.9995f, -4515.0019f,
            -6769.5717f, -4122.4272f,
            -6838.2651f, -3874.2792f,
            -6851.1314f, -3659.1179f,
            -6624.6845f, -3063.3843f,
            -6416.9067f, -2570.1301f,
            -5959.8466f, -2287.2634f,
            -5947.9135f, -1866.5028f,
            -5947.9135f, -820.48810f,
            -5876.7114f, -3.5138000f,
            -5876.7114f, 917.640700f,
            -6099.3603f, 1153.28840f,
            -6021.8989f, 1638.18090f,
            -6091.6176f, 2335.88920f,
            -6744.9946f, 2393.48550f,
            -6973.8608f, 3077.02810f,
            -7068.7241f, 4376.23040f,
            -7142.1211f, 4808.43310f
        };

        pos.clear();

        size = my_sizeof(feralasThousandNeedlesSouthLimit) / my_sizeof(feralasThousandNeedlesSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (feralasThousandNeedlesSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, feralasThousandNeedlesSouthLimit[i], feralasThousandNeedlesSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "feralasThousandNeedlesSouthLimit" << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig->hasLog("gos.csv"))
    {
        for (GameObjectData const* gData : WorldPosition().getGameObjectsNear())
        {
            GameObjectTemplate const* data = sObjectMgr->GetGameObjectTemplate(gData->id);
            if (!data)
                continue;

            WorldPosition point = WorldPosition(gData->mapid, gData->posX, gData->posY, gData->posZ, gData->orientation);

            std::string name = data->name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << data->type << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig->log("gos.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig->hasLog("zones.csv"))
    {
        std::unordered_map<std::string, std::vector<WorldPosition>> zoneLocs;

        std::vector<WorldPosition> Locs;

        for (auto& u : units)
        {
            WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            std::string const& name = std::to_string(u.map) + point.getAreaName();

            if (zoneLocs.find(name) == zoneLocs.end())
                zoneLocs.insert_or_assign(name, Locs);

            zoneLocs.find(name)->second.push_back(point);
        }

        for (auto& loc : zoneLocs)
        {
            if (loc.second.empty())
                continue;

            if (!sTravelNodeMap.getMapOffset(loc.second.front().getMapId()) && loc.second.front().getMapId() != 0)
                continue;

            std::vector<WorldPosition> points = loc.second;;

            std::ostringstream out;

            WorldPosition pos = WorldPosition(points, WP_MEAN_CENTROID);

            out << "\"center\"" << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            pos.printWKT(out);

            out << "\n";

            out << "\"area\"" << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            point.printWKT(points, out, 0);

            sPlayerbotAIConfig->log("zones.csv", out.str().c_str());
        }
    }

    bool printStrategyMap = false;

    if (printStrategyMap && sPlayerbotAIConfig->hasLog("strategy.csv"))
    {
        static std::map<uint8, std::string> classes;
        static std::map<uint8, std::map<uint8, std::string> > specs;
        classes[CLASS_DRUID] = "druid";
        specs[CLASS_DRUID][0] = "balance";
        specs[CLASS_DRUID][1] = "feral combat";
        specs[CLASS_DRUID][2] = "restoration";

        classes[CLASS_HUNTER] = "hunter";
        specs[CLASS_HUNTER][0] = "beast mastery";
        specs[CLASS_HUNTER][1] = "marksmanship";
        specs[CLASS_HUNTER][2] = "survival";

        classes[CLASS_MAGE] = "mage";
        specs[CLASS_MAGE][0] = "arcane";
        specs[CLASS_MAGE][1] = "fire";
        specs[CLASS_MAGE][2] = "frost";

        classes[CLASS_PALADIN] = "paladin";
        specs[CLASS_PALADIN][0] = "holy";
        specs[CLASS_PALADIN][1] = "protection";
        specs[CLASS_PALADIN][2] = "retribution";

        classes[CLASS_PRIEST] = "priest";
        specs[CLASS_PRIEST][0] = "discipline";
        specs[CLASS_PRIEST][1] = "holy";
        specs[CLASS_PRIEST][2] = "shadow";

        classes[CLASS_ROGUE] = "rogue";
        specs[CLASS_ROGUE][0] = "assasination";
        specs[CLASS_ROGUE][1] = "combat";
        specs[CLASS_ROGUE][2] = "subtlety";

        classes[CLASS_SHAMAN] = "shaman";
        specs[CLASS_SHAMAN][0] = "elemental";
        specs[CLASS_SHAMAN][1] = "enhancement";
        specs[CLASS_SHAMAN][2] = "restoration";

        classes[CLASS_WARLOCK] = "warlock";
        specs[CLASS_WARLOCK][0] = "affliction";
        specs[CLASS_WARLOCK][1] = "demonology";
        specs[CLASS_WARLOCK][2] = "destruction";

        classes[CLASS_WARRIOR] = "warrior";
        specs[CLASS_WARRIOR][0] = "arms";
        specs[CLASS_WARRIOR][1] = "fury";
        specs[CLASS_WARRIOR][2] = "protection";

        classes[CLASS_DEATH_KNIGHT] = "dk";
        specs[CLASS_DEATH_KNIGHT][0] = "blood";
        specs[CLASS_DEATH_KNIGHT][1] = "frost";
        specs[CLASS_DEATH_KNIGHT][2] = "unholy";

        //Use randombot 0.
        std::ostringstream cout;
        cout << sPlayerbotAIConfig->randomBotAccountPrefix << 0;
        std::string const& accountName = cout.str();

        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (results)
        {

            Field* fields = results->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            WorldSession* session = new WorldSession(accountId, nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), LOCALE_enUS, 0, false, false, 0);

            std::vector<std::pair<std::pair<uint32, uint32>, uint32>> classSpecLevel;

            std::unordered_map<std::string, std::vector<std::pair<std::pair<uint32, uint32>, uint32>>> actions;

            std::ostringstream out;

            for (uint8 race = RACE_HUMAN; race < MAX_RACES; race++)
            {
                for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                {
                    if (cls != 10)
                    {
                        std::unique_ptr<CharacterCreateInfo> characterInfo = std::make_unique<CharacterCreateInfo>("dummy", race, cls, 1, 1, 1, 1, 1, 1, 0, WorldPacket());
                        Player* player = new Player(session);
                        if (player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), characterInfo.get()))
                        {

                            for (uint8 tab = 0; tab < 3; tab++)
                            {
                                TalentSpec newSpec;
                                if (tab == 0)
                                    newSpec = TalentSpec(player, "1-0-0");
                                else if (tab == 1)
                                    newSpec = TalentSpec(player, "0-1-0");
                                else
                                    newSpec = TalentSpec(player, "0-0-1");

                                for (uint32 lvl = 1; lvl < MAX_LEVEL; lvl++)
                                {
                                    player->SetLevel(lvl);

                                    std::ostringstream tout;
                                    newSpec.ApplyTalents(player, &tout);

                                    PlayerbotAI* botAI = new PlayerbotAI(player);

                                    botAI->ResetStrategies(false);

                                    AiObjectContext* con = botAI->GetAiObjectContext();

                                    std::vector<std::string> tstrats;
                                    std::set<std::string> strategies;
                                    std::set<std::string> sstrats;

                                    tstrats = botAI->GetStrategies(BOT_STATE_COMBAT);
                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = botAI->GetStrategies(BOT_STATE_NON_COMBAT);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = botAI->GetStrategies(BOT_STATE_DEAD);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(sstrats.begin(), sstrats.end());

                                    for (auto& stratName : strategies)
                                    {
                                        Strategy* strat = con->GetStrategy(stratName);

                                        if (strat->getDefaultActions())
                                            for (int32 i = 0; i < NextAction::size(strat->getDefaultActions()); i++)
                                            {
                                                NextAction* nextAction = strat->getDefaultActions()[i];

                                                std::ostringstream aout;
                                                aout << nextAction->getRelevance() << "," << nextAction->getName() << ",,S:" << stratName;

                                                if (actions.find(aout.str().c_str()) != actions.end())
                                                    classSpecLevel = actions.find(aout.str().c_str())->second;
                                                else
                                                    classSpecLevel.clear();

                                                classSpecLevel.push_back(std::make_pair(std::make_pair(cls, tab), lvl));

                                                actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                            }

                                        std::vector<TriggerNode*> triggers;
                                        strat->InitTriggers(triggers);
                                        for (auto& triggerNode : triggers)
                                        {
                                            //out << " TN:" << triggerNode->getName();

                                            if (Trigger* trigger = con->GetTrigger(triggerNode->getName()))
                                            {
                                                triggerNode->setTrigger(trigger);

                                                NextAction** nextActions = triggerNode->getHandlers();

                                                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                                                {
                                                    NextAction* nextAction = nextActions[i];
                                                    //out << " A:" << nextAction->getName() << "(" << nextAction->getRelevance() << ")";

                                                    std::ostringstream aout;
                                                    aout << nextAction->getRelevance() << "," << nextAction->getName() << "," << triggerNode->getName() << "," << stratName;

                                                    if (actions.find(aout.str().c_str()) != actions.end())
                                                        classSpecLevel = actions.find(aout.str().c_str())->second;
                                                    else
                                                        classSpecLevel.clear();

                                                    classSpecLevel.push_back(std::make_pair(std::make_pair(cls, tab), lvl));

                                                    actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                                }
                                            }
                                        }
                                    }

                                    delete botAI;
                                }
                            }
                        }

                        delete player;
                    }
                }
            }

            std::vector<std::string> actionKeys;

            for (auto& action : actions)
                actionKeys.push_back(action.first);

            std::sort(actionKeys.begin(), actionKeys.end(), [](std::string const& i, std::string const& j)
            {
                std::stringstream is(i);
                std::stringstream js(j);
                float iref, jref;
                std::string iact, jact, itrig, jtrig, istrat, jstrat;
                is >> iref >> iact >> itrig >> istrat;
                js >> jref >> jact >> jtrig >> jstrat;

                if (iref > jref)
                    return true;

                if (iref == jref && istrat < jstrat)
                    return true;

                if (iref == jref && !(istrat > jstrat) && iact < jact)
                    return true;

                if (iref == jref && !(istrat > jstrat) && !(iact > jact) && itrig < jtrig)
                    return true;

                return false;
            });

            sPlayerbotAIConfig->log("strategy.csv", "relevance, action, trigger, strategy, classes");

            for (auto& actionkey : actionKeys)
            {
                if (actions.find(actionkey)->second.size() != (MAX_LEVEL - 1) * (MAX_CLASSES - 1))
                {
                    classSpecLevel = actions.find(actionkey)->second;

                    std::vector<std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>>> classs;

                    for (auto cl : classSpecLevel)
                    {
                        uint32 minLevel = MAX_LEVEL; uint32 maxLevel = 0;

                        uint32 cls = cl.first.first;
                        uint32 tb = cl.first.second;

                        if (std::find_if(classs.begin(), classs.end(), [cls, tb](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                        {
                            return i.first.first == cls && i.first.second == tb;
                        }) == classs.end())
                        {
                            for (auto cll : classSpecLevel)
                            {
                                if (cll.first.first == cl.first.first && cll.first.second == cl.first.second)
                                {
                                    minLevel = std::min(minLevel, cll.second);
                                    maxLevel = std::max(maxLevel, cll.second);
                                }
                            }

                            classs.push_back(std::make_pair(cl.first, std::make_pair(minLevel, maxLevel)));
                        }
                    }

                    out << actionkey;

                    if (classs.size() != 9 * 3)
                    {
                        out << ",";

                        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                        {
                            bool a[3] = { false,false,false };
                            uint32 min[3] = { 0,0,0 };
                            uint32 max[3] = { 0,0,0 };

                            if (std::find_if(classs.begin(), classs.end(), [cls](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                            {
                                return i.first.first == cls;
                            }) == classs.end())
                                continue;

                            for (uint32 tb = 0; tb < 3; tb++)
                            {
                                auto tcl = std::find_if(classs.begin(), classs.end(), [cls, tb](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                {
                                    return i.first.first == cls && i.first.second == tb;
                                });

                                if (tcl == classs.end())
                                    continue;

                                a[tb] = true;
                                min[tb] = tcl->second.first;
                                max[tb] = tcl->second.second;
                            }

                            if (a[0] && a[1] && a[2] && min[0] == min[1] == min[2] && max[0] == max[1] == max[2])
                            {
                                if (min[0] != 1 || max[0] != MAX_LEVEL - 1)
                                    out << classes[cls] << "(" << min[0] << "-" << max[0] << ")";
                                else
                                    out << classes[cls];

                                if (cls != classs.back().first.first)
                                    out << ";";
                            }
                            else
                            {
                                for (uint32 tb = 0; tb < 3; tb++)
                                {
                                    if (!a[tb])
                                        continue;

                                    if (min[tb] != 1 || max[tb] != MAX_LEVEL - 1)
                                        out << specs[cls][tb] << " " << classes[cls] << "(" << min[tb] << "-" << max[tb] << ")";
                                    else
                                        out << specs[cls][tb] << " " << classes[cls];

                                    if (cls != classs.back().first.first || tb != classs.back().first.second)
                                        out << ";";
                                }
                            }
                        }
                    }
                    else
                        "all";

                    out << "\n";
                }
                else
                    out << actionkey << "\n";
            }

            sPlayerbotAIConfig->log("strategy.csv", out.str().c_str());
        }
    }

    /*

    sPlayerbotAIConfig->openLog(7, "w");

    //Zone area map REMOVE!
    uint32 k = 0;
    for (auto& node : sTravelNodeMap->getNodes())
    {
        WorldPosition* pos = node->getPosition();
        //map area
        for (uint32 x = 0; x < 2000; x++)
        {
            for (uint32 y = 0; y < 2000; y++)
            {
                if (!pos->getMap())
                    continue;

                float nx = pos->getX() + (x*5)-5000.0f;
                float ny = pos->getY() + (y*5)-5000.0f;
                float nz = pos->getZ() + 100.0f;

                //pos->getMap()->GetHitPosition(nx, ny, nz + 200.0f, nx, ny, nz, -0.5f);

                if (!pos->getMap()->GetHeightInRange(nx, ny, nz, 5000.0f)) // GetHeight can fail
                    continue;

                WorldPosition  npos = WorldPosition(pos->getMapId(), nx, ny, nz, 0.0);
                uint32 area = path.getArea(npos.getMapId(), npos.getX(), npos.getY(), npos.getZ());

                std::ostringstream out;
                out << std::fixed << area << "," << npos.getDisplayX() << "," << npos.getDisplayY();
                sPlayerbotAIConfig->log(7, out.str().c_str());
            }
        }
        k++;

        if (k > 0)
            break;
    }

    //Explore map output (REMOVE!)

    sPlayerbotAIConfig->openLog(5, "w");
    for (auto i : exploreLocs)
    {
        for (auto j : i.second->getPoints())
        {
            std::ostringstream out;
            string name = i.second->getTitle();
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            out << std::fixed << std::setprecision(2) << name.c_str() << "," << i.first << "," << j->getDisplayX() << "," << j->getDisplayY() << "," << j->getX() << "," << j->getY() << "," << j->getZ();
            sPlayerbotAIConfig->log(5, out.str().c_str());
        }
    }

    */
}

uint32 TravelMgr::getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    QuestRelationBounds rbounds;                        // QuestRelations (quest-giver)
    QuestRelationBounds irbounds;                       // InvolvedRelations (quest-finisher)

    uint32 questId = pQuest->GetQuestId();

    if (questgiver > 0)
    {
        rbounds = sObjectMgr->GetCreatureQuestRelationBounds(questgiver);
        irbounds = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver);
    }
    else
    {
        rbounds = sObjectMgr->GetGOQuestRelationBounds(questgiver * -1);
        irbounds = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (QuestRelations::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(questId)) || (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (QuestRelations::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if (status == QUEST_STATUS_NONE)                    // For all other cases the mark is handled either at some place else, or with involved-relations already
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && pPlayer->IsQuestRewarded(questId)))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 || pPlayer->getLevel() <= pPlayer->GetQuestLevel(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

//Selects a random WorldPosition from a list. Use a distance weighted distribution.
std::vector<WorldPosition*> TravelMgr::getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points, uint32 amount)
{
    std::vector<WorldPosition*> retVec;

    if (points.size() < 2)
    {
        if (points.size() == 1)
            retVec.push_back(points[0]);

        return std::move(retVec);
    }

    retVec = points;

    std::vector<uint32> weights;

    //List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
     //std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

     //List of weights based on distance (Twice the distance = half the weight). Caps out at 200.0000 range.
    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point)
    {
        return 200000 / (1 + point.distance(center));
    });

    Acore::Containers::RandomShuffle(retVec);

    std::vector<float> dists;

    //Total sum of all those weights.
    /*
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    //Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        //Pick a random number in that range.
        uint32 rnd = urand(0, sum);

        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] && (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) == retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }*/

    return std::move(retVec);
}

std::vector<WorldPosition> TravelMgr::getNextPoint(WorldPosition center, std::vector<WorldPosition> points, uint32 amount)
{
    std::vector<WorldPosition> retVec;

    if (points.size() == 1)
    {
        retVec.push_back(points[0]);
        return retVec;
    }

    //List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    std::vector<uint32> weights;

    std::transform(points.begin(), points.end(), std::back_inserter(weights), [center](WorldPosition point) { return 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

    //Total sum of all those weights.
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    //Pick a random number in that range.
    uint32 rnd = urand(0, sum);

    //Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] && (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) == retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }

    if (!retVec.empty())
        return retVec;

    assert(!"No valid point found.");

    return std::move(retVec);
}

QuestStatusData* TravelMgr::getQuestStatus(Player* bot, uint32 questId)
{
    return &bot->getQuestStatusMap()[questId];
}

bool TravelMgr::getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective)
{
    uint32 questId = pQuest->GetQuestId();
    if (!bot->IsActiveQuest(questId))
        return false;

    if (bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
        return false;

    QuestStatusData* questStatus = sTravelMgr->getQuestStatus(bot, questId);

    uint32  reqCount = pQuest->RequiredItemCount[objective];
    uint32  hasCount = questStatus->ItemCount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    reqCount = pQuest->RequiredNpcOrGoCount[objective];
    hasCount = questStatus->CreatureOrGOCount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    return false;
}

std::vector<TravelDestination*> TravelMgr::getQuestTravelDestinations(Player* bot, int32 questId, bool ignoreFull, bool ignoreInactive, float maxDistance, bool ignoreObjectives)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    if (questId == -1)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (dest->isFull(ignoreFull))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
    }
    else
    {
        auto i = quests.find(questId);

        if (i != quests.end())
        {
            for (auto& dest : i->second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (dest->isFull(ignoreFull))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : i->second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (dest->isFull(ignoreFull))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }

    return std::move(retTravelLocations);
}

std::vector<TravelDestination*> TravelMgr::getRpgTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive, float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : rpgNpcs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return std::move(retTravelLocations);
}

std::vector<TravelDestination*> TravelMgr::getExploreTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : exploreLocs)
    {
        if (!ignoreInactive && !dest.second->isActive(bot))
            continue;

        if (dest.second->isFull(ignoreFull))
            continue;

        retTravelLocations.push_back(dest.second);
    }

    return std::move(retTravelLocations);
}

std::vector<TravelDestination*> TravelMgr::getGrindTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive, float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : grindMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return std::move(retTravelLocations);
}

void TravelMgr::setNullTravelTarget(Player* player)
{
    if (!player)
        return;

    if (!player->GetPlayerbotAI())
        return;

    TravelTarget* target = player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

    if (target)
        target->setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
}

void TravelMgr::addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance, bool makeShortcuts)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return;

    //Calculate shortcuts.
    if (makeShortcuts)
    {
        for (auto& mapTransfers : mapTransfersMap)
        {
            uint32 sMapt = mapTransfers.first.first;
            uint32 eMapt = mapTransfers.first.second;

            for (auto& mapTransfer : mapTransfers.second)
            {
                if (eMapt == sMap && sMapt != eMap) // [S1 >MT> E1 -> S2] >THIS> E2
                {
                    float newDistToEnd = mapTransDistance(*mapTransfer.getPointFrom(), start) + portalDistance;
                    if (mapTransDistance(*mapTransfer.getPointFrom(), end) > newDistToEnd)
                        addMapTransfer(*mapTransfer.getPointFrom(), end, newDistToEnd, false);
                }

                if (sMapt == eMap && eMapt != sMap) // S1 >THIS> [E1 -> S2 >MT> E2]
                {
                    float newDistToEnd = portalDistance + mapTransDistance(end, *mapTransfer.getPointTo());
                    if (mapTransDistance(start, *mapTransfer.getPointTo()) > newDistToEnd)
                        addMapTransfer(start, *mapTransfer.getPointTo(), newDistToEnd, false);
                }
            }
        }
    }

    //Add actual transfer.
    auto mapTransfers = mapTransfersMap.find(std::make_pair(start.getMapId(), end.getMapId()));

    if (mapTransfers == mapTransfersMap.end())
        mapTransfersMap.insert({ { sMap, eMap }, {mapTransfer(start, end, portalDistance)} });
    else
        mapTransfers->second.push_back(mapTransfer(start, end, portalDistance));
};

void TravelMgr::loadMapTransfers()
{
    for (auto& node : sTravelNodeMap->getNodes())
    {
        for (auto& link : *node->getLinks())
        {
            addMapTransfer(*node->getPosition(), *link.first->getPosition(), link.second->getDistance());
        }
    }
}

float TravelMgr::mapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.distance(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({ sMap, eMap });
    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {

        float dist = mapTrans.distance(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

float TravelMgr::fastMapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.fDist(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({ sMap, eMap });

    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.fDist(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

QuestTravelDestination::QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
{
    questId = questId1;
    questTemplate = sObjectMgr->GetQuestTemplate(questId);
}

bool QuestTravelDestination::isActive(Player* bot)
{
    return bot->IsActiveQuest(questId);
}

bool QuestObjectiveTravelDestination::isCreature()
{
    return GetQuestTemplate()->RequiredNpcOrGo[objective] > 0;
}

uint32 QuestObjectiveTravelDestination::ReqCreature()
{
    return isCreature() ? GetQuestTemplate()->RequiredNpcOrGo[objective] : 0;
}

uint32 QuestObjectiveTravelDestination::ReqGOId()
{
    return !isCreature() ? abs(GetQuestTemplate()->RequiredNpcOrGo[objective]) : 0;
}

uint32 QuestObjectiveTravelDestination::ReqCount()
{
    return GetQuestTemplate()->RequiredNpcOrGoCount[objective];
}

void TravelMgr::printGrid(uint32 mapId, int x, int y, string type)
{
    string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig.hasLog(fileName))
    {
        WorldPosition p = WorldPosition(mapId, 0, 0, 0, 0);

        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr();
        out << "+00, " << 0 << 0 << x << "," << y << ", " << type << ",";
        p.printWKT(p.fromGridPair(GridPair(x, y)), out, 1, true);
        sPlayerbotAIConfig.log(fileName, out.str().c_str());
    }
}

void TravelMgr::printObj(WorldObject* obj, string type)
{
    string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig.hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell const& cell = obj->GetCurrentCell();

        vector<WorldPosition> vcell, vgrid;
        vcell = p.fromCellPair(p.getCellPair());
        vgrid = p.gridFromCellPair(p.getCellPair());

        {
            ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetObjectGuid().GetEntry() << "," << obj->GetObjectGuid().GetCounter() << "," << cell.GridX() << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vcell, out, 1, true);
            sPlayerbotAIConfig.log(fileName, out.str().c_str());
        }

        {
            ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetObjectGuid().GetEntry() << "," << obj->GetObjectGuid().GetCounter() << "," << cell.GridX() << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vgrid, out, 1, true);
            sPlayerbotAIConfig.log(fileName, out.str().c_str());
        }
    }

    fileName = "unload_obj.csv";

    if (sPlayerbotAIConfig.hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell const& cell = obj->GetCurrentCell();
        {
            ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetObjectGuid().GetEntry() << "," << obj->GetObjectGuid().GetCounter() << "," << cell.GridX() << "," << cell.GridY() << ", " << type << ",";

            p.printWKT({ p }, out, 0);
            sPlayerbotAIConfig.log(fileName, out.str().c_str());
        }
    }
}
