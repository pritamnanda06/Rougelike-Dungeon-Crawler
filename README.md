# Rougelike-Dungeon-Crawler


<div align="center">

# 🏰 Shadows of the Keep

### *The stairs go down. The torches go out. How deep will you go?*

![C++](https://img.shields.io/badge/C%2B%2B-11-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Genre](https://img.shields.io/badge/genre-roguelike-8B0000?style=for-the-badge)
![Interface](https://img.shields.io/badge/interface-terminal-2E2E2E?style=for-the-badge)
![Dependencies](https://img.shields.io/badge/dependencies-none-2EA043?style=for-the-badge)
![Files](https://img.shields.io/badge/source-single%20file-F2A900?style=for-the-badge)

</div>

---

## 📜 The Legend

Beneath the ruined castle lies a keep that no map has ever charted. Its halls rearrange themselves every time a hero sets foot on the stairs. Goblins squat in the dark, bats nest in the rafters, and something large is guarding the third staircase.

You carry a rusty blade, twenty hit points, and no plan whatsoever.

**Shadows of the Keep** is a turn-based ASCII roguelike that runs entirely in your terminal. It is one C++ file with no libraries and no graphics, and no two dungeons are the same. It is also a guided tour of the C++ language in disguise: every monster, potion, and corridor is built to show off a different language feature (see the [Grimoire](#-the-grimoire-c-concepts-inside)).

```text
============ SHADOWS OF THE KEEP ============
Depth: 3   Kills: 11   Gold: 94
HP 17/26  ATK 5  [#############-------]  POISONED
Pack: [+8 +6]   Enemies left: 6

        ##########
        #........#          ###########
        #..!.....#          #.........#
        #....@...############....O....#
        #........#..........b........$#
        ##########          #....>....#
                            #.........#
                            ###########

Venom Bat bites you for 2. You are poisoned!
w/a/s/d move, x rest, e drink, v save, l load, i info, q quit
>
```

---

## ⚔️ Features at a Glance

| | |
|---|---|
| 🗺️ **Procedural dungeons** | Fresh 40×24 mazes of rooms and corridors every level |
| 🔦 **Fog of war** | You see a radius of 6 tiles; explored rooms stay on your map |
| 👹 **Five monsters** | Each has its own AI, from sluggish Orcs to erratic Bats |
| 👑 **Boss fights** | A self-healing Goblin Chief guards every third staircase |
| 🧪 **Potions and poison** | Hoard potions in your backpack and drink them when you need them. A potion also cures poison. |
| 💾 **Save and load** | Checksummed binary saves detect corrupt files |
| 🏆 **Hall of Fame** | Every run is logged and the top five are ranked |
| 🔍 **Info screen** | Press `i` to look behind the curtain: live object counts, RTTI, constructor and destructor traces |

---

## 🚀 Quick Start

You need any C++11 compiler (`g++`, `clang++`, MSVC) and a terminal.

```bash
# 1. Forge the executable
g++ -std=c++11 -Wall -o shadows main.cpp

# 2. Descend
./shadows
```

> 💡 Replace `main.cpp` with whatever you named the source file.

---

## 🎮 How to Play

Walk into a monster to attack it and walk over an item to pick it up. Every action costs one turn, and then the monsters get theirs.

### Controls

Type your command(s) and press **Enter**.

| Key | Action |
|:---:|--------|
| `w` `a` `s` `d` | Move up, left, down, right (bump an enemy to attack) |
| `x` | Rest and catch your breath (+1 HP) |
| `e` | Drink a potion from your backpack |
| `v` | Save your game |
| `l` | Load your saved game |
| `i` | Open the info screen |
| `q` | Flee the dungeon (quit) |

> ⌨️ **Combo input:** every character is its own command, so `dddd` takes four steps right and `ddde` walks three steps and then drinks a potion. A failed command (a wall, an empty pack, a file error) costs no turn and cancels the rest of the line.

### Reading the Map

| Symbol | Meaning | Symbol | Meaning |
|:---:|---|:---:|---|
| `@` | You | `g` | Goblin |
| `#` | Wall | `G` | Goblin Chief 👑 |
| `.` | Floor | `b` | Bat |
| `>` | Stairs down | `B` | Venom Bat ☠️ |
| `!` | Healing potion | `O` | Orc |
| `$` | Gold pile | `/` | Sword (+1 ATK) |

---

## 👹 The Bestiary

| Monster | First seen | Temperament |
|---------|:----------:|-------------|
| **Goblin** `g` | Depth 1 | Simple and stubborn. It sees you, chases you, and stabs you. |
| **Bat** `b` | Depth 2 | Flutters about at random half the time, which makes it hard to pin down. |
| **Orc** `O` | Depth 3 | A brute with a big health pool and hard hits, but it only acts every other turn. |
| **Venom Bat** `B` | Depth 4 | A Bat with a toxic bite. A hit poisons you for 3 turns. |
| **Goblin Chief** `G` | Every 3rd depth | Extra health and damage. It roars and heals itself every third turn while you are in sight. |

---

## 🧭 Survival Tips

1. **Fight in corridors.** Enemies chase in straight lines, so a narrow passage keeps you from being surrounded.
2. **Never waste a potion at full health.** Potions are also your only cure for poison.
3. **Kill Orcs on their off-turn.** They act every other turn, so hit them right after they swing.
4. **Hunt the swords (`/`).** Each one is a permanent +1 attack, and it adds up fast.
5. **Respect the third floor.** A Goblin Chief is waiting, so arrive stocked with potions.
6. **Save before stairs.** Remember that saving does not keep your potions.

---

## 🏆 Scoring

```text
points = depth × 100  +  kills × 10  +  gold
```

When your run ends, you get a full death report: kills by monster type, a stack-based level history, and your standing among your best runs.

### Files the game creates

| File | Contents |
|------|----------|
| `savegame.dat` | Binary save with checksum validation |
| `scores.txt` | Human-readable log of every run |

> ⚠️ **Save quirk:** a save stores depth, HP, attack, gold, and kills. Carried potions are **not** saved, and loading generates a fresh level.

---

## 🧙 The Grimoire: C++ Concepts Inside

This project was built as a teaching piece, so almost every feature is a C++ lesson wearing a costume.

<details>
<summary><b>🔮 Click to open the full spellbook</b></summary>

<br>

| Concept | Where to find it |
|---------|------------------|
| Namespaces | `util` and `rl` |
| Overloading and default arguments | `Dice::roll`, `Dice::chance`, `util::banner` |
| Function and class templates (with non-type parameters) | `clampTo<T>`, `Stack<T>`, `Grid<T,COLS,ROWS>`, `findActive`, `purgeInactive` |
| Operator overloading | `Vec2` (`+ - += == !=`, unary `-`, `<<`), `Backpack` (`+=`, `[]`, `<<`) |
| User-defined conversions | `Gold` ⇄ `int` |
| Static members | `Dice::rolls`, `Entity::liveCount`, `Trace`, `MessageLog` |
| `const` correctness | `Entity::id`, `ZERO`, `DIRS`, `const` methods everywhere |
| Abstract classes and pure virtuals | `Entity`, `Enemy`, `World` |
| Polymorphism and late binding | `interact()`, `takeTurn()`, `describe()` |
| Multilevel, multiple, and hybrid inheritance | `Goblin → GoblinChief`, `Bat : Enemy, Flying`, `VenomBat : Bat, Poisonous` |
| Virtual base classes (the diamond problem) | `Trait` shared by `Flying` and `Poisonous` |
| Constructor and destructor order | Live `Trace` log on the info screen |
| Rule of three and deep copy | `Backpack` (copy constructor, assignment, destructor, self-assignment guard) |
| Deleted copy operations | `Stack<T>` |
| Object slicing | `reportByValue` vs `reportByReference` |
| RTTI and cross-casting | `dynamic_cast` to `Flying` and `Poisonous` in `showInfo()` |
| Exception hierarchy | `GameError → MoveError / BackpackError / FileError` |
| Binary and text streams | `SaveManager` |
| `struct`, `union`, checksums | `SaveData`, `Word`, `checksumOf` |
| STL and functors | `stable_partition`, `count_if`, `sort`, `accumulate`, `map`, `vector` |

</details>

### 🗂️ Code Map

```text
util                          helper functions
rl
├── Dice · Vec2 · Gold        small utility types
├── Trace · MessageLog        global logs
├── GameError                 exceptions
│   ├── MoveError
│   ├── BackpackError
│   └── FileError
├── SaveManager               binary + text file I/O
├── Stack<T> · Grid<T,C,R>    class templates
├── Trait ◄─ Flying           virtual-base "capabilities"
│         ◄─ Poisonous
├── Entity                    abstract base of everything on the map
│   ├── Actor                 anything with HP
│   │   ├── Player
│   │   └── Enemy
│   │       ├── Goblin ── GoblinChief
│   │       ├── Orc
│   │       └── Bat ── VenomBat
│   └── Item
│       ├── Potion
│       ├── GoldPile
│       └── Sword
├── Backpack                  dynamic array, rule of three
├── Dungeon                   generation + fog of war
└── Game                      main loop, rendering, turn logic
```

---

## 🕯️ Known Curses

- Enemies chase greedily (no pathfinding), so a clever wall can foil them.
- The screen is never cleared, so each turn prints below the last.
- Save files are raw binary structs, so they are tied to the platform's `int` size and endianness.

## 🛠️ Ideas for Your Own Quest

- Smarter monster pathfinding (BFS or A*)
- Armor, scrolls, keys, and traps
- Full inventory persistence in saves
- Cross-platform screen clearing and instant (no-Enter) input
- A shop where gold finally has a use

Pull requests and bug reports are welcome. Bring torches.

---

<div align="center">

*May your potions be plentiful and your stairs be near.* 🕯️

</div>
