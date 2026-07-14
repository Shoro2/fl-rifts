/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// Water Rift bosses for fl-rifts. Two sustain-themed variants share one
// ScriptedAI and branch on entry: waterboss1 (Ichyron) is an adds + geyser
// fight, waterboss2 (Nethys) an interrupt check (a real-cast self-heal behind a
// damage-reduction bubble). Both mirror the Shadow boss rotation shape and are
// Frost-school water.
//
// Ichyron's Rising Tide adds are self-contained helper creatures (entry 80052,
// ScriptName npc_fl_water_globule): they swim to the boss and, on arrival, heal
// it and vanish unless players kill them first. All spell IDs are stock 3.3.5a
// NPC abilities (no custom DBC).

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"

enum WaterBossEntries
{
    NPC_WATER_BOSS_ICHYRON = 80048,
    NPC_WATER_BOSS_NETHYS   = 80049,
    NPC_WATER_GLOBULE       = 80052
};

enum WaterBossSpells
{
    SPELL_WATER_BLAST       = 54237, // Frost damage + knockback burst
    SPELL_WATER_BOLT_VOLLEY = 54241, // raid Frost AoE
    SPELL_FROST_SHOCK       = 12548, // single-target Frost + slow
    SPELL_GEYSER            = 37478, // signature: knock-up hazard
    SPELL_FROSTBOLT         = 15497, // Frost filler
    SPELL_FROSTBOLT_VOLLEY  = 70759, // raid Frost AoE (Nethys variant)
    SPELL_HEALING_WAVE      = 12491, // signature: interruptible self-heal
    SPELL_PROTECTIVE_BUBBLE = 54306  // damage-reduction shield
};

enum WaterBossEvents
{
    EVENT_WATER_BLAST = 1,
    EVENT_WATER_BOLT_VOLLEY,
    EVENT_FROST_SHOCK,
    EVENT_GEYSER,
    EVENT_FROSTBOLT,
    EVENT_RISING_TIDE,
    EVENT_FROSTBOLT_VOLLEY,
    EVENT_HEALING_TIDE,
    EVENT_BUBBLE
};

enum WaterBossMisc
{
    GLOBULE_COUNT        = 3,
    GLOBULE_HEAL_PCT     = 5,
    GLOBULE_ARRIVE_DIST  = 4,
    GLOBULE_DURATION_MS  = 30000
};

// -- Rising Tide globule helper ---------------------------------------------

class npc_fl_water_globule : public CreatureScript
{
public:
    npc_fl_water_globule() : CreatureScript("npc_fl_water_globule") { }

    struct npc_fl_water_globuleAI : public ScriptedAI
    {
        npc_fl_water_globuleAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(WorldObject* summoner) override
        {
            if (summoner && summoner->ToUnit())
            {
                _bossGuid = summoner->GetGUID();
                me->GetMotionMaster()->MoveFollow(
                    summoner->ToUnit(), 0.0f, 0.0f);
            }
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            Creature* boss = ObjectAccessor::GetCreature(*me, _bossGuid);
            if (!boss || !boss->IsAlive())
            {
                me->DespawnOrUnsummon();
                return;
            }

            if (me->GetDistance(boss) <= float(GLOBULE_ARRIVE_DIST))
            {
                boss->ModifyHealth(
                    boss->CountPctFromMaxHealth(GLOBULE_HEAL_PCT));
                me->DespawnOrUnsummon();
            }
        }

    private:
        ObjectGuid _bossGuid;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fl_water_globuleAI(creature);
    }
};

// -- Water bosses ------------------------------------------------------------

class boss_fl_water : public CreatureScript
{
public:
    boss_fl_water() : CreatureScript("boss_fl_water") { }

    struct boss_fl_waterAI : public ScriptedAI
    {
        boss_fl_waterAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
            _tideStarted = false;
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_WATER_BLAST, urand(9000, 11000));
            events.ScheduleEvent(EVENT_FROST_SHOCK, urand(7000, 9000));

            if (me->GetEntry() == NPC_WATER_BOSS_NETHYS)
            {
                events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(12000, 15000));
                events.ScheduleEvent(EVENT_HEALING_TIDE, urand(20000, 25000));
                events.ScheduleEvent(EVENT_BUBBLE, urand(18000, 22000));
            }
            else
            {
                events.ScheduleEvent(EVENT_WATER_BOLT_VOLLEY, urand(12000, 16000));
                events.ScheduleEvent(EVENT_GEYSER, urand(12000, 15000));
                events.ScheduleEvent(EVENT_FROSTBOLT, urand(5000, 7000));
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/,
            DamageEffectType /*type*/, SpellSchoolMask /*school*/) override
        {
            if (!_tideStarted &&
                me->GetEntry() == NPC_WATER_BOSS_ICHYRON &&
                HealthBelowPct(50))
            {
                _tideStarted = true;
                events.ScheduleEvent(EVENT_RISING_TIDE, 1000);
            }
        }

        void SummonGlobules()
        {
            for (uint8 i = 0; i < GLOBULE_COUNT; ++i)
                me->SummonCreature(NPC_WATER_GLOBULE,
                    me->GetPositionX() + frand(-15.0f, 15.0f),
                    me->GetPositionY() + frand(-15.0f, 15.0f),
                    me->GetPositionZ(), me->GetOrientation(),
                    TEMPSUMMON_TIMED_DESPAWN, GLOBULE_DURATION_MS);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();

            switch (events.ExecuteEvent())
            {
                case EVENT_WATER_BLAST:
                    if (Unit* target = SelectTarget(
                        SelectTargetMethod::Random, 0, 0.0f, false))
                        me->CastSpell(target, SPELL_WATER_BLAST, false);
                    events.Repeat(urand(9000, 11000));
                    break;
                case EVENT_WATER_BOLT_VOLLEY:
                    DoCastAOE(SPELL_WATER_BOLT_VOLLEY);
                    events.Repeat(urand(12000, 16000));
                    break;
                case EVENT_FROST_SHOCK:
                    if (Unit* target = SelectTarget(
                        SelectTargetMethod::MaxThreat, 0, 0.0f, false))
                        me->CastSpell(target, SPELL_FROST_SHOCK, false);
                    events.Repeat(urand(7000, 9000));
                    break;
                case EVENT_GEYSER:
                    DoCastRandomTarget(SPELL_GEYSER);
                    events.Repeat(urand(12000, 15000));
                    break;
                case EVENT_FROSTBOLT:
                    DoCastVictim(SPELL_FROSTBOLT);
                    events.Repeat(urand(5000, 7000));
                    break;
                case EVENT_RISING_TIDE:
                    SummonGlobules();
                    events.Repeat(urand(23000, 27000));
                    break;
                case EVENT_FROSTBOLT_VOLLEY:
                    DoCastAOE(SPELL_FROSTBOLT_VOLLEY);
                    events.Repeat(urand(12000, 15000));
                    break;
                case EVENT_HEALING_TIDE:
                    // non-triggered: keeps its cast time so players can
                    // interrupt/silence it.
                    DoCastSelf(SPELL_HEALING_WAVE, false);
                    events.Repeat(urand(20000, 25000));
                    break;
                case EVENT_BUBBLE:
                    DoCastSelf(SPELL_PROTECTIVE_BUBBLE);
                    events.Repeat(urand(18000, 22000));
                    break;
                default:
                    break;
            }
        }

    private:
        bool _tideStarted = false;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_fl_waterAI(creature);
    }
};

void AddSC_BossWaterScript()
{
    new npc_fl_water_globule();
    new boss_fl_water();
}
