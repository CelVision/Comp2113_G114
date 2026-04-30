# Comp2113_G114
Comp2113 group work

Team Members

Liu Guren  
Zhou Yuzhou  
Shen Ziheng  
Ma Taoran  

Basic Information of Our Game

This is a tower defense game where players can accumulate currency to build defensive structures and fend off enemy attacks. There are a total of nine levels, and more buildings will be unlocked as you progress through the stages. At the end, there is an endless mode where you can use all the unlocked buildings. Enemy attacks will become increasingly intense until they overwhelm the player. After being defeated, your score will be calculated, so aim for a higher ranking on the leaderboard. Enjoy the game!

Game Scene Arrangement

1.base camp("[B]")  hp: 10/10 -> 7/10 -> 4/10 -> 1/10

the target that the player must protect, every enemy that reaches it will cause one point damage to the base, the hp of the base may decline as the difficulty of the game increases, but the player will have a way to heal the base

2.path("-")  hp: ∞

the path restricts the enemy's movement

3.The location where structures can be set (".")

players can put structures on these area

4.The location where structures can't be set("")

players can't put structures on these area

Defensive Structures

1.arrow tower("A")  cost: $50  hitpoints: 100  attack speed: 1s  hitrange: 5

a basic defensive tower, conducts single-target attack, has a middle range of attack, unlocked in the first level

2.laser tower("L")  cost: $100  hitpoints: 50  attack speed: 1.5s  hitrange: ∞

a denfensive tower that attacks all enemies in a row infront of it, unlocked in the second level

3.frost tower("F")  cost: $50  hitpoints: 10  attack speed: 1s  hitrange: 5, 3*3  slowdown: 50%

a defensive tower that provides the players with crowd control ability, conducts multi-target attack, unlocked in the third level

4.earthquake tower("E")  cost: $200  hitpoints: 100  attack speed: 1s  hitrange: 5

a defensive tower that conducts multi-target attack to enemies in a circle range, it can kill a group of low-hp enemies at once but the cost of it is rather high, unlocked in the fourth level

5.hell tower("H")  cost: $200  hitpoints: 50% of current hp(at least 50, atmost 500)  attack speed: 1s  hitrange: 5

a denfensive tower that conducts single-target attack with a percentage damage, it can easily deal with high-hp enemies, it will first attack the enemy with the highest hp within range, unlocked in the fifth level

6.thief tower("T")  cost: $200  hitpoints:0  bonus: +25% currency gained  range: 5 

a defensive tower that increase the currency gained when killing the enemies, unlocked in the sixth level

7.armor penetration tower("P")  cost: $150  hitpoints: 50(hp)/50(armor)  attackspeed: 1s  hitrange: 5

a defensive tower that conducts single-target attack and breaks the armor(the enemy only bears 1 point damage for every hit when the armor exists) of the enemies, unlocked in the seventh level

8.war drum tower("D")  cost: $150  hitpoints: 0  bonus: +25% hitpoints range: 3

a defensive tower that dooesn't attack but provides buffs to all towers within 5*5 area, unlocked in the eighth level

9.vampire tower("V")  cost: $400  hitpoints: 100  attack speed: 1s  hitrange: 5  heal: 0.05% of the damage

a defensive tower that can heal the base camp for 0.05% of the damage it caused, in average, it can heal 1 hp for the base camp every 20 seconds, unlocked in the ninth level

Enemies Types

1.pigman ("p") hp: 200 armor: 0 speed: 2

2.houndling("h") hp: 150 armor: 0 speed: 4

3.werewolf*("W") hp: 350 armor: 100 speed: 3

an enemy that runs fast with medium hp, needs frost towers to slow it down

4.mini mammon("m") hp: 500 armor: 0 speed: 1.5

5.Armored mammon*("M") hp: 1000 armor:500 speed: 1.0

an enemy that has high-hp, the hell towers can lower its hp in a short period of time

6.Birdman ("^") hp: 150 armor: 0 speed: 2.0 [Flying]

7.Batman ("’") hp: 50 armor: 0 speed: 5.0 [Flying]

8.Pigman Berserker* ("P") hp: 400 armor: 400 speed: 2.0 -> 4.0 (no armor)

9.Spiderman ("S") hp: 300 armor: 0 speed: 3 slow effect: 50% 3*3 5s

10.Alpha Wolf ("A") hp: 500 armor: 200 speed: 1.5 summon cooldown: 5s
Summon 2 houndlings.

11.Dragon 
(1) ("^D^") hp: 2000 armor: 0 speed: 1.5 
damage area: 3*3 
cooldown: 15s
(2) ("D") hp: 2000 armor: 1000 speed: 1
Slow effect: 50% full-screen 5s
cooldown: 15s
(3) ("!D!") hp: 1000 armor: 0 speed: 2
