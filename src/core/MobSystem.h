#ifndef MOBSYSTEM_H
#define MOBSYSTEM_H

#include "GameData.h"
#include <queue>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <cctype>

using namespace std;

// Forward declarations for console functions
void setCursorPosition(int x, int y);
void setTextColor(int color);
void resetTextColor();

// ============ MOB MODIFIER STRUCTURE (for dynamic stats management) ============

struct MobModifier {
    // Multiplicative and additive modifiers for mob stats
    double speedMultiplier;      // 1.0 = no change, 0.5 = 50% slower, 2.0 = 2x faster
    double hpMultiplier;         // 1.0 = no change, 2.0 = double HP
    int armorBonus;              // Flat armor bonus (e.g., +50 armor)
    double goldMultiplier;       // 1.0 = no change, 1.5 = 1.5x gold reward
    double damageMultiplier;     // For mob's damage output to towers/base
    int slowEffect;              // Additional slow percentage (stacks with existing)
    double slowDuration;         // Override slow duration in seconds
    bool isSlowed;               // Whether mob is currently slowed
    double slowedUntilTime;      // Game time when slow expires
    
    MobModifier() 
        : speedMultiplier(1.0), hpMultiplier(1.0), armorBonus(0), 
          goldMultiplier(1.0), damageMultiplier(1.0), slowEffect(0), 
          slowDuration(0), isSlowed(false), slowedUntilTime(0.0) {}
};

// ============ MOB INSTANCE STRUCTURE ============

struct MobInstance {
    int mobType;           // Index into mobs vector
    double posRow, posCol; // Current floating-point position
    double velocityRow, velocityCol; // Velocity per frame
    int targetWaypoint;   // Current target waypoint in path
    int health;            // Current health
    int maxHealth;         // Max health (base * hpMultiplier)
    bool isAlive;
    double lastUpdateTime;
    int spawnWaveId;      // Which wave this mob belongs to
    double moveSpeed;     // tiles per second (base)
    double modifiedSpeed; // tiles per second (after modifiers applied)
    int routeIndex;       // Cached route used by this mob
    int routeStepIndex;   // Next waypoint index in route
    bool passedCheckpoint;
    int checkpointIndex;
    bool reachedBase;
    
    MobModifier modifier; // Dynamic modifiers for this mob
    int baseGold;         // Base gold reward before multiplier
    int spawnPointRow;    // Original spawn row
    int spawnPointCol;    // Original spawn column

    MobInstance(int type, double row, double col, double speed, int wave)
            : mobType(type), posRow(row), posCol(col), targetWaypoint(0), 
                isAlive(true), lastUpdateTime(0), spawnWaveId(wave), moveSpeed(speed),
                modifiedSpeed(speed), routeIndex(-1), routeStepIndex(0), 
                passedCheckpoint(false), checkpointIndex(-1), reachedBase(false),
                spawnPointRow((int)row), spawnPointCol((int)col), baseGold(0), maxHealth(0) {
            velocityRow = 0;
            velocityCol = 0;
    }
};

// ============ SPAWN EVENT & WAVE (Level Design) ============

struct SpawnEvent {
    double spawnTime;      // Time in seconds when this mob spawns
    int mobType;           // Type of mob to spawn
    int spawnRow;          // Spawn row
    int spawnCol;          // Spawn column (spawn point)
    double speed;          // Mob speed (tiles per second)
    int routeIndex;        // Precomputed navigation route for this spawn point
};

struct NavigationRoute {
    int spawnRow;
    int spawnCol;
    vector<pair<int, int>> nodes;     // Spawn -> base path nodes
    vector<char> commands;             // Direction commands: U/D/L/R
    pair<int, int> checkpoint;         // Debug checkpoint on the road
    int checkpointNodeIndex;           // Index of checkpoint in nodes
};

struct LevelWave {
    int waveNumber;
    vector<SpawnEvent> spawnEvents;
    double totalDuration;  // Total time until wave completes
};

// ============ MOB SYSTEM MANAGER ============

