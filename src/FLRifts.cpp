/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedAI/ScriptedCreature.h"

bool eventActive = false , riftSpawned = false;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
std::list<Creature*> creatureList = {};
Creature* riftCreature;

void FLR_init() {
    //reset stuff
    creepsAlive = 0;
    waveNumber = 0;
    creatureList = {};
}

void FLR_clear() {
    // clear everything
    std::list<Creature*>::iterator it;
    for (it = creatureList.begin(); it != creatureList.end(); it++)
    {
        
    }
}



class DelayedWaveSpawn : public BasicEvent
{
public:
    DelayedWaveSpawn() : BasicEvent() { }

    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        waveNumber++;
        return true;
    }

private:
    TempSummon* _summon;
    ObjectGuid _playerGUID;
};


// Add player scripts
class FLRiftsPlayer : public PlayerScript
{
public:
    FLRiftsPlayer() : PlayerScript("FLRiftsPlayer") { }

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Hello World from Rifts-Module!");
        }
    }
};

class FLRiftsGameEvent : public GameEventScript
{
public:
    FLRiftsGameEvent() : GameEventScript("FLRiftsGameEvent") { }

    void OnStart(uint16 EventID) override {
        if (EventID == 120) {
            eventActive = true;
            FLR_init();
            sWorld->SendWorldText(LANG_EVENTMESSAGE, "Starting Rift Event.");



            CreatureData const* data = sObjectMgr->GetCreatureData(12000);

            sObjectMgr->AddCreatureToGrid(12000, data);
        }
        
    }

    void OnStop(uint16 EventID) override {
        if (EventID == 120) {
            eventActive = false;
            FLR_clear();
            sWorld->SendWorldText(LANG_EVENTMESSAGE, "Rift Event has ended.");


        }
    }


};

class FLRiftsCreatureRift : public CreatureScript
{
public:
    FLRiftsCreatureRift() : CreatureScript("FLRiftsCreatureRift") { }

    struct FLRiftsCreatureRiftAI : public ScriptedAI {

        FLRiftsCreatureRiftAI(Creature* creature) : ScriptedAI(creature)
        {
            // Constructor, define variables here
        }


        void UpdateAI(uint32 diff) {
            if (eventActive) {
                switch (waveNumber) {
                case 0:
                    //start spawning
                    uint32 myX, myY, myZ, myO;



                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            
                            creatureList.push_front(me->SummonCreature(RAND(1, 2, 3, 4, 5, 6, 7, 8, 9), myX, myY, myZ, myO, TEMPSUMMON_TIMED_DESPAWN, 600));
                            creepsAlive++;
                        }
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(120000));
                    }
                    break;
                case 1:
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            creatureList.push_front(me->SummonCreature(RAND(1, 2, 3, 4, 5, 6, 7, 8, 9), myX, myY, myZ, myO, TEMPSUMMON_TIMED_DESPAWN, 600));
                            creepsAlive++;
                        }
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(120000));
                    }
                    break;
                case 2:
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            creatureList.push_front(me->SummonCreature(RAND(1, 2, 3, 4, 5, 6, 7, 8, 9), myX, myY, myZ, myO, TEMPSUMMON_TIMED_DESPAWN, 600));
                            creepsAlive++;
                        }
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(120000));
                    }
                    break;
                case 3:
                    //end event
                    if (creepsAlive == 0) {
                        eventActive = false;
                        FLR_clear();
                    }
                    
                    break;
                default:
                    break;
                }
            }
        }



    private:
        // Declare variables here


    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new FLRiftsCreatureRiftAI(creature);
    }
};

class FLRiftsCreatureTrash : public CreatureScript
{
public:
    FLRiftsCreatureTrash() : CreatureScript("FLRiftsCreatureTrash") { }


    struct FLRiftsCreatureTrashAI : public ScriptedAI {
        FLRiftsCreatureTrashAI(Creature* creature) : ScriptedAI(creature)
        {
            // Constructor, define variables here
        }


        void JustDied(Unit* /*killer*/) override {
            creepsAlive--;
        }
    
    private:
        // Declare variables here


    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new FLRiftsCreatureTrashAI(creature);
    }
};

class FLRiftsCreatureSpawner : public CreatureScript
{
public:
    FLRiftsCreatureSpawner() : CreatureScript("FLRiftsCreatureSpawner") { }


    struct FLRiftsCreatureTrashAI : public ScriptedAI {
        FLRiftsCreatureTrashAI(Creature* creature) : ScriptedAI(creature)
        {
            // Constructor, define variables here
        }


        void UpdateAI(uint32 /*diff*/) override {
            if (waveNumber == 0 && creepsAlive == 0 && !riftSpawned) {
                // create rift
                riftCreature = me->SummonCreature(90017, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                riftSpawned = true;
            }
            else if (waveNumber == 3 && creepsAlive == 0 && riftSpawned) {
                //clean rift
                riftCreature->DespawnOrUnsummon(0);
                riftSpawned = false;
                // todo loot
            }
        }

    private:
        // Declare variables here


    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new FLRiftsCreatureTrashAI(creature);
    }
};

// Add all scripts in one
void AddFLRiftsScripts()
{
    new FLRiftsPlayer();

    new FLRiftsGameEvent();

    new FLRiftsCreatureRift();
    new FLRiftsCreatureTrash();
    new FLRiftsCreatureSpawner();
}
