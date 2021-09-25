/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChooseRpgTargetAction.h"
#include "BattlegroundMgr.h"
#include "ChatHelper.h"
#include "Event.h"
#include "PossibleRpgTargetsValue.h"
#include "Playerbot.h"
#include "TravelMgr.h"

bool ChooseRpgTargetAction::CanTrain(ObjectGuid guid)
{
    Creature* creature = botAI->GetCreature(guid);
    if (!creature)
        return false;

    if (!creature->IsValidTrainerForPlayer(bot))
        return false;

    // check present spell in trainer spell list
    TrainerSpellData const* cSpells = creature->GetTrainerSpells();
    if (!cSpells)
    {
        return false;
    }

    float fDiscountMod = bot->GetReputationPriceDiscount(creature);

    TrainerSpellData const* trainer_spells = cSpells;
    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        if (!tSpell)
            continue;

        TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
        if (state != TRAINER_SPELL_GREEN)
            continue;

        uint32 spellId = tSpell->spell;
        SpellInfo const* pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        uint32 cost = uint32(floor(tSpell->spellCost * fDiscountMod));
        if (cost > bot->GetMoney())
            continue;

        return true;
    }

    return false;
}

BattlegroundTypeId ChooseRpgTargetAction::CanQueueBg(ObjectGuid guid)
{
    for (uint32 i = 1; i < MAX_BATTLEGROUND_QUEUE_TYPES; i++)
    {
        BattlegroundQueueTypeId queueTypeId = (BattlegroundQueueTypeId)i;

        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);

        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
            continue;

        if (bot->getLevel() < bg->GetMinLevel())
            continue;

        // check if already in queue
        if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
            continue;

        std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> battleMastersCache = sRandomPlayerbotMgr->getBattleMastersCache();

        for (auto& entry : battleMastersCache[TEAM_NEUTRAL][bgTypeId])
            if (entry == guid.GetEntry())
                return bgTypeId;

        for (auto& entry : battleMastersCache[bot->GetTeamId()][bgTypeId])
            if (entry == guid.GetEntry())
                return bgTypeId;
    }

    return BATTLEGROUND_TYPE_NONE;
}

uint32 ChooseRpgTargetAction::HasSameTarget(ObjectGuid guid)
{
    if (botAI->HasRealPlayerMaster())
        return 0;

    uint32 num = 0;

    GuidVector nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (ObjectGuid const guid : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player)
            continue;

        PlayerbotAI* botAI = player->GetPlayerbotAI();
        if (!botAI)
            continue;

        if (!botAI->AllowActive(GRIND_ACTIVITY))
            continue;

        if (botAI->GetAiObjectContext()->GetValue<ObjectGuid>("rpg target")->Get() != guid)
            continue;

        num++;
    }

    return num;
}

