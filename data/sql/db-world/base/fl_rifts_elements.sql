-- ============================================================================
-- fl-rifts — Fire / Water / Air element content (self-contained module SQL)
-- ============================================================================
-- Lives at data/sql/db-world/ so AzerothCore's dbimport applies it on BOTH a
-- stock DB (CI dry-run) and the populated Forgotten Land server.
--
-- Strategy so it is safe on both:
--   * Entries that already exist in the FL DB (the reserved rift NPCs, shadow
--     bosses and element trash) get an INSERT IGNORE minimal stub — created on a
--     stock DB (so the worldserver boots with an empty Errors.log), ignored on
--     the FL DB (its real rows win).
--   * The genuinely-new creatures (support mobs 80050/80030, boss helpers
--     80051/80052) get explicit definitions (identical on both DBs).
--   * ScriptName / AIName / smart_scripts then apply on top for everyone.
--
-- Stub validity (each avoids a startup LOG_ERROR): faction 14 (valid template),
-- unit_class 1, CreatureDisplayID 11686 (Invisible Stalker, in every 3.3.5a
-- client). All ability spell IDs are stock 3.3.5a NPC abilities (no DBC patch).
-- ============================================================================

-- ---------------------------------------------------------------------------
-- 1. Minimal stubs for entries that live in the FL DB (ignored there).
-- ---------------------------------------------------------------------------
INSERT IGNORE INTO `creature_template` (`entry`,`name`,`faction`,`unit_class`,`minlevel`,`maxlevel`) VALUES
  -- rift controllers + spawner
  (90014,'Water Rift',14,1,80,80),(90015,'Air Rift',14,1,80,80),
  (90016,'Fire Rift',14,1,80,80),(90017,'Shadow Rift',14,1,80,80),
  (90018,'Rift Spawn Location',14,1,80,80),
  -- shadow bosses
  (80036,'Shadow Rift Boss',14,1,80,80),(80037,'Shadow Rift Boss',14,1,80,80),
  -- element bosses
  (80040,'Emberlord Kaz''reth',14,1,80,80),(80042,'Conflagrator Ashmaw',14,1,80,80),
  (80034,'Stormcaller Vaelryn',14,1,80,80),(80038,'Galewind Tempestarii',14,1,80,80),
  (80048,'Ichyron, the Drowning Tide',14,1,80,80),(80049,'Nethys, Keeper of the Deep',14,1,80,80),
  -- element trash that receives SmartAI below
  (80039,'Emberforged Brute',14,1,80,80),(80041,'Cinder Adept',14,1,80,80),(80047,'Ashfang Harrier',14,1,80,80),
  (80031,'Galecharged Marauder',14,1,80,80),(80032,'Tempest Caller',14,1,80,80),(80033,'Squallbinder',14,1,80,80),
  (80043,'Tideguard Brute',14,1,80,80),(80044,'Abyssal Tide-caller',14,1,80,80),
  (80045,'Riptide Binder',14,1,80,80),(80046,'Coral Tidepriest',14,1,80,80);

INSERT IGNORE INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`) VALUES
  (90014,0,11686,1,1),(90015,0,11686,1,1),(90016,0,11686,1,1),(90017,0,11686,1,1),(90018,0,11686,1,1),
  (80036,0,11686,1,1),(80037,0,11686,1,1),
  (80040,0,11686,1,1),(80042,0,11686,1,1),(80034,0,11686,1,1),(80038,0,11686,1,1),
  (80048,0,11686,1,1),(80049,0,11686,1,1),
  (80039,0,11686,1,1),(80041,0,11686,1,1),(80047,0,11686,1,1),
  (80031,0,11686,1,1),(80032,0,11686,1,1),(80033,0,11686,1,1),
  (80043,0,11686,1,1),(80044,0,11686,1,1),(80045,0,11686,1,1),(80046,0,11686,1,1);

-- ---------------------------------------------------------------------------
-- 2. New creatures: reassigned Support mobs (fire4 80050, air4 80030) and the
--    boss signature helpers (tornado 80051, globule 80052). Explicit defs.
-- ---------------------------------------------------------------------------
DELETE FROM `creature_template` WHERE `entry` IN (80050,80030,80051,80052);
INSERT INTO `creature_template` (`entry`,`name`,`subname`,`faction`,`unit_class`,`minlevel`,`maxlevel`,`HealthModifier`) VALUES
  (80050,'Flamewaker Zealot','Fire Rift',14,2,80,80,1),
  (80030,'Windsworn Zealot','Air Rift',14,2,80,80,1),
  (80051,'Raging Tornado',NULL,14,1,80,80,1),
  (80052,'Rising Tide Globule',NULL,14,1,80,80,0.3);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (80050,80030,80051,80052);
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`) VALUES
  (80050,0,12030,1,1),  -- Flamewaker
  (80030,0,18404,1,1),  -- Living Cyclone (air)
  (80051,0,18404,1,1),  -- tornado
  (80052,0,20782,1,1);  -- Water Globule

