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
#include <unordered_set>
#include <map>
#include <cctype>

using namespace std;

// Forward declarations for console functions
void setCursorPosition(int x, int y);
void setTextColor(int color);
void resetTextColor();

// ============ MOB MODIFIER STRUCTURE (for dynamic stats management) ============

struct MobModifier {
    double speedMultiplier;
    double hpMultiplier;
    int armorBonus;
    double goldMultiplier;
    double damageMultiplier;
    int slowEffect;
    double slowDuration;
    bool isSlowed;
    double slowedUntilTime;
    
    MobModifier() 
        : speedMultiplier(1.0), hpMultiplier(1.0), armorBonus(0), 
          goldMultiplier(1.0), damageMultiplier(1.0), slowEffect(0), 
          slowDuration(0), isSlowed(false), slowedUntilTime(0.0) {}
};

// ============ MOB INSTANCE STRUCTURE ============

struct MobInstance {
    int mobType;
    double posRow, posCol;
    double velocityRow, velocityCol;
    int targetWaypoint;
    int health;
    int maxHealth;
    int armor;
    int maxArmor;
    bool isAlive;
    double lastUpdateTime;
    int spawnWaveId;
    double moveSpeed;
    double modifiedSpeed;
    int routeIndex;
    int routeStepIndex;
    bool passedCheckpoint;
    int checkpointIndex;
    bool reachedBase;
    double lastHitTime;
    
    MobModifier modifier;
    int baseGold;
    int spawnPointRow;
    int spawnPointCol;
    int prevRenderRow;
    int prevRenderCol;
    int lastGridRow;
    int lastGridCol;

    MobInstance(int type, double row, double col, double speed, int wave)
            : mobType(type), posRow(row), posCol(col), targetWaypoint(0), 
                isAlive(true), lastUpdateTime(0), spawnWaveId(wave), moveSpeed(speed),
                modifiedSpeed(speed), routeIndex(-1), routeStepIndex(0), 
                passedCheckpoint(false), checkpointIndex(-1), reachedBase(false),
                lastHitTime(-9999.0), spawnPointRow((int)row), spawnPointCol((int)col), prevRenderRow((int)row), prevRenderCol((int)col), lastGridRow((int)row), lastGridCol((int)col), baseGold(0), maxHealth(0), armor(0), maxArmor(0) {
            velocityRow = 0;
            velocityCol = 0;
    }
};

// ============ SPAWN EVENT & WAVE (Level Design) ============

struct SpawnEvent {
    double spawnTime;
    int mobType;
    int spawnRow;
    int spawnCol;
    double speed;
    int routeIndex;
};

struct NavigationRoute {
    int spawnRow;
    int spawnCol;
    vector<pair<int, int>> nodes;
    vector<char> commands;
    pair<int, int> checkpoint;
    int checkpointNodeIndex;
};

struct LevelWave {
    int waveNumber;
    vector<SpawnEvent> spawnEvents;
    double totalDuration;
};

// ============ MOB SYSTEM MANAGER ============

class MobSystemManager {
private:
    vector<MobInstance> activeMobs;
    vector<LevelWave> waves;
    vector<pair<int, int>> globalPath;
    vector<NavigationRoute> navigationRoutes;
    unordered_map<long long, double> towerNextAttackTime;
    unordered_set<long long> knownTowerKeys;
    unordered_map<long long, int> vampireKillCounts;
    vector<pair<int, int>> towerDashFlashQueue;
    vector<pair<pair<int, int>, string>> towerAttackFlashQueue;  // Store tower type for animation
    bool demoRenderDirty;
    map<pair<int, int>, vector<pair<pair<int,int>, pair<int,int>>>> spawnPointPaths;
    
    int currentWaveIndex;
    double currentWaveTime;
    double gameTime;
    
    vector<Mob>& mobTypes;
    GameMap& gameMap;
    const vector<Tower>* towersRef;
    vector<int> mobGolds;
    bool useDemoAlgorithm;
    bool demoManualWaveActive;
    bool demoWaitingForNextWave;
    
    // ============ Path building helper functions ============
    string buildMapFilePath(int level) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "data/maps/map_for_level%d.txt", level);
        return string(buffer);
    }
    
    string buildLevelDesignPath(int level) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "data/levels/level%d_design.txt", level);
        return string(buffer);
    }
    
    string buildMobDataPath() {
        return "data/meta/MobData.txt";
    }
    
