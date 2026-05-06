#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "GameData.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <map>
#include <queue>
#include <algorithm>
#include <climits>

using namespace std;

// ============ CONSOLE COLOR UTILITIES ============

// Windows console color codes
const int COLOR_YELLOW = 14;
const int COLOR_GREEN = 10;
const int COLOR_RED = 12;
const int COLOR_CYAN = 11;
const int COLOR_WHITE = 15;
const int COLOR_GRAY = 8;
const int COLOR_BLACK = 0;

void setTextColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetTextColor() {
    setTextColor(COLOR_WHITE);
}

// ============ GAME MAP MANAGEMENT ============

class GameMapManager {
private:
    GameMap gameMap;
    vector<Tower> towers;
    vector<Mob> mobs;
    
public:
    GameMapManager() {
        initializeEmptyMap();
        loadTowerData();
        loadMobData();
    }
    
    void initializeEmptyMap() {
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                gameMap.grid[i][j].type = BLOCKED;
                gameMap.grid[i][j].displayChar = ' ';
                gameMap.grid[i][j].towerIndex = -1;
                gameMap.grid[i][j].mobIndex = -1;
            }
        }
    }
    
    void loadTowerData() {
        towers.clear();
        
        // Arrow Tower
        towers.push_back({'A', "Arrow Tower", 50, 100, 1.0, 4, 
                         {"<->", "| |", "==="}, "Basic tower, single-target", 0, 0, 0, 0, 0, false});
        
        // Laser Tower
        towers.push_back({'L', "Laser Tower", 100, 50, 1.5, 999, 
                         {"<o>", "| |", "^^^"}, "Row attack tower", 0, 0, 0, 0, 0, false});
        
        // Frost Tower
        towers.push_back({'F', "Frost Tower", 50, 10, 1.0, 5, 
                         {"/*\\", "| |", "^^^"}, "Crowd control, slows enemies", 50, 0, 0, 0, 0, false});
        
        // Earthquake Tower
        towers.push_back({'E', "Earthquake Tower", 200, 100, 1.0, 5, 
                         {"<!>", "| |", "^^^"}, "Multi-target in circle", 0, 0, 0, 0, 0, false});
        
        // Hell Tower
        towers.push_back({'H', "Hell Tower", 200, 50, 1.0, 5, 
                         {"<%>", "| |", "==="}, "Percentage damage, targets high-hp", 0, 50, 0, 0, 0, false});
        
        // Thief Tower
        towers.push_back({'T', "Thief Tower", 200, 0, 0, 5, 
                         {"<$>", "| |", "..."}, "Increases currency gained", 0, 0, 25, 0, 0, false});
        
        // Armor Penetration Tower
        towers.push_back({'P', "Armor Penetration Tower", 150, 50, 1.0, 5, 
                         {"<^>", "| |", "==="}, "Breaks enemy armor", 0, 0, 0, 0, 0, true});
        
        // War Drum Tower
        towers.push_back({'D', "War Drum Tower", 150, 0, 0, 5, 
                         {"[ ]", "| |", "..."}, "Buffs towers within range", 0, 0, 0, 25, 0, false});
        
        // Vampire Tower
        towers.push_back({'V', "Vampire Tower", 400, 100, 1.0, 5, 
                         {"<+>", "| |", "==="}, "Heals base camp from damage", 0, 0, 0, 0, 5, false});
    }
    
    void loadMobData() {
        mobs.clear();
        
        mobs.push_back({'p', "Pigman", 200, 0, 2.0, "Common enemy", false, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'h', "Houndling", 150, 0, 4.0, "Fast common enemy", false, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'W', "Werewolf", 350, 100, 3.0, "Fast with medium hp", false, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'m', "Mini Mammon", 500, 0, 1.5, "Slow high-hp enemy", false, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'M', "Armored Mammon", 1000, 500, 1.0, "High hp and armor", false, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'^', "Birdman", 150, 0, 2.0, "Flying enemy", true, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'\'', "Batman", 50, 0, 5.0, "Fast flying enemy", true, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'P', "Pigman Berserker", 400, 400, 2.0, "Speeds up when armor breaks", false, 0, 0, 0, 0, 0, 0, 0});
        mobs.push_back({'S', "Spiderman", 300, 0, 3.0, "Slows towers", false, 50, 9, 5.0, 0, 0, 0, 0});
        mobs.push_back({'A', "Alpha Wolf", 500, 200, 1.5, "Summons houndlings", false, 0, 0, 0, 5, 2, 0, 0});
        mobs.push_back({'^', "Dragon Type 1", 2000, 0, 1.5, "Boss with area damage", false, 0, 9, 0, 0, 0, 9, 15});
        mobs.push_back({'D', "Dragon Type 2", 2000, 1000, 1.0, "Boss with armor and slow", false, 50, 999, 5.0, 0, 0, 0, 15});
        mobs.push_back({'!', "Dragon Type 3", 1000, 0, 2.0, "Fast boss", false, 0, 0, 0, 0, 0, 0, 0});
    }
    
    void createPathForLevel(int level, vector<pair<int, int>>& pathCoordinates) {
        // Clear existing non-spawn PATHs (preserve spawn points)
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                if (gameMap.grid[i][j].type == PATH && gameMap.grid[i][j].displayChar != '+') {
                    gameMap.grid[i][j].type = BUILDABLE;
                    gameMap.grid[i][j].displayChar = '.';
                }
            }
        }
        
        // Create a path from spawn points (rows 1-3, col 0) to base (row 21, col 31)
        // Strategy: go down/up to row 21, then go right to column 31
        
        if (level == 1) {
            // For level 1: Start at spawn row (use row 2 as centerline), go right then down to base
            int pathRow = 2;  // Middle spawn row
            
            // Path 1: Horizontal from column 1 to near base column, then down to base
            // Go right from spawn (column 0) to column 20
            for (int col = 1; col <= 20; col++) {
                if (gameMap.grid[pathRow][col].displayChar != '+') {
                    gameMap.grid[pathRow][col].type = PATH;
                    gameMap.grid[pathRow][col].displayChar = '-';
                    pathCoordinates.push_back({pathRow, col});
                }
            }
            
            // Go down from row 2 to row 21 (at column 20)
            for (int row = pathRow + 1; row <= 21; row++) {
                if (gameMap.grid[row][20].displayChar != 'M') {  // Don't overwrite base
                    gameMap.grid[row][20].type = PATH;
                    gameMap.grid[row][20].displayChar = '-';
                    pathCoordinates.push_back({row, 20});
                }
            }
            
            // Go right from column 20 to base column 31 (at row 21)
            for (int col = 21; col <= 31; col++) {
                if (gameMap.grid[21][col].displayChar != 'M' && gameMap.grid[21][col].displayChar != '#') {
                    gameMap.grid[21][col].type = PATH;
                    gameMap.grid[21][col].displayChar = '-';
                    pathCoordinates.push_back({21, col});
                }
            }
            
            // Create alternate paths from rows 1 and 3 down to the main row 2 path
            // Row 1 down to row 2
            for (int row = 1; row < 2; row++) {
                if (gameMap.grid[row][1].displayChar != '+') {
                    gameMap.grid[row][1].type = PATH;
                    gameMap.grid[row][1].displayChar = '-';
                    pathCoordinates.push_back({row, 1});
                }
            }
            // Row 3 down to row 2
            for (int row = 3; row < 21; row++) {
                if (gameMap.grid[row][1].displayChar != '+') {
                    gameMap.grid[row][1].type = PATH;
                    gameMap.grid[row][1].displayChar = '-';
                    pathCoordinates.push_back({row, 1});
                }
                if (row > 2) break;  // Only do next row
            }
            
        } else if (level <= 9) {
            // For other levels: Create winding path
            vector<pair<int, int>> path = generateWavyPath(level);
            for (auto& coord : path) {
                if (gameMap.grid[coord.first][coord.second].displayChar != '+' && 
                    gameMap.grid[coord.first][coord.second].displayChar != 'M') {
                    gameMap.grid[coord.first][coord.second].type = PATH;
                    gameMap.grid[coord.first][coord.second].displayChar = '-';
                    pathCoordinates.push_back(coord);
                }
            }
        }
        
        gameMap.pathCoordinates = pathCoordinates;
    }
    
    vector<pair<int, int>> generateWavyPath(int level) {
        vector<pair<int, int>> path;
        
        // Generate a wavy/winding path based on level difficulty
        int startRow = 8 + (level % 3) * 3;
        int col = 1;
        int row = startRow;
        int direction = 1;  // 1 = down, -1 = up
        int segmentLength = 5 + level;
        int segmentCount = 0;
        
        while (col < 98) {
            path.push_back({row, col});
            col++;
            segmentCount++;
            
            if (segmentCount >= segmentLength) {
                direction *= -1;  // Change direction
                segmentCount = 0;
            }
            
            row += direction;
            
            // Keep row within bounds
            if (row < 2) row = 2;
            if (row > GameMap::ROWS - 4) row = GameMap::ROWS - 4;
        }
        
        return path;
    }
    
    void initializeBuildableAreas() {
        for (int i = 1; i < GameMap::ROWS - 1; i++) {
            for (int j = 1; j < GameMap::COLS - 1; j++) {
                if (gameMap.grid[i][j].type != PATH && gameMap.grid[i][j].type != BASE) {
                    gameMap.grid[i][j].type = BUILDABLE;
                    gameMap.grid[i][j].displayChar = '.';
                    gameMap.grid[i][j].towerIndex = -1;
                    gameMap.grid[i][j].towerPosRow = -1;
                    gameMap.grid[i][j].towerPosCol = -1;
                }
            }
        }
    }
    
    void loadMapFromFile(int level) {
        // Initialize all tiles as BLOCKED
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                gameMap.grid[i][j].type = BLOCKED;
                gameMap.grid[i][j].displayChar = ' ';
                gameMap.grid[i][j].towerIndex = -1;
                gameMap.grid[i][j].towerPosRow = -1;
                gameMap.grid[i][j].towerPosCol = -1;
                gameMap.grid[i][j].mobIndex = -1;
            }
        }
        
        // Read map file
        string filename = buildMapFilePath(level);
        ifstream mapFile(filename);
        
        if (!mapFile.is_open()) {
            cout << "Error: Could not load map file " << filename << endl;
            return;
        }
        
        string line;
        int row = 0;
        int spawnCharCount = 0, baseCharCount = 0, pathCharCount = 0;
        
        while (getline(mapFile, line) && row < GameMap::ROWS) {
            for (int col = 0; col < (int)line.length() && col < GameMap::COLS; col++) {
                char ch = line[col];
                
                if (ch == '.') {
                    // Buildable area
                    gameMap.grid[row][col].type = BUILDABLE;
                    gameMap.grid[row][col].displayChar = '.';
                } else if (ch == '+') {
                    // Spawn point (road/path)
                    gameMap.grid[row][col].type = PATH;
                    gameMap.grid[row][col].displayChar = '+';
                    spawnCharCount++;
                    cerr << "DEBUG loadMapFromFile: Found '+' at (" << row << ", " << col << ")" << endl;
                } else if (ch == '-') {
                    // Road path
                    gameMap.grid[row][col].type = PATH;
                    gameMap.grid[row][col].displayChar = '-';
                    pathCharCount++;
                } else if (ch == 'M') {
                    // Base camp
                    gameMap.grid[row][col].type = BASE;
                    gameMap.grid[row][col].displayChar = 'M';
                    baseCharCount++;
                    cerr << "DEBUG loadMapFromFile: Found 'M' at (" << row << ", " << col << ")" << endl;
                } else if (ch == '#') {
                    // Blocked terrain
                    gameMap.grid[row][col].type = BLOCKED;
                    gameMap.grid[row][col].displayChar = '#';
                } else if (ch == ' ') {
                    // Space = walkable path
                    gameMap.grid[row][col].type = PATH;
                    gameMap.grid[row][col].displayChar = ' ';
                } else {
                    // Unknown characters default to blocked
                    gameMap.grid[row][col].type = BLOCKED;
                    gameMap.grid[row][col].displayChar = ' ';
                }
            }
            // Fill remaining columns as BLOCKED if line is shorter
            for (int col = line.length(); col < GameMap::COLS; col++) {
                gameMap.grid[row][col].type = BLOCKED;
                gameMap.grid[row][col].displayChar = ' ';
            }
            row++;
        }
        
        cerr << "DEBUG loadMapFromFile: Loaded map from " << filename << " - Found " << spawnCharCount << " '+', " << baseCharCount << " 'M', " << pathCharCount << " '-'" << endl;
        mapFile.close();
    }
    
    // Load map data into backend 2D array for game logic processing
    // Array mapping:
    //   '+' = spawn point
    //   '.' = tower buildable grid
    //   'B' = base position
    //   ' ' = road/walkable path (empty)
    //   (empty) = blocked terrain
    void loadMapToArray(int level, char stateArray[33][147]) {
        // Initialize array with empty spaces
        for (int i = 0; i < 33; i++) {
            for (int j = 0; j < 147; j++) {
                stateArray[i][j] = ' ';  // Default to empty
            }
        }
        
        // Read map file
        string filename = buildMapFilePath(level);
        ifstream mapFile(filename);
        
        if (!mapFile.is_open()) {
            cout << "Error: Could not load map file " << filename << endl;
            return;
        }
        
        string line;
        int row = 0;
        
        while (getline(mapFile, line) && row < 33) {
            for (int col = 0; col < (int)line.length() && col < 147; col++) {
                char ch = line[col];
                
                if (ch == '+') {
                    // Spawn point
                    stateArray[row][col] = '+';
                } else if (ch == '.') {
                    // Tower buildable grid
                    stateArray[row][col] = '.';
                } else if (ch == '-') {
                    // Road/path - leave as empty space
                    stateArray[row][col] = ' ';
                } else if (ch == 'M') {
                    // Base camp - convert to 'B'
                    stateArray[row][col] = 'B';
                } else if (ch == '#') {
                    // Blocked terrain - leave as empty
                    stateArray[row][col] = ' ';
                } else {
                    // Everything else (space, etc.) - leave empty
                    stateArray[row][col] = ' ';
                }
            }
            row++;
        }
        
        mapFile.close();
    }
    
    void setBaseCamp(int row, int col) {
        gameMap.grid[row][col].type = BASE;
        gameMap.grid[row][col].displayChar = 'B';
    }
    
    void renderGameMap(int selectedCol = -1, int selectedRow = -1) {
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                Tile& tile = gameMap.grid[i][j];
                
                // Check if this is the selected buildable position
                bool isSelected = (i == selectedRow && j == selectedCol);
                
                switch (tile.type) {
                    case PATH:
                        setTextColor(COLOR_YELLOW);
                        cout << tile.displayChar;
                        resetTextColor();
                        break;
                    case BUILDABLE:
                        if (isSelected) {
                            setTextColor(COLOR_CYAN);
                            cout << "█";  // Highlight selected area
                            resetTextColor();
                        } else {
                            setTextColor(COLOR_GRAY);
                            cout << tile.displayChar;
                            resetTextColor();
                        }
                        break;
                    case TOWER:
                        setTextColor(COLOR_WHITE);
                        cout << tile.displayChar;
                        resetTextColor();
                        break;
                    case BASE:
                        setTextColor(COLOR_RED);
                        cout << "B";
                        resetTextColor();
                        break;
                    case BLOCKED:
                        cout << " ";
                        break;
                }
            }
            cout << endl;
        }
    }
    
    bool canPlaceTower(int centerRow, int centerCol, int towerIndex) {
        // Tower is 3x3, centered at given position
        int startRow = centerRow - 1;
        int startCol = centerCol - 1;
        int endRow = centerRow + 1;
        int endCol = centerCol + 1;
        
        // Check boundaries (all cells must be within valid map range)
        if (startRow < 0 || endRow >= GameMap::ROWS || 
            startCol < 0 || endCol >= GameMap::COLS) {
            return false;
        }
        
        // Check each cell in 3x3 area
        for (int r = startRow; r <= endRow; r++) {
            for (int c = startCol; c <= endCol; c++) {
                TileType type = gameMap.grid[r][c].type;
                
                // Cannot place on path or base or already occupied towers or blocked terrain
                if (type == PATH || type == BASE || type == TOWER || type == BLOCKED) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    void placeTowerAt(int centerRow, int centerCol, int towerIndex) {
        if (!canPlaceTower(centerRow, centerCol, towerIndex)) {
            return;
        }
        
        int startRow = centerRow - 1;
        int startCol = centerCol - 1;
        int endRow = centerRow + 1;
        int endCol = centerCol + 1;
        
        // Place tower in 3x3 area with ASCII art
        for (int r = startRow; r <= endRow; r++) {
            for (int c = startCol; c <= endCol; c++) {
                gameMap.grid[r][c].type = TOWER;
                gameMap.grid[r][c].towerIndex = towerIndex;
                
                // Store position within 3x3 tower grid (0-2, 0-2)
                int posRow = r - startRow;  // 0, 1, or 2
                int posCol = c - startCol;  // 0, 1, or 2
                gameMap.grid[r][c].towerPosRow = posRow;
                gameMap.grid[r][c].towerPosCol = posCol;
                
                // Set display character from ASCII art
                if (posRow < 3 && posCol < (int)towers[towerIndex].art[posRow].length()) {
                    gameMap.grid[r][c].displayChar = towers[towerIndex].art[posRow][posCol];
                } else {
                    gameMap.grid[r][c].displayChar = ' ';
                }
            }
        }
    }
    
    bool isPathInSelection(int selRow, int selCol) {
        // Check if any tile in 3x3 selection contains a PATH tile
        for (int r = selRow - 1; r <= selRow + 1; r++) {
            for (int c = selCol - 1; c <= selCol + 1; c++) {
                if (gameMap.grid[r][c].type == PATH) {
                    return true;
                }
            }
        }
        return false;
    }
    
    void renderGameMapWithSelection(int selRow, int selCol, int previewTowerIndex = -1, bool flash = false) {
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                Tile& tile = gameMap.grid[i][j];
                
                // Check if in selection bracket (3x3 area)
                bool inSelection = (i >= selRow - 1 && i <= selRow + 1 && 
                                   j >= selCol - 1 && j <= selCol + 1);
                
                // Check if in preview area
                bool inPreview = inSelection && previewTowerIndex >= 0;
                
                // Check if selection is on a road
                bool pathInSelection = isPathInSelection(selRow, selCol);
                
                if (inPreview && flash) {
                    // Flash preview - red if on path, yellow if on buildable
                    if (pathInSelection) {
                        setTextColor(COLOR_RED);
                    } else {
                        setTextColor(COLOR_YELLOW);
                    }
                    if (i == selRow && j == selCol) {
                        cout << towers[previewTowerIndex].symbol;
                    } else {
                        cout << "█";
                    }
                    resetTextColor();
                } else if (inSelection) {
                    // Show selection bracket - red if on path, cyan if on buildable
                    if (pathInSelection) {
                        setTextColor(COLOR_RED);
                    } else {
                        setTextColor(COLOR_CYAN);
                    }
                    if ((i == selRow - 1 || i == selRow + 1) && 
                        (j >= selCol - 1 && j <= selCol + 1)) {
                        cout << "─";  // Top/bottom bracket
                    } else if ((j == selCol - 1 || j == selCol + 1) && 
                               (i >= selRow - 1 && i <= selRow + 1)) {
                        cout << "│";  // Left/right bracket
                    } else {
                        cout << " ";
                    }
                    resetTextColor();
                } else {
                    // Normal rendering
                    switch (tile.type) {
                        case PATH:
                            setTextColor(COLOR_YELLOW);
                            cout << tile.displayChar;
                            resetTextColor();
                            break;
                        case BUILDABLE:
                            setTextColor(COLOR_GRAY);
                            cout << tile.displayChar;
                            resetTextColor();
                            break;
                        case TOWER:
                            setTextColor(COLOR_WHITE);
                            cout << tile.displayChar;
                            resetTextColor();
                            break;
                        case BASE:
                            setTextColor(COLOR_RED);
                            cout << "B";
                            resetTextColor();
                            break;
                        case BLOCKED:
                            cout << " ";
                            break;
                    }
                }
            }
            cout << endl;
        }
    }
    
    Tower* getTower(int index) {
        if (index >= 0 && index < (int)towers.size()) {
            return &towers[index];
        }
        return nullptr;
    }
    
    Mob* getMob(int index) {
        if (index >= 0 && index < (int)mobs.size()) {
            return &mobs[index];
        }
        return nullptr;
    }
    
    vector<Tower>& getAllTowers() {
        return towers;
    }
    
    vector<Mob>& getAllMobs() {
        return mobs;
    }
    
    GameMap& getGameMap() {
        return gameMap;
    }

    // ============ BACKEND PATH SYSTEM (for game logic) ============
    
    // Structure to hold path commands generated from backend array
    struct PathCommand {
        pair<int, int> startPos;
        pair<int, int> endPos;
        string description;  // e.g., "go left 10 tiles"
    };

    // Spawn a mob at a spawn point by finding adjacent empty (walkable) space
    // Returns the spawn position (row, col) on success, or (-1, -1) if no space available
    pair<int, int> spawnMobAtSpawnPoint(const char stateArray[33][147], int spawnRow, int spawnCol) {
        // Check adjacent cells (up, down, left, right) for empty space (space character)
        int dr[] = {-1, 1, 0, 0};
        int dc[] = {0, 0, -1, 1};
        
        for (int i = 0; i < 4; i++) {
            int newRow = spawnRow + dr[i];
            int newCol = spawnCol + dc[i];
            
            // Check boundaries
            if (newRow < 0 || newRow >= 33 || newCol < 0 || newCol >= 147) {
                continue;
            }
            
            // Check if adjacent cell is empty (road/walkable path)
            if (stateArray[newRow][newCol] == ' ') {
                return make_pair(newRow, newCol);
            }
        }
        
        // No adjacent empty space found
        return make_pair(-1, -1);
    }

    // Load paths from backend array using Dijkstra's algorithm
    // Scans for all spawn points (+) and base (B), computes shortest paths
    // Returns a map of spawn point to path commands
    map<pair<int, int>, vector<PathCommand>> loadPathsFromBackendArray(const char stateArray[33][147]) {
        map<pair<int, int>, vector<PathCommand>> spawnToPaths;
        
        // Find base position
        pair<int, int> basePos = make_pair(-1, -1);
        for (int i = 0; i < 33; i++) {
            for (int j = 0; j < 147; j++) {
                if (stateArray[i][j] == 'B') {
                    basePos = make_pair(i, j);
                    break;
                }
            }
            if (basePos.first != -1) break;
        }
        
        if (basePos.first == -1) {
            cerr << "Error: No base found in backend array!" << endl;
            return spawnToPaths;
        }
        
        // Find all spawn points and compute shortest paths using Dijkstra
        for (int i = 0; i < 33; i++) {
            for (int j = 0; j < 147; j++) {
                if (stateArray[i][j] == '+') {
                    pair<int, int> spawnPos = make_pair(i, j);
                    
                    // Run Dijkstra from this spawn point to base
                    vector<pair<int, int>> path = dijkstraShortestPath(stateArray, spawnPos, basePos);
                    
                    if (!path.empty()) {
                        // Convert path to detailed commands
                        vector<PathCommand> commands = generatePathCommands(path);
                        spawnToPaths[spawnPos] = commands;
                    }
                }
            }
        }
        
        return spawnToPaths;
    }

    // Dijkstra's algorithm to find shortest path in backend array
    // Walkable tiles are: ' ' (road), 'B' (base)
    vector<pair<int, int>> dijkstraShortestPath(const char stateArray[33][147], 
                                                 pair<int, int> start, 
                                                 pair<int, int> goal) {
        // Distance and parent tracking
        vector<vector<int>> dist(33, vector<int>(147, INT_MAX));
        vector<vector<pair<int, int>>> parent(33, vector<pair<int, int>>(147, make_pair(-1, -1)));
        
        // Priority queue: (distance, row, col)
        priority_queue<pair<int, pair<int, int>>, 
                      vector<pair<int, pair<int, int>>>,
                      greater<pair<int, pair<int, int>>>> pq;
        
        dist[start.first][start.second] = 0;
        pq.push(make_pair(0, start));
        
        int dr[] = {-1, 1, 0, 0};
        int dc[] = {0, 0, -1, 1};
        
        while (!pq.empty()) {
            int d = pq.top().first;
            pair<int, int> current = pq.top().second;
            pq.pop();
            
            int row = current.first;
            int col = current.second;
            
            // If we reached the goal
            if (row == goal.first && col == goal.second) {
                break;
            }
            
            // Skip if we've already found a better path
            if (d > dist[row][col]) {
                continue;
            }
            
            // Explore neighbors
            for (int i = 0; i < 4; i++) {
                int newRow = row + dr[i];
                int newCol = col + dc[i];
                
                // Check boundaries
                if (newRow < 0 || newRow >= 33 || newCol < 0 || newCol >= 147) {
                    continue;
                }
                
                // Check if walkable (space or base)
                char cell = stateArray[newRow][newCol];
                if (cell != ' ' && cell != 'B') {
                    continue;
                }
                
                int newDist = d + 1;
                if (newDist < dist[newRow][newCol]) {
                    dist[newRow][newCol] = newDist;
                    parent[newRow][newCol] = make_pair(row, col);
                    pq.push(make_pair(newDist, make_pair(newRow, newCol)));
                }
            }
        }
        
        // Reconstruct path from goal to start
        vector<pair<int, int>> path;
        if (dist[goal.first][goal.second] != INT_MAX) {
            pair<int, int> current = goal;
            while (current.first != -1 && current.second != -1) {
                path.push_back(current);
                current = parent[current.first][current.second];
            }
            reverse(path.begin(), path.end());
        }
        
        return path;
    }

    // Generate detailed path commands from a sequence of coordinates
    // e.g., [(0,5), (0,6), (0,7), (0,8), (1,8), (1,9)] -> 
    //       "go right 3 tiles, go down 1 tile, go right 1 tile"
    vector<PathCommand> generatePathCommands(const vector<pair<int, int>>& path) {
        vector<PathCommand> commands;
        
        if (path.size() < 2) return commands;
        
        int i = 0;
        while (i < (int)path.size() - 1) {
            pair<int, int> current = path[i];
            int direction = 0;  // 0=up, 1=down, 2=left, 3=right
            string dirName;
            int count = 1;
            
            // Determine direction
            int rowDelta = path[i + 1].first - current.first;
            int colDelta = path[i + 1].second - current.second;
            
            if (rowDelta == -1) {
                direction = 0;
                dirName = "up";
            } else if (rowDelta == 1) {
                direction = 1;
                dirName = "down";
            } else if (colDelta == -1) {
                direction = 2;
                dirName = "left";
            } else if (colDelta == 1) {
                direction = 3;
                dirName = "right";
            } else {
                i++;
                continue;
            }
            
            // Count consecutive steps in same direction
            pair<int, int> commandStart = current;
            i++;
            while (i < (int)path.size() - 1) {
                rowDelta = path[i + 1].first - path[i].first;
                colDelta = path[i + 1].second - path[i].second;
                
                if ((direction == 0 && rowDelta == -1 && colDelta == 0) ||
                    (direction == 1 && rowDelta == 1 && colDelta == 0) ||
                    (direction == 2 && rowDelta == 0 && colDelta == -1) ||
                    (direction == 3 && rowDelta == 0 && colDelta == 1)) {
                    count++;
                    i++;
                } else {
                    break;
                }
            }
            
            // Create command
            PathCommand cmd;
            cmd.startPos = commandStart;
            cmd.endPos = path[i];
            cmd.description = "go " + dirName + " " + to_string(count) + " tile" + (count > 1 ? "s" : "");
            commands.push_back(cmd);
        }
        
        return commands;
    }

    // ============ REAL-TIME TOWER BACKEND UPDATES ============
    
    // Update backend array when tower is placed
    // Marks entire 3x3 area as 'T' for tower
    void updateBackendArrayTowerPlaced(char stateArray[33][147], int centerRow, int centerCol) {
        int startRow = centerRow - 1;
        int startCol = centerCol - 1;
        int endRow = centerRow + 1;
        int endCol = centerCol + 1;
        
        // Mark entire 3x3 area as tower ('T')
        for (int i = startRow; i <= endRow; i++) {
            for (int j = startCol; j <= endCol; j++) {
                if (i >= 0 && i < 33 && j >= 0 && j < 147) {
                    stateArray[i][j] = 'T';
                }
            }
        }
    }
    
    // Update backend array when tower is sold
    // Restores entire 3x3 area back to '.' (buildable)
    void updateBackendArrayTowerSold(char stateArray[33][147], int centerRow, int centerCol) {
        int startRow = centerRow - 1;
        int startCol = centerCol - 1;
        int endRow = centerRow + 1;
        int endCol = centerCol + 1;
        
        // Restore entire 3x3 area to buildable ('.')
        for (int i = startRow; i <= endRow; i++) {
            for (int j = startCol; j <= endCol; j++) {
                if (i >= 0 && i < 33 && j >= 0 && j < 147) {
                    stateArray[i][j] = '.';
                }
            }
        }
    }
};

#endif // GAMEMAP_H
