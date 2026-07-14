# Forgotten Land Rifts — Element design (Fire / Water / Air)

Design reference for the non-Shadow Rift elements. The Rift event
(`src/FLRifts.cpp`) is element-agnostic: a spawner opens a Rift, waits two
minutes, runs **two trash waves of 10** drawn at random from the element's four
trash entries, then **one boss** (random of the element's two boss entries), and
locks for 30 minutes. Only the theme — which creatures spawn and what spells they
use — changes per element. This document specifies Fire, Water and Air to play
*like the Shadow portal, only a different element*.

All spell IDs are **stock WoW 3.3.5a client spells already cast as NPC abilities
in shipped AzerothCore scripts** — no `Spell.dbc` patching and no custom spell
IDs are needed. Each ID was verified against the on-disk AzerothCore source.

## Archetype framework

The four trash slots are the same four archetypes in every element, re-skinned:

| Slot | Config key | Archetype | Role |
|---|---|---|---|
| 1 | `<el>1` | Melee Bruiser | frontline melee + point-blank burst when surrounded |
| 2 | `<el>2` | Caster | ranged element bolt + occasional element AoE |
| 3 | `<el>3` | Disruptor | element CC/debuff + light secondary nuke |
| 4 | `<el>4` | Support | buffs/heals/shields allies — punishes leaving it alive |

Each element has **two bosses** that share the Shadow boss's rotation shape
(gap-closer/burst · AoE · single-target · filler) plus **one signature mechanic**
that defines the element.

Shadow reference kit (`src/FLRifts_bosses.cpp`): Charge `74399` + Knockback
`26478`, Silence `64189`, Strike `62130`, Cleave `70670`. (`26478` is C'thun's
Massive Ground Rupture visual reused school-neutral, not a true knockback.)

**Spell delivery.** Simple on-cooldown casters/bruisers use
`creature_template_spell` (auto-cast on the current target). Anything needing a
condition — burst *only when surrounded*, or **friendly-target buffs/heals** —
uses SmartAI. Bosses cast from their C++ `ScriptedAI`.

---

## 🔥 Fire — "stand out of the fire" ground hazards (Fire school)

### Trash

| Slot / Entry | Name | Spells |
|---|---|---|
| 1 · `fire1` 80039 | Emberforged Brute | Flame Breath `56908` (frontal cone) · Eruption `19497` (PBAoE when ≥3 in melee) |
| 2 · `fire2` 80041 | Cinder Adept | Fireball `19391` (bolt) · Rain of Fire `19717` (ground AoE) |
| 3 · `fire3` 80047 | Ashfang Harrier | Magma Shackles `19496` (AoE snare) · Immolate `20294` (DoT) · Flame Spear `19781` (bolt) |
| 4 · `fire4` **80050** | Flamewaker Zealot | Frenzy `19451` (ally attack-haste) · Inspire `19779` (ally HoT) — SmartAI, friendly |

### Bosses

| Entry | Name | Signature | Rotation |
|---|---|---|---|
| `fireboss1` 80040 | Emberlord Kaz'reth | **Scorched Ground** — Scorch `62546` seeds lingering flame patches (`62548`) | Charge `74399`+`26478` · Flame Breath `56908` · Cleave `56909` · Fireball `19391` · Scorch `62546` |
| `fireboss2` 80042 | Conflagrator Ashmaw | **Rain-of-fire patches** — Flame Strike `44192` drops burning ground (`44191`) | Living Bomb `20475` · Inferno `19695` · Pyroblast `36819` · Fireball `19391` · Flame Strike `44192` |

Signature is a scheduled `DoCast`; the ground patch is carried by the spell's
summoned trigger NPC (Ignis-`33123` / Kael-`24666`). If those NPCs are absent in
the world DB, substitute Rain of Fire `19717` — a self-contained persistent
ground-fire area spell.

## 💨 Air — knockback & storm mobility chaos (Nature school)

### Trash

| Slot / Entry | Name | Spells |
|---|---|---|
| 1 · `air1` 80031 | Galecharged Marauder | Lightning Whirl `61915` (multi-strike) · Overload `61869` (PB nova when surrounded) |
| 2 · `air2` 80032 | Tempest Caller | Lightning Bolt `53044` (bolt) · Chain Lightning `48140` (arc AoE) |
| 3 · `air3` 80033 | Squallbinder | Chains of Ice `58464` (root/snare) · Static Overload `52658` (splash DoT+knock) · Lightning Bolt `53044` |
| 4 · `air4` **80030** | Windsworn Zealot | Bloodlust `54516` (+35% ally haste) · Windfury Totem `65990` (optional) — SmartAI |

> The Disruptor uses Chains of Ice rather than Cyclone `65859`: Cyclone grants the
> player 6s of full invulnerability (wipes threat, shields raid damage). Keep
> Cyclone only if that "remove a player from the fight" effect is intended.

### Bosses

| Entry | Name | Signature | Rotation |
|---|---|---|---|
| `airboss1` 80034 | Stormcaller Vaelryn | **Wandering Tornadoes** — summons mobile cyclone NPCs (`80051`) that chase players and knock them up | Charge `32323`+Stormhammer `62042` · Lightning Nova `52960` · Arc Lightning `52921` · Chain Lightning `62131` |
| `airboss2` 80038 | Galewind Tempestarii | **Raid-wide knockback** — Thundering Stomp `60925` scatters melee, chained into a storm | Charge `32323` · Thundering Stomp `60925` · Lightning Nova `52960` · Lightning Bolt `53044` · Arc Lightning `52921` |

The tornado (`80051`, ScriptName `npc_fl_air_tornado`) wanders randomly and pulses
Cyclone Strike `56855` (PB knockback) every ~1.5s for 15s.

## 🌊 Water — sustain: adds & interruptible heals (Frost school)

### Trash

| Slot / Entry | Name | Spells |
|---|---|---|
| 1 · `water1` 80043 | Tideguard Brute | Water Blast `54237` (ST frost+knockback) · Frost Nova `15532` (PB root when surrounded) |
| 2 · `water2` 80044 | Abyssal Tide-caller | Frostbolt `15497` (bolt) · Water Bolt Volley `54241` (AoE) |
| 3 · `water3` 80045 | Riptide Binder | Chains of Ice `58464` (snare) · Frost Shock `12548` (bolt+slow) |
| 4 · `water4` 80046 | Coral Tidepriest | Chain Heal `54481` (ally heal) · Protective Bubble `54306` (ally shield) — SmartAI, friendly |

### Bosses

| Entry | Name | Signature | Rotation |
|---|---|---|---|
| `waterboss1` 80048 | Ichyron, the Drowning Tide | **Adds + geyser** — at 50% & every ~25s summons globule adds (`80052`) that heal the boss on arrival; Geyser `37478` knock-up | Water Blast `54237` · Water Bolt Volley `54241` · Frost Shock `12548` · Geyser `37478` · Frostbolt `15497` |
| `waterboss2` 80049 | Nethys, Keeper of the Deep | **Interrupt check** — real-cast Healing Wave `12491` on self players must kick/silence, behind Protective Bubble `54306` | Water Blast `54237` · Frostbolt Volley `70759` · Frost Shock `12548` · Healing Wave `12491` · Protective Bubble `54306` |

The globule (`80052`, ScriptName `npc_fl_water_globule`) swims to Ichyron and, on
arrival, heals it for 5% max HP and vanishes — unless players kill it first.

> Protective Bubble `54306` on a generic creature is a flat damage-reduction
> absorb; the "burn the charges to break it" behavior is Ichoron-script specific
> and needs a small AuraScript to reproduce.

---

## Entry-ID fixes

The original config had two problems, resolved here:
- **`air4` `80037` → `80030`**: `80037` was also `shadowboss2`, so an Air wave
  would have summoned a Shadow boss as trash.
- **`fire4` `80017` → `80050`**: `80017` was the only entry outside the
  `80027–80049` band.

Both reassigned Support mobs (`80050`, `80030`) are new creatures to author.

## Implementation status

| Piece | Where | Status |
|---|---|---|
| Random element per Rift | `src/FLRifts.cpp` `SpawnRift()` + `FLRifts.Element.*` config | ✅ code |
| Config entry fixes + element toggles | `conf/fl-rifts.conf.dist` | ✅ code |
| Fire / Air / Water boss AIs + helper NPCs | `src/FLRifts_boss_{fire,air,water}.cpp`, loader | ✅ code |
| Boss ScriptName bindings | `sql/world/updates/2026_07_14_00_fl_rifts_elements.sql` | ✅ runnable |
| Trash spell loadouts, new Support/helper creatures, Support SmartAI | same SQL (scaffold), needs live-DB reconciliation | ⬜ content |
| Build + in-game verification | per module workflow | ⬜ pending |

The C++ compiles as a self-contained module (auto-globbed `src/*.cpp`); the boss
signature helpers degrade gracefully if their helper NPC rows are missing
(`SummonCreature` simply no-ops). Content authoring (display IDs, faction, trash
SmartAI) and the in-game test pass are the remaining steps, tracked in `todo.md`.
