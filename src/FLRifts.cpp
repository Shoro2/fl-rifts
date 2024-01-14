/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedAI/ScriptedCreature.h"
#include "Creature.h"
#include "Map.h"
#include "ObjectMgr.h"

bool eventActive = false, riftSpawned = false, waiting = false, debug_flrifts = true, onlyOnce = true;
float posX, posY, posZ, posO;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
std::list<uint32> creatureList = {};
Creature* riftCreature;
const uint32 WAVE_COUNTER_WORLD_STATE_ID = 1000;



void FLR_init() {
    if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "init Rift");
    //reset stuff
    creepsAlive = 0;
    waveNumber = 0;
    creatureList = {};
    eventActive = true;
    riftSpawned = true;
    onlyOnce = true;
}

class DelayedRiftSpawn : public BasicEvent
{
public:
    DelayedRiftSpawn() : BasicEvent() {  }

    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/)
    {
        if(debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "DelayedRiftSpawn");
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
        if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "DelayedWaveSpawn");
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

        void UpdateWorldState(Creature* creature)
        {
            uint32 creatureZoneId = creature->GetZoneId();
            Map* map = creature->GetMap();
            Map::PlayerList const& players = map->GetPlayers();
            for (const auto& itr : players) {
                Player* player = itr.GetSource();
                if (!player) continue; // Skip if null

                // Check if the player is in the same zone as the creature
                if (player->GetZoneId() == creatureZoneId) {
                    // Perform your actions with the player here
                    // Example: player->DoSomething();
                    player->SendUpdateWorldState(WAVE_COUNTER_WORLD_STATE_ID, waveNumber);
                }
            }

        }

        void SummonedCreatureDespawn(Creature* /*summon*/) override {
            if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "Killed trash");
            creepsAlive--;
        }

        void summonRiftCreeps() {
            posX = me->GetPositionX() + (rand() % 40 - 20);
            posY = me->GetPositionY() + (rand() % 40 - 20);
            posZ = me->GetPositionZ();
            posO = me->GetOrientation();
            uint8 npc_spawn_1, npc_spawn_2, npc_spawn_3, npc_spawn_4;
            try {
                switch (me->GetEntry()) {
                case 90017: //shadow
                    npc_spawn_1 = sConfigMgr->GetOption<uint8>("FLRifts.shadow1", 80027);
                    npc_spawn_2 = sConfigMgr->GetOption<uint8>("FLRifts.shadow2", 80028);
                    npc_spawn_3 = sConfigMgr->GetOption<uint8>("FLRifts.shadow3", 80029);
                    npc_spawn_4 = sConfigMgr->GetOption<uint8>("FLRifts.shadow4", 80035); 
                    break;
                case 90016: //fire
                    npc_spawn_1 = sConfigMgr->GetOption<uint8>("FLRifts.fire1", 80039);
                    npc_spawn_2 = sConfigMgr->GetOption<uint8>("FLRifts.fire2", 80041);
                    npc_spawn_3 = sConfigMgr->GetOption<uint8>("FLRifts.fire3", 80047);
                    npc_spawn_4 = sConfigMgr->GetOption<uint8>("FLRifts.fire4", 80017);
                    break;
                case 90015: //air
                    npc_spawn_1 = sConfigMgr->GetOption<uint8>("FLRifts.air1", 80031);
                    npc_spawn_2 = sConfigMgr->GetOption<uint8>("FLRifts.air2", 80032);
                    npc_spawn_3 = sConfigMgr->GetOption<uint8>("FLRifts.air3", 80033);
                    npc_spawn_4 = sConfigMgr->GetOption<uint8>("FLRifts.air4", 80037);

                    break;
                case 90014: //water
                    npc_spawn_1 = sConfigMgr->GetOption<uint8>("FLRifts.water1", 80043);
                    npc_spawn_2 = sConfigMgr->GetOption<uint8>("FLRifts.water2", 80044);
                    npc_spawn_3 = sConfigMgr->GetOption<uint8>("FLRifts.water3", 80045);
                    npc_spawn_4 = sConfigMgr->GetOption<uint8>("FLRifts.water4", 80046);
                    break;
                }
                riftSummon = me->SummonCreature(RAND(npc_spawn_1, npc_spawn_2, npc_spawn_3, npc_spawn_4), posX, posY, posZ, posO, TEMPSUMMON_CORPSE_DESPAWN);
                summonFloorZ = riftSummon->GetFloorZ();
                riftSummon->NearTeleportTo(riftSummon->GetPositionX(), riftSummon->GetPositionY(), summonFloorZ, false);
                riftSummon->UpdateGroundPositionZ(riftSummon->GetPositionX(), riftSummon->GetPositionY(), summonFloorZ);
                creepsAlive++;
            }
            catch (const std::exception& e) {
                LOG_ERROR("scripts", "Error in summonRiftCreeps : % s", e.what());
            }

            
        }

        void UpdateAI(uint32 /*diff*/) override {
            if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false) && eventActive) {
                
                switch (waveNumber) {
                case 0:
                    if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "Spawning w1");
                    //start spawning
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            summonRiftCreeps();
                        }
                        waveNumber++;
                        UpdateWorldState(me);
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
                            summonRiftCreeps();
                        }
                        waveNumber++;
                        UpdateWorldState(me);
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
                        posZ = (me->GetPositionZ()) + 5;
                        posO = me->GetOrientation();
                        switch (me->GetEntry()) {
                        case 90017:
                            me->SummonCreature(80036, posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN);
                            break;
                        case 90016:
                            me->SummonCreature(RAND(80040, 80042), posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN);
                            break;
                        case 90015:
                            me->SummonCreature(RAND(80034, 80038), posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN);
                            break;
                        case 90014:
                            me->SummonCreature(RAND(80048, 80049), posX, posY, posZ, posO, TEMPSUMMON_MANUAL_DESPAWN);
                            break;
                        default:
                            break;
                        }
                        creepsAlive++;

                        waveNumber++;
                        UpdateWorldState(me);
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
        float summonFloorZ;
        Creature* riftSummon;

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new FLRiftsCreatureRiftAI(creature);
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

        void UpdateWorldState(Creature* creature)
        {
            uint32 creatureZoneId = creature->GetZoneId();
            Map* map = creature->GetMap();
            Map::PlayerList const& players = map->GetPlayers();
            for (const auto& itr : players) {
                Player* player = itr.GetSource();
                if (!player) continue; // Skip if null

                // Check if the player is in the same zone as the creature
                if (player->GetZoneId() == creatureZoneId) {
                    // Perform your actions with the player here
                    // Example: player->DoSomething();
                    player->SendUpdateWorldState(WAVE_COUNTER_WORLD_STATE_ID, waveNumber);
                }
            }
            
        }

        void UpdateAI(uint32 /*diff*/) override {
            if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false)) {
                if (waveNumber == 0 && creepsAlive == 0 && !riftSpawned && onlyOnce) {
                    onlyOnce = false;
                    // create rift
                    UpdateWorldState(me);
                    Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                    if (!PlayerList.IsEmpty())
                        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                            i->GetSource()->PlayerTalkClass->SendPointOfInterest(1100);
                    if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "Query MySQL");
                    
                    QueryResult qr = WorldDatabase.Query("SELECT guid FROM creature WHERE id1 = 90018 ORDER BY RAND() LIMIT 1");

                    uint32 targetGUID = (*qr)[0].Get<uint32>();
                    uint32 targetMap = me->GetMap()->GetId();
                    std::ostringstream ss;
                    ss << "guid: " << targetGUID << "map: " << targetMap;
                    sWorld->SendWorldText(LANG_EVENTMESSAGE, ss.str().c_str());
                    Creature* targetSummoner = ObjectAccessor::GetSpawnedCreatureByDBGUID(targetMap, targetGUID);
                    if (!targetSummoner) {
                        if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "No Summoner");
                        onlyOnce = true;
                        return;
                    }
                    try {
                        riftCreature = targetSummoner->SummonCreature(90017, targetSummoner->GetPositionX(), targetSummoner->GetPositionY(), ((targetSummoner->GetPositionZ()) + 5), targetSummoner->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                    }
                    catch (const std::exception& e) {
                        LOG_ERROR("scripts", "Error in FLRiftsCreatureSpawner:UpdateAI : % s", e.what());
                    }
                    if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "Spawned Rift");

                    std::ostringstream sd;
                    sd << "A new rift has formed, find it and defeat all waves of Demons and fight the big boss to receive additional loot!";
                    sWorld->SendWorldText(LANG_EVENTMESSAGE, sd.str().c_str());
                    FLR_init();


                    
                }
                else if (waveNumber == 6 && creepsAlive == 0 && riftSpawned) {
                    //clean rift
                    if (debug_flrifts) sWorld->SendWorldText(LANG_EVENTMESSAGE, "Close Rift");
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
    new FLRiftsCreatureSpawner();
}
