#include "botpch.h"
#include "../../playerbot.h"
#include "PartyMemberWithoutItemValue.h"

#include "../../ServerFacade.h"
using namespace botAI;

class PlayerWithoutItemPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutItemPredicate(PlayerbotAI* botAI, std::string const& item) :
        PlayerbotAIAware(botAI), FindPlayerPredicate(), item(item) { }

public:
    virtual bool Check(Unit* unit)
    {
        Pet* pet = dynamic_cast<Pet*>(unit);
        if (pet && (pet->getPetType() == MINI_PET || pet->getPetType() == SUMMON_PET))
            return false;

        if (!sServerFacade->IsAlive(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

        PlayerbotAI* botAi = member->GetPlayerbotAI();
        if (!botAi)
            return false;

        return !botbotAI->GetAiObjectContext()->GetValue<uint8>("item count", item)->Get();
    }

private:
    string item;
};

Unit* PartyMemberWithoutItemValue::Calculate()
{
    FindPlayerPredicate *predicate = CreatePredicate();
    Unit *unit = FindPartyMember(*predicate);
    delete predicate;
    return unit;
}

FindPlayerPredicate* PartyMemberWithoutItemValue::CreatePredicate()
{
    return new PlayerWithoutItemPredicate(botAI, qualifier);
}

class PlayerWithoutFoodPredicate : public PlayerWithoutItemPredicate
{
public:
    PlayerWithoutFoodPredicate(PlayerbotAI* botAI) : PlayerWithoutItemPredicate(botAI, "conjured food") { }

public:
    virtual bool Check(Unit* unit)
    {
        if (!PlayerWithoutItemPredicate::Check(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

        return member->getClass() != CLASS_MAGE;
    }

};

class PlayerWithoutWaterPredicate : public PlayerWithoutItemPredicate
{
public:
    PlayerWithoutWaterPredicate(PlayerbotAI* botAI) : PlayerWithoutItemPredicate(botAI, "conjured water") { }

public:
    virtual bool Check(Unit* unit)
    {
        if (!PlayerWithoutItemPredicate::Check(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

        uint8 cls = member->getClass();
        return cls == CLASS_DRUID ||
                cls == CLASS_HUNTER ||
                cls == CLASS_PALADIN ||
                cls == CLASS_PRIEST ||
                cls == CLASS_SHAMAN ||
                cls == CLASS_WARLOCK;
    }

};

FindPlayerPredicate* PartyMemberWithoutFoodValue::CreatePredicate()
{
    return new PlayerWithoutFoodPredicate(botAI);
}

FindPlayerPredicate* PartyMemberWithoutWaterValue::CreatePredicate()
{
    return new PlayerWithoutWaterPredicate(botAI);
}