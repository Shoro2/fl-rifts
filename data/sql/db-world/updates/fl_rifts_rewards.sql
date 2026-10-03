-- ============================================================================
-- fl-rifts - Fire / Water / Air Rifts pay the Shadow Rift rewards
-- ============================================================================
-- The element Rifts reuse FL "[PH] <element> Rift monster/boss" placeholders
-- without Rift loot: every element trash and boss creature, and Shadow boss 2
-- (80037), has `lootid` 0 - except Fire support 80017, which inherited the
-- stock world loot of Raging Flame 30847 (Hoary Crystals, Crystallized Fire,
-- world-drop references). Finishing a Fire, Water or Air Rift paid nothing.
-- The element content mirrors the accepted Shadow Rift tuning, so it mirrors
-- its loot as well (operator, 2026-10-03):
--   trash 90001 (Shadow trash 80027-80029/80035): Bloody Trophy x1-5
--   boss  90002 (Shadow boss 80036): Bloody Trophy x100, The Blood Trails
--         Badge and one item of group 1
-- 80017 drops 30847 on purpose: it has no world spawn, so only Fire Rift kills
-- change, and table 30847 itself stays for Raging Flame.
-- Wave trash corpses stay lootable for a minute (FLRifts.cpp,
-- RIFT_CREEP_CORPSE_MS); before, they vanished on the next update.
--
-- Sorts after base/fl_rifts_elements.sql, whose stubs create these entries on
-- a stock DB. The EXISTS guards keep a stock DB (CI), which has neither loot
-- table, free of references to loot it does not have.
-- ============================================================================

UPDATE `creature_template` SET `lootid`=90001
WHERE `entry` IN
  (80017,80031,80032,80033,80039,80041,80043,80044,80045,80046,80047,80175)
  AND EXISTS (SELECT 1 FROM `creature_loot_template` WHERE `Entry`=90001);

UPDATE `creature_template` SET `lootid`=90002
WHERE `entry` IN (80034,80037,80038,80040,80042,80048,80049)
  AND EXISTS (SELECT 1 FROM `creature_loot_template` WHERE `Entry`=90002);
