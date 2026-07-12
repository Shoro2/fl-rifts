/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Containers.h"
#include "ScriptedAI/ScriptedCreature.h"
#include "Creature.h"
#include "Map.h"

#include <vector>

namespace
{
using Acore::Containers::SelectRandomContainerElement;

constexpr uint32 NPC_SHADOW_RIFT = 90017;
constexpr uint32 NPC_RIFT_SPAWN_LOCATION = 90018;
constexpr uint32 RIFT_RETRY_DELAY_MS = 5000;
constexpr uint32 RIFT_WAVE_SIZE = 10;
}

bool eventActive = false, riftSpawned = false, waiting = false, debug_flrifts = true, onlyOnce = true;
float posX, posY, posZ, posO;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
std::list<uint32> creatureList = {};
Creature* riftCreature;
const uint32 WAVE_COUNTER_WORLD_STATE_ID = 1000;



void FLR_init() {
    if (debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "init Rift");
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
        if(debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "DelayedRiftSpawn");
        waveNumber = 0;
        eventActive = true;
        onlyOnce = true;
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
        if (debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "DelayedWaveSpawn");
        waveNumber++;
        waiting = false;
        return true;
    }

private:

};

class DelayedWaveRetry : public BasicEvent
{
public:
    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        eventActive = true;
        return true;
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
            if (debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "Killed trash");
            creepsAlive--;
        }

        bool SummonRiftCreep()
        {
            float summonX = me->GetPositionX() + (rand() % 40 - 20);
            float summonY = me->GetPositionY() + (rand() % 40 - 20);
            float summonZ = me->GetPositionZ();
            float summonOrientation = me->GetOrientation();
            uint32 npcSpawn1;
            uint32 npcSpawn2;
            uint32 npcSpawn3;
            uint32 npcSpawn4;

            switch (me->GetEntry())
            {
                case NPC_SHADOW_RIFT:
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.shadow1", 80027);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.shadow2", 80028);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.shadow3", 80029);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.shadow4", 80035);
                    break;
                case 90016: // fire
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire1", 80039);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire2", 80041);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire3", 80047);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire4", 80017);
                    break;
                case 90015: // air
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air1", 80031);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air2", 80032);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air3", 80033);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air4", 80037);
                    break;
                case 90014: // water
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water1", 80043);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water2", 80044);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water3", 80045);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water4", 80046);
                    break;
                default:
                    LOG_ERROR("module.fl-rifts",
                        "Rift creature entry {} has no wave configuration.",
                        me->GetEntry());
                    return false;
            }

            Creature* riftSummon = me->SummonCreature(
                RAND(npcSpawn1, npcSpawn2, npcSpawn3, npcSpawn4),
                summonX, summonY, summonZ, summonOrientation,
                TEMPSUMMON_CORPSE_DESPAWN);
            if (!riftSummon)
                return false;

            float summonFloorZ = riftSummon->GetFloorZ();
            riftSummon->NearTeleportTo(
                riftSummon->GetPositionX(), riftSummon->GetPositionY(),
                summonFloorZ, false);
            riftSummon->UpdateGroundPositionZ(
                riftSummon->GetPositionX(), riftSummon->GetPositionY(),
                summonFloorZ);
            creepsAlive++;
            return true;
        }

        bool SummonRiftWave()
        {
            uint32 summonedCount = 0;
            for (uint32 i = 0; i < RIFT_WAVE_SIZE; ++i)
                if (SummonRiftCreep())
                    ++summonedCount;

            if (!summonedCount)
            {
                LOG_ERROR("module.fl-rifts",
                    "Failed to summon rift wave {} on map {}; "
                    "retrying in {} ms.",
                    waveNumber, me->GetMapId(), RIFT_RETRY_DELAY_MS);
                return false;
            }

            if (summonedCount < RIFT_WAVE_SIZE)
            {
                LOG_WARN("module.fl-rifts",
                    "Rift wave {} summoned only {} of {} creatures on map {}.",
                    waveNumber, summonedCount, RIFT_WAVE_SIZE, me->GetMapId());
            }

            return true;
        }

        void ScheduleRiftRetry()
        {
            eventActive = false;
            me->m_Events.AddEventAtOffset(
                new DelayedWaveRetry(), Milliseconds(RIFT_RETRY_DELAY_MS));
        }

        void UpdateAI(uint32 /*diff*/) override {
            if (sConfigMgr->GetOption<bool>("FLRifts.Enable", false) && eventActive) {
                
                switch (waveNumber) {
                case 0:
                    if (debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "Spawning w1");
                    //start spawning
                    if (creepsAlive == 0) {
                        if (!SummonRiftWave())
                        {
                            ScheduleRiftRetry();
                            break;
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
                        if (!SummonRiftWave())
                        {
                            ScheduleRiftRetry();
                            break;
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
                        ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "The Rift was closed, next one approaching in 30 minutes.");
                        eventActive = false;
                        waveNumber++;
                    }

                    break;
                default:
                    break;
                }
            }
        }



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
                    /*
                    Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                    if (!PlayerList.IsEmpty()){

                        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        {
                            i->GetSource()->PlayerTalkClass->SendPointOfInterest(1100);
                        }
                    }
                    */
                    std::vector<Creature*> spawnLocations;
                    auto& loadedCreatures =
                        me->GetMap()->GetCreatureBySpawnIdStore();
                    for (auto const& pair : loadedCreatures)
                    {
                        Creature* spawnLocation = pair.second;
                        if (!spawnLocation || !spawnLocation->IsAlive())
                            continue;

                        if (spawnLocation->GetEntry() ==
                            NPC_RIFT_SPAWN_LOCATION)
                            spawnLocations.push_back(spawnLocation);
                    }

                    if (spawnLocations.empty())
                    {
                        LOG_ERROR("module.fl-rifts",
                            "No loaded rift spawn location (entry {}) found on "
                            "map {}; "
                            "retrying in {} ms.",
                            NPC_RIFT_SPAWN_LOCATION, me->GetMapId(),
                            RIFT_RETRY_DELAY_MS);
                        me->m_Events.AddEventAtOffset(
                            new DelayedRiftSpawn(),
                            Milliseconds(RIFT_RETRY_DELAY_MS));
                        return;
                    }

                    Creature* targetSummoner =
                        SelectRandomContainerElement(spawnLocations);
                    ObjectGuid::LowType targetSpawnId =
                        targetSummoner->GetSpawnId();
                    uint32 targetMapId = targetSummoner->GetMapId();
                    LOG_DEBUG("module.fl-rifts",
                        "Selected rift spawn {} on map {}.",
                        targetSpawnId, targetMapId);
                    riftCreature = targetSummoner->SummonCreature(
                        NPC_SHADOW_RIFT,
                        targetSummoner->GetPositionX(),
                        targetSummoner->GetPositionY(),
                        targetSummoner->GetPositionZ() + 5,
                        targetSummoner->GetOrientation(),
                        TEMPSUMMON_MANUAL_DESPAWN);
                    if (!riftCreature)
                    {
                        LOG_ERROR("module.fl-rifts",
                            "Failed to summon a rift at spawn {} on map {}; "
                            "retrying in {} ms.",
                            targetSpawnId, targetMapId, RIFT_RETRY_DELAY_MS);
                        me->m_Events.AddEventAtOffset(
                            new DelayedRiftSpawn(),
                            Milliseconds(RIFT_RETRY_DELAY_MS));
                        return;
                    }

                    if (debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "Spawned Rift");

                    std::ostringstream sd;
                    sd << "A new rift has formed, find it and defeat all waves of Demons and fight the big boss to receive additional loot!";
                    ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, sd.str().c_str());
                    FLR_init();


                    
                }
                else if (waveNumber == 6 && creepsAlive == 0 && riftSpawned) {
                    //clean rift
                    if (debug_flrifts) ChatHandler(nullptr).SendWorldText(LANG_EVENTMESSAGE, "Close Rift");
                    riftCreature->DespawnOrUnsummon(Milliseconds(0));
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
