/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Chat.h"
#include "Config.h"
#include "Containers.h"
#include "Creature.h"
#include "GameObject.h"
#include "GameTime.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedAI/ScriptedCreature.h"
#include "StringFormat.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{
using Acore::Containers::SelectRandomContainerElement;

constexpr uint32 NPC_WATER_RIFT = 90014;
constexpr uint32 NPC_AIR_RIFT = 90015;
constexpr uint32 NPC_FIRE_RIFT = 90016;
constexpr uint32 NPC_SHADOW_RIFT = 90017;
constexpr uint32 NPC_RIFT_SPAWN_LOCATION = 90018;

constexpr uint32 GO_AIR_RIFT_VISUAL = 192819;
constexpr uint32 GO_WATER_RIFT_VISUAL = 195706;

constexpr uint32 RIFT_START_DELAY_MS = 2 * MINUTE * IN_MILLISECONDS;
constexpr uint32 RIFT_RESPAWN_DELAY_MS = 30 * MINUTE * IN_MILLISECONDS;
constexpr uint32 RIFT_RETRY_DELAY_MS = 5 * IN_MILLISECONDS;
constexpr uint32 RIFT_WAVE_DELAY_MS = 10 * IN_MILLISECONDS;
constexpr uint32 RIFT_WAVE_SIZE = 10;

constexpr uint32 RIFT_POI_FLAGS = 99;
constexpr uint32 RIFT_POI_ICON = 7;
constexpr uint32 RIFT_POI_IMPORTANCE = 0;
constexpr char RIFT_POI_NAME[] = "Active Rift";
constexpr char RIFT_ADDON_PREFIX[] = "FLRIFTS";
constexpr char RIFT_ADDON_REQUEST[] = "FLRIFTS\tREQUEST";
constexpr uint32 WAVE_COUNTER_WORLD_STATE_ID = 1000;

enum class RiftPhase : uint8
{
    Inactive,
    Countdown,
    Active
};

struct RiftEventState
{
    RiftPhase Phase = RiftPhase::Inactive;
    ObjectGuid ControllerGuid;
    ObjectGuid RiftGuid;
    uint32 MapId = 0;
    uint32 ZoneId = 0;
    float PositionX = 0.0f;
    float PositionY = 0.0f;
    TimePoint StartTime;
    std::unordered_set<ObjectGuid::LowType> MarkedPlayers;
};

RiftEventState riftState;
std::recursive_mutex riftStateMutex;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
bool waitingForNextWave = false;
bool waveRetryPending = false;
bool riftSpawnUnlocked = true;
bool debugFLRifts = true;

bool IsPlayerInRiftZone(Player const* player, uint32 zoneId)
{
    return riftState.Phase != RiftPhase::Inactive &&
        player->GetMapId() == riftState.MapId &&
        zoneId == riftState.ZoneId;
}

void SendRiftAddonMessage(Player* player, std::string const& payload)
{
    std::string message = RIFT_ADDON_PREFIX;
    message += '\t';
    message += payload;

    WorldPacket data;
    ChatHandler::BuildChatPacket(
        data, CHAT_MSG_WHISPER, LANG_ADDON, player, player, message);
    player->GetSession()->SendPacket(&data);
}

uint32 GetRemainingCountdownSeconds()
{
    auto remaining = std::chrono::duration_cast<Milliseconds>(
        riftState.StartTime - GameTime::Now()).count();
    return remaining > 0 ? uint32((remaining + 999) / 1000) : 0;
}

uint8 GetDisplayedWave()
{
    if (waveNumber <= 1)
        return 1;
    if (waveNumber <= 3)
        return 2;
    return 3;
}

void SendRiftUiState(Player* player)
{
    switch (riftState.Phase)
    {
        case RiftPhase::Countdown:
            SendRiftAddonMessage(player, Acore::StringFormat(
                "COUNTDOWN|{}", GetRemainingCountdownSeconds()));
            break;
        case RiftPhase::Active:
            SendRiftAddonMessage(player, Acore::StringFormat(
                "WAVE|{}|{}|{}", GetDisplayedWave(), creepsAlive,
                waveNumber >= 4 ? 1 : 0));
            break;
        case RiftPhase::Inactive:
            SendRiftAddonMessage(player, "HIDE");
            break;
    }
}

