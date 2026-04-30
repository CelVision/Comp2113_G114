#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "GameData.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <conio.h>

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
        towers.push_back({'A', "Arrow Tower", 50, 100, 1.0, 5, 
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
        // Clear existing path
        for (int i = 0; i < GameMap::ROWS; i++) {
            for (int j = 0; j < GameMap::COLS; j++) {
                if (gameMap.grid[i][j].type == PATH) {
                    gameMap.grid[i][j].type = BUILDABLE;
                    gameMap.grid[i][j].displayChar = '.';
                }
            }
        }
        
        // Simple example paths based on level
        // Path goes from left side to right side with curves
        
        if (level == 1) {
            // Straight path with a curve
            int row = 16;  // Middle row
            for (int col = 1; col < 98; col++) {
                gameMap.grid[row][col].type = PATH;
                gameMap.grid[row][col].displayChar = '-';
                pathCoordinates.push_back({row, col});
            }
        } else if (level <= 9) {
            // More complex winding path based on level
            // Example: Snake-like pattern
            vector<pair<int, int>> path = generateWavyPath(level);
            for (auto& coord : path) {
                gameMap.grid[coord.first][coord.second].type = PATH;
                gameMap.grid[coord.first][coord.second].displayChar = '-';
                pathCoordinates.push_back(coord);
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
                }
            }
        }
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
        
        // Check boundaries
        if (startRow < 1 || endRow >= GameMap::ROWS - 1 || 
            startCol < 1 || endCol >= GameMap::COLS - 1) {
            return false;
        }
        
        // Check each cell in 3x3 area
        for (int r = startRow; r <= endRow; r++) {
            for (int c = startCol; c <= endCol; c++) {
                TileType type = gameMap.grid[r][c].type;
                
                // Cannot place on path or base or already occupied towers
                if (type == PATH || type == BASE || type == TOWER) {
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
        
        // Place tower in 3x3 area (center gets the symbol, rest are blocked)
        for (int r = startRow; r <= endRow; r++) {
            for (int c = startCol; c <= endCol; c++) {
                gameMap.grid[r][c].type = TOWER;
                if (r == centerRow && c == centerCol) {
                    gameMap.grid[r][c].displayChar = towers[towerIndex].symbol;
                    gameMap.grid[r][c].towerIndex = towerIndex;
                } else {
                    gameMap.grid[r][c].displayChar = '#';  // Tower body
                    gameMap.grid[r][c].towerIndex = towerIndex;
                }
            }
        }
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
                
                if (inPreview && flash) {
                    // Flash preview in yellow
                    setTextColor(COLOR_YELLOW);
                    if (i == selRow && j == selCol) {
                        cout << towers[previewTowerIndex].symbol;
                    } else {
                        cout << "█";
                    }
                    resetTextColor();
                } else if (inSelection) {
                    // Show selection bracket
                    setTextColor(COLOR_CYAN);
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
};

#endif // GAMEMAP_H
