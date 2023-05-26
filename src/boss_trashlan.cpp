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
    SPELL_CHARGE = 74399,

};

enum Phases
{
    PHASE_NONE,
    PHASE_GROUNDED, // Phase 1
    PHASE_AIRPHASE, // Phase 2 - Airphase - 60% health
    PHASE_LANDED    // Phase 3 - Landed after Airphase - 40% health
};


class BossEncounterScript : public BossAI
{
public:
    BossEncounterScript(Creature* pCreature) : BossAI(pCreature, 0)
    {
        Initialize();
    }

    void Initialize()
    {
    }

    void SetPhase(uint8 ph)
    {
        events.Reset();
        Phase = ph;
        switch (ph)
        {
        case PHASE_GROUNDED:
            events.ScheduleEvent(EVENT_SPELL_WINGBUFFET, urand(10000, 20000));
            events.ScheduleEvent(EVENT_SPELL_FLAMEBREATH, urand(10000, 20000));
            events.ScheduleEvent(EVENT_SPELL_TAILSWEEP, urand(15000, 20000));
            events.ScheduleEvent(EVENT_SPELL_CLEAVE, urand(2000, 5000));
            break;
        case PHASE_AIRPHASE:
            events.ScheduleEvent(EVENT_START_PHASE_2, 0);
            break;
        case PHASE_LANDED:
            events.ScheduleEvent(EVENT_START_PHASE_3, 5000);
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
        SetPhase(PHASE_GROUNDED);

        instance->DoStopTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_TIMED_START_EVENT); // just in case at reset some players already left the instance
        instance->DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_TIMED_START_EVENT);
        BossAI::JustEngagedWith(who);

        me->SummonCreature(NPC_ONYXIAN_LAIR_GUARD, -167.837936f, -200.549332f, -66.343231f, 5.598287f, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        // Implement boss update behavior (abilities, movement, etc.)
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        // Implement boss death behavior
    }

    void EnterEvadeMode() override
    {
        // Implement boss evade behavior
    }
};

void AddSC_BossEncounterScript()
{
    new BossEncounterScript();
}

// Register the boss script with the server
void AddSC_CustomScripts()
{
    AddSC_BossEncounterScript();
}