class MobSystemManager {
private:
    vector<MobInstance> activeMobs;
    vector<LevelWave> waves;
    vector<pair<int, int>> globalPath; // Path from spawn to base for all mobs
    vector<NavigationRoute> navigationRoutes;
    
    int currentWaveIndex;
    double currentWaveTime;
    double gameTime;
    
    vector<Mob>& mobTypes;
    GameMap& gameMap;
    vector<int> mobGolds;
    
public:
        // Accept references to player's money and base HP so mob system can
        // award gold and deduct base HP when appropriate.
        int &moneyRef;
        int &baseHPRef;

        MobSystemManager(vector<Mob>& mobTypes_, GameMap& gameMap_, int &money_, int &baseHP_) 
                : mobTypes(mobTypes_), gameMap(gameMap_), currentWaveIndex(0), 
                    currentWaveTime(0), gameTime(0), moneyRef(money_), baseHPRef(baseHP_) {
                loadMobGolds();
        }
    
    // Load level design from file (e.g., "data/levels/level1_design.txt")
    void loadLevelDesign(int level) {
        waves.clear();
        navigationRoutes.clear();
        currentWaveTime = 0;
        gameTime = 0;
        string filename = buildLevelDesignPath(level);
        ifstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Could not load level design: " << filename << endl;
            return;
        }
        
        string line;
        LevelWave currentWave;
        currentWave.waveNumber = 0;
        currentWave.totalDuration = 0;
        double designTime = 0;
        
        while (getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '-' || line[0] == '[') continue;
            
            // Check for wave header (e.g., "1.1", "2.1")
            if (line.find_first_of("0123456789") == 0) {
                // Extract wave data format: "X.Y mobname from row R"
                // Example: "1.1 pigman from row 3 left side"
                
                int waveNum = stoi(line.substr(0, line.find('.')));
                
                if (currentWave.waveNumber != waveNum && currentWave.waveNumber > 0) {
                    waves.push_back(currentWave);
                    currentWave.spawnEvents.clear();
                    currentWave.totalDuration = 0;
                    currentWave.totalDuration = 0;
                }
                
                currentWave.waveNumber = waveNum;
                
                // Parse spawn data
                SpawnEvent event;
                event.spawnTime = designTime;
                event.routeIndex = -1;
                
                // Extract mob type and row
                if (line.find("pigman") != string::npos) {
                    event.mobType = 0; // Pigman
                    event.speed = 2.0; // tiles per second
                } else if (line.find("houndling") != string::npos) {
                    event.mobType = 1;
                    event.speed = 4.0;
                } else if (line.find("werewolf") != string::npos) {
                    event.mobType = 2;
                    event.speed = 3.0;
                } else {
                    continue; // Unknown mob type
                }
                
                // Extract row number
                size_t rowPos = line.find("row");
                if (rowPos != string::npos) {
                    size_t numStart = rowPos + 4;
                    event.spawnRow = stoi(line.substr(numStart));
                }
                
                // Find spawn column on this row (prefer an explicit '+' spawn tile)
                event.spawnCol = findSpawnColumnForRow(event.spawnRow);
                event.routeIndex = findRouteIndexForSpawn(event.spawnRow, event.spawnCol);
                
                currentWave.spawnEvents.push_back(event);
                currentWave.totalDuration = event.spawnTime + 5.0; // Default 5s between spawns
            }
            
