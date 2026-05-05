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
                lastHitTime(-9999.0), spawnPointRow((int)row), spawnPointCol((int)col), prevRenderRow((int)row), prevRenderCol((int)col), lastGridRow((int)row), lastGridCol((int)col), baseGold(0), maxHealth(0) {
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
    vector<pair<int, int>> towerDashFlashQueue;
    bool demoRenderDirty;
    
    int currentWaveIndex;
    double currentWaveTime;
    double gameTime;
    
    vector<Mob>& mobTypes;
    GameMap& gameMap;
    vector<int> mobGolds;
    bool useDemoAlgorithm;
    bool demoManualWaveActive;
    bool demoWaitingForNextWave;
    
public:
    int &moneyRef;
    int &baseHPRef;

    MobSystemManager(vector<Mob>& mobTypes_, GameMap& gameMap_, int &money_, int &baseHP_, bool demoMode = false) 
        : mobTypes(mobTypes_), gameMap(gameMap_), currentWaveIndex(0), 
            currentWaveTime(0), gameTime(0), moneyRef(money_), baseHPRef(baseHP_),
            useDemoAlgorithm(demoMode), demoManualWaveActive(false), demoWaitingForNextWave(false), demoRenderDirty(true) {
        loadMobGolds();
        if (useDemoAlgorithm) demoWaitingForNextWave = true;
    }
    
    // ============ 修正后的 loadLevelDesign ============
    void loadLevelDesign(int level) {
        waves.clear();
        currentWaveTime = 0;
        gameTime = 0;

        if (useDemoAlgorithm) {
            loadDemoLevelDesign(level);
            return;
        }

        string filename = buildLevelDesignPath(level);
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Could not load level design: " << filename << endl;
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
                        for (int i = 0; i < count; ++i) {
                            SpawnEvent ev;
                            ev.spawnTime = waveTime;
                            ev.mobType = mobType;
                            ev.spawnRow = spawnRow;
                            ev.spawnCol = spawnCol;
                            ev.speed = mobTypes[mobType].speed;
                            ev.routeIndex = findRouteIndexForSpawn(spawnRow, spawnCol);
                            currentWave.spawnEvents.push_back(ev);
                            lastSpawnTime = waveTime;
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
                        ev.spawnRow = spawnRow;
                        ev.spawnCol = spawnCol;
                        ev.speed = mobTypes[mobType].speed;
                        ev.routeIndex = findRouteIndexForSpawn(spawnRow, spawnCol);
                        currentWave.spawnEvents.push_back(ev);
                        lastSpawnTime = waveTime;
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
        const double spawnGap = 0.7;
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
        if (!useDemoAlgorithm) return;
        if (currentWaveIndex >= (int)waves.size()) return;
        if (demoManualWaveActive) return;
        if (!activeMobs.empty()) return;
        demoManualWaveActive = true;
        demoWaitingForNextWave = false;
        demoRenderDirty = true;
        currentWaveTime = 0;
    }

    bool isWaitingForNextWave() const {
        if (!useDemoAlgorithm) return false;
        if (currentWaveIndex >= (int)waves.size()) return false;
        return demoWaitingForNextWave;
    }

    bool allWavesSpawned() const {
        return currentWaveIndex >= (int)waves.size();
    }

    bool consumeDemoRenderDirty() {
        if (!useDemoAlgorithm) return true;
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
        vector<vector<pair<int, int>>> parent(GameMap::ROWS, vector<pair<int, int>>(GameMap::COLS, make_pair(-1, -1)));
        queue<pair<int, int>> q;
        q.push(make_pair(startRow, startCol));
        visited[startRow][startCol] = true;
        bool found = false;
        while (!q.empty()) {
            pair<int, int> current = q.front(); q.pop();
            int row = current.first, col = current.second;
            if (row == baseRow && col == baseCol) { found = true; break; }
            int dr[4] = {-1, 1, 0, 0};
            int dc[4] = {0, 0, -1, 1};
            for (int i = 0; i < 4; i++) {
                int nextRow = row + dr[i], nextCol = col + dc[i];
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

    vector<char> buildCommandsFromPath(const vector<pair<int, int>>& path) const {
        vector<char> commands;
        if (path.size() < 2) return commands;
        for (size_t i = 1; i < path.size(); i++) {
            int rowDelta = path[i].first - path[i-1].first;
            int colDelta = path[i].second - path[i-1].second;
            if (rowDelta == -1 && colDelta == 0) commands.push_back('U');
            else if (rowDelta == 1 && colDelta == 0) commands.push_back('D');
            else if (rowDelta == 0 && colDelta == -1) commands.push_back('L');
            else if (rowDelta == 0 && colDelta == 1) commands.push_back('R');
            else commands.push_back('?');
        }
        return commands;
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

    // ============ 塔攻击逻辑（已完整，保持不变） ============
    void processTowerAttacks(const vector<Tower>& towers) {
        vector<pair<int, int>> towerCenters = getPlacedTowerCenters();
        for (const auto& center : towerCenters) {
            int centerRow = center.first, centerCol = center.second;
            const Tile& centerTile = gameMap.grid[centerRow][centerCol];
            int towerTypeIndex = centerTile.towerIndex;
            if (towerTypeIndex < 0 || towerTypeIndex >= (int)towers.size()) continue;
            const Tower& tower = towers[towerTypeIndex];
            long long key = makeTowerKey(centerRow, centerCol);
            double& nextAttackTime = towerNextAttackTime[key];
            double finalAttackSpeed = tower.attackSpeed;
            for (int r = centerRow - 5; r <= centerRow + 5; ++r) {
                for (int c = centerCol - 5; c <= centerCol + 5; ++c) {
                    if (r < 0 || r >= GameMap::ROWS || c < 0 || c >= GameMap::COLS) continue;
                    const Tile& tile = gameMap.grid[r][c];
                    if (tile.type == TOWER && tile.towerIndex >= 0 && tile.towerPosRow == 1 && tile.towerPosCol == 1) {
                        const Tower& nearbyTower = towers[tile.towerIndex];
                        if (nearbyTower.name == "War Drum Tower") {
                            finalAttackSpeed *= (1.0 + nearbyTower.hpBuffPercent / 100.0);
                            break;
                        }
                    }
                }
            }
            if (nextAttackTime > gameTime) continue;

            if (tower.name == "Arrow Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                    if (dist <= tower.hitRange && dist < bestDist) {
                        bestDist = dist;
                        bestMobIndex = i;
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    damageToMob(target, tower.hitpoints);
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                }
            }
            else if (tower.name == "Laser Tower") {
                int targetRow = centerRow;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    if (mobTypes[mob.mobType].isFlying) continue;
                    int mobRow = (int)round(mob.posRow);
                    if (mobRow == targetRow) {
                        damageToMob(mob, tower.hitpoints);
                        mob.lastHitTime = gameTime;
                    }
                }
                nextAttackTime = gameTime + finalAttackSpeed;
                towerDashFlashQueue.push_back({centerRow-1, centerCol});
            }
            else if (tower.name == "Frost Tower") {
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    if (abs(mobRow - centerRow) <= 1 && abs(mobCol - centerCol) <= 1) {
                        damageToMob(mob, tower.hitpoints);
                        MobModifier slowMod;
                        slowMod.slowEffect = (int)tower.slowdownPercent;
                        slowMod.isSlowed = true;
                        slowMod.slowDuration = 3.0;
                        applyMobModifier(mob, slowMod, gameTime);
                        mob.lastHitTime = gameTime;
                    }
                }
                nextAttackTime = gameTime + finalAttackSpeed;
                towerDashFlashQueue.push_back({centerRow-1, centerCol});
            }
            else if (tower.name == "Earthquake Tower") {
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                    if (dist <= tower.hitRange) {
                        damageToMob(mob, tower.hitpoints);
                        mob.lastHitTime = gameTime;
                    }
                }
                nextAttackTime = gameTime + finalAttackSpeed;
                towerDashFlashQueue.push_back({centerRow-1, centerCol});
            }
            else if (tower.name == "Hell Tower") {
                int bestMobIndex = -1, highestHp = 0;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                    if (dist <= tower.hitRange && mob.health > highestHp) {
                        highestHp = mob.health;
                        bestMobIndex = i;
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int damage = target.health * tower.damagePercent / 100;
                    if (damage < 50) damage = 50;
                    if (damage > 500) damage = 500;
                    if (getEffectiveMobArmor(target) > 0) damage = 50;
                    damageToMob(target, damage);
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                }
            }
            else if (tower.name == "Armor Penetration Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                    if (dist <= tower.hitRange && dist < bestDist) {
                        bestDist = dist;
                        bestMobIndex = i;
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int baseDamage = tower.hitpoints;
                    int finalDamage = (getEffectiveMobArmor(target) > 0) ? baseDamage * 2 : baseDamage;
                    damageToMob(target, finalDamage);
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                }
            }
            else if (tower.name == "Vampire Tower") {
                int bestMobIndex = -1;
                double bestDist = 1e18;
                for (int i = 0; i < (int)activeMobs.size(); ++i) {
                    MobInstance& mob = activeMobs[i];
                    if (!mob.isAlive) continue;
                    int mobRow = (int)round(mob.posRow), mobCol = (int)round(mob.posCol);
                    double dist = sqrt(pow(mobRow - centerRow, 2) + pow(mobCol - centerCol, 2));
                    if (dist <= tower.hitRange && dist < bestDist) {
                        bestDist = dist;
                        bestMobIndex = i;
                    }
                }
                if (bestMobIndex >= 0) {
                    MobInstance& target = activeMobs[bestMobIndex];
                    int damage = tower.hitpoints;
                    bool dead = damageToMob(target, damage);
                    if (!dead) {
                        int healAmount = static_cast<int>(damage * tower.healPercent / 100.0);
                        baseHPRef += healAmount;
                        if (baseHPRef > 10) baseHPRef = 10;
                    }
                    target.lastHitTime = gameTime;
                    nextAttackTime = gameTime + finalAttackSpeed;
                    towerDashFlashQueue.push_back({centerRow-1, centerCol});
                }
            }
        }
    }
    
    // ============ 修正后的 update 函数（波次切换逻辑） ============
    void update(double dt, const vector<Tower>& towers) {
        gameTime += dt;
        if (!(useDemoAlgorithm && !demoManualWaveActive)) {
            currentWaveTime += dt;
        }
        
        if (currentWaveIndex < (int)waves.size()) {
            LevelWave& wave = waves[currentWaveIndex];
            if (useDemoAlgorithm) {
                if (demoManualWaveActive) {
                    for (auto& event : wave.spawnEvents) {
                        if (event.spawnTime <= currentWaveTime && event.spawnTime >= currentWaveTime - dt) {
                            MobInstance newMob(event.mobType, (double)event.spawnRow, (double)event.spawnCol, event.speed, currentWaveIndex);
                            newMob.health = mobTypes[event.mobType].hp;
                            newMob.maxHealth = mobTypes[event.mobType].hp;
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
                            }
                            updateMobDynamicStats(newMob, gameTime);
                            if (routeNodes != nullptr && routeNodes->size() >= 2) {
                                double nextRow = (*routeNodes)[1].first, nextCol = (*routeNodes)[1].second;
                                double dist = sqrt(pow(nextRow - newMob.posRow, 2) + pow(nextCol - newMob.posCol, 2));
                                if (dist > 0) {
                                    newMob.velocityRow = (nextRow - newMob.posRow) / dist * newMob.modifiedSpeed;
                                    newMob.velocityCol = (nextCol - newMob.posCol) / dist * newMob.modifiedSpeed;
                                }
                            }
                            activeMobs.push_back(newMob);
                        }
                    }
                    if (currentWaveTime > wave.totalDuration && activeMobs.empty()) {
                        demoManualWaveActive = false;
                        demoWaitingForNextWave = true;
                        currentWaveIndex++;
                        currentWaveTime = 0;
                        demoRenderDirty = true;
                    }
                }
            } else {
                // 非 demo 模式：按时间产生敌人
                for (auto& event : wave.spawnEvents) {
                    if (event.spawnTime <= currentWaveTime && event.spawnTime >= currentWaveTime - dt) {
                        MobInstance newMob(event.mobType, (double)event.spawnRow, (double)event.spawnCol, event.speed, currentWaveIndex);
                        newMob.health = mobTypes[event.mobType].hp;
                        newMob.maxHealth = mobTypes[event.mobType].hp;
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
                        }
                        updateMobDynamicStats(newMob, gameTime);
                        if (routeNodes != nullptr && routeNodes->size() >= 2) {
                            double nextRow = (*routeNodes)[1].first, nextCol = (*routeNodes)[1].second;
                            double dist = sqrt(pow(nextRow - newMob.posRow, 2) + pow(nextCol - newMob.posCol, 2));
                            if (dist > 0) {
                                newMob.velocityRow = (nextRow - newMob.posRow) / dist * newMob.modifiedSpeed;
                                newMob.velocityCol = (nextCol - newMob.posCol) / dist * newMob.modifiedSpeed;
                            }
                        }
                        activeMobs.push_back(newMob);
                        demoRenderDirty = true;
                    }
                }

                // ========== 修正后的波次完成判断 ==========
                bool allSpawned = (currentWaveTime >= wave.totalDuration);
                int remainingInWave = 0;
                for (const auto& m : activeMobs) {
                    if (m.spawnWaveId == currentWaveIndex && m.isAlive) remainingInWave++;
                }
                for (const auto& e : wave.spawnEvents) {
                    if (e.spawnTime > currentWaveTime) remainingInWave++;
                }
                if (allSpawned && remainingInWave == 0) {
                    currentWaveIndex++;
                    currentWaveTime = 0;
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
                double nextRow = (*routeNodes)[mob.routeStepIndex].first, nextCol = (*routeNodes)[mob.routeStepIndex].second;
                double dist = sqrt(pow(nextRow - mob.posRow, 2) + pow(nextCol - mob.posCol, 2));
                if (dist < 1.0) {
                    if (!mob.passedCheckpoint && mob.checkpointIndex >= 0 && mob.routeStepIndex >= mob.checkpointIndex) {
                        mob.passedCheckpoint = true;
                    }
                    mob.routeStepIndex++;
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
        for (const auto &cell : towerDashFlashQueue) {
            int row = cell.first, col = cell.second;
            if (row < 0 || row >= GameMap::ROWS || col < 0 || col >= GameMap::COLS) continue;
            setCursorPosition(col, mapStartLine + row);
            setTextColor(14);
            cout << '-';
            resetTextColor();
        }
        towerDashFlashQueue.clear();
    }
    
    void renderMobs(int mapStartLine, bool mapWasRedrawn = true) {
        vector<pair<int,int>> currentPositions;
        currentPositions.reserve(activeMobs.size());
        for (auto &mob : activeMobs) {
            if (!mob.isAlive) continue;
            int r = (int)round(mob.posRow), c = (int)round(mob.posCol);
            if (r >= 0 && r < GameMap::ROWS && c >= 0 && c < GameMap::COLS) {
                currentPositions.push_back({r,c});
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
                        case TOWER: setTextColor(15); cout << t.displayChar; resetTextColor(); break;
                        case BASE: setTextColor(12); cout << 'B'; resetTextColor(); break;
                        case BLOCKED: cout << ' '; break;
                        default: cout << t.displayChar; break;
                    }
                }
            }
        }
        for (auto& mob : activeMobs) {
            if (!mob.isAlive) continue;
            int screenRow = (int)round(mob.posRow), screenCol = (int)round(mob.posCol);
            if (screenRow >= 0 && screenRow < GameMap::ROWS && screenCol >= 0 && screenCol < GameMap::COLS) {
                bool isPigman = false;
                if (mob.mobType >= 0 && mob.mobType < (int)mobTypes.size()) {
                    const string& mobName = mobTypes[mob.mobType].name;
                    isPigman = (mobName.find("Pigman") != string::npos || mobName.find("pigman") != string::npos);
                }
                if (useDemoAlgorithm && isPigman && !mapWasRedrawn && screenRow == mob.prevRenderRow && screenCol == mob.prevRenderCol) {
                    continue;
                }
                setCursorPosition(screenCol, mapStartLine + screenRow);
                setTextColor(11);
                char mobChar = mobTypes[mob.mobType].symbol;
                if (gameTime - mob.lastHitTime < 0.18) setTextColor(12);
                if (useDemoAlgorithm && (mobTypes[mob.mobType].name.find("Pigman") != string::npos || mobTypes[mob.mobType].name.find("pigman") != string::npos)) {
                    mobChar = 'p';
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
                if (line.find("HP:") != string::npos) {
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
                spawnRow = stoi(line.substr(numPos, numEnd - numPos));
                spawnCol = (line.find("left") != string::npos) ? 1 : (line.find("right") != string::npos) ? GameMap::COLS - 2 : 50;
            }
        } else if (line.find("column") != string::npos) {
            size_t colPos = line.find("column");
            numPos = colPos + 6;
            while (numPos < line.length() && !isdigit(line[numPos])) numPos++;
            if (numPos < line.length()) {
                size_t numEnd = numPos;
                while (numEnd < line.length() && isdigit(line[numEnd])) numEnd++;
                spawnCol = stoi(line.substr(numPos, numEnd - numPos));
                spawnRow = (line.find("top") != string::npos) ? 1 : (line.find("down") != string::npos) ? GameMap::ROWS - 2 : 16;
            }
        }
    }

    void applyMobModifier(MobInstance& mob, const MobModifier& modifier, double currentGameTime) {
        mob.modifier.speedMultiplier *= modifier.speedMultiplier;
        mob.modifier.hpMultiplier *= modifier.hpMultiplier;
        mob.modifier.goldMultiplier *= modifier.goldMultiplier;
        mob.modifier.damageMultiplier *= modifier.damageMultiplier;
        mob.modifier.armorBonus += modifier.armorBonus;
        mob.modifier.slowEffect += modifier.slowEffect;
        if (modifier.isSlowed) {
            mob.modifier.isSlowed = true;
            mob.modifier.slowedUntilTime = currentGameTime + modifier.slowDuration;
            if (modifier.slowDuration > 0) mob.modifier.slowDuration = modifier.slowDuration;
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

    bool damageToMob(MobInstance& mob, int damageAmount) {
        mob.health -= damageAmount;
        if (mob.health <= 0) {
            mob.health = 0;
            mob.isAlive = false;
            return true;
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
};

#endif