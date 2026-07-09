#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"

enum Texts
{
    SAY_DEATH = 0,
    SAY_ENGAGE = 1,
    SAY_ONKILL = 2,
    SAY_EVADE = 3
};

enum Spells
{
    SPELL_1 = 12345,
    SPELL_2 = 12345,
    SPELL_3 = 12345

};

enum Phases
{
    PHASE_ONE,
    PHASE_TWO, 
    PHASE_THREE 

};

enum Events
{
    EVENT_SPELL_WINGBUFFET = 1,
    EVENT_SPELL_FLAMEBREATH = 2,
    EVENT_SPELL_TAILSWEEP = 3,
    EVENT_SPELL_CLEAVE = 4,
    EVENT_START_PHASE_2 = 5,
    EVENT_START_PHASE_3 = 6

};


class boss_template : public CreatureScript
{
public:
    boss_template() : CreatureScript("boss_template") { }

    struct boss_templateAI : public BossAI
    {
        boss_templateAI(Creature* creature) : BossAI(creature, 0) { }
        void Initialize()
        {
        }


        void SetPhase(uint8 ph)
        {
            events.Reset();
            Phase = ph;
            switch (ph)
            {
            case PHASE_ONE:
                events.ScheduleEvent(EVENT_SPELL_WINGBUFFET, Milliseconds(urand(10000, 20000)));
                events.ScheduleEvent(EVENT_SPELL_FLAMEBREATH, Milliseconds(urand(10000, 20000)));
                events.ScheduleEvent(EVENT_SPELL_TAILSWEEP, Milliseconds(urand(15000, 20000)));
                events.ScheduleEvent(EVENT_SPELL_CLEAVE, Milliseconds(urand(2000, 5000)));
                break;
            case PHASE_TWO:
                events.ScheduleEvent(EVENT_START_PHASE_2, Milliseconds(0));
                break;
            case PHASE_THREE:
                events.ScheduleEvent(EVENT_START_PHASE_3, Milliseconds(5000));
                break;
            }
        }

        void Reset() override
        {
            // Implement boss reset behavior
            Initialize();
        }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_ENGAGE);
            SetPhase(PHASE_ONE);
            BossAI::JustEngagedWith(who);
        }

        void UpdateAI(uint32 diff) override
        {
            // Implement boss update behavior (abilities, movement, etc.)
            if (!UpdateVictim() || !CheckInRoom())
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
            case EVENT_SPELL_WINGBUFFET:

                events.Repeat(Milliseconds(15000));
                break;
            case EVENT_START_PHASE_2:
                if (me->HasUnitFlag(UNIT_FLAG_DISARMED))
                {
                    events.Repeat(Milliseconds(5000));
                }
                else
                {
                    events.Repeat(Milliseconds(urand(18000, 21000)));
                }
                break;
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            // Implement boss death behavior
        }

    private:
        uint8 Phase;
    };
};
    

    




void AddSC_BossTemplateScript()
{
    new boss_template();
}
