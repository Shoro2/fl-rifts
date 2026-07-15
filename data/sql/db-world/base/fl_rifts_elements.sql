-- ============================================================================
-- fl-rifts — Fire / Water / Air element content (self-contained module SQL)
-- ============================================================================
-- Lives at data/sql/db-world/ so AzerothCore's dbimport applies it on BOTH a
-- stock DB (CI dry-run) and the populated Forgotten Land server.
--
-- Entry map (reconciled 2026-07-14 against the live FL DB): the element trash
-- and bosses reuse the FL "[PH] <element> Rift monster/boss" placeholder
-- creatures, which already carry combat-ready stats (faction 16, level 80/82,
-- rank, health/damage mods) and display models. Only FOUR genuinely-new
-- creatures are authored: the Water support (80175), the Rising Tide globule
-- (80176), the wandering tornado (80177), and the Flame Strike trap (80178).
--
-- An earlier draft targeted 80050 / 80030 / 80051 / 80052, but on the FL DB
-- those are REAL NPCs — 80050 "Yorg Stormheart" (a quest-giver), 80030
-- "Nil'un", 80051 "Arcane Magical Anomaly" — so they are never touched now.
-- The FL [PH] element labels are authoritative: 80031-80034/80038 are Water,
-- 80043-80046/80048-80049 are Air (the reverse of the first draft).
--
--   Fire  (rift 90016): trash 80039 80041 80047 80017 · bosses 80040 80042
--   Water (rift 90014): trash 80031 80032 80033 80175 · bosses 80034 80038 · globule 80176
--   Air   (rift 90015): trash 80043 80044 80045 80046 · bosses 80048 80049 · tornado 80177
--   Fire helper: Flame Strike trap 80178
--
-- Strategy so it is safe on both DBs:
--   * Reserved/placeholder entries get an INSERT IGNORE minimal stub — created
--     on a stock DB (so the worldserver boots with a clean Errors.log), ignored
--     on the FL DB (its real, richer rows win).
--   * The three new creatures get explicit definitions (identical on both DBs).
--   * ScriptName / AIName / smart_scripts then apply on top for everyone.
-- All ability spell IDs are stock 3.3.5a NPC abilities (no DBC patch).
-- ============================================================================

-- ---------------------------------------------------------------------------
-- 1. Minimal stubs for reserved/placeholder entries (ignored on the FL DB,
--    where the real [PH] rows already exist with proper stats + models).
-- ---------------------------------------------------------------------------
INSERT IGNORE INTO `creature_template` (`entry`,`name`,`faction`,`unit_class`,`minlevel`,`maxlevel`) VALUES
  -- rift controllers + spawner
  (90014,'Water Rift',16,1,80,80),(90015,'Air Rift',16,1,80,80),
  (90016,'Fire Rift',16,1,80,80),(90017,'Shadow Rift',16,1,80,80),
  (90018,'Rift Spawn Location',16,1,80,80),
  -- shadow bosses
  (80036,'Shadow Rift Boss',16,1,80,80),(80037,'Shadow Rift Boss',16,1,80,80),
  -- fire trash + bosses
  (80039,'Emberforged Brute',16,1,80,80),(80041,'Cinder Adept',16,1,80,80),
  (80047,'Ashfang Harrier',16,1,80,80),(80017,'Flamewaker Zealot',16,1,80,80),
  (80040,'Emberlord Kaz''reth',16,1,80,80),(80042,'Conflagrator Ashmaw',16,1,80,80),
  -- water trash (80175 support is a new creature below) + bosses
  (80031,'Tideguard Brute',16,1,80,80),(80032,'Abyssal Tide-caller',16,1,80,80),
  (80033,'Riptide Binder',16,1,80,80),
  (80034,'Ichyron, the Drowning Tide',16,1,80,80),(80038,'Nethys, Keeper of the Deep',16,1,80,80),
  -- air trash + bosses
  (80043,'Galecharged Marauder',16,1,80,80),(80044,'Tempest Caller',16,1,80,80),
  (80045,'Squallbinder',16,1,80,80),(80046,'Windsworn Zealot',16,1,80,80),
  (80048,'Stormcaller Vaelryn',16,1,80,80),(80049,'Galewind Tempestarii',16,1,80,80);