void SendRiftPointOfInterest(Player* player, bool visible)
{
    float positionX = visible ? riftState.PositionX : player->GetPositionX();
    float positionY = visible ? riftState.PositionY : player->GetPositionY();
    uint32 icon = visible ? RIFT_POI_ICON : 0;
    uint32 flags = visible ? RIFT_POI_FLAGS : 0;
    std::string name = visible ? RIFT_POI_NAME : "";

    WorldPacket data(SMSG_GOSSIP_POI, 21 + name.size());
    data << flags;
    data << positionX;
    data << positionY;
    data << icon;
    data << RIFT_POI_IMPORTANCE;
    data << name;
    player->GetSession()->SendPacket(&data);
}

void ShowRiftMarker(Player* player)
{
    SendRiftPointOfInterest(player, true);
    riftState.MarkedPlayers.insert(player->GetGUID().GetCounter());
}

void HideRiftMarker(Player* player)
{
    if (!riftState.MarkedPlayers.erase(player->GetGUID().GetCounter()))
        return;

    SendRiftPointOfInterest(player, false);
}

void SyncRiftPlayerState(Player* player, uint32 zoneId)
{
    std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
    if (IsPlayerInRiftZone(player, zoneId))
    {
        ShowRiftMarker(player);
        SendRiftUiState(player);
    }
    else
    {
        HideRiftMarker(player);
        SendRiftAddonMessage(player, "HIDE");
    }
}

template <typename Callback>
void ForEachPlayerInRiftZone(Map* map, Callback&& callback)
{
    if (!map || map->GetId() != riftState.MapId)
        return;

    for (auto const& reference : map->GetPlayers())
    {
        Player* player = reference.GetSource();
        if (player && player->GetZoneId() == riftState.ZoneId)
            callback(player);
    }
}

void UpdateWaveWorldState(Creature* creature)
{
    uint32 zoneId = creature->GetZoneId();
    for (auto const& reference : creature->GetMap()->GetPlayers())
    {
        Player* player = reference.GetSource();
        if (player && player->GetZoneId() == zoneId)
        {
            player->SendUpdateWorldState(
                WAVE_COUNTER_WORLD_STATE_ID, waveNumber);
            SendRiftUiState(player);
        }
    }
}

void BeginRiftCountdown(Creature* controller, Creature* rift)
{
    std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
    creepsAlive = 0;
    waveNumber = 0;
    waitingForNextWave = false;
    waveRetryPending = false;

    riftState.Phase = RiftPhase::Countdown;
    riftState.ControllerGuid = controller->GetGUID();
    riftState.RiftGuid = rift->GetGUID();
    riftState.MapId = rift->GetMapId();
    riftState.ZoneId = rift->GetZoneId();
    riftState.PositionX = rift->GetPositionX();
    riftState.PositionY = rift->GetPositionY();
    riftState.StartTime = GameTime::Now() +
        Milliseconds(RIFT_START_DELAY_MS);

    ForEachPlayerInRiftZone(rift->GetMap(), [](Player* player)
    {
        ShowRiftMarker(player);
        SendRiftUiState(player);
    });
}

void ActivateRiftOnSchedule()
{
    std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
    if (riftState.Phase != RiftPhase::Countdown)
        return;

    if (GameTime::Now() < riftState.StartTime)
        return;

    riftState.Phase = RiftPhase::Active;
    LOG_INFO("module.fl-rifts",
        "Rift countdown completed in zone {} on map {}; wave spawning is "
        "now active.", riftState.ZoneId, riftState.MapId);
    ChatHandler(nullptr).SendWorldText(
        LANG_EVENTMESSAGE, "The Rift invasion has begun!");
}

void ClearRiftEvent(Creature* controller)
{
    std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
    ObjectGuid riftGuid = riftState.RiftGuid;
    ForEachPlayerInRiftZone(controller->GetMap(), [](Player* player)
    {
        HideRiftMarker(player);
        SendRiftAddonMessage(player, "HIDE");
    });
    riftState.MarkedPlayers.clear();

    riftState = RiftEventState{};
    creepsAlive = 0;
    waveNumber = 0;
    waitingForNextWave = false;
    waveRetryPending = false;

    if (Creature* rift = ObjectAccessor::GetCreature(*controller, riftGuid))
        rift->DespawnOrUnsummon(Milliseconds(0));
}

class DelayedRiftSpawn : public BasicEvent
{
public:
    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
        riftSpawnUnlocked = true;
        return true;
    }
};

class DelayedWaveSpawn : public BasicEvent
{
public:
    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
        ++waveNumber;
        waitingForNextWave = false;
        return true;
    }
};

