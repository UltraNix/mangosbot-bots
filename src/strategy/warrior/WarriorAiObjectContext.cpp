/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarriorAiObjectContext.h"
#include "ArmsWarriorStrategy.h"
#include "FuryWarriorStrategy.h"
#include "GenericWarriorNonCombatStrategy.h"
#include "TankWarriorStrategy.h"
#include "WarriorActions.h"
#include "WarriorTriggers.h"
#include "NamedObjectContext.h"
#include "PullStrategy.h"
#include "Playerbot.h"

class WarriorStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
        WarriorStrategyFactoryInternal()
        {
            creators["nc"] = &WarriorStrategyFactoryInternal::nc;
            creators["pull"] = &WarriorStrategyFactoryInternal::pull;
            creators["aoe"]  = &warrior::StrategyFactoryInternal::warrior_aoe;
        }

    private:
        static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarriorNonCombatStrategy(botAI); }
        static Strategy* warrior_aoe(PlayerbotAI* ai) { return new WarrirorAoeStrategy(ai); }
        static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
};

class WarriorCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
        WarriorCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
        {
            creators["tank"] = &WarriorCombatStrategyFactoryInternal::tank;
            creators["arms"] = &warrior::CombatStrategyFactoryInternal::arms;
            creators["fury"] = &warrior::CombatStrategyFactoryInternal::fury;
        }

    private:
        static Strategy* tank(PlayerbotAI* botAI) { return new TankWarriorStrategy(botAI); }
        static Strategy* arms(PlayerbotAI* ai) { return new ArmsWarriorStrategy(ai); }
        static Strategy* fury(PlayerbotAI* ai) { return new FuryWarriorStrategy(ai); }
};

class WarriorTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
    public:
        WarriorTriggerFactoryInternal()
        {
            creators["hamstring"] = &WarriorTriggerFactoryInternal::hamstring;
            creators["victory rush"] = &WarriorTriggerFactoryInternal::victory_rush;
            creators["death wish"] = &WarriorTriggerFactoryInternal::death_wish;
            creators["battle shout"] = &WarriorTriggerFactoryInternal::battle_shout;
            creators["rend"] = &WarriorTriggerFactoryInternal::rend;
            creators["rend on attacker"] = &WarriorTriggerFactoryInternal::rend_on_attacker;
            creators["bloodrage"] = &WarriorTriggerFactoryInternal::bloodrage;
            creators["shield bash"] = &WarriorTriggerFactoryInternal::shield_bash;
            creators["disarm"] = &WarriorTriggerFactoryInternal::disarm;
            creators["concussion blow"] = &WarriorTriggerFactoryInternal::concussion_blow;
            creators["sword and board"] = &WarriorTriggerFactoryInternal::SwordAndBoard;
            creators["shield bash on enemy healer"] = &WarriorTriggerFactoryInternal::shield_bash_on_enemy_healer;
            creators["battle stance"] = &WarriorTriggerFactoryInternal::battle_stance;
            creators["defensive stance"] = &WarriorTriggerFactoryInternal::defensive_stance;
            creators["berserker stance"] = &WarriorTriggerFactoryInternal::berserker_stance;
            creators["shield block"] = &WarriorTriggerFactoryInternal::shield_block;
            creators["sunder armor"] = &TriggerFactoryInternal::sunder_armor;
            creators["revenge"] = &TriggerFactoryInternal::revenge;
            creators["overpower"] = &TriggerFactoryInternal::overpower;
            creators["mocking blow"] = &TriggerFactoryInternal::mocking_blow;
            creators["rampage"] = &TriggerFactoryInternal::rampage;
            creators["mortal strike"] = &TriggerFactoryInternal::mortal_strike;
            creators["thunder clap on snare target"] = &TriggerFactoryInternal::thunder_clap_on_snare_target;
            creators["thunder clap"] = &TriggerFactoryInternal::thunder_clap;
            creators["bloodthirst"] = &TriggerFactoryInternal::bloodthirst;
            creators["berserker rage"] = &TriggerFactoryInternal::berserker_rage;
            creators["pummel on enemy healer"] = &TriggerFactoryInternal::pummel_on_enemy_healer;
            creators["pummel"] = &TriggerFactoryInternal::pummel;
            creators["intercept on enemy healer"] = &TriggerFactoryInternal::intercept_on_enemy_healer;
            creators["intercept"] = &TriggerFactoryInternal::intercept;
            creators["taunt on snare target"] = &TriggerFactoryInternal::taunt_on_snare_target;
            creators["commanding shout"] = &TriggerFactoryInternal::commanding_shout;
            creators["intercept on snare target"] = &TriggerFactoryInternal::intercept_on_snare_target;
            creators["spell reflection"] = &TriggerFactoryInternal::spell_reflection;
            creators["sudden death"] = &TriggerFactoryInternal::sudden_death;
            creators["instant slam"] = &TriggerFactoryInternal::instant_slam;
            creators["shockwave"] = &TriggerFactoryInternal::shockwave;
            creators["shockwave on snare target"] = &TriggerFactoryInternal::shockwave_on_snare_target;
            creators["taste for blood"] = &TriggerFactoryInternal::taste_for_blood;
        }

    private:
        static Trigger* shield_block(PlayerbotAI* botAI) { return new ShieldBlockTrigger(botAI); }
        static Trigger* defensive_stance(PlayerbotAI* botAI) { return new DefensiveStanceTrigger(botAI); }
        static Trigger* berserker_stance(PlayerbotAI* ai) { return new BerserkerStanceTrigger(ai); }
        static Trigger* battle_stance(PlayerbotAI* botAI) { return new BattleStanceTrigger(botAI); }
        static Trigger* hamstring(PlayerbotAI* botAI) { return new HamstringTrigger(botAI); }
        static Trigger* victory_rush(PlayerbotAI* botAI) { return new VictoryRushTrigger(botAI); }
        static Trigger* death_wish(PlayerbotAI* botAI) { return new DeathWishTrigger(botAI); }
        static Trigger* battle_shout(PlayerbotAI* botAI) { return new BattleShoutTrigger(botAI); }
        static Trigger* rend(PlayerbotAI* botAI) { return new RendDebuffTrigger(botAI); }
        static Trigger* rend_on_attacker(PlayerbotAI* botAI) { return new RendDebuffOnAttackerTrigger(botAI); }
        static Trigger* bloodrage(PlayerbotAI* ai) { return new BloodrageBuffTrigger(ai); }
        static Trigger* shield_bash(PlayerbotAI* botAI) { return new ShieldBashInterruptSpellTrigger(botAI); }
        static Trigger* disarm(PlayerbotAI* botAI) { return new DisarmDebuffTrigger(botAI); }
        static Trigger* concussion_blow(PlayerbotAI* botAI) { return new ConcussionBlowTrigger(botAI); }
        static Trigger* SwordAndBoard(PlayerbotAI* botAI) { return new SwordAndBoardTrigger(botAI); }
        static Trigger* shield_bash_on_enemy_healer(PlayerbotAI* botAI) { return new ShieldBashInterruptEnemyHealerSpellTrigger(botAI); }
        static Trigger* intercept_on_snare_target(PlayerbotAI* ai) { return new InterceptSnareTrigger(ai); }
        static Trigger* spell_reflection(PlayerbotAI* ai) { return new SpellReflectionTrigger(ai); }
        static Trigger* taste_for_blood(PlayerbotAI* ai) { return new TasteForBloodTrigger(ai); }
        static Trigger* shockwave_on_snare_target(PlayerbotAI* ai) { return new ShockwaveSnareTrigger(ai); }
        static Trigger* shockwave(PlayerbotAI* ai) { return new ShockwaveTrigger(ai); }
        static Trigger* instant_slam(PlayerbotAI* ai) { return new SlamInstantTrigger(ai); }
        static Trigger* sudden_death(PlayerbotAI* ai) { return new SuddenDeathTrigger(ai); }
        static Trigger* commanding_shout(PlayerbotAI* ai) { return new CommandingShoutTrigger(ai); }
        static Trigger* taunt_on_snare_target(PlayerbotAI* ai) { return new TauntSnareTrigger(ai); }
        static Trigger* intercept(PlayerbotAI* ai) { return new InterceptInterruptSpellTrigger(ai); }
        static Trigger* intercept_on_enemy_healer(PlayerbotAI* ai) { return new InterceptInterruptEnemyHealerSpellTrigger(ai); }
        static Trigger* pummel(PlayerbotAI* ai) { return new PummelInterruptSpellTrigger(ai); }
        static Trigger* pummel_on_enemy_healer(PlayerbotAI* ai) { return new PummelInterruptEnemyHealerSpellTrigger(ai); }
        static Trigger* berserker_rage(PlayerbotAI* ai) { return new BerserkerRageBuffTrigger(ai); }
        static Trigger* bloodthirst(PlayerbotAI* ai) { return new BloodthirstBuffTrigger(ai); }
        static Trigger* thunder_clap_on_snare_target(PlayerbotAI* ai) { return new ThunderClapSnareTrigger(ai); }
        static Trigger* thunder_clap(PlayerbotAI* ai) { return new ThunderClapTrigger(ai); }
        static Trigger* mortal_strike(PlayerbotAI* ai) { return new MortalStrikeDebuffTrigger(ai); }
        static Trigger* rampage(PlayerbotAI* ai) { return new RampageAvailableTrigger(ai); }
        static Trigger* mocking_blow(PlayerbotAI* ai) { return new MockingBlowTrigger(ai); }
        static Trigger* overpower(PlayerbotAI* ai) { return new OverpowerAvailableTrigger(ai); }
        static Trigger* revenge(PlayerbotAI* ai) { return new RevengeAvailableTrigger(ai); }
        static Trigger* sunder_armor(PlayerbotAI* ai) { return new SunderArmorDebuffTrigger(ai); }
};


