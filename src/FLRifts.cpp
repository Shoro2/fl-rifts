/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedAI/ScriptedCreature.h"

bool eventActive = false, riftSpawned = false, waiting = false;
float posX, posY, posZ, posO;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
std::list<TempSummon*> creatureList = {};
Creature* riftCreature;
uint32 riftNumber = rand() % 4;



void FLR_init() {
    //reset stuff
    creepsAlive = 0;
    waveNumber = 0;
    creatureList = {};
    eventActive = true;
    riftSpawned = true;
    riftNumber = rand() % 4;
    sWorld->SendWorldText(LANG_EVENTMESSAGE, "init");
}

void FLR_clear() {
    // clear everything
    std::list<TempSummon*>::iterator it;
    for (it = creatureList.begin(); it != creatureList.end(); it++)
    {
        TempSummon* currentCreature = *it;
        currentCreature->DespawnOrUnsummon(0);
    }
}



class DelayedWaveSpawn : public BasicEvent
{
public:
    DelayedWaveSpawn() : BasicEvent() { }

    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        waveNumber++;
        waiting = false;
        sWorld->SendWorldText(LANG_EVENTMESSAGE, "done waiting");
        return true;
    }

private:

};

class DelayedRiftSpawn : public BasicEvent
{
public:
    DelayedRiftSpawn() : BasicEvent() { }

    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        waveNumber=0;
        eventActive = true;
        return true;
    }

private:

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

class FLRiftsCreatureRift : public CreatureScript
{
public:
    FLRiftsCreatureRift() : CreatureScript("FLRiftsCreatureRift") { }

    struct FLRiftsCreatureRiftAI : public ScriptedAI {

        FLRiftsCreatureRiftAI(Creature* creature) : ScriptedAI(creature)
        {
            // Constructor, define variables here
            
        }
        

        void UpdateAI(uint32 /*diff*/) {
            if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false) && eventActive) {
                switch (waveNumber) {
                case 0:
                    //start spawning
                    if (creepsAlive == 0) {
                        
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "Spawning Wave 1");
                        for (size_t i = 0; i < 10; i++)
                        {
                            posX = me->GetPositionX() + (rand() % 10 - 5);
                            posY = me->GetPositionY() + (rand() % 10 - 5);
                            posZ = me->GetPositionZ() + (rand() % 10 - 5);
                            posO = me->GetOrientation();
                            creatureList.push_front(me->SummonCreature(80010, posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 1: // wait
                    if (creepsAlive == 0 && waiting == false) {
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "Waiting for Wave 2"); 
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(60000));
                        waiting = true;
                    }
                    break;
                case 2:
                    if (creepsAlive == 0) {
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "Spawning Wave 2");
                        for (size_t i = 0; i < 10; i++)
                        {
                            posX = me->GetPositionX() + (rand() % 10 - 5);
                            posY = me->GetPositionY() + (rand() % 10 - 5);
                            posZ = me->GetPositionZ() + (rand() % 10 - 5);
                            posO = me->GetOrientation();
                            creatureList.push_front(me->SummonCreature(80010, posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 3: //wait
                    if (creepsAlive == 0 && waiting == false) {
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "Waiting for Wave 3");
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(60000));
                        waiting = true;
                    }
                    break;
                case 4:
                    if (creepsAlive == 0) {
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "Spawning Wave 3");
                        for (size_t i = 0; i < 10; i++)
                        {
                            posX = me->GetPositionX() + (rand() % 10 - 5);
                            posY = me->GetPositionY() + (rand() % 10 - 5);
                            posZ = me->GetPositionZ() + (rand() % 10 - 5);
                            posO = me->GetOrientation();
                            creatureList.push_front(me->SummonCreature(80010, posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 5:
                    //end event
                    
                    if (creepsAlive == 0) {
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "Finished Event, respawning Rift in 120 seconds.");
                        eventActive = false;
                        FLR_clear();
                        me->m_Events.AddEvent(new DelayedRiftSpawn(), me->m_Events.CalculateTime(120000));
                        waveNumber++;
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
            sWorld->SendWorldText(LANG_EVENTMESSAGE, "creep killed");
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
            if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false)) {
                if (waveNumber == 0 && creepsAlive == 0 && !riftSpawned) {
                    // create rift

                    
                    QueryResult qr = WorldDatabase.Query("SELECT guid FROM creature WHERE id1 = 90018 ORDER BY RAND() LIMIT 1");
                    uint32 targetGUID = (*qr)[0].Get<uint32>();
                    uint32 myGUID = me->GetGUID().GetRawValue();

                    std::ostringstream ss;
                    ss << "Try spwan: GUID: " << targetGUID << ", RiftNo: " << riftNumber;
                    sWorld->SendWorldText(LANG_EVENTMESSAGE, ss.str().c_str());

                    if (myGUID == targetGUID && riftNumber == 0) {
                        riftCreature = me->SummonCreature(90017, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        FLR_init();
                    }
                }
                else if (waveNumber == 5 && creepsAlive == 0 && riftSpawned) {
                    //clean rift
                    riftCreature->DespawnOrUnsummon(0);
                    riftSpawned = false;
                    // todo loot
                }
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

    new FLRiftsCreatureRift();
    new FLRiftsCreatureTrash();
    new FLRiftsCreatureSpawner();
}
