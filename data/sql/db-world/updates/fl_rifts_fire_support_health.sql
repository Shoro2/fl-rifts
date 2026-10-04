-- Fire support 80017 inherits exp=2 on FL, but a fresh stock stub has exp=0.
-- Match the level-80/class-2 exp=0 supports' 17 * basehp0 health in both cases.
-- Keep exp unchanged: it also selects melee damage. Only HealthModifier changes.
-- Decimal precision prevents a rounded ratio from adding one HP after ceil.
UPDATE `creature_template` AS `c`
INNER JOIN `creature_classlevelstats` AS `s`
    ON `s`.`level` = `c`.`minlevel` AND `s`.`class` = `c`.`unit_class`
SET `c`.`HealthModifier` = 17.000000000000 * `s`.`basehp0` /
    CASE `c`.`exp`
        WHEN 0 THEN `s`.`basehp0`
        WHEN 2 THEN `s`.`basehp2`
    END
WHERE `c`.`entry` = 80017
    AND `c`.`minlevel` = 80 AND `c`.`maxlevel` = 80
    AND `c`.`unit_class` = 2 AND `c`.`exp` IN (0,2);