class DelayedWaveRetry : public BasicEvent
{
public:
    bool Execute(uint64 /*eventTime*/, uint32 /*updateTime*/) override
    {
        std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
        waveRetryPending = false;
        return true;
    }
};

class FLRiftsCreatureRift : public CreatureScript
{
public:
    FLRiftsCreatureRift() : CreatureScript("FLRiftsCreatureRift") { }

    struct FLRiftsCreatureRiftAI : public ScriptedAI
    {
        FLRiftsCreatureRiftAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->RemoveAllGameObjects();

            uint32 visualEntry;
            switch (me->GetEntry())
            {
                case NPC_AIR_RIFT:
                    visualEntry = GO_AIR_RIFT_VISUAL;
                    break;
                case NPC_WATER_RIFT:
                    visualEntry = GO_WATER_RIFT_VISUAL;
                    break;
                default:
                    return;
            }

            GameObject* visual = me->SummonGameObject(
                visualEntry, me->GetPositionX(), me->GetPositionY(),
                me->GetPositionZ(), me->GetOrientation(), 0.0f, 0.0f, 0.0f,
                0.0f, 0);
            if (!visual)
            {
                LOG_ERROR("module.fl-rifts",
                    "Failed to summon Rift visual gameobject {} for Rift {}.",
                    visualEntry, me->GetEntry());
                return;
            }

            visual->SetGameObjectFlag(GO_FLAG_NOT_SELECTABLE);
        }

        void SummonedCreatureDespawn(Creature* /*summon*/) override
        {
            std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
            if (creepsAlive > 0)
                --creepsAlive;
            else
                LOG_WARN("module.fl-rifts",
                    "Received a summon despawn with no tracked rift "
                    "creatures.");

            UpdateWaveWorldState(me);
        }

