/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICSPELLACTIONS_H
#define _PLAYERBOT_GENERICSPELLACTIONS_H

#include "Action.h"
#include "Value.h"

class Event;
class PlayerbotAI;
class Unit;

#define BEGIN_SPELL_ACTION(clazz, name) \
class clazz : public CastSpellAction \
{ \
    public: \
        clazz(PlayerbotAI* botAI) : CastSpellAction(botAI, name) { } \


#define END_SPELL_ACTION() \
};

#define BEGIN_DEBUFF_ACTION(clazz, name) \
class clazz : public CastDebuffSpellAction \
{ \
    public: \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, name) { } \

#define BEGIN_RANGED_SPELL_ACTION(clazz, name) \
class clazz : public CastSpellAction \
{ \
    public: \
        clazz(PlayerbotAI* botAI) : CastSpellAction(botAI, name) { } \

#define BEGIN_MELEE_SPELL_ACTION(clazz, name) \
class clazz : public CastMeleeSpellAction \
{ \
    public: \
        clazz(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, name) { } \


#define END_RANGED_SPELL_ACTION() \
};


#define BEGIN_BUFF_ON_PARTY_ACTION(clazz, name) \
class clazz : public BuffOnPartyAction \
{ \
    public: \
        clazz(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, name) { }

class CastSpellAction : public Action
{
    public:
        CastSpellAction(PlayerbotAI* botAI, std::string const& spell) : Action(botAI, spell), range(botAI->GetRange("spell")), spell(spell) { }

        std::string const& GetTargetName() override { return "current target"; };
        bool Execute(Event event) override;
        bool isPossible() override;
		bool isUseful() override;
        ActionThreatType getThreatType() override { return ACTION_THREAT_SINGLE; }

        NextAction** getPrerequisites() override;

    protected:
        std::string spell;
		float range;
};

class CastAuraSpellAction : public CastSpellAction
{
	public:
		CastAuraSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell) { }

		bool isUseful() override;
};

class CastMeleeSpellAction : public CastSpellAction
{
    public:
        CastMeleeSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell)
        {
			range = ATTACK_DISTANCE;
		}
};

class CastDebuffSpellAction : public CastAuraSpellAction
{
    public:
        CastDebuffSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastAuraSpellAction(botAI, spell) { }
};

class CastDebuffSpellOnAttackerAction : public CastAuraSpellAction
{
    public:
        CastDebuffSpellOnAttackerAction(PlayerbotAI* botAI, std::string const& spell) : CastAuraSpellAction(botAI, spell) { }

        Value<Unit*>* GetTargetValue() override;
        std::string const& getName() override { return spell + " on attacker"; }
        ActionThreatType getThreatType() override { return ACTION_THREAT_AOE; }
};

class CastBuffSpellAction : public CastAuraSpellAction
{
	public:
		CastBuffSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastAuraSpellAction(botAI, spell)
		{
			range = botAI->GetRange("spell");
		}

        std::string const& GetTargetName() override { return "self target"; }
};

class CastEnchantItemAction : public CastSpellAction
{
	public:
	    CastEnchantItemAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell)
		{
			range = botAI->GetRange("spell");
		}

        bool isPossible() override;
        std::string const& GetTargetName() override { return "self target"; }
};

class CastHealingSpellAction : public CastAuraSpellAction
{
    public:
        CastHealingSpellAction(PlayerbotAI* botAI, std::string const& spell, uint8 estAmount = 15.0f) : CastAuraSpellAction(botAI, spell), estAmount(estAmount)
        {
			range = botAI->GetRange("spell");
        }

        std::string const& GetTargetName() override { return "self target"; }
        bool isUseful() override;
        ActionThreatType getThreatType() override { return ACTION_THREAT_AOE; }

    protected:
        uint8 estAmount;
};

class CastAoeHealSpellAction : public CastHealingSpellAction
{
    public:
    	CastAoeHealSpellAction(PlayerbotAI* botAI, std::string const& spell, uint8 estAmount = 15.0f) : CastHealingSpellAction(botAI, spell, estAmount) { }

		std::string const& GetTargetName() override { return "party member to heal"; }
        bool isUseful() override;
};

class CastCureSpellAction : public CastSpellAction
{
	public:
		CastCureSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell)
		{
			range = botAI->GetRange("spell");
		}

		std::string const& GetTargetName() override { return "self target"; }
};

class PartyMemberActionNameSupport
{
	public:
		PartyMemberActionNameSupport(std::string const& spell)
		{
			name = std::string(spell) + " on party";
		}

		virtual std::string const& getName() { return name; }

	private:
		std::string name;
};

class HealPartyMemberAction : public CastHealingSpellAction, public PartyMemberActionNameSupport
{
    public:
        HealPartyMemberAction(PlayerbotAI* botAI, std::string const& spell, uint8 estAmount = 15.0f) :
			CastHealingSpellAction(botAI, spell, estAmount), PartyMemberActionNameSupport(spell) { }

		std::string const& GetTargetName() override { return "party member to heal"; }
		std::string const& getName() override { return PartyMemberActionNameSupport::getName(); }
};

class ResurrectPartyMemberAction : public CastSpellAction
{
	public:
		ResurrectPartyMemberAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell) { }

		std::string const& GetTargetName() override { return "party member to resurrect"; }
};

class CurePartyMemberAction : public CastSpellAction, public PartyMemberActionNameSupport
{
    public:
        CurePartyMemberAction(PlayerbotAI* botAI, std::string const& spell, uint32 dispelType) :
            CastSpellAction(botAI, spell), PartyMemberActionNameSupport(spell), dispelType(dispelType) { }

		Value<Unit*>* GetTargetValue() override;
		std::string const& getName() override { return PartyMemberActionNameSupport::getName(); }

    protected:
        uint32 dispelType;
};

class BuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
{
    public:
        BuffOnPartyAction(PlayerbotAI* botAI, std::string const& spell) :
			CastBuffSpellAction(botAI, spell), PartyMemberActionNameSupport(spell) { }

		Value<Unit*>* GetTargetValue() override;
		std::string const& getName() override { return PartyMemberActionNameSupport::getName(); }
};

class CastShootAction : public CastSpellAction
{
    public:
        CastShootAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shoot") { }

        ActionThreatType getThreatType() override { return ACTION_THREAT_NONE; }
};

class CastLifeBloodAction : public CastHealingSpellAction
{
	public:
		CastLifeBloodAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lifeblood") { }
};

class CastGiftOfTheNaaruAction : public CastHealingSpellAction
{
	public:
		CastGiftOfTheNaaruAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "gift of the naaru") { }
};

class CastArcaneTorrentAction : public CastBuffSpellAction
{
    public:
        CastArcaneTorrentAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane torrent") { }
};

class CastManaTapAction : public CastBuffSpellAction
{
    public:
        CastManaTapAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mana tap") { }
};

class CastWarStompAction : public CastSpellAction
{
    public:
        CastWarStompAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "war stomp") { }
};

class CastSpellOnEnemyHealerAction : public CastSpellAction
{
    public:
        CastSpellOnEnemyHealerAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell) { }

        Value<Unit*>* GetTargetValue() override;
        std::string const& getName() override { return spell + " on enemy healer"; }
};

class CastSnareSpellAction : public CastDebuffSpellAction
{
    public:
        CastSnareSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastDebuffSpellAction(botAI, spell) { }

        Value<Unit*>* GetTargetValue() override;
        std::string const& getName() override { return spell + " on snare target"; }
};

#endif