bool ChooseRpgTargetAction::Execute(Event event)
{
    TravelTarget* travelTarget = context->GetValue<TravelTarget*>("travel target")->Get();
    GuidVector possibleTargets = AI_VALUE(GuidVector, "possible rpg targets");
    GuidVector possibleObjects = AI_VALUE(GuidVector, "nearest game objects no los");
    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();

    if (!possibleObjects.empty())
    {
        possibleTargets.insert(possibleTargets.end(), possibleObjects.begin(), possibleObjects.end());
    }

    if (possibleTargets.empty())
        return false;

    uint32 maxPriority = 1;

    //First handing in quests
    GuidVector targets;
    for (ObjectGuid const guid : possibleTargets)
    {
        GameObject* go = botAI->GetGameObject(guid);
        Unit* unit = botAI->GetUnit(guid);
        if (!go && !unit)
            continue;

        if (!ignoreList.empty() && ignoreList.find(guid) != ignoreList.end() && urand(0, 100) < 10) //10% chance to retry ignored.
            continue;

        uint32 priority = 1;

        if (unit)
        {
            if (!isFollowValid(bot, unit))
                continue;

            if (unit->IsVendor())
                if (AI_VALUE(uint8, "bag space") > 80)
                    priority = 100;

            if (unit->IsArmorer())
                if (AI_VALUE(uint8, "bag space") > 80 || (AI_VALUE(uint8, "durability") < 80 && AI_VALUE(uint32, "repair cost") < bot->GetMoney()))
                    priority = 95;

            uint32 dialogStatus = bot->GetQuestDialogStatus(unit);
            if (dialogStatus == DIALOG_STATUS_REWARD2 || dialogStatus == DIALOG_STATUS_REWARD || dialogStatus == DIALOG_STATUS_REWARD_REP)
                priority = 90;
            else if (CanTrain(guid) || dialogStatus == DIALOG_STATUS_AVAILABLE)
                priority = 80;
            else if (travelTarget->getDestination() && travelTarget->getDestination()->getEntry() == unit->GetEntry())
                priority = 70;
            else if (unit->IsInnkeeper() && AI_VALUE2(float, "distance", "home bind") > 1000.0f)
                priority = 60;
            else if (unit->IsBattleMaster() && CanQueueBg(guid) != BATTLEGROUND_TYPE_NONE)
                priority = 50;
        }
        else
        {
            if (!go->isSpawned() || go->GetGoState() != GO_STATE_READY)
                continue;

            if (!isFollowValid(bot, go))
                continue;

            uint32 dialogStatus = bot->GetQuestDialogStatus(go);
            if (dialogStatus == DIALOG_STATUS_REWARD2 || dialogStatus == DIALOG_STATUS_REWARD || dialogStatus == DIALOG_STATUS_REWARD_REP)
                priority = 90;
            else if (dialogStatus == DIALOG_STATUS_AVAILABLE)
                priority = 80;
            else if (travelTarget->getDestination() && travelTarget->getDestination()->getEntry() * -1 == go->GetEntry())
                priority = 70;
            //else if (urand(1, 100) > 10)
                //continue;
        }

        if (botAI->HasStrategy("debug rpg", BOT_STATE_NON_COMBAT))
        {
            std::ostringstream out;
            out << "rpg option: ";
            if (unit)
                out << chat->formatWorldobject(unit);
            if (go)
                out << chat->formatGameobject(go);

            out << " " << priority;

            botAI->TellMasterNoFacing(out);
        }

        if (priority < maxPriority)
            continue;

        if (HasSameTarget(guid) > urand(5, 15))
            continue;

        if (priority > maxPriority)
            targets.clear();

        maxPriority = priority;

        targets.push_back(guid);
    }

    if (targets.empty())
    {
        LOG_INFO("playerbots", "%s can't choose RPG target: all %zu are not available", bot->GetName().c_str(), possibleTargets.size());
        ignoreList.clear(); //Clear ignore list.
        context->GetValue<GuidSet&>("ignore rpg target")->Set(ignoreList);
        context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);
        return false;
    }

    ObjectGuid guid = targets[urand(0, targets.size() - 1)];
    if (!guid)
    {
        context->GetValue<ObjectGuid>("rpg target")->Set(ObjectGuid::Empty);
        return false;
    }

    GameObject* go = botAI->GetGameObject(guid);
    Unit* unit = botAI->GetUnit(guid);

    if (botAI->HasStrategy("debug", BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "found: ";

        if (unit)
            out << chat->formatWorldobject(unit);

        if (go)
            out << chat->formatGameobject(go);

        out << " " << maxPriority;

        botAI->TellMasterNoFacing(out);
    }

    context->GetValue<ObjectGuid>("rpg target")->Set(guid);

    return true;
}

bool ChooseRpgTargetAction::isUseful()
{
    return botAI->AllowActive(RPG_ACTIVITY) && !bot->IsInCombat() && !context->GetValue<ObjectGuid>("rpg target")->Get()
        && !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling() && !context->GetValue<GuidVector>("possible rpg targets")->Get().empty();
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldObject* target)
{
    WorldLocation location;
    target->GetPosition(location.m_positionX, location.m_positionY, location.m_positionZ);
    return isFollowValid(bot, location);
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldLocation location)
{
    PlayerbotAI* botAI = bot->GetPlayerbotAI();
    Player* master = botAI->GetGroupMaster();
    if (!master || bot == master)
        return true;

    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return true;

    if (bot->GetDistance(master) > sPlayerbotAIConfig->rpgDistance * 2)
        return true;

    float distance = master->GetDistance(location.GetPositionX(), location.GetPositionY(), location.GetPositionZ());

    if (!master->isMoving() && distance < sPlayerbotAIConfig->sightDistance)
        return true;

    if (distance < sPlayerbotAIConfig->lootDistance)
        return true;

    return false;
}
