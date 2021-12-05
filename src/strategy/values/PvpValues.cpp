/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PvpValues.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "Playerbot.h"
#include "ServerFacade.h"

Unit* FlagCarrierValue::Calculate()
{
    Unit* carrier = nullptr;

    if (botAI->GetBot()->InBattleground())
    {
        if (botAI->GetBot()->GetBattlegroundTypeId() == BattlegroundTypeId::BATTLEGROUND_WS)
        {
            BattlegroundWS *bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();

            if ((!sameTeam && bot->GetTeam() == HORDE || (sameTeam && bot->GetTeam() == ALLIANCE)) && !bg->GetFlagPickerGUID(TEAM_INDEX_HORDE).IsEmpty())
                carrier = bg->GetBgMap()->GetPlayer(bg->GetFlagPickerGUID(TEAM_INDEX_HORDE));

            if ((!sameTeam && bot->GetTeam() == ALLIANCE || (sameTeam && bot->GetTeam() == HORDE)) && !bg->GetFlagPickerGUID(TEAM_INDEX_ALLIANCE).IsEmpty())
                carrier = bg->GetBgMap()->GetPlayer(bg->GetFlagPickerGUID(TEAM_INDEX_ALLIANCE));

            if (carrier)
            {
                if (ignoreRange || bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig->sightDistance))
                {
                    return carrier;
                }
                else
                    return nullptr;
            }
        }

        if (ai->GetBot()->GetBattleGroundTypeId() == BATTLEGROUND_EY)
        {
            BattleGroundEY* bg = (BattleGroundEY*)ai->GetBot()->GetBattleGround();

            if (bg->GetFlagCarrierGuid().IsEmpty())
                return nullptr;

            Player* fc = bg->GetBgMap()->GetPlayer(bg->GetFlagCarrierGuid());
            if (!fc)
                return nullptr;

            if (!sameTeam && (fc->GetTeam() != bot->GetTeam()))
                carrier = fc;

            if (sameTeam && (fc->GetTeam() == bot->GetTeam()))
                carrier = fc;

            if (carrier)
            {
                if (ignoreRange || bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig.sightDistance))
                {
                    return carrier;
                }
                else
                    return nullptr;
            }
        }
    }

    return carrier;
}

std::vector<CreatureData const*> BgMastersValue::Calculate()
{
    BattlegroundTypeId bgTypeId = (BattlegroundTypeId)stoi(qualifier);

    std::vector<uint32> entries;
    std::map<Team, std::map<BattlegroundTypeId, std::vector<uint32>>> battleMastersCache = sRandomPlayerbotMgr->getBattleMastersCache();
    entries.insert(entries.end(), battleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId].begin(), battleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId].end());
    entries.insert(entries.end(), battleMastersCache[ALLIANCE][bgTypeId].begin(), battleMastersCache[ALLIANCE][bgTypeId].end());
    entries.insert(entries.end(), battleMastersCache[HORDE][bgTypeId].begin(), battleMastersCache[HORDE][bgTypeId].end());

    std::vector<CreatureData const*> bmGuids;

    for (auto entry : entries)
    {
        for (auto creaturePair : WorldPosition().getCreaturesNear(0, entry))
        {
            bmGuids.push_back(creaturePair);
        }
    }

    return std::move(bmGuids);
}

CreatureData const* BgMasterValue::Calculate()
{
    CreatureData const* bmPair = NearestBm(false);
    if (!bmPair)
        bmPair = NearestBm(true);

    return bmPair;
}

CreatureData const* BgMasterValue::NearestBm(bool allowDead)
{
    WorldPosition botPos(bot);

    std::vector<CreatureData const*> bmPairs = AI_VALUE2(std::vector<CreatureData const*>, "bg masters", qualifier);

    float rDist;
    CreatureData const* rbmPair = nullptr;

    for (auto& bmPair : bmPairs)
    {
        ObjectGuid bmGuid(HIGHGUID_UNIT, bmPair->second.id, bmPair->first);

        if (!bmPair)
            continue;

        WorldPosition bmPos(bmPair);

        float dist = botPos.distance(bmPos); //This is the aproximate travel distance.

        //Did we already find a closer unit that is not dead?
        if (rbmPair && rDist <= dist)
            continue;

        CreatureInfo const* bmTemplate = ObjectMgr::GetCreatureTemplate(bmPair->second.id);
        if (!bmTemplate)
            continue;

        FactionTemplateEntry const* bmFactionEntry = sFactionTemplateStore.LookupEntry(bmTemplate->Faction);

        //Is the unit hostile?
        if (botAI->getReaction(bmFactionEntry) < REP_NEUTRAL)
            continue;

        AreaTableEntry const* area = bmPos.getArea();

        if (!area)
            continue;

        //Is the area hostile?
        if (area->team == 4 && bot->GetTeam() == ALLIANCE)
            continue;
        if (area->team == 2 && bot->GetTeam() == HORDE)
            continue;

        if (!allowDead)
        {
            Unit* unit = botAI->GetUnit(bmPair);

            if (!unit)
                continue;

            //Is the unit dead?
            if (unit->getDeathState() == DEAD)
                continue;
        }

        rbmPair = bmPair;
        rDist = dist;
    }

    return rbmPair;
}

BattleGroundTypeId RpgBgTypeValue::Calculate()
{
    GuidPosition guidPosition = AI_VALUE(GuidPosition, "rpg target");

    if (guidPosition)
        for (uint32 i = 1; i < MAX_BATTLEGROUND_QUEUE_TYPES; i++)
        {
            BattleGroundQueueTypeId queueTypeId = (BattleGroundQueueTypeId)i;

            BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);

            BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            if (!bg)
                continue;

            if (bot->getLevel() < bg->GetMinLevel())
                continue;

            // check if already in queue
            if (bot->InBattleGroundQueueForBattleGroundQueueType(queueTypeId))
                continue;

            map<Team, map<BattleGroundTypeId, list<uint32>>> battleMastersCache = sRandomPlayerbotMgr.getBattleMastersCache();

            for (auto& entry : battleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId])
                if (entry == guidPosition.GetEntry())
                    return bgTypeId;

            for (auto& entry : battleMastersCache[bot->GetTeam()][bgTypeId])
                if (entry == guidPosition.GetEntry())
                    return bgTypeId;
        }

    return BATTLEGROUND_TYPE_NONE;
}