INSERT IGNORE INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`) VALUES
  (90014,0,11686,1,1),(90015,0,11686,1,1),(90016,0,11686,1,1),(90017,0,11686,1,1),(90018,0,11686,1,1),
  (80036,0,11686,1,1),(80037,0,11686,1,1),
  (80039,0,11686,1,1),(80041,0,11686,1,1),(80047,0,11686,1,1),(80017,0,11686,1,1),
  (80040,0,11686,1,1),(80042,0,11686,1,1),
  (80031,0,11686,1,1),(80032,0,11686,1,1),(80033,0,11686,1,1),
  (80034,0,11686,1,1),(80038,0,11686,1,1),
  (80043,0,11686,1,1),(80044,0,11686,1,1),(80045,0,11686,1,1),(80046,0,11686,1,1),
  (80048,0,11686,1,1),(80049,0,11686,1,1);

-- Existing FL placeholders must be renamed explicitly: INSERT IGNORE preserves
-- their rich live rows, including the old [PH] names.
UPDATE `creature_template` SET
  `name`=CASE `entry`
    WHEN 80017 THEN 'Flamewaker Zealot'
    WHEN 80031 THEN 'Tideguard Brute'
    WHEN 80032 THEN 'Abyssal Tide-caller'
    WHEN 80033 THEN 'Riptide Binder'
    WHEN 80034 THEN 'Ichyron, the Drowning Tide'
    WHEN 80037 THEN 'Vorath, the Hollow King'
    WHEN 80038 THEN 'Nethys, Keeper of the Deep'
    WHEN 80039 THEN 'Emberforged Brute'
    WHEN 80040 THEN 'Emberlord Kaz''reth'
    WHEN 80041 THEN 'Cinder Adept'
    WHEN 80042 THEN 'Conflagrator Ashmaw'
    WHEN 80043 THEN 'Galecharged Marauder'
    WHEN 80044 THEN 'Tempest Caller'
    WHEN 80045 THEN 'Squallbinder'
    WHEN 80046 THEN 'Windsworn Zealot'
    WHEN 80047 THEN 'Ashfang Harrier'
    WHEN 80048 THEN 'Stormcaller Vaelryn'
    WHEN 80049 THEN 'Galewind Tempestarii'
  END,
  `subname`=CASE
    WHEN `entry` IN (80017,80039,80040,80041,80042,80047) THEN 'Fire Rift'
    WHEN `entry` IN (80031,80032,80033,80034,80038) THEN 'Water Rift'
    WHEN `entry` IN (80043,80044,80045,80046,80048,80049) THEN 'Air Rift'
    WHEN `entry`=80037 THEN 'Shadow Rift'
  END
WHERE `entry` IN
  (80017,80031,80032,80033,80034,80037,80038,80039,80040,80041,80042,
   80043,80044,80045,80046,80047,80048,80049);

-- Every Rift is a non-interactive world hazard. Preserve its existing flags
-- and add UNIT_FLAG_NOT_SELECTABLE (0x02000000) idempotently.
UPDATE `creature_template`
SET `unit_flags`=`unit_flags` | 0x02000000
WHERE `entry` IN (90014,90015,90016,90017);

-- Class drives the level-based power pool. The migrated placeholders were all
-- warriors (class 1), leaving caster mana bars empty. Keep frontline mobs as
-- warriors, use mage (8) for offensive casters and paladin (2) for supports.
UPDATE `creature_template` SET `unit_class`=8,`ManaModifier`=1
WHERE `entry` IN (80032,80033,80041,80044,80045,80047);
UPDATE `creature_template` SET `unit_class`=2,`ManaModifier`=1
WHERE `entry` IN (80017,80046,80175);
UPDATE `creature_template` SET `unit_class`=8,`ManaModifier`=2
WHERE `entry` IN (80034,80038,80042,80048,80049);

-- Exact, idempotent values based on the migrated FL model scales:
-- Fire Rift 1.0 -> 3.0, Fire boss 2 0.6 -> 1.2, Air boss 1 0.7 -> 1.05.
UPDATE `creature_template_model` SET `DisplayScale`=3
WHERE `CreatureID`=90016;
UPDATE `creature_template_model` SET `DisplayScale`=1.2
WHERE `CreatureID`=80042;
UPDATE `creature_template_model` SET `DisplayScale`=1.05
WHERE `CreatureID`=80048;

-- ---------------------------------------------------------------------------
-- 2. New creatures: Water support (80175), Rising Tide globule (80176), the
--    wandering tornado (80177), and Flame Strike trap (80178). Absent on both
--    stock and FL, so explicit definitions.
--    Displays reuse IDs confirmed present in the FL client (placeholders — tune
--    in game); the globule is deliberately small and fragile.
-- ---------------------------------------------------------------------------
INSERT INTO `creature_template` (`entry`,`name`,`subname`,`faction`,`unit_class`,`minlevel`,`maxlevel`,`HealthModifier`,`DamageModifier`,`unit_flags`) VALUES
  (80175,'Coral Tidepriest','Water Rift',16,2,80,80,1.5,2,0),
  (80176,'Rising Tide Globule',NULL,16,1,80,80,0.3,1,0),
  (80177,'Raging Tornado',NULL,16,1,80,80,1,1,0x02000002),
  (80178,'Flame Strike Trap',NULL,16,1,82,82,1,1,0x02000002)
ON DUPLICATE KEY UPDATE
  `name`=VALUES(`name`),
  `subname`=VALUES(`subname`),
  `faction`=VALUES(`faction`),
  `unit_class`=VALUES(`unit_class`),
  `minlevel`=VALUES(`minlevel`),
  `maxlevel`=VALUES(`maxlevel`),
  `HealthModifier`=VALUES(`HealthModifier`),
  `DamageModifier`=VALUES(`DamageModifier`),
  `unit_flags`=VALUES(`unit_flags`);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (80175,80176,80177,80178);
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`) VALUES
  (80175,0,17953,1,1),    -- water caster display (placeholder)
  (80176,0,5564,0.4,1),   -- small water blob (placeholder)
  (80177,0,18404,1,1),    -- Living Cyclone (matches the air boss theme)
  (80178,0,11686,1,1);    -- invisible stalker; spell 44191 supplies the visual

