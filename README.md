# Comp2113_G114
Comp2113 group work

## Team Members

Liu Guren  
Zhou Yuzhou  
Shen Ziheng  
Ma Taoran  

## Basic Information of Our Game

This is a tower defense game where players can accumulate currency to build defensive structures and fend off enemy attacks. There are a total of ten selectable levels, with level 10 serving as the endless mode where you can use all the unlocked buildings. Enemy attacks will become increasingly intense until they overwhelm the player. After being defeated, your score will be calculated, so aim for a higher ranking on the leaderboard. Enjoy the game!

## Implemented Features

The game implements several key features to provide an engaging tower defense experience:

1. **Tower Placement System**: Players can place various types of towers on buildable areas to defend against enemies.
2. **Enemy Wave System**: Enemies spawn in waves with increasing difficulty across levels.
3. **Combat Mechanics**: Towers automatically attack enemies within range, with different tower types having unique abilities.
4. **Resource Management**: Players earn currency by defeating enemies and use it to build towers.
5. **Level Progression**: 10 levels with escalating difficulty, including an endless mode.
6. **User Interface**: Console-based UI for level selection, game display, and tower placement.

### How Coding Elements Support the Features

1. **Generation of random events**: Used in enemy spawning and wave generation to create unpredictable gameplay. For example, random enemy types and timings in waves keep the game challenging and replayable.

2. **Data structures for storing data**: Structs like `Tower`, `Mob`, and `GameMap` store game data efficiently. Vectors are used for dynamic arrays of towers, mobs, and map tiles, supporting the tower placement and enemy management features.

3. **Dynamic memory management**: Vectors and other containers handle dynamic allocation of game objects like towers and enemies, allowing the game to scale with different levels and player actions.

4. **File input/output (e.g., for loading/saving data)**: The game loads level maps, enemy data, and tower configurations from text files in the `data/` directory, enabling easy content updates and supporting the multiple difficulty levels feature.

5. **Program codes in multiple files**: The codebase is organized into multiple files (`Game.cpp`, `GameData.h`, `GameMap.cpp`, `MobSystem.h`, `UI.cpp`), promoting modularity and maintainability, which supports all features by separating concerns like game logic, data structures, and UI.

6. **Multiple Difficulty Levels**: The game offers 10 selectable levels with increasing complexity, allowing players to choose their preferred challenge level. This is supported by file-based level data and progressive enemy scaling.

## Non-Standard C/C++ Libraries

The project uses the following non-standard C/C++ libraries:

- `<conio.h>`: Used for console input/output functions like `getch()` for user input handling in the UI.
- `<windows.h>`: Used for Windows-specific console manipulation functions like `SetConsoleCursorPosition()` and `GetStdHandle()` to control cursor position and screen clearing, supporting the game's console-based interface.

These libraries enable the console UI features, allowing for real-time game display and user interaction.

## Compilation and Execution Instructions

### Prerequisites
- Windows operating system
- g++ compiler (MinGW or similar)

### Compilation
1. Open a terminal in the project root directory.
2. Run the following command to compile:
   ```
   make
   ```
   Or manually:
   ```
   g++ -std=c++11 -Wall src/core/Game.cpp -o Game.exe
   ```

### Execution
1. After compilation, run the game:
   ```
   .\Game.exe
   ```
2. Follow the on-screen prompts to enter your name and select a level.
3. Use the game controls to place towers and defend against enemies.

### Quick Start
- Select a level from 1-10.
- Use arrow keys to navigate the tower selection menu.
- Place towers on buildable areas (marked with '.') to defend the base camp ('M').
- Survive waves of enemies to progress and earn a high score.

Resource Packages

The runtime resources are grouped by package-style paths:

1. data/maps/map_for_levelX.txt - level map package, X is the 1-based level index
2. data/levels/levelX_design.txt - wave package for the same 1-based level index
3. data/meta/MobData.txt - mob data package
4. data/meta/TowerData.txt - tower data package
5. docs/UI_Design.txt - UI layout package
6. docs/towers.txt / docs/mobs.txt - ASCII art and design reference packages

Game Scene Arrangement

1.base camp("M")  hp: 10/10

