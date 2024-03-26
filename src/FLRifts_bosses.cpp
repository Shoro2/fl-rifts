#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

enum Texts
{
    SAY_DEATH = 1,
    SAY_ENGAGE = 0,
    SAY_ONKILL = 2,
    SAY_EVADE = 3
};

enum Spells
{
    SPELL_CHARGE = 74399,
    SPELL_KNOCKBACK = 26478,
    SPELL_SILENCE = 64189,
    SPELL_STRIKE = 62130,
    SPELL_CLEAVE = 70670
};

enum Events
{
    EVENT_SPELL_CHARGE = 1,
    EVENT_SPELL_KNOCKBACK = 2,
    EVENT_SPELL_SILENCE = 3,
    EVENT_SPELL_STRIKE = 4,
    EVENT_SPELL_CLEAVE = 5,
};


class boss_shadow : public CreatureScript
{
public:
    boss_shadow() : CreatureScript("boss_shadow") { }

    struct boss_shadowAI : public ScriptedAI
    {
        boss_shadowAI(Creature* creature) : ScriptedAI(creature) {

        }
        void Initialize()
        {
            events.ScheduleEvent(EVENT_SPELL_CHARGE, urand(9000, 11000));
            events.ScheduleEvent(EVENT_SPELL_SILENCE, urand(18000, 22000));
            events.ScheduleEvent(EVENT_SPELL_STRIKE, urand(7000, 9000));
            events.ScheduleEvent(EVENT_SPELL_CLEAVE, urand(12000, 15000));
        }


        void Reset() override
        {
            // Implement boss reset behavior
            Talk(SAY_EVADE);
            events.Reset();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            Talk(SAY_ENGAGE);
            //ScriptedAI::JustEngagedWith(who);
            Initialize();
            
        }

        void UpdateAI(uint32 diff) override
        {
            // Implement boss update behavior (abilities, movement, etc.)
            if (!UpdateVictim())
            {
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
            {
                return;
            }

            DoMeleeAttackIfReady();


            switch (events.ExecuteEvent())
            {
            case EVENT_SPELL_CHARGE:
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 0, false))
                {
                    AttackStart(target);
                    me->CastSpell(target, SPELL_CHARGE, false);
                    me->CastSpell(target, SPELL_KNOCKBACK, false);
                }
                if (chargeCounter > 5) {
                    chargeCounter++;
                    events.RepeatEvent(500);

                }
                else {
                    chargeCounter = 0;
                    events.RepeatEvent(20000);

                }

                break;
            case EVENT_SPELL_SILENCE:
                DoCastAOE(SPELL_SILENCE, false);
                events.RepeatEvent(urand(18000, 21000));
                break;

            case EVENT_SPELL_STRIKE:
                events.RepeatEvent(urand(6000, 8000));
                if (Unit* target = SelectTarget(SelectTargetMethod::MaxThreat, 0, 0, false))
                {
                    me->CastSpell(target, SPELL_STRIKE, false);
                }
                break;

            case EVENT_SPELL_CLEAVE:
                DoCastAOE(SPELL_CLEAVE, false);
                events.RepeatEvent(urand(12000, 16000));
                break;
           

            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            // Implement boss death behavior
            Talk(SAY_DEATH);
        }

    private:
        uint8 chargeCounter = 0;

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_shadowAI(creature);
    }
};







void AddSC_BossShadowScript()
{
    new boss_shadow();
}
