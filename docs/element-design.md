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
| 4 · `fire4` **80017** | Flamewaker Zealot | Frenzy `19451` (ally attack-haste) · Inspire `19779` (ally HoT) — SmartAI, friendly |

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
| 1 · `air1` 80043 | Galecharged Marauder | Lightning Whirl `61915` (multi-strike) · Overload `61869` (PB nova when surrounded) |
| 2 · `air2` 80044 | Tempest Caller | Lightning Bolt `53044` (bolt) · Chain Lightning `48140` (arc AoE) |
| 3 · `air3` 80045 | Squallbinder | Chains of Ice `58464` (root/snare) · Static Overload `52658` (splash DoT+knock) · Lightning Bolt `53044` |
| 4 · `air4` 80046 | Windsworn Zealot | Bloodlust `54516` (+35% ally haste) · Windfury Totem `65990` (optional) — SmartAI |

> The Disruptor uses Chains of Ice rather than Cyclone `65859`: Cyclone grants the
> player 6s of full invulnerability (wipes threat, shields raid damage). Keep
> Cyclone only if that "remove a player from the fight" effect is intended.

### Bosses

| Entry | Name | Signature | Rotation |
|---|---|---|---|
| `airboss1` 80048 | Stormcaller Vaelryn | **Wandering Tornadoes** — summons mobile cyclone NPCs (`80177`) that chase players and knock them up | Charge `32323`+Stormhammer `62042` · Lightning Nova `52960` · Arc Lightning `52921` · Chain Lightning `62131` |
| `airboss2` 80049 | Galewind Tempestarii | **Raid-wide knockback** — Thundering Stomp `60925` scatters melee, chained into a storm | Charge `32323` · Thundering Stomp `60925` · Lightning Nova `52960` · Lightning Bolt `53044` · Arc Lightning `52921` |

The tornado (`80177`, ScriptName `npc_fl_air_tornado`) wanders randomly and pulses
Cyclone Strike `56855` (PB knockback) every ~1.5s for 15s.

## 🌊 Water — sustain: adds & interruptible heals (Frost school)

### Trash

| Slot / Entry | Name | Spells |
|---|---|---|
| 1 · `water1` 80031 | Tideguard Brute | Water Blast `54237` (ST frost+knockback) · Frost Nova `15532` (PB root when surrounded) |
| 2 · `water2` 80032 | Abyssal Tide-caller | Frostbolt `15497` (bolt) · Water Bolt Volley `54241` (AoE) |
| 3 · `water3` 80033 | Riptide Binder | Chains of Ice `58464` (snare) · Frost Shock `12548` (bolt+slow) |
| 4 · `water4` **80175** | Coral Tidepriest | Chain Heal `54481` (ally heal) · Protective Bubble `54306` (ally shield) — SmartAI, friendly |

### Bosses

| Entry | Name | Signature | Rotation |
|---|---|---|---|
| `waterboss1` 80034 | Ichyron, the Drowning Tide | **Adds + geyser** — at 50% & every ~25s summons globule adds (`80176`) that heal the boss on arrival; Geyser `37478` knock-up | Water Blast `54237` · Water Bolt Volley `54241` · Frost Shock `12548` · Geyser `37478` · Frostbolt `15497` |
| `waterboss2` 80038 | Nethys, Keeper of the Deep | **Interrupt check** — real-cast Healing Wave `12491` on self players must kick/silence, behind Protective Bubble `54306` | Water Blast `54237` · Frostbolt Volley `70759` · Frost Shock `12548` · Healing Wave `12491` · Protective Bubble `54306` |

The globule (`80176`, ScriptName `npc_fl_water_globule`) swims to Ichyron and, on
arrival, heals it for 5% max HP and vanishes — unless players kill it first.

> Protective Bubble `54306` on a generic creature is a flat damage-reduction
> absorb; the "burn the charges to break it" behavior is Ichoron-script specific
> and needs a small AuraScript to reproduce.

---

## Entry map — reconciled with the live FL DB (2026-07-14)

An earlier draft placed the element creatures on `80050 / 80030 / 80051 / 80052`
and treated `80031-80034` as Air / `80043-80049` as Water. Verified against the
live `acore_world` (and the original FL `update_world`), that map was wrong:
those four "new" entries are **real FL NPCs** — `80050` *Yorg Stormheart* (a
quest-giver for *The Exobeast* / *Chapter 1*), `80030` *Nil'un*, `80051` *Arcane
Magical Anomaly* — and the FL `[PH]` labels put `80031-80034/80038` on **Water**,
`80043-80046/80048-80049` on **Air** (the reverse of the draft).

The content is unchanged; it now reuses the FL `[PH] <element> Rift monster/boss`
placeholders (which already carry hostile faction 16, level 80/82, rank and
display models) and adds only the three genuinely-new creatures:

| Element | trash 1-4 | bosses | helper |
|---|---|---|---|
| 🔥 Fire (rift 90016) | 80039 · 80041 · 80047 · **80017** | 80040 · 80042 | — |
| 🌊 Water (rift 90014) | 80031 · 80032 · 80033 · **80175** (new) | 80034 · 80038 | globule **80176** (new) |
| 💨 Air (rift 90015) | 80043 · 80044 · 80045 · 80046 | 80048 · 80049 | tornado **80177** (new) |

`80038` ("[PH] Water Rift boss 2") carried a stray `boss_eloxin` ScriptName; the
real Eloxin boss is `80067`, so `80038` is overridden to `boss_fl_water` safely.
`80037` stays untouched (it is `shadowboss2` in the Shadow config).

## Implementation status

| Piece | Where | Status |
|---|---|---|
| Random element per Rift | `src/FLRifts.cpp` `SpawnRift()` + `FLRifts.Element.*` config | ✅ code |
| Config entry fixes + element toggles | `conf/fl-rifts.conf.dist` | ✅ code |
| Fire / Air / Water boss AIs + helper NPCs | `src/FLRifts_boss_{fire,air,water}.cpp`, loader | ✅ code |
| Boss ScriptName bindings | `data/sql/db-world/base/fl_rifts_elements.sql` | ✅ runnable |
| New creatures (80175/80176/80177) + models | same SQL | ✅ runnable |
| Trash + Support abilities via SmartAI | same SQL | ✅ runnable¹ |
| Entry map reconciled with live FL DB | SQL + conf + `FLRifts.cpp` defaults | ✅ code |
| Build (MSVC RelWithDebInfo, `dcore_bin`) | operator box | ✅ T1 clean |
| DB apply + boot Errors.log check | live `acore_world` | ⬜ pending |
| In-game verification per element | live FL server | ⬜ pending (operator eyes) |

¹ The reused Fire/Water/Air `[PH]` trash slots carry no existing SmartAI or C++
script (confirmed), so §4 of the SQL authors it fresh.

The C++ compiles as a self-contained module (auto-globbed `src/*.cpp`); the boss
signature helpers degrade gracefully if their helper NPC rows are missing
(`SummonCreature` simply no-ops). Content authoring (display IDs, faction, trash
SmartAI) and the in-game test pass are the remaining steps, tracked in `todo.md`.
