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

1.base camp("[B]"): hp 10 -> 7 -> 4 -> 1
the target that the player must protect, every enemy that reaches it will cause one point damage to the base

2.path("-"): hp ∞
the path restricts the enemy's movement

3.The location where structures can be set (".")：
players can put structures on these area

4.The location where structures can't be set("")
players can't put structures on these area

Defensive Structures

1.currency generator("$"): cost $50  hitpoint 0  efficiency $50/10s
the most foundational tower, support the players with sufficient currency, unlocked in the first level

2.arrow tower("A"): cost $50  hitpoint 100  attackspeed 1s  hitrange 5
a basic defensive tower, conducts single-target attack, has a middle range of attack, unlocked in the first level

3.lazer tower("L"): cost $100  hitpoint 50  attackspeed 1.5s  hitrange ∞
a denfensive tower that attacks all enemies in a row infront of it, unlocked in the second level

4.forst tower("F"): cost $50  hitpoint 10  attackspeed 1s  hitrange 5, 3*3  slowdown 50%
a defensive tower that provides the players with crowd control ability, conducts multi-target attack, unlocked in the third level

5.earth quake tower("E"): cost $200  hitpoint 100  attackspeed 1s  hitrange 5
a defensive tower that conducts multi-target attack to enemies in a circle range, it can kill a group of low-hp enenmies at once but the cost of it is rather high, unlocked in the fourth level

6.currency recycler("_"): cost $50 hitpoint 0 efficiency $20/enemy
an upgrade accessory that allows a defensive tower to gain currency when killing enemies, it can effectively solve the problem of not having enough currency to build structures, unlocked in the fifth level

7.hell tower("H"): cost $200  hitpoint 50%(at least 50)  attackspeed 1s  hitrange 5
a denfensive tower that conducts single-target attack with a percentage damage, it can easily deal with high-hp enemies, it will first attack the enemy with the highest hp within range, unlocked in the sixth level

8.