class WarriorAiObjectContextInternal : public NamedObjectContext<Action>
{
    public:
        WarriorAiObjectContextInternal()
        {
            creators["devastate"] = &WarriorAiObjectContextInternal::devastate;
            creators["overpower"] = &WarriorAiObjectContextInternal::overpower;
            creators["charge"] = &WarriorAiObjectContextInternal::charge;
            creators["bloodthirst"] = &WarriorAiObjectContextInternal::bloodthirst;
            creators["rend"] = &WarriorAiObjectContextInternal::rend;
            creators["rend on attacker"] = &WarriorAiObjectContextInternal::rend_on_attacker;
            creators["mocking blow"] = &WarriorAiObjectContextInternal::mocking_blow;
            creators["death wish"] = &WarriorAiObjectContextInternal::death_wish;
            creators["berserker rage"] = &WarriorAiObjectContextInternal::berserker_rage;
            creators["victory rush"] = &WarriorAiObjectContextInternal::victory_rush;
            creators["execute"] = &WarriorAiObjectContextInternal::execute;
            creators["defensive stance"] = &WarriorAiObjectContextInternal::defensive_stance;
            creators["hamstring"] = &WarriorAiObjectContextInternal::hamstring;
            creators["shield bash"] = &WarriorAiObjectContextInternal::shield_bash;
            creators["shield block"] = &WarriorAiObjectContextInternal::shield_block;
            creators["bloodrage"] = &WarriorAiObjectContextInternal::bloodrage;
            creators["battle stance"] = &WarriorAiObjectContextInternal::battle_stance;
            creators["heroic strike"] = &WarriorAiObjectContextInternal::heroic_strike;
            creators["intimidating shout"] = &WarriorAiObjectContextInternal::intimidating_shout;
            creators["demoralizing shout"] = &WarriorAiObjectContextInternal::demoralizing_shout;
            creators["challenging shout"] = &WarriorAiObjectContextInternal::challenging_shout;
            creators["shield wall"] = &WarriorAiObjectContextInternal::shield_wall;
            creators["battle shout"] = &WarriorAiObjectContextInternal::battle_shout;
            creators["battle shout taunt"] = &WarriorAiObjectContextInternal::battle_shout_taunt;
            creators["thunder clap"] = &WarriorAiObjectContextInternal::thunder_clap;
            creators["taunt"] = &WarriorAiObjectContextInternal::taunt;
            creators["revenge"] = &WarriorAiObjectContextInternal::revenge;
            creators["slam"] = &WarriorAiObjectContextInternal::slam;
            creators["shield slam"] = &WarriorAiObjectContextInternal::shield_slam;
            creators["disarm"] = &WarriorAiObjectContextInternal::disarm;
            creators["sunder armor"] = &WarriorAiObjectContextInternal::sunder_armor;
            creators["last stand"] = &WarriorAiObjectContextInternal::last_stand;
            creators["shockwave on snare target"] = &AiObjectContextInternal::shockwave_on_snare_target;
            creators["shockwave"] = &WarriorAiObjectContextInternal::shockwave;
            creators["cleave"] = &WarriorAiObjectContextInternal::cleave;
            creators["concussion blow"] = &WarriorAiObjectContextInternal::concussion_blow;
            creators["shield bash on enemy healer"] = &WarriorAiObjectContextInternal::shield_bash_on_enemy_healer;
            creators["berserker stance"] = &AiObjectContextInternal::berserker_stance;
            creators["commanding shout"] = &AiObjectContextInternal::commanding_shout;
            creators["retaliation"] = &AiObjectContextInternal::retaliation;
            creators["mortal strike"] = &AiObjectContextInternal::mortal_strike;
            creators["sweeping strikes"] = &AiObjectContextInternal::sweeping_strikes;
            creators["intercept"] = &AiObjectContextInternal::intercept;
            creators["whirlwind"] = &AiObjectContextInternal::whirlwind;
            creators["pummel"] = &AiObjectContextInternal::pummel;
            creators["pummel on enemy healer"] = &AiObjectContextInternal::pummel_on_enemy_healer;
            creators["recklessness"] = &AiObjectContextInternal::recklessness;
            creators["piercing howl"] = &AiObjectContextInternal::piercing_howl;
            creators["rampage"] = &AiObjectContextInternal::rampage;
            creators["intervene"] = &AiObjectContextInternal::intervene;
            creators["spell reflection"] = &AiObjectContextInternal::spell_reflection;
            creators["thunder clap on snare target"] = &AiObjectContextInternal::thunder_clap_on_snare_target;
            creators["taunt on snare target"] = &AiObjectContextInternal::taunt_on_snare_target;
            creators["intercept on enemy healer"] = &AiObjectContextInternal::intercept_on_enemy_healer;
            creators["intercept on snare target"] = &AiObjectContextInternal::intercept_on_snare_target;
            creators["bladestorm"] = &AiObjectContextInternal::bladestorm;
            creators["heroic throw"] = &AiObjectContextInternal::heroic_throw;
            creators["heroic throw on snare target"] = &AiObjectContextInternal::heroic_throw_on_snare_target;
            creators["shattering throw"] = &AiObjectContextInternal::shattering_throw;
        }

