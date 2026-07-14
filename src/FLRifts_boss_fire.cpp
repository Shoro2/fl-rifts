/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// Fire Rift bosses for fl-rifts. Two thematic variants share one ScriptedAI and
// branch on the creature entry: fireboss1 is a melee-leaning Emberlord, fireboss2
// a caster-leaning Conflagrator. Both mirror the Shadow boss rotation shape
// (gap-closer/burst, AoE, single-target, filler) and add the Fire signature:
// persistent "stand out of the fire" ground hazards.
//
// All spell IDs are stock 3.3.5a NPC abilities (no custom DBC). The signature
// spells self-summon their ground effect through the standard core NPCs
// (Scorch 62546 -> 33123, Flame Strike 44192 -> 24666); if those helper NPCs
// are absent in the world DB, swap the signature cast to Rain of Fire (19717),
// a self-contained persistent ground-fire area spell.

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum FireBossEntries
{
    NPC_FIRE_BOSS_EMBERLORD     = 80040,
    NPC_FIRE_BOSS_CONFLAGRATOR  = 80042
};

enum FireBossSpells
{
    SPELL_FIRE_CHARGE       = 74399, // neutral gap-closer (Shadow-boss reference)
    SPELL_FIRE_KNOCKBACK    = 26478, // neutral, paired with the charge
    SPELL_FLAME_BREATH      = 56908, // frontal-cone Fire AoE
    SPELL_FIRE_CLEAVE       = 56909, // single-target melee
    SPELL_FIREBALL          = 19391, // ranged Fire filler
    SPELL_SCORCH            = 62546, // signature: seeds Scorched Ground patches
    SPELL_LIVING_BOMB       = 20475, // delayed detonation burst
    SPELL_INFERNO           = 19695, // point-blank Fire aura
    SPELL_PYROBLAST         = 36819, // heavy single-target nuke
    SPELL_FLAME_STRIKE      = 44192  // signature: rain-of-fire ground patches
};

enum FireBossEvents
{
    EVENT_FIRE_CHARGE = 1,
    EVENT_FLAME_BREATH,
    EVENT_FIRE_CLEAVE,
    EVENT_FIREBALL,
    EVENT_SCORCH,
    EVENT_LIVING_BOMB,
    EVENT_INFERNO,
    EVENT_PYROBLAST,
    EVENT_FLAME_STRIKE
};

class boss_fl_fire : public CreatureScript
{
public:
    boss_fl_fire() : CreatureScript("boss_fl_fire") { }

    struct boss_fl_fireAI : public ScriptedAI
    {
        boss_fl_fireAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
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
                events.ScheduleEvent(EVENT_FLAME_BREATH, Milliseconds(urand(13000, 16000)));
                events.ScheduleEvent(EVENT_FIRE_CLEAVE, Milliseconds(urand(7000, 9000)));
                events.ScheduleEvent(EVENT_FIREBALL, Milliseconds(urand(5000, 7000)));
                events.ScheduleEvent(EVENT_SCORCH, Milliseconds(urand(11000, 13000)));
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
                case EVENT_FLAME_BREATH:
                    DoCastVictim(SPELL_FLAME_BREATH);
                    events.Repeat(Milliseconds(urand(14000, 16000)));
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
                    DoCastRandomTarget(SPELL_SCORCH);
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
                    DoCastRandomTarget(SPELL_FLAME_STRIKE);
                    events.Repeat(Milliseconds(urand(14000, 16000)));
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_fl_fireAI(creature);
    }
};

void AddSC_BossFireScript()
{
    new boss_fl_fire();
}