        bool SummonRiftCreep()
        {
            float summonX = me->GetPositionX() + frand(-20.0f, 20.0f);
            float summonY = me->GetPositionY() + frand(-20.0f, 20.0f);
            float summonZ = me->GetPositionZ();
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
                case NPC_FIRE_RIFT:
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire1", 80039);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire2", 80041);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire3", 80047);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.fire4", 80017);
                    break;
                case NPC_AIR_RIFT:
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air1", 80043);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air2", 80044);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air3", 80045);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.air4", 80046);
                    break;
                case NPC_WATER_RIFT:
                    npcSpawn1 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water1", 80031);
                    npcSpawn2 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water2", 80032);
                    npcSpawn3 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water3", 80033);
                    npcSpawn4 = sConfigMgr->GetOption<uint32>(
                        "FLRifts.water4", 80175);
                    break;
                default:
                    LOG_ERROR("module.fl-rifts",
                        "Rift creature entry {} has no wave configuration.",
                        me->GetEntry());
                    return false;
            }

            Creature* summon = me->SummonCreature(
                RAND(npcSpawn1, npcSpawn2, npcSpawn3, npcSpawn4),
                summonX, summonY, summonZ, me->GetOrientation(),
                TEMPSUMMON_CORPSE_DESPAWN);
            if (!summon)
                return false;

            float floorZ = summon->GetFloorZ();
            summon->NearTeleportTo(
                summon->GetPositionX(), summon->GetPositionY(), floorZ,
                false);
            summon->UpdateGroundPositionZ(
                summon->GetPositionX(), summon->GetPositionY(), floorZ);
            ++creepsAlive;
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
                    "Failed to summon rift wave {} on map {}; retrying in "
                    "{} ms.", waveNumber, me->GetMapId(),
                    RIFT_RETRY_DELAY_MS);
                return false;
            }

            if (summonedCount < RIFT_WAVE_SIZE)
            {
                LOG_WARN("module.fl-rifts",
                    "Rift wave {} summoned only {} of {} creatures on map {}.",
                    waveNumber, summonedCount, RIFT_WAVE_SIZE,
                    me->GetMapId());
            }

            return true;
        }

        bool SummonRiftBoss()
        {
            uint32 bossEntry;
            switch (me->GetEntry())
            {
                case NPC_SHADOW_RIFT:
                    bossEntry = RAND(
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.shadowboss1", 80036),
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.shadowboss2", 80037));
                    break;
                case NPC_FIRE_RIFT:
                    bossEntry = RAND(
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.fireboss1", 80040),
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.fireboss2", 80042));
                    break;
                case NPC_AIR_RIFT:
                    bossEntry = RAND(
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.airboss1", 80048),
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.airboss2", 80049));
                    break;
                case NPC_WATER_RIFT:
                    bossEntry = RAND(
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.waterboss1", 80034),
                        sConfigMgr->GetOption<uint32>(
                            "FLRifts.waterboss2", 80038));
                    break;
                default:
                    return false;
            }

            Creature* boss = me->SummonCreature(
                bossEntry,
                me->GetPositionX() + frand(-20.0f, 20.0f),
                me->GetPositionY() + frand(-20.0f, 20.0f),
                me->GetPositionZ() + 5.0f, me->GetOrientation(),
                TEMPSUMMON_MANUAL_DESPAWN);
            if (!boss)
                return false;

            ++creepsAlive;
            return true;
        }

        void ScheduleWaveRetry()
        {
            waveRetryPending = true;
            me->m_Events.AddEventAtOffset(
                new DelayedWaveRetry(), Milliseconds(RIFT_RETRY_DELAY_MS));
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
            if (!sConfigMgr->GetOption<bool>("FLRifts.Enable", false) ||
                riftState.Phase != RiftPhase::Active || waveRetryPending)
                return;

            switch (waveNumber)
            {
                case 0:
                case 2:
                    if (creepsAlive == 0)
                    {
                        if (!SummonRiftWave())
                        {
                            ScheduleWaveRetry();
                            break;
                        }

                        ++waveNumber;
                        UpdateWaveWorldState(me);
                    }
                    break;
                case 1:
                case 3:
                    if (creepsAlive == 0 && !waitingForNextWave)
                    {
                        me->m_Events.AddEventAtOffset(
                            new DelayedWaveSpawn(),
                            Milliseconds(RIFT_WAVE_DELAY_MS));
                        waitingForNextWave = true;
                    }
                    break;
                case 4:
                    if (creepsAlive == 0)
                    {
                        if (!SummonRiftBoss())
                        {
                            LOG_ERROR("module.fl-rifts",
                                "Failed to summon the rift boss on map {}; "
                                "retrying in {} ms.", me->GetMapId(),
                                RIFT_RETRY_DELAY_MS);
                            ScheduleWaveRetry();
                            break;
                        }

                        ++waveNumber;
                        UpdateWaveWorldState(me);
                    }
                    break;
                case 5:
                    if (creepsAlive == 0)
                    {
                        ChatHandler(nullptr).SendWorldText(
                            LANG_EVENTMESSAGE,
                            "The Rift was closed, next one approaching in "
                            "30 minutes.");
                        ++waveNumber;
                    }
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new FLRiftsCreatureRiftAI(creature);
    }
};

class FLRiftsCreatureSpawner : public CreatureScript
{
public:
    FLRiftsCreatureSpawner() : CreatureScript("FLRiftsCreatureSpawner") { }

    struct FLRiftsCreatureSpawnerAI : public ScriptedAI
    {
        FLRiftsCreatureSpawnerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
            if (riftState.Phase == RiftPhase::Inactive ||
                summon->GetGUID() != riftState.RiftGuid)
                return;

            LOG_WARN("module.fl-rifts",
                "Active Rift {} despawned before event completion; "
                "resetting the event.", summon->GetGUID().ToString());
            ClearRiftEvent(me);
            riftSpawnUnlocked = false;
            ScheduleSpawnRetry(RIFT_RETRY_DELAY_MS);
        }

        void ScheduleSpawnRetry(uint32 delay)
        {
            me->m_Events.AddEventAtOffset(
                new DelayedRiftSpawn(), Milliseconds(delay));
        }

        void SpawnRift()
        {
            riftSpawnUnlocked = false;

            std::vector<Creature*> spawnLocations;
            auto& loadedCreatures = me->GetMap()->GetCreatureBySpawnIdStore();
            for (auto const& pair : loadedCreatures)
            {
                Creature* spawnLocation = pair.second;
                if (spawnLocation && spawnLocation->IsAlive() &&
                    spawnLocation->GetEntry() == NPC_RIFT_SPAWN_LOCATION)
                    spawnLocations.push_back(spawnLocation);
            }

            if (spawnLocations.empty())
            {
                LOG_ERROR("module.fl-rifts",
                    "No loaded rift spawn location (entry {}) found on map "
                    "{}; retrying in {} ms.", NPC_RIFT_SPAWN_LOCATION,
                    me->GetMapId(), RIFT_RETRY_DELAY_MS);
                ScheduleSpawnRetry(RIFT_RETRY_DELAY_MS);
                return;
            }

            std::vector<uint32> riftEntries;
            if (sConfigMgr->GetOption<bool>("FLRifts.Element.Shadow", true))
                riftEntries.push_back(NPC_SHADOW_RIFT);
            if (sConfigMgr->GetOption<bool>("FLRifts.Element.Fire", true))
                riftEntries.push_back(NPC_FIRE_RIFT);
            if (sConfigMgr->GetOption<bool>("FLRifts.Element.Air", true))
                riftEntries.push_back(NPC_AIR_RIFT);
            if (sConfigMgr->GetOption<bool>("FLRifts.Element.Water", true))
                riftEntries.push_back(NPC_WATER_RIFT);
            if (riftEntries.empty())
                riftEntries.push_back(NPC_SHADOW_RIFT);

            uint32 riftEntry = SelectRandomContainerElement(riftEntries);

            Creature* target = SelectRandomContainerElement(spawnLocations);
            Creature* rift = target->SummonCreature(
                riftEntry, target->GetPositionX(),
                target->GetPositionY(), target->GetPositionZ() + 5.0f,
                target->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
            if (!rift)
            {
                LOG_ERROR("module.fl-rifts",
                    "Failed to summon a rift at spawn {} on map {}; "
                    "retrying in {} ms.", target->GetSpawnId(),
                    target->GetMapId(), RIFT_RETRY_DELAY_MS);
                ScheduleSpawnRetry(RIFT_RETRY_DELAY_MS);
                return;
            }

            LOG_INFO("module.fl-rifts",
                "Spawned rift {} at spawn {} in zone {} on map {}; waves "
                "begin in {} seconds.", rift->GetGUID().ToString(),
                target->GetSpawnId(), rift->GetZoneId(), rift->GetMapId(),
                RIFT_START_DELAY_MS / IN_MILLISECONDS);
            ChatHandler(nullptr).SendWorldText(
                LANG_EVENTMESSAGE,
                "A new Rift has formed. Its invasion begins in two minutes!");
            BeginRiftCountdown(me, rift);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            std::lock_guard<std::recursive_mutex> lock(riftStateMutex);
            if (!sConfigMgr->GetOption<bool>("FLRifts.Enable", false))
                return;

            if (riftState.Phase == RiftPhase::Inactive)
            {
                if (riftSpawnUnlocked && waveNumber == 0 && creepsAlive == 0)
                    SpawnRift();
                return;
            }

            if (riftState.ControllerGuid != me->GetGUID())
                return;

            if (riftState.Phase == RiftPhase::Active &&
                waveNumber == 6 && creepsAlive == 0)
            {
                if (debugFLRifts)
                    LOG_DEBUG("module.fl-rifts", "Closing completed Rift.");

                ClearRiftEvent(me);
                ScheduleSpawnRetry(RIFT_RESPAWN_DELAY_MS);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new FLRiftsCreatureSpawnerAI(creature);
    }
};

class FLRiftsWorldScript : public WorldScript
{
public:
    FLRiftsWorldScript() : WorldScript(
        "FLRiftsWorldScript", { WORLDHOOK_ON_UPDATE })
    {
    }

    void OnUpdate(uint32 /*diff*/) override
    {
        ActivateRiftOnSchedule();
    }
};

class FLRiftsPlayerScript : public PlayerScript
{
public:
    FLRiftsPlayerScript() : PlayerScript("FLRiftsPlayerScript",
        { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_UPDATE_ZONE,
            PLAYERHOOK_ON_MAP_CHANGED,
            PLAYERHOOK_ON_BEFORE_SEND_CHAT_MESSAGE })
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        SyncRiftPlayerState(player, player->GetZoneId());
    }

    void OnPlayerUpdateZone(
        Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        SyncRiftPlayerState(player, newZone);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        SyncRiftPlayerState(player, player->GetZoneId());
    }

    void OnPlayerBeforeSendChatMessage(
        Player* player, uint32& /*type*/, uint32& lang,
        std::string& message) override
    {
        if (lang == LANG_ADDON && message == RIFT_ADDON_REQUEST)
            SyncRiftPlayerState(player, player->GetZoneId());
    }
};
}

void AddFLRiftsScripts()
{
    new FLRiftsCreatureRift();
    new FLRiftsCreatureSpawner();
    new FLRiftsWorldScript();
    new FLRiftsPlayerScript();
}
