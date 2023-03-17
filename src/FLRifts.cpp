/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedAI/ScriptedCreature.h"
#include<iostream>
#include <algorithm>

bool eventActive = false;
uint32 creepsAlive = 0;
uint8 waveNumber = 0;
std::list<Creature*> creatureList = {};

void FLR_init() {
    //reset stuff
    creepsAlive = 0;
    waveNumber = 0;
}

void FLR_clear() {
    // clear everything
    std::list<Creature*>::iterator it;
    for (it = creatureList.begin(); it != creatureList.end(); it++)
    {
        
    }
}

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
        }
    }

    void OnStop(uint16 EventID) override {
        if (EventID == 120) {
            eventActive = false;
            FLR_clear();
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
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            creatureList.push_front(me->SummonCreature(RAND(1, 2, 3, 4, 5, 6, 7, 8, 9), x, y, z, o, TEMPSUMMON_TIMED_DESPAWN, 600));
                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 1:
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            creatureList.push_front(me->SummonCreature(RAND(1, 2, 3, 4, 5, 6, 7, 8, 9), x, y, z, o, TEMPSUMMON_TIMED_DESPAWN, 600));
                            creepsAlive++;
                        }
                        waveNumber++;
                    }
                    break;
                case 2:
                    if (creepsAlive == 0) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            creatureList.push_front(me->SummonCreature(RAND(1, 2, 3, 4, 5, 6, 7, 8, 9), x, y, z, o, TEMPSUMMON_TIMED_DESPAWN, 600));
                            creepsAlive++;
                        }
                        waveNumber++;
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




};

// Add all scripts in one
void AddFLRiftsScripts()
{
    new FLRiftsPlayer();

    new FLRiftsGameEvent();

    new FLRiftsCreatureRift();
    new FLRiftsCreatureTrash();
}