            // Check for wait command: "[wait for X seconds]"
            if (line.find("[wait") != string::npos) {
                size_t numStart = line.find("for") + 4;
                double waitTime = stod(line.substr(numStart));
                designTime += waitTime;
            }
        }
        
        if (currentWave.waveNumber > 0) {
            waves.push_back(currentWave);
        }
        
        file.close();
    }

    // Parse data/meta/MobData.txt to extract gold rewards per mob name
    void loadMobGolds() {
        mobGolds.clear();
        mobGolds.resize(mobTypes.size(), 0);

        ifstream f(buildMobDataPath());
        if (!f.is_open()) return;

        string line;
        string currentName;
        // Map name -> gold
        unordered_map<string,int> goldMap;

        while (getline(f, line)) {
            if (line.find("Enemy") == 0) {
                size_t colon = line.find(':');
                if (colon != string::npos) {
                    currentName = line.substr(colon+1);
                    while (!currentName.empty() && isspace(currentName[0])) currentName.erase(0,1);
                    while (!currentName.empty() && isspace(currentName.back())) currentName.pop_back();
                }
            }
            if (line.find("Gold:") != string::npos && !currentName.empty()) {
                size_t pos = line.find("Gold:");
                string num = line.substr(pos+5);
                while (!num.empty() && isspace(num[0])) num.erase(0,1);
                int g = stoi(num);
                goldMap[currentName] = g;
                currentName.clear();
            }
        }
        f.close();

        for (size_t i = 0; i < mobTypes.size(); ++i) {
            auto it = goldMap.find(mobTypes[i].name);
            if (it != goldMap.end()) mobGolds[i] = it->second;
            else mobGolds[i] = 0;
        }
    }
    
    // Precompute navigation routes for every distinct spawn point.
    void findGlobalPath() {
        globalPath.clear();
        navigationRoutes.clear();

        int baseRow = -1, baseCol = -1;
        for (int r = 0; r < GameMap::ROWS; r++) {
            for (int c = 0; c < GameMap::COLS; c++) {
                if (gameMap.grid[r][c].type == BASE) {
                    baseRow = r;
                    baseCol = c;
                    break;
                }
            }
            if (baseRow != -1) break;
        }

        if (baseRow == -1) {
            cerr << "Could not find base!" << endl;
            return;
        }

        for (int row = 0; row < GameMap::ROWS; row++) {
            for (int col = 0; col < GameMap::COLS; col++) {
                if (!isSpawnTile(row, col)) continue;

                vector<pair<int, int>> path = findPathFromSpawn(row, col, baseRow, baseCol);
                if (path.empty()) continue;

                NavigationRoute route;
                route.spawnRow = row;
                route.spawnCol = col;
                route.nodes = path;
                route.commands = buildCommandsFromPath(path);
                route.checkpointNodeIndex = (path.size() > 2) ? (int)path.size() / 2 : 0;
                route.checkpoint = path[route.checkpointNodeIndex];
                navigationRoutes.push_back(route);
            }
        }

        if (!navigationRoutes.empty()) {
            globalPath = navigationRoutes[0].nodes;
        }
    }

    bool isWalkableTile(int row, int col) const {
        if (row < 0 || row >= GameMap::ROWS || col < 0 || col >= GameMap::COLS) return false;
        return gameMap.grid[row][col].type == PATH || gameMap.grid[row][col].type == BASE;
    }

    bool isSpawnTile(int row, int col) const {
        if (!isWalkableTile(row, col)) return false;
        return gameMap.grid[row][col].displayChar == '+';
    }

    int findSpawnColumnForRow(int spawnRow) const {
        if (spawnRow < 0 || spawnRow >= GameMap::ROWS) return 0;

        for (int col = 0; col < GameMap::COLS; col++) {
            if (gameMap.grid[spawnRow][col].displayChar == '+') return col;
        }

        for (int col = 0; col < GameMap::COLS; col++) {
            if (gameMap.grid[spawnRow][col].type == PATH) return col;
        }

        return 0;
    }

    int findRouteIndexForSpawn(int spawnRow, int spawnCol) const {
        for (int i = 0; i < (int)navigationRoutes.size(); i++) {
            if (navigationRoutes[i].spawnRow == spawnRow && navigationRoutes[i].spawnCol == spawnCol) {
                return i;
            }
        }
        return -1;
    }

    vector<pair<int, int>> findPathFromSpawn(int startRow, int startCol, int baseRow, int baseCol) const {
        vector<vector<bool>> visited(GameMap::ROWS, vector<bool>(GameMap::COLS, false));
        vector<vector<pair<int, int>>> parent(
            GameMap::ROWS,
            vector<pair<int, int>>(GameMap::COLS, make_pair(-1, -1))
        );
        queue<pair<int, int>> q;

        q.push(make_pair(startRow, startCol));
        visited[startRow][startCol] = true;

        bool found = false;
        while (!q.empty()) {
            pair<int, int> current = q.front();
            q.pop();

            int row = current.first;
            int col = current.second;
            if (row == baseRow && col == baseCol) {
                found = true;
                break;
            }

            int dr[4] = {-1, 1, 0, 0};
            int dc[4] = {0, 0, -1, 1};
            for (int i = 0; i < 4; i++) {
                int nextRow = row + dr[i];
                int nextCol = col + dc[i];
                if (!isWalkableTile(nextRow, nextCol)) continue;
                if (visited[nextRow][nextCol]) continue;

                visited[nextRow][nextCol] = true;
                parent[nextRow][nextCol] = make_pair(row, col);
                q.push(make_pair(nextRow, nextCol));
            }
        }

        if (!found) {
            cerr << "No valid path from spawn (" << startRow << "," << startCol << ") to base!" << endl;
            return vector<pair<int, int>>();
        }

        vector<pair<int, int>> reversedPath;
        int currentRow = baseRow;
        int currentCol = baseCol;
        while (!(currentRow == startRow && currentCol == startCol)) {
            reversedPath.push_back(make_pair(currentRow, currentCol));
            pair<int, int> previous = parent[currentRow][currentCol];
            if (previous.first == -1 || previous.second == -1) {
                cerr << "Path reconstruction failed for spawn (" << startRow << "," << startCol << ")!" << endl;
                return vector<pair<int, int>>();
            }
            currentRow = previous.first;
            currentCol = previous.second;
        }
        reversedPath.push_back(make_pair(startRow, startCol));

        return vector<pair<int, int>>(reversedPath.rbegin(), reversedPath.rend());
    }

    vector<char> buildCommandsFromPath(const vector<pair<int, int>>& path) const {
        vector<char> commands;
        if (path.size() < 2) return commands;

        for (size_t i = 1; i < path.size(); i++) {
            int rowDelta = path[i].first - path[i - 1].first;
            int colDelta = path[i].second - path[i - 1].second;

            if (rowDelta == -1 && colDelta == 0) commands.push_back('U');
            else if (rowDelta == 1 && colDelta == 0) commands.push_back('D');
            else if (rowDelta == 0 && colDelta == -1) commands.push_back('L');
            else if (rowDelta == 0 && colDelta == 1) commands.push_back('R');
            else commands.push_back('?');
        }

        return commands;
    }
    
    // Update all mobs each frame (dt = delta time in seconds)
    void update(double dt) {
        gameTime += dt;
        currentWaveTime += dt;
        
        // Spawn new mobs if current wave is active
        if (currentWaveIndex < (int)waves.size()) {
            LevelWave& wave = waves[currentWaveIndex];
            
            for (auto& event : wave.spawnEvents) {
                if (event.spawnTime <= currentWaveTime && 
                    event.spawnTime > currentWaveTime - dt) { // Spawned this frame
                    
                    MobInstance newMob(event.mobType, (double)event.spawnRow, 
                                      (double)event.spawnCol, event.speed, currentWaveIndex);
                    
                    // Initialize stats from mob type data
                    newMob.health = mobTypes[event.mobType].hp;
                    newMob.maxHealth = mobTypes[event.mobType].hp;
                    newMob.baseGold = mobGolds[event.mobType];
                    newMob.modifiedSpeed = event.speed;
                    
                    // Initialize modifier (defaults to no modifications)
                    newMob.modifier = MobModifier();
                    
                    // Set route and checkpoint info
                    newMob.routeIndex = event.routeIndex;
                    newMob.routeStepIndex = 0;
                    newMob.passedCheckpoint = false;
                    newMob.checkpointIndex = -1;
                    newMob.spawnPointRow = event.spawnRow;
                    newMob.spawnPointCol = event.spawnCol;

                    const vector<pair<int, int>>* routeNodes = nullptr;
                    if (newMob.routeIndex >= 0 && newMob.routeIndex < (int)navigationRoutes.size()) {
                        routeNodes = &navigationRoutes[newMob.routeIndex].nodes;
                        newMob.checkpointIndex = navigationRoutes[newMob.routeIndex].checkpointNodeIndex;
                    } else if (!globalPath.empty()) {
                        routeNodes = &globalPath;
                    }
                    
                    // Update dynamic stats based on current game state
                    updateMobDynamicStats(newMob, gameTime);
                    
                    // Set initial velocity towards first waypoint
                    if (routeNodes != nullptr && routeNodes->size() >= 2) {
                        double nextRow = (*routeNodes)[1].first;
                        double nextCol = (*routeNodes)[1].second;
                        double dist = sqrt(pow(nextRow - newMob.posRow, 2) + 
                                         pow(nextCol - newMob.posCol, 2));
                        if (dist > 0) {
                            newMob.velocityRow = (nextRow - newMob.posRow) / dist * newMob.modifiedSpeed;
                            newMob.velocityCol = (nextCol - newMob.posCol) / dist * newMob.modifiedSpeed;
                        }
                    }
                    
                    activeMobs.push_back(newMob);
                }
            }
            
            // Check if wave is complete
            if (currentWaveTime > wave.totalDuration) {
                currentWaveIndex++;
                currentWaveTime = 0;
            }
        }
        
        // Update mob positions and handle collision avoidance
        for (auto& mob : activeMobs) {
            if (!mob.isAlive) continue;
            
            // Update dynamic stats each frame (handles slow effects, buffs, etc.)
            updateMobDynamicStats(mob, gameTime);

            const vector<pair<int, int>>* routeNodes = nullptr;
            if (mob.routeIndex >= 0 && mob.routeIndex < (int)navigationRoutes.size()) {
                routeNodes = &navigationRoutes[mob.routeIndex].nodes;
            } else {
                routeNodes = &globalPath;
            }
            
            // Update position
            mob.posRow += mob.velocityRow * dt;
            mob.posCol += mob.velocityCol * dt;
            
            // Update waypoint if reached
            if (routeNodes != nullptr && mob.routeStepIndex < (int)routeNodes->size()) {
                double nextRow = (*routeNodes)[mob.routeStepIndex].first;
                double nextCol = (*routeNodes)[mob.routeStepIndex].second;
                double dist = sqrt(pow(nextRow - mob.posRow, 2) + pow(nextCol - mob.posCol, 2));
                
                if (dist < 1.0) {
                    if (!mob.passedCheckpoint && mob.checkpointIndex >= 0 && mob.routeStepIndex >= mob.checkpointIndex) {
                        mob.passedCheckpoint = true;
                    }

                    mob.routeStepIndex++;
                    
                    // Update velocity to next waypoint using modified speed
                    if (mob.routeStepIndex < (int)routeNodes->size()) {
                        nextRow = (*routeNodes)[mob.routeStepIndex].first;
                        nextCol = (*routeNodes)[mob.routeStepIndex].second;
                        dist = sqrt(pow(nextRow - mob.posRow, 2) + pow(nextCol - mob.posCol, 2));
                        if (dist > 0) {
                            mob.velocityRow = (nextRow - mob.posRow) / dist * mob.modifiedSpeed;
                            mob.velocityCol = (nextCol - mob.posCol) / dist * mob.modifiedSpeed;
                        }
                    }
                }
            }
            
            // Check if reached base
            if (routeNodes != nullptr && mob.routeStepIndex >= (int)routeNodes->size()) {
                mob.isAlive = false;
                mob.reachedBase = true;
                mob.passedCheckpoint = true;
                // Deduct one HP from base per mob reaching
                baseHPRef -= 1;
                if (baseHPRef < 0) baseHPRef = 0;
            }
        }
        
        // Keep mobs strictly on walkable tiles (PATH/BASE) by snapping any drift.
        for (auto &mob : activeMobs) {
            if (!mob.isAlive) continue;

            const vector<pair<int, int>>* routeNodes = nullptr;
            if (mob.routeIndex >= 0 && mob.routeIndex < (int)navigationRoutes.size()) {
                routeNodes = &navigationRoutes[mob.routeIndex].nodes;
            } else {
                routeNodes = &globalPath;
            }

            int r = (int)round(mob.posRow);
            int c = (int)round(mob.posCol);
            if (r < 0 || r >= GameMap::ROWS || c < 0 || c >= GameMap::COLS) continue;

            if (!(gameMap.grid[r][c].type == PATH || gameMap.grid[r][c].type == BASE)) {
                double bestDist = 1e9;
                int bestR = -1, bestC = -1;

                if (routeNodes != nullptr) {
                    for (size_t i = 0; i < routeNodes->size(); i++) {
                        int nr = (*routeNodes)[i].first;
                        int nc = (*routeNodes)[i].second;
                        double d = sqrt(pow((double)nr - mob.posRow, 2) + pow((double)nc - mob.posCol, 2));
                        if (d < bestDist) {
                            bestDist = d;
                            bestR = nr;
                            bestC = nc;
                        }
                    }
                }

                if (bestR != -1) {
                    mob.posRow = (double)bestR;
                    mob.posCol = (double)bestC;
                }
            }
        }
        
        // Handle removal of dead mobs and award gold for killed mobs
        vector<MobInstance> survivors;
        survivors.reserve(activeMobs.size());

        for (auto &m : activeMobs) {
            if (m.isAlive) {
                survivors.push_back(m);
            } else {
                // If mob died due to reaching base, do not award gold
                if (!m.reachedBase) {
                    // Award gold based on mob reward (includes modifications like goldMultiplier)
                    int goldReward = getMobReward(m);
                    moneyRef += goldReward;
                }
            }
        }

        activeMobs.swap(survivors);
    }
    
    // Render mobs on game map
    void renderMobs(int mapStartLine) {
        for (auto& mob : activeMobs) {
            if (!mob.isAlive) continue;
            
            int screenRow = (int)round(mob.posRow);
            int screenCol = (int)round(mob.posCol);
            
            if (screenRow >= 0 && screenRow < GameMap::ROWS && 
                screenCol >= 0 && screenCol < GameMap::COLS) {
                
                setCursorPosition(screenCol, mapStartLine + screenRow);
                
                // Set mob color (CYAN = 11)
                setTextColor(11);
                
                char mobChar = mobTypes[mob.mobType].symbol;
                cout << mobChar;
                
                resetTextColor();
            }
        }
    }
    
    // Getters
    int getActiveMobCount() const { return activeMobs.size(); }
    vector<MobInstance>& getMobs() { return activeMobs; }
    int getCurrentWaveIndex() const { return currentWaveIndex; }
    const vector<NavigationRoute>& getNavigationRoutes() const { return navigationRoutes; }
    int getTotalSpawnsCurrentWave() {
        if (currentWaveIndex < 0 || currentWaveIndex >= (int)waves.size()) return 0;
        return (int)waves[currentWaveIndex].spawnEvents.size();
    }

    int getRemainingInCurrentWave() {
        if (currentWaveIndex < 0 || currentWaveIndex >= (int)waves.size()) return 0;
        int remaining = 0;
        for (const auto &m : activeMobs) if (m.spawnWaveId == currentWaveIndex && m.isAlive) remaining++;
        for (const auto &e : waves[currentWaveIndex].spawnEvents) if (e.spawnTime > currentWaveTime) remaining++;
        return remaining;
    }

    // ============ NEW FUNCTIONS FOR SPAWN DATA MANAGEMENT ============

    // Load complete mob stats from MobData.txt (including HP, armor, speed, special effects)
    void loadMobStatsFromFile() {
        string filename = buildMobDataPath();
        ifstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Warning: Could not load mob stats from " << filename << endl;
            return;
        }
        
        string line;
        int currentMobIndex = -1;
        
        while (getline(file, line)) {
            // Parse "Enemy N: MobName"
            if (line.find("Enemy") == 0 && line.find(':') != string::npos) {
                size_t colonPos = line.find(':');
                string mobInfo = line.substr(colonPos + 1);
                
                // Extract mob index from "Enemy N:"
                size_t spacePos = line.find(' ', 6);
                if (spacePos != string::npos) {
                    string numStr = line.substr(6, spacePos - 6);
                    currentMobIndex = stoi(numStr) - 1; // Convert to 0-based index
                }
            }
            
            // Parse individual stats
            if (currentMobIndex >= 0 && currentMobIndex < (int)mobTypes.size()) {
                if (line.find("HP:") != string::npos) {
                    size_t pos = line.find("HP:");
                    string numStr = line.substr(pos + 3);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0, 1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].hp = stoi(numStr);
                }
                else if (line.find("Armor:") != string::npos) {
                    size_t pos = line.find("Armor:");
                    string numStr = line.substr(pos + 6);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0, 1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].armor = stoi(numStr);
                }
                else if (line.find("Speed:") != string::npos) {
                    size_t pos = line.find("Speed:");
                    string numStr = line.substr(pos + 6);
                    while (!numStr.empty() && !isdigit(numStr[0]) && numStr[0] != '.') numStr.erase(0, 1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].speed = stod(numStr);
                }
                else if (line.find("Gold:") != string::npos) {
                    size_t pos = line.find("Gold:");
                    string numStr = line.substr(pos + 5);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0, 1);
                    if (!numStr.empty()) mobGolds[currentMobIndex] = stoi(numStr);
                }
                else if (line.find("Flying:") != string::npos) {
                    mobTypes[currentMobIndex].isFlying = (line.find("Yes") != string::npos);
                }
            }
        }
        
        file.close();
    }

    // Parse a single spawn line from level design file
    // Format: "X mob from row R [left/right/top/bottom side]"
    // Returns: (count, mobType, spawnRow, spawnCol, -1 if parse failed)
    void parseSpawnLine(const string& line, int& count, int& mobType, int& spawnRow, int& spawnCol) {
        count = 0;
        mobType = -1;
        spawnRow = -1;
        spawnCol = -1;
        
        // Extract count (first number)
        size_t numPos = 0;
        while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
        if (numPos < line.length()) {
            size_t numEnd = numPos;
            while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
            count = stoi(line.substr(numPos, numEnd - numPos));
        }
        
        // Extract mob name and determine type
        if (line.find("pigman") != string::npos && line.find("berserker") == string::npos) {
            mobType = 0; // Pigman
        } else if (line.find("houndling") != string::npos) {
            mobType = 1; // Houndling
        } else if (line.find("werewolf") != string::npos) {
            mobType = 2; // Werewolf
        } else if (line.find("mini mammon") != string::npos) {
            mobType = 3; // Mini Mammon
        } else if (line.find("armored mammon") != string::npos) {
            mobType = 4; // Armored Mammon
        } else if (line.find("birdman") != string::npos) {
            mobType = 5; // Birdman
        } else if (line.find("batman") != string::npos) {
            mobType = 6; // Batman
        } else if (line.find("berserker") != string::npos) {
            mobType = 7; // Pigman Berserker
        } else if (line.find("spiderman") != string::npos) {
            mobType = 8; // Spiderman
        } else if (line.find("alpha wolf") != string::npos) {
            mobType = 9; // Alpha Wolf
        } else if (line.find("dragon") != string::npos) {
            // Determine which dragon type
            if (line.find("type 1") != string::npos || line.find("^D^") != string::npos) {
                mobType = 10; // Dragon Type 1
            } else if (line.find("type 2") != string::npos) {
                mobType = 11; // Dragon Type 2
            } else if (line.find("type 3") != string::npos || line.find("!D!") != string::npos) {
                mobType = 12; // Dragon Type 3
            }
        }
        
        // Extract spawn row/column
        if (line.find("row") != string::npos) {
            size_t rowPos = line.find("row");
            numPos = rowPos + 3;
            while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
            if (numPos < line.length()) {
                size_t numEnd = numPos;
                while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
                spawnRow = stoi(line.substr(numPos, numEnd - numPos));
                // Determine column based on spawn side
                spawnCol = (line.find("left") != string::npos) ? 1 : 
                           (line.find("right") != string::npos) ? GameMap::COLS - 2 : 50;
            }
        } else if (line.find("column") != string::npos) {
            size_t colPos = line.find("column");
            numPos = colPos + 6;
            while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
            if (numPos < line.length()) {
                size_t numEnd = numPos;
                while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
                spawnCol = stoi(line.substr(numPos, numEnd - numPos));
                // Determine row based on spawn side
                spawnRow = (line.find("top") != string::npos) ? 1 : 
                           (line.find("down") != string::npos) ? GameMap::ROWS - 2 : 16;
            }
        }
    }

    // Apply a modifier to a mob (e.g., speed buff, HP debuff)
    void applyMobModifier(MobInstance& mob, const MobModifier& modifier, double currentGameTime) {
        // Apply multiplicative modifiers
        mob.modifier.speedMultiplier *= modifier.speedMultiplier;
        mob.modifier.hpMultiplier *= modifier.hpMultiplier;
        mob.modifier.goldMultiplier *= modifier.goldMultiplier;
        mob.modifier.damageMultiplier *= modifier.damageMultiplier;
        
        // Apply additive modifiers
        mob.modifier.armorBonus += modifier.armorBonus;
        mob.modifier.slowEffect += modifier.slowEffect;
        
        // Handle slow effect
        if (modifier.isSlowed) {
            mob.modifier.isSlowed = true;
            mob.modifier.slowedUntilTime = currentGameTime + modifier.slowDuration;
            if (modifier.slowDuration > 0) {
                mob.modifier.slowDuration = modifier.slowDuration;
            }
        }
        
        // Recalculate effective stats
        updateMobDynamicStats(mob, currentGameTime);
    }

    // Update mob's dynamic stats based on modifiers (call each frame or after applying modifiers)
    void updateMobDynamicStats(MobInstance& mob, double currentGameTime) {
        // Update modified speed based on slowdown effect
        mob.modifiedSpeed = mob.moveSpeed * mob.modifier.speedMultiplier;
        
        // Apply slow effect if active
        if (mob.modifier.isSlowed && currentGameTime < mob.modifier.slowedUntilTime) {
            double slowPercent = mob.modifier.slowEffect / 100.0;
            mob.modifiedSpeed *= (1.0 - slowPercent);
        } else if (mob.modifier.isSlowed && currentGameTime >= mob.modifier.slowedUntilTime) {
            // Slow effect expired
            mob.modifier.isSlowed = false;
            mob.modifier.slowEffect = 0;
        }
        
        // Ensure non-negative speed
        if (mob.modifiedSpeed < 0.1) mob.modifiedSpeed = 0.1;
        
        // Calculate effective HP
        if (mobTypes[mob.mobType].hp > 0) {
            mob.maxHealth = (int)(mobTypes[mob.mobType].hp * mob.modifier.hpMultiplier);
            // Ensure health doesn't exceed max
            if (mob.health > mob.maxHealth) {
                mob.health = mob.maxHealth;
            }
        }
        
        // Store base gold for reward calculation
        if (mobGolds[mob.mobType] > 0) {
            mob.baseGold = (int)(mobGolds[mob.mobType] * mob.modifier.goldMultiplier);
        }
    }

    // Remove all slow effects from a mob
    void removeMobSlow(MobInstance& mob, double currentGameTime) {
        mob.modifier.isSlowed = false;
        mob.modifier.slowEffect = 0;
        mob.modifier.slowedUntilTime = 0;
        updateMobDynamicStats(mob, currentGameTime);
    }

    // Deal damage to a mob and update health
    // Returns true if mob dies
    bool damageToMob(MobInstance& mob, int damageAmount) {
        mob.health -= damageAmount;
        if (mob.health <= 0) {
            mob.health = 0;
            mob.isAlive = false;
            return true;
        }
        return false;
    }

    // Heal a mob to a maximum
    void healMob(MobInstance& mob, int healAmount) {
        mob.health += healAmount;
        if (mob.health > mob.maxHealth) {
            mob.health = mob.maxHealth;
        }
    }

    // Get the effective armor of a mob after modifiers
    int getEffectiveMobArmor(const MobInstance& mob) const {
        if (mob.mobType < 0 || mob.mobType >= (int)mobTypes.size()) return 0;
        return mobTypes[mob.mobType].armor + mob.modifier.armorBonus;
    }

    // Get the gold reward for a mob
    int getMobReward(const MobInstance& mob) const {
        return mob.baseGold;
    }
};

#endif