    private:
        static Action* devastate(PlayerbotAI* botAI) { return new CastDevastateAction(botAI); }
        static Action* last_stand(PlayerbotAI* botAI) { return new CastLastStandAction(botAI); }
        static Action* shockwave(PlayerbotAI* botAI) { return new CastShockwaveAction(botAI); }
        static Action* shockwave_on_snare_target(PlayerbotAI* ai) { return new CastShockwaveSnareAction(ai); }
        static Action* cleave(PlayerbotAI* botAI) { return new CastCleaveAction(botAI); }
        static Action* concussion_blow(PlayerbotAI* botAI) { return new CastConcussionBlowAction(botAI); }
        static Action* taunt(PlayerbotAI* botAI) { return new CastTauntAction(botAI); }
        static Action* revenge(PlayerbotAI* botAI) { return new CastRevengeAction(botAI); }
        static Action* slam(PlayerbotAI* botAI) { return new CastSlamAction(botAI); }
        static Action* shield_slam(PlayerbotAI* botAI) { return new CastShieldSlamAction(botAI); }
        static Action* disarm(PlayerbotAI* botAI) { return new CastDisarmAction(botAI); }
        static Action* sunder_armor(PlayerbotAI* botAI) { return new CastSunderArmorAction(botAI); }
        static Action* overpower(PlayerbotAI* botAI) { return new CastOverpowerAction(botAI); }
        static Action* charge(PlayerbotAI* botAI) { return new CastChargeAction(botAI); }
        static Action* bloodthirst(PlayerbotAI* botAI) { return new CastBloodthirstAction(botAI); }
        static Action* rend(PlayerbotAI* botAI) { return new CastRendAction(botAI); }
        static Action* rend_on_attacker(PlayerbotAI* botAI) { return new CastRendOnAttackerAction(botAI); }
        static Action* mocking_blow(PlayerbotAI* botAI) { return new CastMockingBlowAction(botAI); }
        static Action* death_wish(PlayerbotAI* botAI) { return new CastDeathWishAction(botAI); }
        static Action* berserker_rage(PlayerbotAI* botAI) { return new CastBerserkerRageAction(botAI); }
        static Action* victory_rush(PlayerbotAI* botAI) { return new CastVictoryRushAction(botAI); }
        static Action* execute(PlayerbotAI* botAI) { return new CastExecuteAction(botAI); }
        static Action* defensive_stance(PlayerbotAI* botAI) { return new CastDefensiveStanceAction(botAI); }
        static Action* hamstring(PlayerbotAI* botAI) { return new CastHamstringAction(botAI); }
        static Action* shield_bash(PlayerbotAI* botAI) { return new CastShieldBashAction(botAI); }
        static Action* shield_block(PlayerbotAI* botAI) { return new CastShieldBlockAction(botAI); }
        static Action* bloodrage(PlayerbotAI* botAI) { return new CastBloodrageAction(botAI); }
        static Action* battle_stance(PlayerbotAI* botAI) { return new CastBattleStanceAction(botAI); }
        static Action* heroic_strike(PlayerbotAI* botAI) { return new CastHeroicStrikeAction(botAI); }
        static Action* intimidating_shout(PlayerbotAI* botAI) { return new CastIntimidatingShoutAction(botAI); }
        static Action* demoralizing_shout(PlayerbotAI* botAI) { return new CastDemoralizingShoutAction(botAI); }
        static Action* challenging_shout(PlayerbotAI* botAI) { return new CastChallengingShoutAction(botAI); }
        static Action* shield_wall(PlayerbotAI* botAI) { return new CastShieldWallAction(botAI); }
        static Action* battle_shout(PlayerbotAI* botAI) { return new CastBattleShoutAction(botAI); }
        static Action* battle_shout_taunt(PlayerbotAI* botAI) { return new CastBattleShoutTauntAction(botAI); }
        static Action* thunder_clap(PlayerbotAI* botAI) { return new CastThunderClapAction(botAI); }
        static Action* shield_bash_on_enemy_healer(PlayerbotAI* botAI) { return new CastShieldBashOnEnemyHealerAction(botAI); }
        static Action* intercept_on_snare_target(PlayerbotAI* ai) { return new CastInterceptOnSnareTargetAction(ai); }
        static Action* intercept_on_enemy_healer(PlayerbotAI* ai) { return new CastInterceptOnEnemyHealerAction(ai); }
        static Action* taunt_on_snare_target(PlayerbotAI* ai) { return new CastTauntOnSnareTargetAction(ai); }
        static Action* thunder_clap_on_snare_target(PlayerbotAI* ai) { return new CastThunderClapSnareAction(ai); }
        static Action* berserker_stance(PlayerbotAI* ai) { return new CastBerserkerStanceAction(ai); }
        static Action* commanding_shout(PlayerbotAI* ai) { return new CastCommandingShoutAction(ai); }
        static Action* retaliation(PlayerbotAI* ai) { return new CastRetaliationAction(ai); }
        static Action* mortal_strike(PlayerbotAI* ai) { return new CastMortalStrikeAction(ai); }
        static Action* sweeping_strikes(PlayerbotAI* ai) { return new CastSweepingStrikesAction(ai); }
        static Action* intercept(PlayerbotAI* ai) { return new CastInterceptAction(ai); }
        static Action* whirlwind(PlayerbotAI* ai) { return new CastWhirlwindAction(ai); }
        static Action* pummel(PlayerbotAI* ai) { return new CastPummelAction (ai); }
        static Action* pummel_on_enemy_healer(PlayerbotAI* ai) { return new CastPummelOnEnemyHealerAction(ai); }
        static Action* recklessness(PlayerbotAI* ai) { return new CastRecklessnessAction(ai); }
        static Action* piercing_howl(PlayerbotAI* ai) { return new CastPiercingHowlAction(ai); }
        static Action* rampage(PlayerbotAI* ai) { return new CastRampageAction(ai); }
        static Action* intervene(PlayerbotAI* ai) { return new CastInterveneAction(ai); }
        static Action* spell_reflection(PlayerbotAI* ai) { return new CastSpellReflectionAction(ai); }
        static Action* shattering_throw(PlayerbotAI* ai) { return new CastShatteringThrowAction(ai); }
        static Action* heroic_throw_on_snare_target(PlayerbotAI* ai) { return new CastHeroicThrowSnareAction(ai); }
        static Action* heroic_throw(PlayerbotAI* ai) { return new CastHeroicThrowAction(ai); }
        static Action* bladestorm(PlayerbotAI* ai) { return new CastBladestormAction(ai); }
};

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new WarriorStrategyFactoryInternal());
    strategyContexts.Add(new WarriorCombatStrategyFactoryInternal());
    actionContexts.Add(new WarriorAiObjectContextInternal());
    triggerContexts.Add(new WarriorTriggerFactoryInternal());
}
