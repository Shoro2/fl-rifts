/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// Fire Rift bosses for fl-rifts. Two thematic variants share one ScriptedAI and
// branch on the creature entry: fireboss1 is a melee-leaning Emberlord,
// fireboss2 a caster-leaning Conflagrator. Both mirror the Shadow boss rotation
// shape (gap-closer/burst, AoE, single-target, filler) and add the Fire
// signature: persistent "stand out of the fire" ground hazards.
//
// All spell IDs are stock 3.3.5a NPC abilities (no custom DBC). Fireboss2 uses
// a module-owned helper creature so its Flame Strike telegraph always detonates
// on time and never leaves stock trigger NPCs behind.

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectAccessor.h"

enum FireBossEntries
{
    NPC_IGNIS_SCORCHED_GROUND    = 33123,
    NPC_FIRE_BOSS_EMBERLORD     = 80040,
    NPC_FIRE_BOSS_CONFLAGRATOR  = 80042,
    NPC_FIRE_FLAME_STRIKE_TRAP  = 80178
};

enum FireBossSpells
{
    SPELL_FIRE_CHARGE       = 74399, // neutral gap-closer (Shadow-boss reference)
    SPELL_FIRE_KNOCKBACK    = 26478, // neutral, paired with the charge
    SPELL_FIRE_CLEAVE       = 56909, // single-target melee
    SPELL_FIREBALL          = 19391, // ranged Fire filler
    SPELL_SCORCHED_GROUND   = 62548, // fixed-position damage patch
    SPELL_LIVING_BOMB       = 20475, // delayed detonation burst
    SPELL_INFERNO           = 19695, // point-blank Fire aura
    SPELL_PYROBLAST         = 36819, // heavy single-target nuke
    SPELL_FLAME_STRIKE_DAMAGE = 44190,
    SPELL_FLAME_STRIKE_VISUAL = 44191
};

enum FireBossEvents
{
    EVENT_FIRE_CHARGE = 1,
    EVENT_FIRE_CLEAVE,
    EVENT_FIREBALL,
    EVENT_SCORCH,
    EVENT_LIVING_BOMB,
    EVENT_INFERNO,
    EVENT_PYROBLAST,
    EVENT_FLAME_STRIKE
};

enum FireBossMisc
{
    SCORCHED_GROUND_DURATION_MS = 15000,
    FLAME_STRIKE_TELEGRAPH_MS = 5000,
    FLAME_STRIKE_TRAP_DURATION_MS = 7000
};

class npc_fl_fire_flame_strike : public CreatureScript
{
public:
    npc_fl_fire_flame_strike()
        : CreatureScript("npc_fl_fire_flame_strike") { }

    struct npc_fl_fire_flame_strikeAI : public ScriptedAI
    {
        npc_fl_fire_flame_strikeAI(Creature* creature)
            : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetImmuneToAll(true);
            _detonationTimer = FLAME_STRIKE_TELEGRAPH_MS;
            _detonated = false;
        }

        void IsSummonedBy(WorldObject* summoner) override
        {
            if (!summoner)
            {
                me->DespawnOrUnsummon();
                return;
            }

            _summonerGuid = summoner->GetGUID();
            DoCastSelf(SPELL_FLAME_STRIKE_VISUAL, true);
        }

        void UpdateAI(uint32 diff) override
        {
            if (_detonated)
                return;

            Creature* boss = ObjectAccessor::GetCreature(*me, _summonerGuid);
            if (!boss || !boss->IsAlive())
            {
                me->DespawnOrUnsummon();
                return;
            }

            if (_detonationTimer > diff)
            {
                _detonationTimer -= diff;
                return;
            }

            _detonated = true;
            me->RemoveAurasDueToSpell(SPELL_FLAME_STRIKE_VISUAL);
            DoCastAOE(SPELL_FLAME_STRIKE_DAMAGE, true);
            me->DespawnOrUnsummon(Milliseconds(1000));
        }

    private:
        ObjectGuid _summonerGuid;
        uint32 _detonationTimer = FLAME_STRIKE_TELEGRAPH_MS;
        bool _detonated = false;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fl_fire_flame_strikeAI(creature);
    }
};

class boss_fl_fire : public CreatureScript
{
public:
    boss_fl_fire() : CreatureScript("boss_fl_fire") { }

    struct boss_fl_fireAI : public ScriptedAI
    {
        boss_fl_fireAI(Creature* creature)
            : ScriptedAI(creature), summons(me) { }