public:
    int &moneyRef;
    int &baseHPRef;

    // Spawn a single mob immediately for diagnostics or testing.
    void spawnSingleMob(int mobType, int spawnRow, int spawnCol, double speed) {
        if (mobType < 0 || mobType >= (int)mobTypes.size()) return;
        int finalRow = spawnRow;
        int finalCol = spawnCol;
        if (!snapSpawnToNearestPlus(finalRow, finalCol)) return;

        MobInstance newMob(mobType, (double)finalRow, (double)finalCol, speed, currentWaveIndex);
        newMob.health = mobTypes[mobType].hp;
        newMob.maxHealth = mobTypes[mobType].hp;
        newMob.armor = mobTypes[mobType].armor;
        newMob.maxArmor = mobTypes[mobType].armor;
        newMob.baseGold = mobGolds[mobType];
        newMob.modifiedSpeed = speed;
        newMob.modifier = MobModifier();
        newMob.routeIndex = findRouteIndexForSpawn(finalRow, finalCol);
        newMob.routeStepIndex = 0;
        newMob.passedCheckpoint = false;
        newMob.checkpointIndex = -1;
        newMob.spawnPointRow = finalRow;
        newMob.spawnPointCol = finalCol;

        const vector<pair<int,int>>* routeNodes = nullptr;
        if (newMob.routeIndex >= 0 && newMob.routeIndex < (int)navigationRoutes.size()) {
            routeNodes = &navigationRoutes[newMob.routeIndex].nodes;
            newMob.checkpointIndex = navigationRoutes[newMob.routeIndex].checkpointNodeIndex;
        } else if (!globalPath.empty()) {
            routeNodes = &globalPath;
        } else {
            int baseRow = -1, baseCol = -1;
            for (int r = 0; r < GameMap::ROWS; ++r) {
                for (int c = 0; c < GameMap::COLS; ++c) {
                    if (gameMap.grid[r][c].type == BASE) { baseRow = r; baseCol = c; break; }
                }
                if (baseRow != -1) break;
            }
            if (baseRow != -1) {
                vector<pair<int,int>> fallbackPath = findPathFromSpawn(finalRow, finalCol, baseRow, baseCol);
                if (fallbackPath.size() >= 2) {
                    NavigationRoute route;
                    route.spawnRow = finalRow;
                    route.spawnCol = finalCol;
                    route.nodes = fallbackPath;
                    route.checkpointNodeIndex = (fallbackPath.size() > 2) ? (int)fallbackPath.size() / 2 : 0;
                    route.checkpoint = fallbackPath[route.checkpointNodeIndex];
                    navigationRoutes.push_back(route);
                    newMob.routeIndex = (int)navigationRoutes.size() - 1;
                    newMob.checkpointIndex = navigationRoutes[newMob.routeIndex].checkpointNodeIndex;
                    routeNodes = &navigationRoutes[newMob.routeIndex].nodes;
                }
            }
        }

        updateMobDynamicStats(newMob, gameTime);
        if (routeNodes != nullptr && routeNodes->size() >= 2) {
            double nextRow = (*routeNodes)[1].first, nextCol = (*routeNodes)[1].second;
            double dist = sqrt(pow(nextRow - newMob.posRow, 2) + pow(nextCol - newMob.posCol, 2));
            if (dist > 0) {
                newMob.velocityRow = (nextRow - newMob.posRow) / dist * newMob.modifiedSpeed;
                newMob.velocityCol = (nextCol - newMob.posCol) / dist * newMob.modifiedSpeed;
            }
            activeMobs.push_back(newMob);
            demoRenderDirty = true;
        }
    }

    // Clear all active mobs (useful for isolated diagnostics)
    void clearActiveMobs() {
        activeMobs.clear();
        demoRenderDirty = true;
    }

    MobSystemManager(vector<Mob>& mobTypes_, GameMap& gameMap_, int &money_, int &baseHP_, bool demoMode = false) 
        : mobTypes(mobTypes_), gameMap(gameMap_), towersRef(nullptr), currentWaveIndex(0), 
            currentWaveTime(0), gameTime(0), moneyRef(money_), baseHPRef(baseHP_),
            useDemoAlgorithm(demoMode), demoManualWaveActive(false), demoWaitingForNextWave(true), demoRenderDirty(true) {
        loadMobGolds();
    }
    
    // ============ 修正后的 loadLevelDesign ============
    void loadLevelDesign(int level) {
        waves.clear();
        currentWaveTime = 0;
        gameTime = 0;
        currentWaveIndex = 0;
        
        // Both demo and regular mode: wait for 'z' key to trigger waves
        demoManualWaveActive = false;
        demoWaitingForNextWave = true;

        if (useDemoAlgorithm) {
            loadDemoLevelDesign(level);
            return;
        }

        string filename = buildLevelDesignPath(level);
        ifstream file(filename);
        if (!file.is_open()) {
            // Create a default wave for testing
            LevelWave wave;
            wave.waveNumber = 1;
            SpawnEvent ev;
            ev.spawnTime = 0;
            ev.mobType = 0;  // pigman
            ev.spawnRow = 1;
            ev.spawnCol = 0;
            ev.speed = 2.0;
            ev.routeIndex = -1;
            wave.spawnEvents.push_back(ev);
            wave.totalDuration = 1.0;
            waves.push_back(wave);
            return;
        }

        string line;
        LevelWave currentWave;
        currentWave.waveNumber = 0;
        double waveTime = 0.0;
        double lastSpawnTime = 0.0;
        bool inWave = false;

        while (getline(file, line)) {
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == string::npos) continue;
            line = line.substr(first);
            if (line.empty() || line[0] == '-') continue;

            // 检测波次标题，如 "1." 或 "1.1"
            if (isdigit(line[0]) && line.find('.') != string::npos) {
                int dotPos = line.find('.');
                int waveNum = stoi(line.substr(0, dotPos));

                if (inWave && currentWave.waveNumber > 0) {
                    currentWave.totalDuration = lastSpawnTime;
                    waves.push_back(currentWave);
                }

                currentWave.waveNumber = waveNum;
                currentWave.spawnEvents.clear();
                waveTime = 0.0;
                lastSpawnTime = 0.0;
                inWave = true;

                // 如果标题行直接包含 spawn 信息，如 "1.1 pigman from row 3 left side"
                size_t contentStart = line.find_first_not_of("0123456789.", dotPos + 1);
                if (contentStart != string::npos) {
                    string spawnPart = line.substr(contentStart);
                    int count, mobType, spawnRow, spawnCol;
                    parseSpawnLine(spawnPart, count, mobType, spawnRow, spawnCol);
                    if (count > 0 && mobType >= 0) {
                        const double spawnGap = 1.0; // 1 second between mobs
                        for (int i = 0; i < count; ++i) {
                            SpawnEvent ev;
                            ev.spawnTime = waveTime;
                            ev.mobType = mobType;
                            int finalSpawnRow = spawnRow;
                            int finalSpawnCol = spawnCol;
                            if (!snapSpawnToNearestPlus(finalSpawnRow, finalSpawnCol)) continue;
                            ev.spawnRow = finalSpawnRow;
                            ev.spawnCol = finalSpawnCol;
                            ev.routeIndex = findRouteIndexForSpawn(finalSpawnRow, finalSpawnCol);
                            ev.speed = mobTypes[mobType].speed;
                            currentWave.spawnEvents.push_back(ev);
                            lastSpawnTime = ev.spawnTime;
                            waveTime += spawnGap;
                        }
                    }
                }
                continue;
            }

            // 处理 [wait X seconds]
            if (line.find("[wait") != string::npos) {
                size_t forPos = line.find("for");
                if (forPos != string::npos) {
                    size_t numStart = forPos + 4;
                    double waitTime = stod(line.substr(numStart));
                    waveTime += waitTime;
                }
                continue;
            }

            // 处理普通 spawn 行，如 "2 pigman from row 3 left side"
            if (isdigit(line[0]) && inWave) {
                int count, mobType, spawnRow, spawnCol;
                parseSpawnLine(line, count, mobType, spawnRow, spawnCol);
                if (count > 0 && mobType >= 0) {
                    for (int i = 0; i < count; ++i) {
                        SpawnEvent ev;
                        ev.spawnTime = waveTime;
                        ev.mobType = mobType;
                        int finalSpawnRow = spawnRow;
                        int finalSpawnCol = spawnCol;
                        if (!snapSpawnToNearestPlus(finalSpawnRow, finalSpawnCol)) continue;
                        ev.spawnRow = finalSpawnRow;
                        ev.spawnCol = finalSpawnCol;
                        ev.routeIndex = findRouteIndexForSpawn(finalSpawnRow, finalSpawnCol);
                        ev.speed = mobTypes[mobType].speed;
                        currentWave.spawnEvents.push_back(ev);
                        lastSpawnTime = ev.spawnTime;
                        // Advance by 1 second between spawns (avoid stacking)
                        waveTime += 1.0;
                    }
                }
                continue;
            }
        }

        if (inWave && currentWave.waveNumber > 0) {
            currentWave.totalDuration = lastSpawnTime;
            waves.push_back(currentWave);
        }

        file.close();
    }

    void loadDemoLevelDesign(int level) {
        waves.clear();
        if (mobTypes.empty()) return;
        if (level != 1) return;

        globalPath.clear();
        int maxCol = GameMap::COLS - 1;
        int maxRow = GameMap::ROWS - 1;
        int spawnRow = 1;
        int c = findSpawnColumnForRow(spawnRow) + 1;
        if (c < 0) c = 0;
        if (c > maxCol) c = maxCol;
        snapSpawnToNearestPlus(spawnRow, c);

        vector<pair<int,int>> nodes;
        nodes.push_back({spawnRow, c});
        for (int step = 1; step <= 140; ++step) {
            int nc = c + step;
            if (nc > maxCol) break;
            nodes.push_back({spawnRow, nc});
        }
        pair<int,int> last = nodes.back();
        int curRow = last.first, curCol = last.second;
        for (int step = 1; step <= 19; ++step) {
            int nr = curRow + step;
            if (nr > maxRow) break;
            nodes.push_back({nr, curCol});
        }
        last = nodes.back();
        curRow = last.first, curCol = last.second;
        for (int step = 1; step <= 111; ++step) {
            int nc = curCol - step;
            if (nc < 0) break;
            nodes.push_back({curRow, nc});
        }
        globalPath = nodes;

        const int waveCounts[4] = {5, 8, 10, 20};
        const double spawnGap = 1.0;
        for (int w = 0; w < 4; ++w) {
            LevelWave wave;
            wave.waveNumber = w + 1;
            wave.totalDuration = 0.0;
            for (int i = 0; i < waveCounts[w]; ++i) {
                SpawnEvent ev;
                ev.spawnTime = i * spawnGap;
                ev.mobType = 0;
                ev.spawnRow = spawnRow;
                ev.spawnCol = c;
                ev.speed = 2.0;
                ev.routeIndex = -1;
                wave.spawnEvents.push_back(ev);
                wave.totalDuration = ev.spawnTime;
            }
            wave.totalDuration += 0.2;
            waves.push_back(wave);
        }
    }

    void triggerNextWave() {
        if (currentWaveIndex >= (int)waves.size()) return;
        if (demoManualWaveActive || !demoWaitingForNextWave) return;
        if (!activeMobs.empty()) return;
        demoManualWaveActive = true;
        demoWaitingForNextWave = false;
        demoRenderDirty = true;
        currentWaveTime = 0;
    }

    // Waiting for next wave only when explicitly set AND there are no active mobs
    bool isWaitingForNextWave() {
        if (currentWaveIndex >= (int)waves.size()) return false;
        return demoWaitingForNextWave && activeMobs.empty();
    }

    bool allWavesSpawned() const {
        return currentWaveIndex >= (int)waves.size();
    }

    bool consumeDemoRenderDirty() {
        bool dirty = demoRenderDirty;
        demoRenderDirty = false;
        return dirty;
    }

    void loadMobGolds() {
        mobGolds.clear();
        mobGolds.resize(mobTypes.size(), 0);
        ifstream f(buildMobDataPath());
        if (!f.is_open()) return;
        string line, currentName;
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
            mobGolds[i] = (it != goldMap.end()) ? it->second : 0;
        }
    }
    
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
        if (baseRow == -1) return;

        vector<pair<int, int>> spawnSources;
        for (int row = 0; row < GameMap::ROWS; row++) {
            for (int col = 0; col < GameMap::COLS; col++) {
                if (isSpawnTile(row, col)) {
                    spawnSources.push_back(make_pair(row, col));
                }
            }
        }
        for (const auto& wave : waves) {
            for (const auto& event : wave.spawnEvents) {
                pair<int, int> spawnPos = make_pair(event.spawnRow, event.spawnCol);
                bool seen = false;
                for (const auto& existing : spawnSources) {
                    if (existing.first == spawnPos.first && existing.second == spawnPos.second) {
                        seen = true;
                        break;
                    }
                }
                if (!seen && spawnPos.first >= 0 && spawnPos.first < GameMap::ROWS && spawnPos.second >= 0 && spawnPos.second < GameMap::COLS) {
                    spawnSources.push_back(spawnPos);
                }
            }
        }

        for (const auto& spawn : spawnSources) {
            vector<pair<int, int>> path = findPathFromSpawn(spawn.first, spawn.second, baseRow, baseCol);
            if (path.empty()) continue;
            NavigationRoute route;
            route.spawnRow = spawn.first;
            route.spawnCol = spawn.second;
            route.nodes = path;
            route.checkpointNodeIndex = (path.size() > 2) ? (int)path.size() / 2 : 0;
            route.checkpoint = path[route.checkpointNodeIndex];
            navigationRoutes.push_back(route);
        }

        if (!navigationRoutes.empty()) {
            globalPath = navigationRoutes[0].nodes;
        } else {
            int spawnRow = 1;
            int maxCol = GameMap::COLS - 1;
            int maxRow = GameMap::ROWS - 1;
            int startCol = findSpawnColumnForRow(spawnRow);
            int c = startCol + 1;
            if (c < 0) c = 0;
            if (c > maxCol) c = maxCol;

            vector<pair<int,int>> nodes;
            nodes.push_back(make_pair(spawnRow, c));
            for (int step = 1; step <= 140; ++step) {
                int nc = c + step;
                if (nc > maxCol) break;
                nodes.push_back(make_pair(spawnRow, nc));
            }
            pair<int,int> last = nodes.back();
            int curRow = last.first, curCol = last.second;
            for (int step = 1; step <= 19; ++step) {
                int nr = curRow + step;
                if (nr > maxRow) break;
                nodes.push_back(make_pair(nr, curCol));
            }
            last = nodes.back();
            curRow = last.first;
            curCol = last.second;
            for (int step = 1; step <= 111; ++step) {
                int nc = curCol - step;
                if (nc < 0) break;
                nodes.push_back(make_pair(curRow, nc));
            }
            globalPath = nodes;
        }

        for (auto& wave : waves) {
            for (auto& event : wave.spawnEvents) {
                event.routeIndex = findRouteIndexForSpawn(event.spawnRow, event.spawnCol);
            }
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

    vector<pair<int, int>> getAllSpawnTiles() const {
        vector<pair<int, int>> spawns;
        for (int row = 0; row < GameMap::ROWS; ++row) {
            for (int col = 0; col < GameMap::COLS; ++col) {
                if (isSpawnTile(row, col)) {
                    spawns.push_back(make_pair(row, col));
                }
            }
        }
        return spawns;
    }

    bool snapSpawnToNearestPlus(int& spawnRow, int& spawnCol) {
        vector<pair<int, int>> spawns = getAllSpawnTiles();
        if (!spawns.empty()) {
            long long bestDist = (1LL << 60);
            int bestRow = spawns[0].first;
            int bestCol = spawns[0].second;
            for (const auto& s : spawns) {
                long long dr = (long long)s.first - (long long)spawnRow;
                long long dc = (long long)s.second - (long long)spawnCol;
                long long dist2 = dr * dr + dc * dc;
                if (dist2 < bestDist) {
                    bestDist = dist2;
                    bestRow = s.first;
                    bestCol = s.second;
                }
            }
            spawnRow = bestRow;
            spawnCol = bestCol;
            return true;
        }

        // Maps without any '+': promote the requested valid tile to a spawn tile.
        if (spawnRow < 0) spawnRow = 0;
        if (spawnRow >= GameMap::ROWS) spawnRow = GameMap::ROWS - 1;
        if (spawnCol < 0) spawnCol = 0;
        if (spawnCol >= GameMap::COLS) spawnCol = GameMap::COLS - 1;

        gameMap.grid[spawnRow][spawnCol].type = PATH;
        gameMap.grid[spawnRow][spawnCol].displayChar = '+';
        return true;
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
        vector<vector<pair<int, int>>> parent(GameMap::ROWS, vector<pair<int, int>>(GameMap::COLS, make_pair(-1, -1)));
        queue<pair<int, int>> q;
        q.push(make_pair(startRow, startCol));
        visited[startRow][startCol] = true;
        bool found = false;

        while (!q.empty()) {
            pair<int, int> current = q.front(); q.pop();
            int row = current.first, col = current.second;
            if (row == baseRow && col == baseCol) { found = true; break; }
            vector<pair<int, int>> directions;
            int horizontalStep = (baseCol >= startCol) ? 1 : -1;
            int verticalStep = (baseRow >= startRow) ? 1 : -1;
            directions.push_back(make_pair(0, horizontalStep));
            directions.push_back(make_pair(verticalStep, 0));
            directions.push_back(make_pair(-verticalStep, 0));
            directions.push_back(make_pair(0, -horizontalStep));
            for (int i = 0; i < 4; i++) {
                int nextRow = row + directions[i].first, nextCol = col + directions[i].second;
                if (!isWalkableTile(nextRow, nextCol)) continue;
                if (visited[nextRow][nextCol]) continue;
                visited[nextRow][nextCol] = true;
                parent[nextRow][nextCol] = make_pair(row, col);
                q.push(make_pair(nextRow, nextCol));
            }
        }

        if (!found) return vector<pair<int, int>>();

        vector<pair<int, int>> reversedPath;
        int currentRow = baseRow, currentCol = baseCol;
        while (!(currentRow == startRow && currentCol == startCol)) {
            reversedPath.push_back(make_pair(currentRow, currentCol));
            pair<int, int> previous = parent[currentRow][currentCol];
            if (previous.first == -1) return vector<pair<int, int>>();
            currentRow = previous.first;
            currentCol = previous.second;
        }
        reversedPath.push_back(make_pair(startRow, startCol));
        return vector<pair<int, int>>(reversedPath.rbegin(), reversedPath.rend());
    }

    long long makeTowerKey(int row, int col) const {
        return (static_cast<long long>(row) << 32) ^ static_cast<unsigned int>(col);
    }

    vector<pair<int, int>> getPlacedTowerCenters() const {
        vector<pair<int, int>> centers;
        for (int row = 0; row < GameMap::ROWS; ++row) {
            for (int col = 0; col < GameMap::COLS; ++col) {
                const Tile& tile = gameMap.grid[row][col];
                if (tile.type == TOWER && tile.towerIndex >= 0 && tile.towerPosRow == 1 && tile.towerPosCol == 1) {
                    centers.push_back(make_pair(row, col));
                }
            }
        }
        return centers;
    }

    bool isTowerWithinAuraRange(int towerRow, int towerCol, int auraRow, int auraCol, int auraRadius) const {
        int overlapRadius = auraRadius + 1;  // 3x3 tower footprint + aura overlap
        return abs(towerRow - auraRow) <= overlapRadius && abs(towerCol - auraCol) <= overlapRadius;
    }

    bool isTowerSlowedBySpider(int towerRow, int towerCol) const {
        for (const auto& mob : activeMobs) {
            if (!mob.isAlive || mob.mobType < 0 || mob.mobType >= (int)mobTypes.size()) continue;
            if (mobTypes[mob.mobType].name.find("Spiderman") == string::npos) continue;
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            if (isTowerWithinAuraRange(towerRow, towerCol, mobRow, mobCol, 2)) {
                return true;
            }
        }
        return false;
    }

    bool isTowerBuffedByWarDrum(int towerRow, int towerCol) const {
        if (towersRef == nullptr) return false;
        for (const auto& towerCenter : getPlacedTowerCenters()) {
            int drumRow = towerCenter.first;
            int drumCol = towerCenter.second;
            const Tile& drumTile = gameMap.grid[drumRow][drumCol];
            if (drumTile.towerIndex < 0 || drumTile.towerIndex >= (int)towersRef->size()) continue;
            const Tower& drumTower = (*towersRef)[drumTile.towerIndex];
            if (drumTower.name.find("War Drum") == string::npos) continue;
            if (isTowerWithinAuraRange(towerRow, towerCol, drumRow, drumCol, 2)) {
                return true;
            }
        }
        return false;
    }

    int countWarDrumBuffedOtherTowers(int drumRow, int drumCol) const {
        int count = 0;
        for (const auto& towerCenter : getPlacedTowerCenters()) {
            int towerRow = towerCenter.first;
            int towerCol = towerCenter.second;
            if (towerRow == drumRow && towerCol == drumCol) continue;
            if (isTowerWithinAuraRange(towerRow, towerCol, drumRow, drumCol, 2)) {
                ++count;
            }
        }
        return count;
    }

    int getTowerAuraColor(int towerRow, int towerCol) const {
        bool slowedBySpider = isTowerSlowedBySpider(towerRow, towerCol);
        bool buffedByWarDrum = isTowerBuffedByWarDrum(towerRow, towerCol);
        if (slowedBySpider && buffedByWarDrum) return -1;
        if (slowedBySpider) return 5;   // purple background
        if (buffedByWarDrum) return 6;   // orange background
        return 0;
    }

    double getTowerAttackSpeedMultiplier(int towerRow, int towerCol) const {
        double multiplier = 1.0;
        if (isTowerBuffedByWarDrum(towerRow, towerCol)) {
            multiplier *= (2.0 / 3.0);  // 50% faster hitspeed -> 1.5x speed -> 2/3 cooldown
        }
        if (isTowerSlowedBySpider(towerRow, towerCol)) {
            multiplier *= 2.0;  // 50% slow -> 2x cooldown
        }
        return multiplier;
    }

    // ============ 塔攻击逻辑（已完整，保持不变） ============
    void processTowerAttacks(const vector<Tower>& towers) {
        towersRef = &towers;
        vector<pair<int, int>> towerCenters = getPlacedTowerCenters();
        // Build current tower key set and initialize newly-placed towers' cooldown
        unordered_set<long long> currentKeys;
        for (const auto &center : towerCenters) {
            long long k = makeTowerKey(center.first, center.second);
            currentKeys.insert(k);
            if (knownTowerKeys.find(k) == knownTowerKeys.end()) {
                // Newly placed tower: reset its next attack time so it can fire immediately
                towerNextAttackTime[k] = 0.0;
                knownTowerKeys.insert(k);
            }
        }
        // Remove keys for towers that no longer exist
        vector<long long> toRemove;
        for (const auto &k : knownTowerKeys) {
            if (currentKeys.find(k) == currentKeys.end()) toRemove.push_back(k);
        }
        for (auto &k : toRemove) {
            knownTowerKeys.erase(k);
            // Also erase nextAttackTime entry to avoid stale entries lingering
            auto it = towerNextAttackTime.find(k);
            if (it != towerNextAttackTime.end()) towerNextAttackTime.erase(it);
        }
        for (const auto& center : towerCenters) {
            int centerRow = center.first, centerCol = center.second;
            const Tile& centerTile = gameMap.grid[centerRow][centerCol];
            int towerTypeIndex = centerTile.towerIndex;
            if (towerTypeIndex < 0 || towerTypeIndex >= (int)towers.size()) continue;
            const Tower& tower = towers[towerTypeIndex];
            long long key = makeTowerKey(centerRow, centerCol);
            double& nextAttackTime = towerNextAttackTime[key];
            double finalAttackSpeed = tower.attackSpeed * getTowerAttackSpeedMultiplier(centerRow, centerCol);
            if (nextAttackTime > gameTime) continue;

            if (tower.name == "Arrow Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange) {
                        double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestMobIndex = i;
                        }
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int armorDmg = (target.armor > 0) ? 1 : 0;
                    damageToMob(target, tower.hitpoints, armorDmg);
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
            else if (tower.name == "Laser Tower") {
                bool hitAny = false;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    if (mobTypes[mob.mobType].isFlying) continue;
                    int mobRow = (int)round(mob.posRow);
                    if (abs(mobRow - centerRow) <= 1) {
                        int armorDmg = (mob.armor > 0) ? 1 : 0;
                        damageToMob(mob, tower.hitpoints, armorDmg);
                        mob.lastHitTime = gameTime;
                        hitAny = true;
                    }
                }
                if (hitAny) {
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
            else if (tower.name == "Frost Tower") {
                int targetMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange) {
                        double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                        if (dist < bestDist) {
                            bestDist = dist;
                            targetMobIndex = i;
                        }
                    }
                }

                if (targetMobIndex >= 0) {
                    int targetRow = (int)round(activeMobs[targetMobIndex].posRow);
                    int targetCol = (int)round(activeMobs[targetMobIndex].posCol);
                    int slowPercent = (int)round(tower.slowdownPercent);
                    if (slowPercent < 0) slowPercent = 0;
                    if (slowPercent > 95) slowPercent = 95;
                    for (int i = 0; i < (int)activeMobs.size(); ++i) {
                        MobInstance& mob = activeMobs[i];
                        if (!mob.isAlive) continue;
                        int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                        if (abs(mobRow - targetRow) <= 1 && abs(mobCol - targetCol) <= 1) {
                            int armorDmg = (mob.armor > 0) ? 1 : 0;
                            damageToMob(mob, tower.hitpoints, armorDmg);
                            // Frost locks onto one mob, then applies slow to all mobs in a 3x3 area around that target.
                            mob.modifier.isSlowed = true;
                            mob.modifier.slowEffect = max(mob.modifier.slowEffect, slowPercent);
                            mob.modifier.slowedUntilTime = max(mob.modifier.slowedUntilTime, gameTime + 3.0);
                            mob.modifier.slowDuration = 3.0;
                            updateMobDynamicStats(mob, gameTime);
                            mob.lastHitTime = gameTime;
                        }
                    }
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
            else if (tower.name == "Earthquake Tower" || tower.symbol == 'E') {
                // Earthquake scans 9x9 (hitRange=4) around tower center, damages all mobs in that square AoE.
                int targetCount = 0;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow);
                    int mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange) {
                        int armorDmg = (mob.armor > 0) ? 1 : 0;
                        damageToMob(mob, tower.hitpoints, armorDmg);
                        mob.lastHitTime = gameTime;
                        targetCount++;
                    }
                }

                // Only consume cooldown/animation if at least one mob was inside range.
                if (targetCount > 0) {
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, "Earthquake Tower"});
                }
            }
            else if (tower.name == "Hell Tower") {
                int bestMobIndex = -1, highestHp = 0;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange && mob.health > highestHp) {
                        highestHp = mob.health;
                        bestMobIndex = i;
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int damage = target.health * tower.damagePercent / 100;
                    if (damage < 10) damage = 10;
                    if (damage > 500) damage = 500;
                    int armorDmg = (target.armor > 0) ? 1 : 0;
                    damageToMob(target, damage, armorDmg);
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
            else if (tower.name == "Thief Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange) {
                        double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestMobIndex = i;
                        }
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int armorDmg = (target.armor > 0) ? 1 : 0;
                    bool dead = damageToMob(target, tower.hitpoints, armorDmg);
                    target.lastHitTime = gameTime;
                    if (dead) {
                        moneyRef += getMobReward(target) * tower.currencyBonus / 100;
                    }
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
            else if (tower.name == "Armor Penetration Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange) {
                        double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestMobIndex = i;
                        }
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int armorDmg = (target.armor > 0) ? 100 : 0;
                    damageToMob(target, tower.hitpoints, armorDmg);
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
            else if (tower.name == "Vampire Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= tower.hitRange && abs(mobCol - centerCol) <= tower.hitRange) {
                        double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestMobIndex = i;
                        }
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int damage = tower.hitpoints;
                    int armorDmg = (target.armor > 0) ? 1 : 0;
                    bool dead = damageToMob(target, damage, armorDmg);
                    if (dead) {
                        int& killCount = vampireKillCounts[key];
                        killCount++;
                        if (killCount >= 10) {
                            baseHPRef += 1;
                            if (baseHPRef > 10) baseHPRef = 10;
                            killCount = 0;
                        }
                    }
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                    towerAttackFlashQueue.push_back({{centerRow-1, centerCol}, tower.name});
                }
            }
        }
    }
    
    // ============ 修正后的 update 函数（波次切换逻辑） ============
    void update(double dt, const vector<Tower>& towers) {
        towersRef = &towers;
        gameTime += dt;
        
        // Both demo and regular modes wait for manual wave trigger via 'z' key
        bool shouldSpawn = demoManualWaveActive;
        if (shouldSpawn) {
            currentWaveTime += dt;
        }
        
        if (currentWaveIndex < (int)waves.size()) {
            LevelWave& wave = waves[currentWaveIndex];
            // Spawn mobs only when wave is manually triggered (both demo and regular mode)
            if (demoManualWaveActive) {
                for (auto& event : wave.spawnEvents) {
                    if (event.spawnTime <= currentWaveTime && event.spawnTime >= currentWaveTime - dt) {
                        MobInstance newMob(event.mobType, (double)event.spawnRow, (double)event.spawnCol, event.speed, currentWaveIndex);
                        newMob.health = mobTypes[event.mobType].hp;
                        newMob.maxHealth = mobTypes[event.mobType].hp;
                        newMob.armor = mobTypes[event.mobType].armor;
                        newMob.maxArmor = mobTypes[event.mobType].armor;
                        newMob.baseGold = mobGolds[event.mobType];
                        newMob.modifiedSpeed = event.speed;
                        newMob.modifier = MobModifier();
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
                        } else {
                            int baseRow = -1, baseCol = -1;
                            for (int row = 0; row < GameMap::ROWS; ++row) {
                                for (int col = 0; col < GameMap::COLS; ++col) {
                                    if (gameMap.grid[row][col].type == BASE) {
                                        baseRow = row;
                                        baseCol = col;
                                        break;
                                    }
                                }
                                if (baseRow != -1) break;
                            }

                            if (baseRow != -1) {
                                vector<pair<int, int>> fallbackPath = findPathFromSpawn((int)round(newMob.posRow), (int)round(newMob.posCol), baseRow, baseCol);
                                if (fallbackPath.size() >= 2) {
                                    NavigationRoute route;
                                    route.spawnRow = (int)round(newMob.posRow);
                                    route.spawnCol = (int)round(newMob.posCol);
                                    route.nodes = fallbackPath;
                                    route.checkpointNodeIndex = (fallbackPath.size() > 2) ? (int)fallbackPath.size() / 2 : 0;
                                    route.checkpoint = fallbackPath[route.checkpointNodeIndex];
                                    navigationRoutes.push_back(route);
                                    newMob.routeIndex = (int)navigationRoutes.size() - 1;
                                    newMob.checkpointIndex = navigationRoutes[newMob.routeIndex].checkpointNodeIndex;
                                    routeNodes = &navigationRoutes[newMob.routeIndex].nodes;
                                }
                            }
                        }
                        updateMobDynamicStats(newMob, gameTime);
                        if (routeNodes != nullptr && routeNodes->size() >= 2) {
                            double nextRow = (*routeNodes)[1].first, nextCol = (*routeNodes)[1].second;
                            double dist = sqrt(pow(nextRow - newMob.posRow, 2) + pow(nextCol - newMob.posCol, 2));
                            if (dist > 0) {
                                newMob.velocityRow = (nextRow - newMob.posRow) / dist * newMob.modifiedSpeed;
                                newMob.velocityCol = (nextCol - newMob.posCol) / dist * newMob.modifiedSpeed;
                            }
                        } else {
                            // No valid route: skip spawn rather than creating a stuck mob.
                            continue;
                        }
                        activeMobs.push_back(newMob);
                        demoRenderDirty = true;
                    }
                }
                // Check if wave is complete
                if (currentWaveTime > wave.totalDuration && activeMobs.empty()) {
                    // Every mode should wait for manual Z press before next wave.
                    demoManualWaveActive = false;
                    demoWaitingForNextWave = true;
                    currentWaveIndex++;
                    currentWaveTime = 0;
                    demoRenderDirty = true;
                }
            }
        }
        
        // 更新所有活着的 mob 的位置
        for (auto& mob : activeMobs) {
            if (!mob.isAlive) continue;
            updateMobDynamicStats(mob, gameTime);
            const vector<pair<int, int>>* routeNodes = nullptr;
            if (mob.routeIndex >= 0 && mob.routeIndex < (int)navigationRoutes.size()) {
                routeNodes = &navigationRoutes[mob.routeIndex].nodes;
            } else {
                routeNodes = &globalPath;
            }
            mob.posRow += mob.velocityRow * dt;
            mob.posCol += mob.velocityCol * dt;
            int currentGridRow = (int)round(mob.posRow), currentGridCol = (int)round(mob.posCol);
            if (currentGridRow != mob.lastGridRow || currentGridCol != mob.lastGridCol) {
                mob.lastGridRow = currentGridRow;
                mob.lastGridCol = currentGridCol;
                demoRenderDirty = true;
            }
            if (routeNodes != nullptr && mob.routeStepIndex < (int)routeNodes->size()) {
                // Ensure we don't get stuck on repeated/identical nodes: skip zero-distance nodes
                double nextRow = (*routeNodes)[mob.routeStepIndex].first, nextCol = (*routeNodes)[mob.routeStepIndex].second;
                double dist = sqrt(pow(nextRow - mob.posRow, 2) + pow(nextCol - mob.posCol, 2));

                // If we're effectively on the target (or target equals current pos), advance until we find a different node
                while (mob.routeStepIndex < (int)routeNodes->size() && dist < 0.6) {
                    if (!mob.passedCheckpoint && mob.checkpointIndex >= 0 && mob.routeStepIndex >= mob.checkpointIndex) {
                        mob.passedCheckpoint = true;
                    }
                    mob.routeStepIndex++;
                    if (mob.routeStepIndex >= (int)routeNodes->size()) break;
                    nextRow = (*routeNodes)[mob.routeStepIndex].first;
                    nextCol = (*routeNodes)[mob.routeStepIndex].second;
                    dist = sqrt(pow(nextRow - mob.posRow, 2) + pow(nextCol - mob.posCol, 2));
                }

                if (mob.routeStepIndex < (int)routeNodes->size() && dist > 1e-6) {
                    mob.velocityRow = (nextRow - mob.posRow) / dist * mob.modifiedSpeed;
                    mob.velocityCol = (nextCol - mob.posCol) / dist * mob.modifiedSpeed;
                }
            }
            if (!useDemoAlgorithm) {
                pair<int, int> basePos = findBaseCamp();
                if (basePos.first != -1) {
                    int roundedRow = (int)round(mob.posRow);
                    int roundedCol = (int)round(mob.posCol);
                    int rowGap = abs(roundedRow - basePos.first);
                    int colGap = abs(roundedCol - basePos.second);
                    if (max(rowGap, colGap) <= 1) {
                        mob.isAlive = false;
                        mob.reachedBase = true;
                        baseHPRef -= 1;
                        if (baseHPRef < 0) baseHPRef = 0;
                    }
                }
            }
            if (routeNodes != nullptr && mob.routeStepIndex >= (int)routeNodes->size()) {
                mob.isAlive = false;
                mob.reachedBase = true;
                baseHPRef -= 1;
                if (baseHPRef < 0) baseHPRef = 0;
            }
        }
        
        // 纠正偏离路径的 mob（非 demo 模式）
        if (!useDemoAlgorithm) {
            for (auto &mob : activeMobs) {
                if (!mob.isAlive) continue;
                const vector<pair<int, int>>* routeNodes = nullptr;
                if (mob.routeIndex >= 0 && mob.routeIndex < (int)navigationRoutes.size()) {
                    routeNodes = &navigationRoutes[mob.routeIndex].nodes;
                } else {
                    routeNodes = &globalPath;
                }
                int r = (int)round(mob.posRow), c = (int)round(mob.posCol);
                if (r < 0 || r >= GameMap::ROWS || c < 0 || c >= GameMap::COLS) continue;
                if (!(gameMap.grid[r][c].type == PATH || gameMap.grid[r][c].type == BASE)) {
                    double bestDist = 1e9;
                    int bestR = -1, bestC = -1;
                    if (routeNodes != nullptr) {
                        for (size_t i = 0; i < routeNodes->size(); i++) {
                            int nr = (*routeNodes)[i].first, nc = (*routeNodes)[i].second;
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
        }

        processTowerAttacks(towers);
        
        // 处理死亡和金币奖励
        vector<MobInstance> survivors;
        survivors.reserve(activeMobs.size());
        for (auto &m : activeMobs) {
            if (m.isAlive) {
                survivors.push_back(m);
            } else {
                if (!m.reachedBase) {
                    moneyRef += getMobReward(m);
                }
                demoRenderDirty = true;
            }
        }
        activeMobs.swap(survivors);
    }

    void renderAttackFlashes(int mapStartLine) {
        for (const auto& entry : towerAttackFlashQueue) {
            int row = entry.first.first, col = entry.first.second;
            const string& towerName = entry.second;
            if (row < 0 || row >= GameMap::ROWS || col < 0 || col >= GameMap::COLS) continue;

            if (towerName == "Earthquake Tower") {
                static const string attackArt[3] = {"< >", "|!|", "^^^"};
                for (int artRow = 0; artRow < 3; ++artRow) {
                    for (int artCol = 0; artCol < 3; ++artCol) {
                        int drawRow = row + artRow;
                        int drawCol = col - 1 + artCol;
                        if (drawRow < 0 || drawRow >= GameMap::ROWS || drawCol < 0 || drawCol >= GameMap::COLS) continue;
                        setCursorPosition(drawCol, mapStartLine + drawRow);
                        setTextColor(10);
                        cout << attackArt[artRow][artCol];
                        resetTextColor();
                    }
                }
            } else {
                setCursorPosition(col, mapStartLine + row);
                
                // Show different symbols based on tower type
                if (towerName == "Arrow Tower") {
                    setTextColor(14);  // Yellow
                    cout << '-';
                } else if (towerName == "Laser Tower") {
                    setTextColor(14);  // Yellow
                    cout << 'o';
                } else if (towerName == "Frost Tower") {
                    setTextColor(11);  // Cyan
                    cout << '*';
                } else if (towerName == "Armor Penetration Tower") {
                    setTextColor(14);  // Yellow
                    cout << '^';
                } else if (towerName == "Hell Tower") {
                    setTextColor(6);  // Orange-ish
                    cout << '%';
                } else if (towerName == "Thief Tower") {
                    setTextColor(14);  // Yellow
                    cout << '$';
                } else if (towerName == "Vampire Tower") {
                    setTextColor(12);  // Red
                    cout << '+';
                } else {
                    setTextColor(14);  // Yellow (default)
                    cout << '-';
                }
                resetTextColor();
            }
        }
        towerAttackFlashQueue.clear();
    }
    
    void renderMobs(int mapStartLine, bool mapWasRedrawn = true) {
        vector<pair<int,int>> currentPositions;
        currentPositions.reserve(activeMobs.size());
        map<pair<int,int>, int> cellCount;
        map<pair<int,int>, int> cellBestMobIdx;
        for (auto &mob : activeMobs) {
            if (!mob.isAlive) continue;
            int r = (int)round(mob.posRow), c = (int)round(mob.posCol);
            if (r >= 0 && r < GameMap::ROWS && c >= 0 && c < GameMap::COLS) {
                currentPositions.push_back({r,c});
                pair<int,int> key = {r, c};
                cellCount[key]++;
                int idx = (int)(&mob - &activeMobs[0]);
                auto it = cellBestMobIdx.find(key);
                if (it == cellBestMobIdx.end()) {
                    cellBestMobIdx[key] = idx;
                } else {
                    int prevIdx = it->second;
                    if (activeMobs[idx].health > activeMobs[prevIdx].health) {
                        it->second = idx;
                    }
                }
            }
        }
        for (auto &mob : activeMobs) {
            if (!mob.isAlive) continue;
            int prevR = mob.prevRenderRow, prevC = mob.prevRenderCol;
            bool prevValid = (prevR >= 0 && prevR < GameMap::ROWS && prevC >= 0 && prevC < GameMap::COLS);
            if (prevValid) {
                bool stillOccupied = false;
                for (auto &p : currentPositions) {
                    if (p.first == prevR && p.second == prevC) { stillOccupied = true; break; }
                }
                if (!stillOccupied) {
                    Tile &t = gameMap.grid[prevR][prevC];
                    setCursorPosition(prevC, mapStartLine + prevR);
                    switch (t.type) {
                        case PATH: setTextColor(14); cout << t.displayChar; resetTextColor(); break;
                        case BUILDABLE: setTextColor(8); cout << t.displayChar; resetTextColor(); break;
                        case TOWER: {
                            int auraColor = getTowerAuraColor(prevR, prevC);
                            if (auraColor == 5) {
                                setTextColor(15 | (5 << 4));
                            } else if (auraColor == 6) {
                                setTextColor(15 | (6 << 4));
                            } else {
                                setTextColor(15);
                            }
                            cout << t.displayChar;
                            resetTextColor();
                            break;
                        }
                        case BASE: setTextColor(12); cout << 'B'; resetTextColor(); break;
                        case BLOCKED: cout << ' '; break;
                        default: cout << t.displayChar; break;
                    }
                }
            }
        }
        for (size_t mobIdx = 0; mobIdx < activeMobs.size(); ++mobIdx) {
            auto& mob = activeMobs[mobIdx];
            if (!mob.isAlive) continue;
            int screenRow = (int)round(mob.posRow), screenCol = (int)round(mob.posCol);
            if (screenRow >= 0 && screenRow < GameMap::ROWS && screenCol >= 0 && screenCol < GameMap::COLS) {
                pair<int,int> key = {screenRow, screenCol};
                bool hasStack = (cellCount[key] > 1);
                if (hasStack && cellBestMobIdx[key] != (int)mobIdx) {
                    // This tile has multiple mobs; only render the highest-health one.
                    continue;
                }

                bool isPigman = false;
                if (mob.mobType >= 0 && mob.mobType < (int)mobTypes.size()) {
                    const string& mobName = mobTypes[mob.mobType].name;
                    isPigman = (mobName.find("Pigman") != string::npos || mobName.find("pigman") != string::npos);
                }
                if (useDemoAlgorithm && isPigman && !mapWasRedrawn && screenRow == mob.prevRenderRow && screenCol == mob.prevRenderCol) {
                    continue;
                }
                setCursorPosition(screenCol, mapStartLine + screenRow);
                char mobChar = mobTypes[mob.mobType].symbol;
                if (hasStack) {
                    // Highlight crowded tiles with colored background.
                    int foreground = 15;
                    if (mob.modifier.isSlowed && gameTime < mob.modifier.slowedUntilTime) {
                        foreground = 13;
                    } else if (mob.armor > 0) {
                        foreground = 1;  // Deep blue for armored mobs in stack
                    } else if (gameTime - mob.lastHitTime < 0.18) {
                        foreground = 12;
                    }
                    int stackedColor = foreground | (4 << 4);  // red background
                    setTextColor(stackedColor);
                } else {
                    int auraColor = getTowerAuraColor(screenRow, screenCol);
                    if (auraColor == 5) {
                        setTextColor(15 | (5 << 4));
                    } else if (auraColor == 6) {
                        setTextColor(15 | (6 << 4));
                    } else if (mob.armor > 0) {
                        setTextColor(1);  // Deep blue for armored mobs
                    } else if (mob.modifier.isSlowed && gameTime < mob.modifier.slowedUntilTime) {
                        setTextColor(13);  // Purple for slowed mobs
                    } else {
                        setTextColor(11);  // Default yellow-green
                    }
                    if (!(mob.modifier.isSlowed && gameTime < mob.modifier.slowedUntilTime) && gameTime - mob.lastHitTime < 0.18) setTextColor(12);
                }
                cout << mobChar;
                resetTextColor();
                mob.prevRenderRow = screenRow;
                mob.prevRenderCol = screenCol;
            }
        }
    }
    
    // Getters
    int getActiveMobCount() const { return activeMobs.size(); }
    int getTotalWaveCount() const { return (int)waves.size(); }
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
    void startNextWave() {
        if (currentWaveIndex < 0 || currentWaveIndex >= (int)waves.size()) return;
        if (currentWaveIndex < (int)waves.size() - 1) {
            currentWaveIndex++;
            currentWaveTime = 0;
        }
    }

    pair<int, int> findBaseCamp() const {
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                if (gameMap.grid[i][j].type == BASE) return make_pair(i, j);
            }
        }
        return make_pair(-1, -1);
    }

    double calculateDistance(int row1, int col1, int row2, int col2) const {
        return sqrt(pow(row1 - row2, 2) + pow(col1 - col2, 2));
    }

    void updateMobPositionGreedy(MobInstance& mob, double dt) {
        if (!mob.isAlive) return;
        pair<int, int> basePos = findBaseCamp();
        if (basePos.first == -1) return;
        int currentRow = (int)round(mob.posRow), currentCol = (int)round(mob.posCol);
        double currentDist = calculateDistance(currentRow, currentCol, basePos.first, basePos.second);
        int dr[] = {-1, 1, 0, 0}, dc[] = {0, 0, -1, 1};
        double bestDist = currentDist;
        int bestRow = currentRow, bestCol = currentCol;
        for (int i = 0; i < 4; i++) {
            int nextRow = currentRow + dr[i], nextCol = currentCol + dc[i];
            if (nextRow >= 0 && nextRow < GameMap::ROWS && nextCol >= 0 && nextCol < GameMap::COLS && isWalkableTile(nextRow, nextCol)) {
                double nextDist = calculateDistance(nextRow, nextCol, basePos.first, basePos.second);
                if (nextDist < bestDist) {
                    bestDist = nextDist;
                    bestRow = nextRow;
                    bestCol = nextCol;
                }
            }
        }
        if (bestRow != currentRow || bestCol != currentCol) {
            double dist = calculateDistance(currentRow, currentCol, bestRow, bestCol);
            if (dist > 0) {
                mob.velocityRow = (bestRow - currentRow) / dist * mob.modifiedSpeed;
                mob.velocityCol = (bestCol - currentCol) / dist * mob.modifiedSpeed;
            }
        } else {
            mob.velocityRow = mob.velocityCol = 0;
        }
        mob.posRow += mob.velocityRow * dt;
        mob.posCol += mob.velocityCol * dt;
        int r = (int)round(mob.posRow), c = (int)round(mob.posCol);
        if (r == basePos.first && c == basePos.second) {
            mob.isAlive = false;
            mob.reachedBase = true;
            baseHPRef--;
            if (baseHPRef < 0) baseHPRef = 0;
        }
    }

    // ============ 辅助函数 (解析、修改器) ============
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
            if (line.find("Enemy") == 0 && line.find(':') != string::npos) {
                size_t colonPos = line.find(':');
                size_t spacePos = line.find(' ', 6);
                if (spacePos != string::npos) {
                    string numStr = line.substr(6, spacePos - 6);
                    currentMobIndex = stoi(numStr) - 1;
                }
            }
            if (currentMobIndex >= 0 && currentMobIndex < (int)mobTypes.size()) {
                if (line.find("Symbol:") != string::npos) {
                    size_t pos = line.find("Symbol:");
                    string sym = line.substr(pos + 7);
                    while (!sym.empty() && isspace(sym[0])) sym.erase(0, 1);
                    if (!sym.empty()) {
                        mobTypes[currentMobIndex].symbol = sym[0];
                    }
                }
                else if (line.find("HP:") != string::npos) {
                    size_t pos = line.find("HP:");
                    string numStr = line.substr(pos + 3);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0,1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].hp = stoi(numStr);
                }
                else if (line.find("Armor:") != string::npos) {
                    size_t pos = line.find("Armor:");
                    string numStr = line.substr(pos + 6);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0,1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].armor = stoi(numStr);
                }
                else if (line.find("Speed:") != string::npos) {
                    size_t pos = line.find("Speed:");
                    string numStr = line.substr(pos + 6);
                    while (!numStr.empty() && !isdigit(numStr[0]) && numStr[0] != '.') numStr.erase(0,1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].speed = stod(numStr);
                }
                else if (line.find("Gold:") != string::npos) {
                    size_t pos = line.find("Gold:");
                    string numStr = line.substr(pos + 5);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0,1);
                    if (!numStr.empty()) mobGolds[currentMobIndex] = stoi(numStr);
                }
                else if (line.find("Flying:") != string::npos) {
                    mobTypes[currentMobIndex].isFlying = (line.find("Yes") != string::npos);
                }
                else if (line.find("Slow Effect:") != string::npos) {
                    // Example: "Slow Effect: 50% slowdown in 3x3 area for 5 seconds"
                    size_t pos = line.find("Slow Effect:");
                    string info = line.substr(pos + 12);

                    string percent = info;
                    while (!percent.empty() && !isdigit(percent[0])) percent.erase(0, 1);
                    if (!percent.empty()) mobTypes[currentMobIndex].slowEffect = stod(percent);

                    if (info.find("3x3") != string::npos) mobTypes[currentMobIndex].slowArea = 9;
                    else if (info.find("full-screen") != string::npos) mobTypes[currentMobIndex].slowArea = 999;

                    size_t forPos = info.find("for");
                    if (forPos != string::npos) {
                        string duration = info.substr(forPos + 3);
                        while (!duration.empty() && !isdigit(duration[0]) && duration[0] != '.') duration.erase(0, 1);
                        if (!duration.empty()) mobTypes[currentMobIndex].slowDuration = stod(duration);
                    }
                }
                else if (line.find("Summon Cooldown:") != string::npos) {
                    size_t pos = line.find("Summon Cooldown:");
                    string numStr = line.substr(pos + 16);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0,1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].summonCooldown = stoi(numStr);
                }
                else if (line.find("Summons:") != string::npos) {
                    size_t pos = line.find("Summons:");
                    string numStr = line.substr(pos + 8);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0,1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].summonCount = stoi(numStr);
                }
                else if (line.find("Damage Area:") != string::npos) {
                    size_t pos = line.find("Damage Area:");
                    string info = line.substr(pos + 12);
                    if (info.find("3x3") != string::npos) mobTypes[currentMobIndex].damageArea = 9;
                }
                else if (line.find("Attack Cooldown:") != string::npos) {
                    size_t pos = line.find("Attack Cooldown:");
                    string numStr = line.substr(pos + 16);
                    while (!numStr.empty() && !isdigit(numStr[0])) numStr.erase(0,1);
                    if (!numStr.empty()) mobTypes[currentMobIndex].attackCooldown = stoi(numStr);
                }
            }
        }
        file.close();
    }

    void parseSpawnLine(const string& line, int& count, int& mobType, int& spawnRow, int& spawnCol) {
        count = 0; mobType = -1; spawnRow = -1; spawnCol = -1;
        size_t numPos = 0;
        while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
        if (numPos < line.length()) {
            size_t numEnd = numPos;
            while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
            count = stoi(line.substr(numPos, numEnd - numPos));
        }
        if (line.find("pigman") != string::npos && line.find("berserker") == string::npos) mobType = 0;
        else if (line.find("houndling") != string::npos) mobType = 1;
        else if (line.find("werewolf") != string::npos) mobType = 2;
        else if (line.find("mini mammon") != string::npos) mobType = 3;
        else if (line.find("armored mammon") != string::npos) mobType = 4;
        else if (line.find("birdman") != string::npos) mobType = 5;
        else if (line.find("batman") != string::npos) mobType = 6;
        else if (line.find("berserker") != string::npos) mobType = 7;
        else if (line.find("spiderman") != string::npos) mobType = 8;
        else if (line.find("alpha wolf") != string::npos) mobType = 9;
        else if (line.find("dragon") != string::npos) {
            if (line.find("type 1") != string::npos || line.find("^D^") != string::npos) mobType = 10;
            else if (line.find("type 2") != string::npos) mobType = 11;
            else if (line.find("type 3") != string::npos || line.find("!D!") != string::npos) mobType = 12;
        }
        if (line.find("row") != string::npos) {
            size_t rowPos = line.find("row");
            numPos = rowPos + 3;
            while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
            if (numPos < line.length()) {
                size_t numEnd = numPos;
                while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
                spawnRow = stoi(line.substr(numPos, numEnd - numPos)) - 1;
                if (spawnRow < 0) spawnRow = 0;
                if (spawnRow >= GameMap::ROWS) spawnRow = GameMap::ROWS - 1;
                spawnCol = (line.find("left") != string::npos) ? 0 : (line.find("right") != string::npos) ? GameMap::COLS - 1 : 50;
            }
        } else if (line.find("column") != string::npos) {
            size_t colPos = line.find("column");
            numPos = colPos + 6;
            while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
            if (numPos < line.length()) {
                size_t numEnd = numPos;
                while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
                spawnCol = stoi(line.substr(numPos, numEnd - numPos));
                spawnRow = (line.find("top") != string::npos || line.find("up") != string::npos) ? 0 : (line.find("down") != string::npos || line.find("bottom") != string::npos) ? GameMap::ROWS - 1 : GameMap::ROWS - 1;
            }
        }
    }

    void applyMobModifier(MobInstance& mob, const MobModifier& modifier, double currentGameTime) {
        mob.modifier.speedMultiplier *= modifier.speedMultiplier;
        mob.modifier.hpMultiplier *= modifier.hpMultiplier;
        mob.modifier.goldMultiplier *= modifier.goldMultiplier;
        mob.modifier.damageMultiplier *= modifier.damageMultiplier;
        mob.modifier.armorBonus += modifier.armorBonus;
        if (modifier.isSlowed) {
            mob.modifier.isSlowed = true;
            if (modifier.slowEffect > mob.modifier.slowEffect) {
                mob.modifier.slowEffect = modifier.slowEffect;
            }
            if (modifier.slowDuration > 0) {
                mob.modifier.slowedUntilTime = max(mob.modifier.slowedUntilTime, currentGameTime + modifier.slowDuration);
                mob.modifier.slowDuration = modifier.slowDuration;
            }
        }
        updateMobDynamicStats(mob, currentGameTime);
    }

    void updateMobDynamicStats(MobInstance& mob, double currentGameTime) {
        mob.modifiedSpeed = mob.moveSpeed * mob.modifier.speedMultiplier;
        if (mob.modifier.isSlowed && currentGameTime < mob.modifier.slowedUntilTime) {
            mob.modifiedSpeed *= (1.0 - mob.modifier.slowEffect / 100.0);
        } else if (mob.modifier.isSlowed && currentGameTime >= mob.modifier.slowedUntilTime) {
            mob.modifier.isSlowed = false;
            mob.modifier.slowEffect = 0;
        }
        if (mob.modifiedSpeed < 0.1) mob.modifiedSpeed = 0.1;
        if (mobTypes[mob.mobType].hp > 0) {
            mob.maxHealth = (int)(mobTypes[mob.mobType].hp * mob.modifier.hpMultiplier);
            if (mob.health > mob.maxHealth) mob.health = mob.maxHealth;
        }
        if (mobGolds[mob.mobType] > 0) {
            mob.baseGold = (int)(mobGolds[mob.mobType] * mob.modifier.goldMultiplier);
        }
    }

    void removeMobSlow(MobInstance& mob, double currentGameTime) {
        mob.modifier.isSlowed = false;
        mob.modifier.slowEffect = 0;
        mob.modifier.slowedUntilTime = 0;
        updateMobDynamicStats(mob, currentGameTime);
    }

    bool damageToMob(MobInstance& mob, int damageAmount, int armorDamage = 0) {
        // First apply armor damage
        if (armorDamage > 0) {
            mob.armor -= armorDamage;
            if (mob.armor < 0) {
                mob.armor = 0;
            }
        }
        
        // Then apply health damage (only if armor is depleted)
        if (mob.armor <= 0) {
            mob.health -= damageAmount;
            if (mob.health <= 0) {
                mob.health = 0;
                mob.isAlive = false;
                return true;
            }
        }
        return false;
    }

    void healMob(MobInstance& mob, int healAmount) {
        mob.health += healAmount;
        if (mob.health > mob.maxHealth) mob.health = mob.maxHealth;
    }

    int getEffectiveMobArmor(const MobInstance& mob) const {
        if (mob.mobType < 0 || mob.mobType >= (int)mobTypes.size()) return 0;
        return mobTypes[mob.mobType].armor + mob.modifier.armorBonus;
    }

    int getMobReward(const MobInstance& mob) const {
        return mob.baseGold;
    }
    
    // Load backend paths from computed spawn points to base
    void loadBackendPaths(const map<pair<int, int>, vector<pair<pair<int,int>, pair<int,int>>>>& backendPaths) {
        spawnPointPaths = backendPaths;
        if (spawnPointPaths.empty()) return;

        auto rebuildNodes = [](const vector<pair<pair<int,int>, pair<int,int>>>& commands) {
            vector<pair<int, int>> nodes;
            if (commands.empty()) return nodes;

            nodes.push_back(commands.front().first);
            for (const auto& command : commands) {
                pair<int, int> current = nodes.back();
                pair<int, int> target = command.second;
                while (current.first != target.first || current.second != target.second) {
                    if (current.first < target.first) current.first++;
                    else if (current.first > target.first) current.first--;
                    else if (current.second < target.second) current.second++;
                    else if (current.second > target.second) current.second--;
                    nodes.push_back(current);
                }
            }
            return nodes;
        };

        for (const auto& entry : spawnPointPaths) {
            vector<pair<int, int>> nodes = rebuildNodes(entry.second);
            if (nodes.size() < 2) continue;

            int existingRouteIndex = findRouteIndexForSpawn(entry.first.first, entry.first.second);
            if (existingRouteIndex >= 0) {
                navigationRoutes[existingRouteIndex].nodes = nodes;
                navigationRoutes[existingRouteIndex].checkpointNodeIndex = (nodes.size() > 2) ? (int)nodes.size() / 2 : 0;
                navigationRoutes[existingRouteIndex].checkpoint = nodes[navigationRoutes[existingRouteIndex].checkpointNodeIndex];
            } else {
                NavigationRoute route;
                route.spawnRow = entry.first.first;
                route.spawnCol = entry.first.second;
                route.nodes = nodes;
                route.checkpointNodeIndex = (nodes.size() > 2) ? (int)nodes.size() / 2 : 0;
                route.checkpoint = nodes[route.checkpointNodeIndex];
                navigationRoutes.push_back(route);
            }
        }

        int baseRow = -1;
        int baseCol = -1;
        for (int row = 0; row < GameMap::ROWS; ++row) {
            for (int col = 0; col < GameMap::COLS; ++col) {
                if (gameMap.grid[row][col].type == BASE) {
                    baseRow = row;
                    baseCol = col;
                    break;
                }
            }
            if (baseRow != -1) break;
        }

        if (baseRow != -1) {
            for (const auto& wave : waves) {
                for (const auto& event : wave.spawnEvents) {
                    if (findRouteIndexForSpawn(event.spawnRow, event.spawnCol) >= 0) continue;
                    vector<pair<int, int>> nodes = findPathFromSpawn(event.spawnRow, event.spawnCol, baseRow, baseCol);
                    if (nodes.size() < 2) continue;

                    NavigationRoute route;
                    route.spawnRow = event.spawnRow;
                    route.spawnCol = event.spawnCol;
                    route.nodes = nodes;
                    route.checkpointNodeIndex = (nodes.size() > 2) ? (int)nodes.size() / 2 : 0;
                    route.checkpoint = nodes[route.checkpointNodeIndex];
                    navigationRoutes.push_back(route);
                }
            }
        }

        if (!navigationRoutes.empty()) {
            globalPath = navigationRoutes[0].nodes;
        }

        for (auto& wave : waves) {
            for (auto& event : wave.spawnEvents) {
                event.routeIndex = findRouteIndexForSpawn(event.spawnRow, event.spawnCol);
            }
        }
    }
    
    // Get path for a specific spawn point
    vector<pair<pair<int,int>, pair<int,int>>> getPathForSpawn(int spawnRow, int spawnCol) {
        auto it = spawnPointPaths.find({spawnRow, spawnCol});
        if (it != spawnPointPaths.end()) {
            return it->second;
        }
        return vector<pair<pair<int,int>, pair<int,int>>>();
    }
};

#endif