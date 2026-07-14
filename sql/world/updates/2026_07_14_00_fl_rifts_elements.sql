-- ============================================================================
-- fl-rifts — Fire / Water / Air element content
-- ============================================================================
-- Binds the new element boss AIs, creates the reassigned Support mobs and the
-- two boss helper NPCs, and gives the trash their element abilities via SmartAI.
--
-- DB-agnostic by design: the new creatures inherit faction/level/stats/model
-- from an existing element sibling (which lives in the live FL world DB) via
-- copy-from-sibling UPDATE/INSERT...SELECT, so no display IDs are hardcoded for
-- the support mobs. All spell IDs are stock 3.3.5a NPC abilities (no DBC patch).
--
-- NOTE: creature_template_spell does NOT auto-cast for a normal hostile mob
-- (default AggressorAI only melees; CombatAI is never auto-assigned) and is
-- ignored under SmartAI — so every ability is driven by smart_scripts here.
-- ============================================================================

-- ---------------------------------------------------------------------------
-- 1. Bind the element boss AIs (C++ ScriptedAI via ScriptName; AIName cleared
--    so the script wins over any inherited SmartAI).
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName` = 'boss_fl_fire',  `AIName` = '' WHERE `entry` IN (80040, 80042);
UPDATE `creature_template` SET `ScriptName` = 'boss_fl_air',   `AIName` = '' WHERE `entry` IN (80034, 80038);
UPDATE `creature_template` SET `ScriptName` = 'boss_fl_water', `AIName` = '' WHERE `entry` IN (80048, 80049);

-- ---------------------------------------------------------------------------
-- 2. New creatures: reassigned Support mobs (fire4 80050, air4 80030) and the
--    boss signature helpers (tornado 80051, globule 80052). Cloned from a live
--    sibling for stats/faction, then overridden.
-- ---------------------------------------------------------------------------
DELETE FROM `creature_template` WHERE `entry` IN (80050, 80030, 80051, 80052);
INSERT INTO `creature_template` (`entry`, `name`, `subname`) VALUES
  (80050, 'Flamewaker Zealot',   'Fire Rift'),
  (80030, 'Windsworn Zealot',    'Air Rift'),
  (80051, 'Raging Tornado',      NULL),
  (80052, 'Rising Tide Globule', NULL);

-- inherit combat stats/faction from an element sibling (fire caster 80041,
-- air caster 80032, water bruiser 80043). If a sibling is absent the new mob
-- keeps template defaults (level 1 / faction 0) — the element trash must exist.
UPDATE `creature_template` t, `creature_template` s SET
  t.`faction`=s.`faction`, t.`minlevel`=s.`minlevel`, t.`maxlevel`=s.`maxlevel`, t.`exp`=s.`exp`,
  t.`unit_class`=s.`unit_class`, t.`type`=s.`type`, t.`family`=s.`family`,
  t.`BaseAttackTime`=s.`BaseAttackTime`, t.`RangeAttackTime`=s.`RangeAttackTime`,
  t.`BaseVariance`=s.`BaseVariance`, t.`RangeVariance`=s.`RangeVariance`, t.`DamageModifier`=s.`DamageModifier`,
  t.`HealthModifier`=s.`HealthModifier`, t.`ManaModifier`=s.`ManaModifier`, t.`ArmorModifier`=s.`ArmorModifier`,
  t.`speed_walk`=s.`speed_walk`, t.`speed_run`=s.`speed_run`, t.`detection_range`=s.`detection_range`
  WHERE t.`entry`=80050 AND s.`entry`=80041;
UPDATE `creature_template` t, `creature_template` s SET
  t.`faction`=s.`faction`, t.`minlevel`=s.`minlevel`, t.`maxlevel`=s.`maxlevel`, t.`exp`=s.`exp`,
  t.`unit_class`=s.`unit_class`, t.`type`=s.`type`, t.`family`=s.`family`,
  t.`BaseAttackTime`=s.`BaseAttackTime`, t.`RangeAttackTime`=s.`RangeAttackTime`,
  t.`BaseVariance`=s.`BaseVariance`, t.`RangeVariance`=s.`RangeVariance`, t.`DamageModifier`=s.`DamageModifier`,
  t.`HealthModifier`=s.`HealthModifier`, t.`ManaModifier`=s.`ManaModifier`, t.`ArmorModifier`=s.`ArmorModifier`,
  t.`speed_walk`=s.`speed_walk`, t.`speed_run`=s.`speed_run`, t.`detection_range`=s.`detection_range`
  WHERE t.`entry` IN (80030, 80051) AND s.`entry`=80032;
UPDATE `creature_template` t, `creature_template` s SET
  t.`faction`=s.`faction`, t.`minlevel`=s.`minlevel`, t.`maxlevel`=s.`maxlevel`, t.`exp`=s.`exp`,
  t.`unit_class`=s.`unit_class`, t.`type`=s.`type`, t.`family`=s.`family`,
  t.`BaseAttackTime`=s.`BaseAttackTime`, t.`RangeAttackTime`=s.`RangeAttackTime`,
  t.`BaseVariance`=s.`BaseVariance`, t.`RangeVariance`=s.`RangeVariance`, t.`DamageModifier`=s.`DamageModifier`,
  t.`HealthModifier`=s.`HealthModifier`, t.`ManaModifier`=s.`ManaModifier`, t.`ArmorModifier`=s.`ArmorModifier`,
  t.`speed_walk`=s.`speed_walk`, t.`speed_run`=s.`speed_run`, t.`detection_range`=s.`detection_range`
  WHERE t.`entry`=80052 AND s.`entry`=80043;

-- role overrides (run AFTER the stat copy)
UPDATE `creature_template` SET `rank`=0, `npcflag`=0, `RegenHealth`=1, `lootid`=0, `AIName`='SmartAI', `ScriptName`='' WHERE `entry` IN (80050, 80030);
UPDATE `creature_template` SET `rank`=0, `npcflag`=0, `RegenHealth`=1, `lootid`=0, `AIName`='', `ScriptName`='npc_fl_air_tornado' WHERE `entry`=80051;
UPDATE `creature_template` SET `rank`=0, `npcflag`=0, `RegenHealth`=1, `lootid`=0, `AIName`='', `ScriptName`='npc_fl_water_globule', `HealthModifier`=0.3 WHERE `entry`=80052;

-- models: supports clone the sibling display; helpers use a fixed elemental display
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (80050, 80030, 80051, 80052);
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`)
  SELECT 80050,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild` FROM `creature_template_model` WHERE `CreatureID`=80041;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`)
  SELECT 80030,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild` FROM `creature_template_model` WHERE `CreatureID`=80032;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`) VALUES
  (80051, 0, 18404, 1, 1),  -- Living Cyclone (tornado visual)
  (80052, 0, 20782, 1, 1);  -- Water Globule (Ichoron-style blob)

