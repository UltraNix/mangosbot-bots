/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PartyMemberValue.h"
#include "Playerbot.h"

Unit* PartyMemberValue::FindPartyMember(std::vector<Player*>* party, FindPlayerPredicate& predicate)
{
    for (Player* player : *party)
    {
        if (Check(player) && predicate.Check(player))
            return player;

        if (Pet* pet = player->GetPet())
            if (Check(pet) && predicate.Check(pet))
                return pet;
    }

    return nullptr;
}

Unit* PartyMemberValue::FindPartyMember(FindPlayerPredicate& predicate)
{
    Player* master = GetMaster();
    GuidVector nearestPlayers = AI_VALUE(GuidVector, "nearest friendly players");

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (ref->GetSource() != bot)
            {
                if (ref->getSubGroup() != bot->GetSubGroup())
                {
                    nearestPlayers.push_back(ref->GetSource()->GetGUID());
                }
                else
                {
                    nearestPlayers.push_front(ref->GetSource()->GetGUID());
                }
            }
        }
    }

    std::vector<Player*> healers;
    std::vector<Player*> tanks;
    std::vector<Player*> others;
    std::vector<Player*> masters;
    if (master)
        masters.push_back(master);

    for (ObjectGuid const guid : nearestPlayers)
    {
        Player* player = botAI->GetUnit(guid)->ToPlayer();
        if (!player || player == bot)
            continue;

        if (botAI->IsHeal(player))
            healers.push_back(player);
        else if (botAI->IsTank(player))
            tanks.push_back(player);
        else if (player != master)
            others.push_back(player);
    }

    std::vector<std::vector<Player*>*> lists;
    lists.push_back(&healers);
    lists.push_back(&tanks);
    lists.push_back(&masters);
    lists.push_back(&others);

    for (std::vector<std::vector<Player*>*>::iterator i = lists.begin(); i != lists.end(); ++i)
    {
        std::vector<Player*>* party = *i;

        Unit* target = FindPartyMember(party, predicate);
        if (target)
            return target;
    }

    return nullptr;
}

bool PartyMemberValue::Check(Unit* player)
{
    return player && player != bot && player->GetMapId() == bot->GetMapId() && bot->GetDistance(player) < sPlayerbotAIConfig->spellDistance &&
        bot->IsWithinLOS(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
}

bool PartyMemberValue::IsTargetOfSpellCast(Player* target, SpellEntryPredicate &predicate)
{
    GuidVector nearestPlayers = AI_VALUE(GuidVector, "nearest friendly players");
    ObjectGuid targetGuid = target ? target->GetGUID() : bot->GetGUID();
    ObjectGuid corpseGuid = target && target->GetCorpse() ? target->GetCorpse()->GetGUID() : ObjectGuid::Empty;

    for (ObjectGuid const guid : nearestPlayers)
    {
        Player* player = botAI->GetUnit(guid)->ToPlayer();
        if (!player || player == bot)
            continue;

        if (player->IsNonMeleeSpellCast(true))
        {
            for (uint8 type = CURRENT_GENERIC_SPELL; type < CURRENT_MAX_SPELL; type++)
            {
                Spell* spell = player->GetCurrentSpell((CurrentSpellTypes)type);
                if (spell && predicate.Check(spell->m_spellInfo))
                {
                    ObjectGuid unitTarget = spell->m_targets.GetUnitTargetGUID();
                    if (unitTarget == targetGuid)
                        return true;

                    ObjectGuid corpseTarget = spell->m_targets.GetCorpseTargetGUID();
                    if (corpseTarget == corpseGuid)
                        return true;
                }
            }
        }
    }

    return false;
}