-- ---------------------------------------------------------------------------
-- 3. Assign every registered C++ ScriptName (idempotent on the FL DB; clears
--    AIName on the bosses so the ScriptedAI wins over any inherited SmartAI).
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName`='FLRiftsCreatureRift'    WHERE `entry` IN (90014,90015,90016,90017);
UPDATE `creature_template` SET `ScriptName`='FLRiftsCreatureSpawner' WHERE `entry`=90018;
UPDATE `creature_template` SET `ScriptName`='boss_shadow'            WHERE `entry` IN (80036,80037);
UPDATE `creature_template` SET `ScriptName`='boss_fl_fire',  `AIName`='' WHERE `entry` IN (80040,80042);
UPDATE `creature_template` SET `ScriptName`='boss_fl_air',   `AIName`='' WHERE `entry` IN (80034,80038);
UPDATE `creature_template` SET `ScriptName`='boss_fl_water', `AIName`='' WHERE `entry` IN (80048,80049);
UPDATE `creature_template` SET `ScriptName`='npc_fl_air_tornado'     WHERE `entry`=80051;
UPDATE `creature_template` SET `ScriptName`='npc_fl_water_globule'   WHERE `entry`=80052;

-- ---------------------------------------------------------------------------
-- 4. SmartAI for trash + support mobs (creature_template_spell does not
--    auto-cast under AggressorAI and is ignored under SmartAI, so drive every
--    ability from smart_scripts). The FL trash carry no prior SmartAI.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN
  (80039,80041,80047,80031,80032,80033,80043,80044,80045,80046,80050,80030);

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN
  (80039,80041,80047,80031,80032,80033,80043,80044,80045,80046,80050,80030);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
  -- Fire trash
  (80039,0,0,0,0,0,100,0,6000,8000,8000,10000,0,0,11,56908,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Emberforged Brute - IC - Flame Breath'),
  (80039,0,1,0,0,0,100,0,10000,14000,15000,20000,0,0,11,19497,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Emberforged Brute - IC - Eruption (point-blank)'),
  (80041,0,0,0,0,0,100,0,2000,4000,3000,5000,0,0,11,19391,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Cinder Adept - IC - Fireball'),
  (80041,0,1,0,0,0,100,0,12000,16000,14000,18000,0,0,11,19717,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Cinder Adept - IC - Rain of Fire (random)'),
  (80047,0,0,0,0,0,100,0,3000,5000,12000,16000,0,0,11,20294,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Immolate'),
  (80047,0,1,0,0,0,100,0,12000,15000,12000,15000,0,0,11,19496,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Magma Shackles (AoE snare)'),
  (80047,0,2,0,0,0,100,0,4000,6000,6000,8000,0,0,11,19781,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Flame Spear'),
  -- Air trash
  (80031,0,0,0,0,0,100,0,8000,12000,8000,12000,0,0,11,61915,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Galecharged Marauder - IC - Lightning Whirl'),
  (80031,0,1,0,0,0,100,0,18000,25000,18000,25000,0,0,11,61869,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Galecharged Marauder - IC - Overload (point-blank)'),
  (80032,0,0,0,0,0,100,0,2000,3000,2500,4000,0,0,11,53044,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Tempest Caller - IC - Lightning Bolt'),
  (80032,0,1,0,0,0,100,0,10000,14000,10000,14000,0,0,11,48140,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Tempest Caller - IC - Chain Lightning (random)'),
  (80033,0,0,0,0,0,100,0,8000,12000,10000,13000,0,0,11,58464,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Squallbinder - IC - Chains of Ice (random)'),
  (80033,0,1,0,0,0,100,0,12000,12000,12000,15000,0,0,11,52658,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Squallbinder - IC - Static Overload (random)'),
  (80033,0,2,0,0,0,100,0,3000,5000,4000,6000,0,0,11,53044,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Squallbinder - IC - Lightning Bolt (filler)'),
  -- Water trash
  (80043,0,0,0,0,0,100,0,6000,8000,6000,8000,0,0,11,54237,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Tideguard Brute - IC - Water Blast'),
  (80043,0,1,0,0,0,100,0,12000,16000,15000,20000,0,0,11,15532,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Tideguard Brute - IC - Frost Nova (point-blank)'),
  (80044,0,0,0,0,0,100,0,2000,3000,2500,4000,0,0,11,15497,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - IC - Frostbolt'),
  (80044,0,1,0,0,0,100,0,8000,12000,10000,14000,0,0,11,54241,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - IC - Water Bolt Volley (AoE)'),
  (80045,0,0,0,0,0,100,0,8000,11000,10000,12000,0,0,11,58464,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Riptide Binder - IC - Chains of Ice (random)'),
  (80045,0,1,0,0,0,100,0,4000,6000,6000,8000,0,0,11,12548,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Riptide Binder - IC - Frost Shock'),
  -- Support mobs (friendly heal/buff; event 14 FRIENDLY_HEALTH, 16 MISSING_BUFF, target 7 = invoker)
  (80050,0,0,0,16,0,100,0,19451,30,15000,20000,0,0,11,19451,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Flamewaker Zealot - Ally missing Frenzy - Cast Frenzy'),
  (80050,0,1,0,14,0,100,0,3000,40,6000,9000,0,0,11,19779,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Flamewaker Zealot - Friendly hurt - Cast Inspire'),
  (80030,0,0,0,16,0,100,0,54516,30,30000,40000,0,0,11,54516,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Windsworn Zealot - Ally missing Bloodlust - Cast Bloodlust'),
  (80046,0,0,0,14,0,100,0,4000,40,7000,11000,0,0,11,54481,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Coral Tidepriest - Friendly hurt - Cast Chain Heal'),
  (80046,0,1,0,16,0,100,0,54306,30,15000,20000,0,0,11,54306,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Coral Tidepriest - Ally missing Bubble - Cast Protective Bubble');