-- ---------------------------------------------------------------------------
-- 3. Support SmartAI for the NEW support mobs (friendly heals/buffs; event 14 =
--    FRIENDLY_HEALTH, 16 = FRIENDLY_MISSING_BUFF, cast on target 7 ACTION_INVOKER
--    which the event pre-selects).
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (80050, 80030);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
  (80050,0,0,0,16,0,100,0,19451,30,15000,20000,0,0,11,19451,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Flamewaker Zealot - Ally missing Frenzy - Cast Frenzy'),
  (80050,0,1,0,14,0,100,0,3000,40,6000,9000,0,0,11,19779,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Flamewaker Zealot - Friendly hurt - Cast Inspire'),
  (80030,0,0,0,16,0,100,0,54516,30,30000,40000,0,0,11,54516,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Windsworn Zealot - Ally missing Bloodlust - Cast Bloodlust');

-- ---------------------------------------------------------------------------
-- 4. Trash + existing Support abilities via SmartAI.
--    ASSUMPTION: the Fire/Air/Water trash (80031-80047) are blank placeholders
--    (these elements never ran), so their SmartAI is authored fresh below.
--    If the live DB already scripts any of these entries, MERGE instead of
--    applying this block wholesale (the DELETEs are scoped per entry).
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN
  (80039,80041,80047,80031,80032,80033,80043,80044,80045,80046);

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN
  (80039,80041,80047,80031,80032,80033,80043,80044,80045,80046);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
  -- Fire: Emberforged Brute (bruiser), Cinder Adept (caster), Ashfang Harrier (disruptor)
  (80039,0,0,0,0,0,100,0,6000,8000,8000,10000,0,0,11,56908,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Emberforged Brute - IC - Flame Breath'),
  (80039,0,1,0,0,0,100,0,10000,14000,15000,20000,0,0,11,19497,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Emberforged Brute - IC - Eruption (point-blank)'),
  (80041,0,0,0,0,0,100,0,2000,4000,3000,5000,0,0,11,19391,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Cinder Adept - IC - Fireball'),
  (80041,0,1,0,0,0,100,0,12000,16000,14000,18000,0,0,11,19717,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Cinder Adept - IC - Rain of Fire (random)'),
  (80047,0,0,0,0,0,100,0,3000,5000,12000,16000,0,0,11,20294,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Immolate'),
  (80047,0,1,0,0,0,100,0,12000,15000,12000,15000,0,0,11,19496,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Magma Shackles (AoE snare)'),
  (80047,0,2,0,0,0,100,0,4000,6000,6000,8000,0,0,11,19781,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ashfang Harrier - IC - Flame Spear'),
  -- Air: Galecharged Marauder (bruiser), Tempest Caller (caster), Squallbinder (disruptor)
  (80031,0,0,0,0,0,100,0,8000,12000,8000,12000,0,0,11,61915,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Galecharged Marauder - IC - Lightning Whirl'),
  (80031,0,1,0,0,0,100,0,18000,25000,18000,25000,0,0,11,61869,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Galecharged Marauder - IC - Overload (point-blank)'),
  (80032,0,0,0,0,0,100,0,2000,3000,2500,4000,0,0,11,53044,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Tempest Caller - IC - Lightning Bolt'),
  (80032,0,1,0,0,0,100,0,10000,14000,10000,14000,0,0,11,48140,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Tempest Caller - IC - Chain Lightning (random)'),
  (80033,0,0,0,0,0,100,0,8000,12000,10000,13000,0,0,11,58464,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Squallbinder - IC - Chains of Ice (random)'),
  (80033,0,1,0,0,0,100,0,12000,12000,12000,15000,0,0,11,52658,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Squallbinder - IC - Static Overload (random)'),
  (80033,0,2,0,0,0,100,0,3000,5000,4000,6000,0,0,11,53044,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Squallbinder - IC - Lightning Bolt (filler)'),
  -- Water: Tideguard Brute (bruiser), Abyssal Tide-caller (caster), Riptide Binder (disruptor)
  (80043,0,0,0,0,0,100,0,6000,8000,6000,8000,0,0,11,54237,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Tideguard Brute - IC - Water Blast'),
  (80043,0,1,0,0,0,100,0,12000,16000,15000,20000,0,0,11,15532,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Tideguard Brute - IC - Frost Nova (point-blank)'),
  (80044,0,0,0,0,0,100,0,2000,3000,2500,4000,0,0,11,15497,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - IC - Frostbolt'),
  (80044,0,1,0,0,0,100,0,8000,12000,10000,14000,0,0,11,54241,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Abyssal Tide-caller - IC - Water Bolt Volley (AoE)'),
  (80045,0,0,0,0,0,100,0,8000,11000,10000,12000,0,0,11,58464,0,0,0,0,0,5,0,0,0,0,0,0,0,0,'Riptide Binder - IC - Chains of Ice (random)'),
  (80045,0,1,0,0,0,100,0,4000,6000,6000,8000,0,0,11,12548,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Riptide Binder - IC - Frost Shock'),
  -- Water: Coral Tidepriest (existing Support - friendly heal/shield)
  (80046,0,0,0,14,0,100,0,4000,40,7000,11000,0,0,11,54481,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Coral Tidepriest - Friendly hurt - Cast Chain Heal'),
  (80046,0,1,0,16,0,100,0,54306,30,15000,20000,0,0,11,54306,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Coral Tidepriest - Ally missing Bubble - Cast Protective Bubble');
