//This program is for level 1 mob generating
//Use data from level1_design.txt to generate mobs for level 1
#include "MobSystem.h"
#include <vector>

// Function to create level 1 mob waves based on level1_design.txt
vector<LevelWave> createLevel1Waves() {
    vector<LevelWave> level1Waves;

    // Wave 1: Single pigman from row 3
    LevelWave wave1;
    wave1.waveNumber = 1;
    wave1.totalDuration = 10.0; // Duration for wave 1

    SpawnEvent event1_1;
    event1_1.spawnTime = 0.0;
    event1_1.mobType = 0; // Pigman (index 0)
    event1_1.spawnRow = 3;
    event1_1.spawnCol = 1; // Left side spawn column
    event1_1.speed = 2.0; // tiles per second
    event1_1.routeIndex = -1; // Will be set by MobSystemManager

    wave1.spawnEvents.push_back(event1_1);
    level1Waves.push_back(wave1);

    // Wave 2: Three pigmen from rows 2, 3, 4
    LevelWave wave2;
    wave2.waveNumber = 2;
    wave2.totalDuration = 15.0;

    // First pigman from row 2
    SpawnEvent event2_1;
    event2_1.spawnTime = 0.0;
    event2_1.mobType = 0;
    event2_1.spawnRow = 2;
    event2_1.spawnCol = 1;
    event2_1.speed = 2.0;
    event2_1.routeIndex = -1; // Will be set by MobSystemManager
    wave2.spawnEvents.push_back(event2_1);

    // Second pigman from row 3
    SpawnEvent event2_2;
    event2_2.spawnTime = 1.0; // Spawn 1 second after first
    event2_2.mobType = 0;
    event2_2.spawnRow = 3;
    event2_2.spawnCol = 1;
    event2_2.speed = 2.0;
    event2_2.routeIndex = -1; // Will be set by MobSystemManager
    wave2.spawnEvents.push_back(event2_2);

    // Third pigman from row 4
    SpawnEvent event2_3;
    event2_3.spawnTime = 2.0; // Spawn 2 seconds after first
    event2_3.mobType = 0;
    event2_3.spawnRow = 4;
    event2_3.spawnCol = 1;
    event2_3.speed = 2.0;
    event2_3.routeIndex = -1; // Will be set by MobSystemManager
    wave2.spawnEvents.push_back(event2_3);

    level1Waves.push_back(wave2);

    // Wave 3: Six pigmen with a 3-second wait
    LevelWave wave3;
    wave3.waveNumber = 3;
    wave3.totalDuration = 25.0;

    // First batch: rows 2, 3, 4
    SpawnEvent event3_1;
    event3_1.spawnTime = 0.0;
    event3_1.mobType = 0;
    event3_1.spawnRow = 2;
    event3_1.spawnCol = 1;
    event3_1.speed = 2.0;
    event3_1.routeIndex = -1; // Will be set by MobSystemManager
    wave3.spawnEvents.push_back(event3_1);

    SpawnEvent event3_2;
    event3_2.spawnTime = 1.0;
    event3_2.mobType = 0;
    event3_2.spawnRow = 3;
    event3_2.spawnCol = 1;
    event3_2.speed = 2.0;
    event3_2.routeIndex = -1; // Will be set by MobSystemManager
    wave3.spawnEvents.push_back(event3_2);

    SpawnEvent event3_3;
    event3_3.spawnTime = 2.0;
    event3_3.mobType = 0;
    event3_3.spawnRow = 4;
    event3_3.spawnCol = 1;
    event3_3.speed = 2.0;
    event3_3.routeIndex = -1; // Will be set by MobSystemManager
    wave3.spawnEvents.push_back(event3_3);

    // Wait 3 seconds, then second batch: rows 2, 3, 4
    SpawnEvent event3_4;
    event3_4.spawnTime = 5.0; // 3 seconds wait
    event3_4.mobType = 0;
    event3_4.spawnRow = 2;
    event3_4.spawnCol = 1;
    event3_4.speed = 2.0;
    event3_4.routeIndex = -1; // Will be set by MobSystemManager
    wave3.spawnEvents.push_back(event3_4);

    SpawnEvent event3_5;
    event3_5.spawnTime = 6.0;
    event3_5.mobType = 0;
    event3_5.spawnRow = 3;
    event3_5.spawnCol = 1;
    event3_5.speed = 2.0;
    event3_5.routeIndex = -1; // Will be set by MobSystemManager
    wave3.spawnEvents.push_back(event3_5);

    SpawnEvent event3_6;
    event3_6.spawnTime = 7.0;
    event3_6.mobType = 0;
    event3_6.spawnRow = 4;
    event3_6.spawnCol = 1;
    event3_6.speed = 2.0;
    event3_6.routeIndex = -1; // Will be set by MobSystemManager
    wave3.spawnEvents.push_back(event3_6);

    level1Waves.push_back(wave3);

    // Wave 4: Eight pigmen with wait - 3+2+3 pattern
    LevelWave wave4;
    wave4.waveNumber = 4;
    wave4.totalDuration = 30.0;

    // First batch: rows 2, 3, 4
    SpawnEvent event4_1;
    event4_1.spawnTime = 0.0;
    event4_1.mobType = 0;
    event4_1.spawnRow = 2;
    event4_1.spawnCol = 1;
    event4_1.speed = 2.0;
    event4_1.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_1);

    SpawnEvent event4_2;
    event4_2.spawnTime = 1.0;
    event4_2.mobType = 0;
    event4_2.spawnRow = 3;
    event4_2.spawnCol = 1;
    event4_2.speed = 2.0;
    event4_2.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_2);

    SpawnEvent event4_3;
    event4_3.spawnTime = 2.0;
    event4_3.mobType = 0;
    event4_3.spawnRow = 4;
    event4_3.spawnCol = 1;
    event4_3.speed = 2.0;
    event4_3.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_3);

    // Wait 3 seconds, then second batch: 2 pigmen per row
    SpawnEvent event4_4;
    event4_4.spawnTime = 5.0;
    event4_4.mobType = 0;
    event4_4.spawnRow = 2;
    event4_4.spawnCol = 1;
    event4_4.speed = 2.0;
    event4_4.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_4);

    SpawnEvent event4_5;
    event4_5.spawnTime = 5.5;
    event4_5.mobType = 0;
    event4_5.spawnRow = 2;
    event4_5.spawnCol = 1;
    event4_5.speed = 2.0;
    event4_5.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_5);

    SpawnEvent event4_6;
    event4_6.spawnTime = 6.0;
    event4_6.mobType = 0;
    event4_6.spawnRow = 3;
    event4_6.spawnCol = 1;
    event4_6.speed = 2.0;
    event4_6.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_6);

    SpawnEvent event4_7;
    event4_7.spawnTime = 6.5;
    event4_7.mobType = 0;
    event4_7.spawnRow = 3;
    event4_7.spawnCol = 1;
    event4_7.speed = 2.0;
    event4_7.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_7);

    SpawnEvent event4_8;
    event4_8.spawnTime = 7.0;
    event4_8.mobType = 0;
    event4_8.spawnRow = 4;
    event4_8.spawnCol = 1;
    event4_8.speed = 2.0;
    event4_8.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_8);

    SpawnEvent event4_9;
    event4_9.spawnTime = 7.5;
    event4_9.mobType = 0;
    event4_9.spawnRow = 4;
    event4_9.spawnCol = 1;
    event4_9.speed = 2.0;
    event4_9.routeIndex = -1; // Will be set by MobSystemManager
    wave4.spawnEvents.push_back(event4_9);

    level1Waves.push_back(wave4);
    
    return level1Waves;
}






