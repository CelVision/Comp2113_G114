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

1.base camp("[B]"): hp 10/10 -> 7/10 -> 4/10 -> 1/10
the target that the player must protect, every enemy that reaches it will cause one point damage to the base

2.path("-"): hp ∞
the path restricts the enemy's movement

3.The location where structures can be set (".")：
players can put structures on these area

4.The location where structures can't be set(""):
players can't put structures on these area

Defensive Structures

1.currency generator("$"): cost $50  hitpoints 0  efficiency $50/10s
the most foundational tower, support the players with sufficient currency, unlocked in the first level

2.arrow tower("A"): cost $50  hitpoints 100  attack speed 1s  hitrange 5
a basic defensive tower, conducts single-target attack, has a middle range of attack, unlocked in the first level

3.laser tower("L"): cost $100  hitpoints 50  attack speed 1.5s  hitrange ∞
a denfensive tower that attacks all enemies in a row infront of it, unlocked in the second level

4.frost tower("F"): cost $50  hitpoints 10  attack speed 1s  hitrange 5, 3*3  slowdown 50%
a defensive tower that provides the players with crowd control ability, conducts multi-target attack, unlocked in the third level

5.earthquake tower("E"): cost $200  hitpoints 100  attack speed 1s  hitrange 5
a defensive tower that conducts multi-target attack to enemies in a circle range, it can kill a group of low-hp enemies at once but the cost of it is rather high, unlocked in the fourth level

6.currency recycler("_"): cost $50 hitpoints 0 efficiency $20/enemy killed
an upgrade accessory that allows a defensive tower to gain currency when killing enemies, it can effectively solve the problem of not having enough currency to build structures, unlocked in the fifth level

7.hell tower("H"): cost $200  hitpoints 50% of current hp(at least 50, atmost 500)  attack speed 1s  hitrange 5
a denfensive tower that conducts single-target attack with a percentage damage, it can easily deal with high-hp enemies, it will first attack the enemy with the highest hp within range, unlocked in the sixth level

8.armor penetration tower("P"): cost $150  hitpoints 50(hp)/50(armor) attackspeed 1s hitrange 5
a defensive tower that conducts single-target attack and breaks the armor(the enemy only bears 1 point damage for every hit when the armor exists) of the enemies, unlocked in the seventh level

9.war drum tower("D"): cost $150  hitpoints 0  bonus 25%  range 3
a defensive tower that dooesn't attack but provides buffs to all towers within 5*5 area, unlocked in the eighth level

10.vampire tower("V"): cost $400  hitpoints 100  attack speed 1s  hitrange 5  heal 0.05% of the damage
a defensive tower that can heal the base camp for 0.05% of the damage it caused, in average, it can heal 1 hp for the base camp every 20 seconds, unlocked in the ninth level

Enemies Types

1.goblins ("g"): hp 200 armor 0 speed 1.0
most common enemies, can be killed by arrow towera within 2 shots

2.skeletons("s"): hp 100  armor 0  speed 1.5
most common enemies, run very fast, always appear in groups, can be easily killed with laser towers

3.fast scout("f"): hp 300  armor 0  speed 1.5
an enemy that runs fast with medium hp, needs frost towers to slow it down

4.orcs("o"): hp 400  armor 0  speed 0.8
an enemy that always appears in an army, walks slow but has high hp, can be killed quickly if they are in the range of the earthquake towers

5.behemoth("b"): hp 1000 armor 0 speed 1.0
an enemy that has high-hp, the hell towers can lower its hp in a short period of time

6.armored troops("a"): hp 100  armor 10 speed 1.0
an enemy that has low-hp but is armed with armor, armor penetration towers can defeat them easily

7.giant overlord("l"): hp 2000 armor 200 speed 0.8
an enemy that has high hp and armor, cooperation between towers is needed to defeat it

8.rapid raider("r"): hp 400 armor 0 speed 2.0
an enemy that runs extremely fast, it can easily reach the base camp, heal the camp with vampire tower
