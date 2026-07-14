/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// Air Rift bosses for fl-rifts. Two storm-themed variants share one ScriptedAI
// and branch on entry: airboss1 (Stormcaller) summons wandering tornadoes,
// airboss2 (Galewind) periodically knocks the whole melee cluster away. Both
// mirror the Shadow boss rotation shape and are pure Nature-school lightning.
//
// The wandering tornado is a self-contained helper creature (entry 80051,
// ScriptName npc_fl_air_tornado) so no external NPC dependency is required.
// All spell IDs are stock 3.3.5a NPC abilities (no custom DBC).

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "MotionMaster.h"

enum AirBossEntries
{
    NPC_AIR_BOSS_STORMCALLER = 80034,
    NPC_AIR_BOSS_GALEWIND     = 80038,
    NPC_AIR_TORNADO           = 80051
};

enum AirBossSpells
{
    SPELL_AIR_CHARGE        = 32323, // gap-closer
    SPELL_STORMHAMMER       = 62042, // post-charge stun + thunderclap
    SPELL_LIGHTNING_NOVA    = 52960, // "get out" Nature nova
    SPELL_ARC_LIGHTNING     = 52921, // chaining single-target
    SPELL_CHAIN_LIGHTNING   = 62131, // filler arc
    SPELL_THUNDERING_STOMP  = 60925, // signature: raid-wide knock-up
    SPELL_LIGHTNING_BOLT    = 53044, // single-target nuke
    SPELL_TORNADO_KNOCK     = 56855  // tornado pulse (Cyclone Strike PB knock)
};

enum AirBossEvents
{
    EVENT_AIR_CHARGE = 1,
    EVENT_LIGHTNING_NOVA,
    EVENT_ARC_LIGHTNING,
    EVENT_CHAIN_LIGHTNING,
    EVENT_SUMMON_TORNADO,
    EVENT_THUNDERING_STOMP,
    EVENT_LIGHTNING_BOLT
};

enum AirBossMisc
{
    TORNADO_DURATION_MS = 15000,
    TORNADO_PULSE_MS    = 1500,
    TORNADO_COUNT       = 2
};

// -- Wandering tornado helper ------------------------------------------------

class npc_fl_air_tornado : public CreatureScript
{
public:
    npc_fl_air_tornado() : CreatureScript("npc_fl_air_tornado") { }

    struct npc_fl_air_tornadoAI : public ScriptedAI
    {
        npc_fl_air_tornadoAI(Creature* creature) : ScriptedAI(creature)
        {
            _pulseTimer = TORNADO_PULSE_MS;
        }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetImmuneToAll(true);
            me->GetMotionMaster()->MoveRandom(12.0f);
            _pulseTimer = TORNADO_PULSE_MS;
        }

        void UpdateAI(uint32 diff) override
        {
            if (_pulseTimer <= diff)
            {
                DoCastAOE(SPELL_TORNADO_KNOCK, true);
                _pulseTimer = TORNADO_PULSE_MS;
            }
            else
                _pulseTimer -= diff;
        }

    private:
        uint32 _pulseTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fl_air_tornadoAI(creature);
    }
};

// -- Air bosses --------------------------------------------------------------

class boss_fl_air : public CreatureScript
{
public:
    boss_fl_air() : CreatureScript("boss_fl_air") { }

    struct boss_fl_airAI : public ScriptedAI
    {
        boss_fl_airAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_AIR_CHARGE, Milliseconds(urand(15000, 20000)));
            events.ScheduleEvent(EVENT_ARC_LIGHTNING, Milliseconds(urand(8000, 10000)));
            events.ScheduleEvent(EVENT_LIGHTNING_NOVA, Milliseconds(urand(18000, 22000)));

            if (me->GetEntry() == NPC_AIR_BOSS_GALEWIND)
            {
                events.ScheduleEvent(EVENT_THUNDERING_STOMP, Milliseconds(urand(18000, 22000)));
                events.ScheduleEvent(EVENT_LIGHTNING_BOLT, Milliseconds(urand(6000, 8000)));
            }
            else
            {
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, Milliseconds(urand(10000, 13000)));
                events.ScheduleEvent(EVENT_SUMMON_TORNADO, Milliseconds(urand(20000, 25000)));
            }
        }

        void SummonTornadoes()
        {
            for (uint8 i = 0; i < TORNADO_COUNT; ++i)
            {
                Unit* target = SelectTarget(
                    SelectTargetMethod::Random, 0, 40.0f, true);
                if (!target)
                    target = me->GetVictim();
                if (!target)
                    return;

                me->SummonCreature(NPC_AIR_TORNADO,
                    target->GetPositionX(), target->GetPositionY(),
                    target->GetPositionZ(), me->GetOrientation(),
                    TEMPSUMMON_TIMED_DESPAWN, TORNADO_DURATION_MS);
            }
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
                case EVENT_AIR_CHARGE:
                    if (Unit* target = SelectTarget(
                        SelectTargetMethod::Random, 0, 0.0f, false))
                    {
                        me->CastSpell(target, SPELL_AIR_CHARGE, false);
                        if (me->GetEntry() == NPC_AIR_BOSS_STORMCALLER)
                            me->CastSpell(target, SPELL_STORMHAMMER, false);
                    }
                    events.Repeat(Milliseconds(urand(15000, 20000)));
                    break;
                case EVENT_ARC_LIGHTNING:
                    DoCastRandomTarget(SPELL_ARC_LIGHTNING);
                    events.Repeat(Milliseconds(urand(8000, 10000)));
                    break;
                case EVENT_LIGHTNING_NOVA:
                    DoCastAOE(SPELL_LIGHTNING_NOVA);
                    events.Repeat(Milliseconds(urand(18000, 22000)));
                    break;
                case EVENT_CHAIN_LIGHTNING:
                    DoCastRandomTarget(SPELL_CHAIN_LIGHTNING);
                    events.Repeat(Milliseconds(urand(10000, 13000)));
                    break;
                case EVENT_SUMMON_TORNADO:
                    SummonTornadoes();
                    events.Repeat(Milliseconds(urand(20000, 25000)));
                    break;
                case EVENT_THUNDERING_STOMP:
                    DoCastAOE(SPELL_THUNDERING_STOMP);
                    // chase the scatter with a storm pulse; Reschedule (not
                    // Schedule) so the self-repeating Nova keeps a single timer
                    // instead of stacking one extra cycle per stomp.
                    events.RescheduleEvent(EVENT_LIGHTNING_NOVA, Milliseconds(1500));
                    events.Repeat(Milliseconds(urand(18000, 22000)));
                    break;
                case EVENT_LIGHTNING_BOLT:
                    DoCastVictim(SPELL_LIGHTNING_BOLT);
                    events.Repeat(Milliseconds(urand(6000, 8000)));
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_fl_airAI(creature);
    }
};

void AddSC_BossAirScript()
{
    new npc_fl_air_tornado();
    new boss_fl_air();
}