the target that the player must protect, every enemy that reaches it will cause one point damage to the base, the hp of the base may decline as the difficulty of the game increases, but the player will have a way to heal the base

2.path(" ")  hp: ∞

the path restricts the enemy's movement

3.The location where structures can be set (".")

players can put structures on these area

Defensive Structures

1.arrow tower("A")  cost: $50  hitpoints: 100  attack speed: 1s  hitrange: 5

a basic defensive tower, conducts single-target attack, has a middle range of attack, unlocked in the first level

2.laser tower("L")  cost: $100  hitpoints: 50  attack speed: 1.5s  hitrange: ∞ (unable to attack flying enenmies)

a denfensive tower that attacks all enemies in a row infront of it, unlocked in the second level

3.frost tower("F")  cost: $50  hitpoints: 10  attack speed: 1s  hitrange: 5, 3*3  slowdown: 50%

a defensive tower that provides the players with crowd control ability, conducts multi-target attack, unlocked in the third level

4.earthquake tower("E")  cost: $200  hitpoints: 100  attack speed: 1s  hitrange: 5 (unable to attack flying enemies)

a defensive tower that conducts multi-target attack to enemies in a circle range, it can kill a group of low-hp enemies at once but the cost of it is rather high, unlocked in the fourth level

5.armor penetration tower("P")  cost: $150  hitpoints: 150(double when attackinig armors)  attackspeed: 1s  hitrange: 5

a defensive tower that conducts single-target attack and breaks the armor(the enemy only bears 1 point damage for every hit when the armor exists) of the enemies, unlocked in the fifth level

6.war drum tower("D")  cost: $150  hitpoints: 0  bonus: +25% hitspeed range: 5

a defensive tower that dooesn't attack but provides buffs to all towers within 5*5 area, unlocked in the sixth level

7.hell tower("H")  cost: $200  hitpoints: 50% of current hp(at least 50, atmost 500, only 50 points to armors)  attack speed: 1s  hitrange: 5

a denfensive tower that conducts single-target attack with a percentage damage, it can easily deal with high-hp enemies, it will first attack the enemy with the highest hp within range, unlocked in the seventh level

8.thief tower("T")  cost: $200  hitpoints:0  bonus: +25% currency gained  range: 5 

a defensive tower that increase the currency gained when killing the enemies, unlocked in the eighth level

9.vampire tower("V")  cost: $400  hitpoints: 100  attack speed: 1s  hitrange: 5  heal: 0.05% of the damage

a defensive tower that can heal the base camp for 0.05% of the damage it caused, in average, it can heal 1 hp for the base camp every 20 seconds, unlocked in the ninth level

Enemies Types

1.pigman ("p") hp: 200 armor: 0 speed: 2  appear in level 1

2.houndling("h") hp: 150 armor: 0 speed: 4  appear in level 2

3.werewolf*("W") hp: 350 armor: 100 speed: 3  appear in level 3

an enemy that runs fast with medium hp, needs frost towers to slow it down

4.mini mammon("m") hp: 500 armor: 0 speed: 1.5  appear in level 4

5.Armored mammon*("M") hp: 1000 armor:500 speed: 1.0  appear in level 5

an enemy that has high-hp, the hell towers can lower its hp in a short period of time

6.Birdman ("^") hp: 150 armor: 0 speed: 2.0 [Flying]   appear in level 6

7.Batman ("’") hp: 50 armor: 0 speed: 5.0 [Flying]  appear in level 6

8.Pigman Berserker* ("P") hp: 400 armor: 400 speed: 2.0 -> 4.0 (no armor) appear in level 7

9.Spiderman ("S") hp: 300 armor: 0 speed: 3 slow effect: 50% 3*3 5s appear in level 7

10.Alpha Wolf* ("A") hp: 500 armor: 200 speed: 1.5 summon cooldown: 5s appear in level 8
Summon 2 houndlings.

11.Dragon****  appear in level 9

(1) ("^D^") hp: 2000 armor: 0 speed: 1.5 
damage area: 3*3 
cooldown: 15s

(2) ("D") hp: 2000 armor: 1000 speed: 1
Slow effect: 50% full-screen 5s
cooldown: 15s

(3) ("!D!") hp: 1000 armor: 0 speed: 2

every extra "*" means an extra hitpoint to the base when attacking it
