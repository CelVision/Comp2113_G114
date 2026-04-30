#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <string>
#include <vector>

using namespace std;

// ============ TOWER DATA STRUCTURES ============

struct Tower {
    char symbol;
    string name;
    int cost;
    int hitpoints;
    double attackSpeed;
    int hitRange;
    string art[3];
    string description;
    
    // Tower-specific properties
    double slowdownPercent;  // For Frost Tower
    int damagePercent;       // For Hell Tower
    int currencyBonus;       // For Thief Tower
    int hpBuffPercent;       // For War Drum Tower
    double healPercent;      // For Vampire Tower
    bool penetratesArmor;    // For Armor Penetration Tower
};

// ============ MOB/ENEMY DATA STRUCTURES ============

struct Mob {
    char symbol;
    string name;
    int hp;
    int armor;
    double speed;
    string description;
    
    // Special properties
    bool isFlying;
    double slowEffect;           // Percentage slow
    int slowArea;                // Area of slow effect (e.g., 3*3 = 9)
    double slowDuration;         // Duration in seconds
    int summonCooldown;          // For summoning mobs
    int summonCount;             // How many mobs to summon
    int damageArea;              // Damage area for boss attacks
    int attackCooldown;          // Cooldown for special attacks
};

// ============ GAME MAP DATA STRUCTURES ============

enum TileType {
    BLOCKED,          // Can't place towers, mobs can't go here
    PATH,             // Mobs traverse here
    BUILDABLE,        // Can place towers here
    TOWER,            // Tower is placed here
    BASE              // Base camp location
};

struct Tile {
    TileType type;
    char displayChar;
    int towerIndex;   // Index of tower placed (-1 if none)
    int mobIndex;     // Index of mob on this tile (-1 if none)
};

struct GameMap {
    static const int ROWS = 33;
    static const int COLS = 99;
    
    Tile grid[ROWS][COLS];
    vector<pair<int, int>> pathCoordinates;  // Path route for mobs to follow
};

// ============ WAVE DATA STRUCTURES ============

struct Wave {
    int waveNumber;
    vector<pair<int, double>> mobSpawns;  // Mob index and spawn time
    int totalReward;
};

// ============ GAME STATE STRUCTURES ============

struct GameState {
    string playerName;
    int currentLevel;
    int currentWave;
    int totalWaves;
    int currentMoney;
    int baseCampHP;
    int baseCampMaxHP;
    int score;
    bool levelComplete;
    bool gameLost;
};

#endif // GAMEDATA_H