-- ---------------------------------------------------------------------------
-- 3. ScriptName / AIName assignment (idempotent on the FL DB). Bosses clear
--    AIName so the C++ ScriptedAI wins over any inherited SmartAI.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName`='FLRiftsCreatureRift'    WHERE `entry` IN (90014,90015,90016,90017);
UPDATE `creature_template` SET `ScriptName`='FLRiftsCreatureSpawner' WHERE `entry`=90018;
UPDATE `creature_template` SET `ScriptName`='boss_shadow', `AIName`='' WHERE `entry` IN (80036,80037);
UPDATE `creature_template` SET `ScriptName`='boss_fl_fire',  `AIName`='' WHERE `entry` IN (80040,80042);
UPDATE `creature_template` SET `ScriptName`='boss_fl_water', `AIName`='' WHERE `entry` IN (80034,80038);
UPDATE `creature_template` SET `ScriptName`='boss_fl_air',   `AIName`='' WHERE `entry` IN (80048,80049);
UPDATE `creature_template` SET `ScriptName`='npc_fl_air_tornado'     WHERE `entry`=80177;
UPDATE `creature_template` SET `ScriptName`='npc_fl_water_globule'   WHERE `entry`=80176;
UPDATE `creature_template` SET `ScriptName`='npc_fl_fire_flame_strike' WHERE `entry`=80178;

-- Fire support 80017 ships with faction 91 on the FL DB; align it to the
-- rift-mob faction (16) so its friendly buff/heal targeting sees allies.
UPDATE `creature_template` SET `faction`=16 WHERE `entry`=80017;

-- ---------------------------------------------------------------------------
-- 4. SmartAI for trash + support mobs. creature_template_spell does not
--    auto-cast under AggressorAI and is ignored under SmartAI, so every ability
--    is driven from smart_scripts. Support mobs use FRIENDLY events (14 =
--    friendly hurt, 16 = friendly missing buff) with target 7 (the invoker) so
--    they heal/buff the ALLY that triggered the event.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN
  (80039,80041,80047,80017,80031,80032,80033,80175,80043,80044,80045,80046);

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN
  (80039,80041,80047,80017,80031,80032,80033,80175,80043,80044,80045,80046);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
  -- Fire trash (80039/80041/80047 + support 80017)
  (80039,0,0,0,0,0,100,0,10000,14000,18000,22000,0,0,11,56908,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Emberforged Brute - IC - Flame Breath'),
  (80041,0,0,0,0,0,100,0,2000,4000,3000,5000,0,0,11,19391,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Cinder Adept - IC - Fireball'),
  (80041,0,1,0,0,0,100,0,12000,16000,14000,18000,0,0,11,19717,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Cinder Adept - IC - Rain of Fire (random)'),
  (80041,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Cinder Adept - Aggro - Keep ranged distance'),
  (80047,0,0,0,0,0,100,0,12000,15000,12000,15000,0,0,11,19496,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Magma Shackles (AoE snare)'),
  (80047,0,1,0,0,0,100,0,4000,6000,6000,8000,0,0,11,19781,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Flame Spear'),
  (80047,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ashfang Harrier - Aggro - Keep ranged distance'),
  (80017,0,0,0,16,0,100,0,3229,30,15000,20000,0,0,11,3229,0,0,0,0,0,26,30,0,0,0,0,0,0,0,'Flamewaker Zealot - Ally missing Quick Bloodlust - Buff closest ally'),
  (80017,0,1,0,14,0,100,0,3000,40,6000,9000,0,0,11,19779,0,0,0,0,0,26,30,0,0,0,0,0,0,0,'Flamewaker Zealot - Friendly hurt - Inspire closest ally'),
  (80017,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Flamewaker Zealot - Aggro - Keep ranged distance'),
  -- Air trash (FL air slots 80043-80046)
  (80043,0,0,0,0,0,100,0,8000,12000,8000,12000,0,0,11,61915,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Galecharged Marauder - IC - Lightning Whirl'),
  (80044,0,0,0,0,0,100,0,2000,3000,2500,4000,0,0,11,53044,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Tempest Caller - IC - Lightning Bolt'),
  (80044,0,1,0,0,0,100,0,10000,14000,10000,14000,0,0,11,48140,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Tempest Caller - IC - Chain Lightning (random)'),
  (80044,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Tempest Caller - Aggro - Keep ranged distance'),
  (80045,0,0,0,0,0,100,0,8000,12000,10000,13000,0,0,11,58464,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Squallbinder - IC - Chains of Ice (random)'),
  (80045,0,1,0,0,0,100,0,12000,12000,12000,15000,0,0,11,52658,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Squallbinder - IC - Static Overload (random)'),
  (80045,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Squallbinder - Aggro - Keep ranged distance'),
  (80046,0,0,0,16,0,100,0,54516,30,30000,40000,0,0,11,54516,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Windsworn Zealot - Ally missing Bloodlust - Cast Bloodlust'),
  (80046,0,1,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Windsworn Zealot - Aggro - Keep ranged distance'),
  -- Water trash (FL water slots 80031-80033 + new support 80175)
  (80031,0,0,0,0,0,100,0,6000,8000,6000,8000,0,0,11,54237,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Tideguard Brute - IC - Water Blast'),
  (80032,0,0,0,0,0,100,0,2000,3000,2500,4000,0,0,11,15497,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - IC - Frostbolt'),
  (80032,0,1,0,0,0,100,0,8000,12000,10000,14000,0,0,11,54241,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - IC - Water Bolt Volley (AoE)'),
  (80032,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - Aggro - Keep ranged distance'),
  (80033,0,0,0,0,0,100,0,8000,11000,10000,12000,0,0,11,58464,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Riptide Binder - IC - Chains of Ice (random)'),
  (80033,0,1,0,0,0,100,0,4000,6000,6000,8000,0,0,11,12548,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Riptide Binder - IC - Frost Shock'),
  (80033,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Riptide Binder - Aggro - Keep ranged distance'),
  (80175,0,0,0,14,0,100,0,4000,40,7000,11000,0,0,11,54481,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Coral Tidepriest - Friendly hurt - Cast Chain Heal'),
  (80175,0,1,0,16,0,100,0,54306,30,15000,20000,0,0,11,54306,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Coral Tidepriest - Ally missing Bubble - Cast Protective Bubble'),
  (80175,0,2,0,4,0,100,0,0,0,0,0,0,0,79,18,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Coral Tidepriest - Aggro - Keep ranged distance');