        void Reset() override
        {
            events.Reset();
            summons.DespawnAll();
        }

        void JustDied(Unit* /*killer*/) override
        {
            summons.DespawnAll();
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            summons.Despawn(summon);
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            if (me->GetEntry() == NPC_FIRE_BOSS_CONFLAGRATOR)
            {
                events.ScheduleEvent(EVENT_LIVING_BOMB, Milliseconds(urand(10000, 13000)));
                events.ScheduleEvent(EVENT_INFERNO, Milliseconds(urand(18000, 22000)));
                events.ScheduleEvent(EVENT_PYROBLAST, Milliseconds(urand(8000, 11000)));
                events.ScheduleEvent(EVENT_FIREBALL, Milliseconds(urand(5000, 7000)));
                events.ScheduleEvent(EVENT_FLAME_STRIKE, Milliseconds(urand(14000, 16000)));
            }
            else
            {
                events.ScheduleEvent(EVENT_FIRE_CHARGE, Milliseconds(urand(9000, 11000)));
                events.ScheduleEvent(EVENT_FIRE_CLEAVE, Milliseconds(urand(7000, 9000)));
                events.ScheduleEvent(EVENT_FIREBALL, Milliseconds(urand(5000, 7000)));
                events.ScheduleEvent(EVENT_SCORCH, Milliseconds(urand(11000, 13000)));
            }
        }

        void SummonFlameStrikeTrap()
        {
            Unit* target = SelectTarget(
                SelectTargetMethod::Random, 0, 40.0f, true);
            if (!target)
                target = me->GetVictim();
            if (!target)
                return;

            me->SummonCreature(NPC_FIRE_FLAME_STRIKE_TRAP,
                target->GetPositionX(), target->GetPositionY(),
                target->GetPositionZ(), me->GetOrientation(),
                TEMPSUMMON_TIMED_DESPAWN,
                FLAME_STRIKE_TRAP_DURATION_MS);
        }

        void SummonScorchedGround()
        {
            Unit* target = SelectTarget(
                SelectTargetMethod::Random, 0, 40.0f, true);
            if (!target)
                target = me->GetVictim();
            if (!target)
                return;

            if (Creature* scorch = me->SummonCreature(NPC_IGNIS_SCORCHED_GROUND,
                target->GetPositionX(), target->GetPositionY(),
                target->GetPositionZ(), me->GetOrientation(),
                TEMPSUMMON_TIMED_DESPAWN,
                SCORCHED_GROUND_DURATION_MS))
                scorch->CastSpell(scorch, SPELL_SCORCHED_GROUND, true);
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
                case EVENT_FIRE_CHARGE:
                    if (Unit* target = SelectTarget(
                        SelectTargetMethod::Random, 0, 0.0f, false))
                    {
                        AttackStart(target);
                        me->CastSpell(target, SPELL_FIRE_CHARGE, false);
                        me->CastSpell(target, SPELL_FIRE_KNOCKBACK, false);
                    }
                    events.Repeat(Milliseconds(urand(18000, 22000)));
                    break;
                case EVENT_FIRE_CLEAVE:
                    DoCastVictim(SPELL_FIRE_CLEAVE);
                    events.Repeat(Milliseconds(urand(7000, 9000)));
                    break;
                case EVENT_FIREBALL:
                    DoCastVictim(SPELL_FIREBALL);
                    events.Repeat(Milliseconds(urand(5000, 7000)));
                    break;
                case EVENT_SCORCH:
                    SummonScorchedGround();
                    events.Repeat(Milliseconds(urand(11000, 13000)));
                    break;
                case EVENT_LIVING_BOMB:
                    DoCastRandomTarget(SPELL_LIVING_BOMB);
                    events.Repeat(Milliseconds(urand(11000, 14000)));
                    break;
                case EVENT_INFERNO:
                    DoCastSelf(SPELL_INFERNO);
                    events.Repeat(Milliseconds(urand(18000, 22000)));
                    break;
                case EVENT_PYROBLAST:
                    DoCastVictim(SPELL_PYROBLAST);
                    events.Repeat(Milliseconds(urand(9000, 11000)));
                    break;
                case EVENT_FLAME_STRIKE:
                    SummonFlameStrikeTrap();
                    events.Repeat(Milliseconds(urand(14000, 16000)));
                    break;
                default:
                    break;
            }
        }

    private:
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_fl_fireAI(creature);
    }
};

void AddSC_BossFireScript()
{
    new npc_fl_fire_flame_strike();
    new boss_fl_fire();
}
