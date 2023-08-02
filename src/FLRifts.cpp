/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedAI/ScriptedCreature.h"

bool eventActive = false, riftSpawned = false, waiting = false;
float posX, posY, posZ + 5, posO;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
std::list<TempSummon*> creatureList = {};
Creature* riftCreature;



void FLR_init() {
    //reset stuff
    creepsAlive = 0;
    waveNumber = 0;
    creatureList = {};
    eventActive = true;
    riftSpawned = true;
    sWorld->SendWorldText(LANG_EVENTMESSAGE, "A new rift has formed in the Land of Exobeast. Deafeat all waves of Demons and fight the big boss to receive additional loot!");
}




class DelayedRiftSpawn : public BasicEvent
{
public:
    DelayedRiftSpawn() : BasicEvent() { }

    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/)
    {
        waveNumber = 0;
        eventActive = true;
        return true;
    }

private:

};

class DelayedWaveSpawn : public BasicEvent
{
public:
    DelayedWaveSpawn() : BasicEvent() { }

    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/)
    {
        waveNumber++;
        waiting = false;
        return true;
    }

private:

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
        // fire: 80039, 80041, 80047, 80017 | 80040, 80042
        // shadow: 80027, 80028, 80029, 80035 | 80030, 80036
        // air: 80043, 80044, 80045, 80046 | 80048, 80049
        // water: 80031, 80032, 80033, 80037 | 80034, 80038



        void UpdateAI(uint32 /*diff*/) {
            if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false) && eventActive) {
                switch (waveNumber) {
                case 0:
                    //start spawning
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            posX = me->GetPositionX() + (rand() % 40 - 20);
                            posY = me->GetPositionY() + (rand() % 40 - 20);
                            posZ + 5 = me->GetPositionZ();
                            posO = me->GetOrientation();
                            switch (me->GetEntry()) {
                            case 90017:
                                creatureList.push_front(me->SummonCreature(RAND(80027, 80028, 80029, 80035), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            case 90016:
                                creatureList.push_front(me->SummonCreature(RAND(80039, 80041, 80047, 80017), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            case 90015:
                                creatureList.push_front(me->SummonCreature(RAND(80031, 80032, 80033, 80037), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            case 90014:
                                creatureList.push_front(me->SummonCreature(RAND(80043, 80044, 80045, 80046), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            default:
                                break;
                            }

                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 1: // wait
                    if (creepsAlive == 0 && waiting == false) {
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(10000));
                        waiting = true;
                    }
                    break;
                case 2:
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            posX = me->GetPositionX() + (rand() % 40 - 20);
                            posY = me->GetPositionY() + (rand() % 40 - 20);
                            posZ + 5 = me->GetPositionZ();
                            posO = me->GetOrientation();
                            switch (me->GetEntry()) {
                            case 90017:
                                creatureList.push_front(me->SummonCreature(RAND(80027, 80028, 80029, 80035), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            case 90016:
                                creatureList.push_front(me->SummonCreature(RAND(80039, 80041, 80047, 80017), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            case 90015:
                                creatureList.push_front(me->SummonCreature(RAND(80031, 80032, 80033, 80037), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            case 90014:
                                creatureList.push_front(me->SummonCreature(RAND(80043, 80044, 80045, 80046), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                                break;
                            default:
                                break;
                            }
                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 3: //wait
                    if (creepsAlive == 0 && waiting == false) {
                        me->m_Events.AddEvent(new DelayedWaveSpawn(), me->m_Events.CalculateTime(10000));
                        waiting = true;
                    }
                    break;
                case 4:
                    if (creepsAlive == 0) {
                        posX = me->GetPositionX() + (rand() % 40 - 20);
                        posY = me->GetPositionY() + (rand() % 40 - 20);
                        posZ + 5 = me->GetPositionZ();
                        posO = me->GetOrientation();
                        switch (me->GetEntry()) {
                        case 90017:
                            creatureList.push_front(me->SummonCreature(80036, posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            break;
                        case 90016:
                            creatureList.push_front(me->SummonCreature(RAND(80040, 80042), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            break;
                        case 90015:
                            creatureList.push_front(me->SummonCreature(RAND(80034, 80038), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            break;
                        case 90014:
                            creatureList.push_front(me->SummonCreature(RAND(80048, 80049), posX, posY, posZ + 5, posO, TEMPSUMMON_MANUAL_DESPAWN));
                            break;
                        default:
                            break;
                        }
                        creepsAlive++;

                        waveNumber++;
                    }
                    break;
                case 5:
                    //end event
                    if (creepsAlive == 0) {
                        sWorld->SendWorldText(LANG_EVENTMESSAGE, "The Rift was closed, next one approaching in 30 minutes.");
                        eventActive = false;
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
                    Creature* targetSummoner = ObjectAccessor::GetSpawnedCreatureByDBGUID(727, targetGUID);

                    riftCreature = targetSummoner->SummonCreature(90017, targetSummoner->GetPositionX(), targetSummoner->GetPositionY(), targetSummoner->GetPositionZ(), targetSummoner->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                    FLR_init();
                }
                else if (waveNumber == 6 && creepsAlive == 0 && riftSpawned) {
                    //clean rift
                    riftCreature->DespawnOrUnsummon(0);
                    riftSpawned = false;
                    me->m_Events.AddEvent(new DelayedRiftSpawn(), me->m_Events.CalculateTime(1800000));
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
    new FLRiftsCreatureRift();
    new FLRiftsCreatureTrash();
    new FLRiftsCreatureSpawner();
}
