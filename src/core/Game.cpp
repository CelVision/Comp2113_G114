#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <conio.h>
#include <windows.h>
#include "GameData.h"
#include "MobSystem.h"
#include "TowerLogic.h"

// Note: GameMap.cpp is included as a header-like implementation file
#include "GameMap.cpp"

using namespace std;

// Constants
const int MAP_COLS = 147;
const int LEVEL_COLS = 5;
const int LEVEL_ROWS = 2;

// Function to clear screen
void clearScreen() {
    system("cls");
}

// Function to set cursor position
void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordinates;
    coordinates.X = x;
    coordinates.Y = y;
    SetConsoleCursorPosition(hConsole, coordinates);
}

// Function to clear specific screen area
void clearMapArea() {
    // Only clear the game map area, not the entire screen
    for (int i = 1; i < 30; i++) {
        setCursorPosition(0, i);
        cout << string(MAP_COLS, ' ');
    }
}

// Function to hide/show cursor
void showCursor(bool show) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = show;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Function to set console window size to fit the game map
void setConsoleSize() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Set buffer size to accommodate map (147 columns + margin, 33 rows + UI)
    COORD bufferSize = {158, 50};
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    
    // Set window size (slightly smaller than buffer to show scrollbars if needed)
    SMALL_RECT windowRect = {0, 0, 157, 49};  // 158x50 window (0-indexed, so 157x49)
    SetConsoleWindowInfo(hConsole, TRUE, &windowRect);
}

// Helper: Check which towers overlap with 3x3 selection
vector<int> getOverlappingTowers(GameMap& gameMap, int selRow, int selCol) {
    vector<int> overlappingTowers;
    set<int> uniqueTowers;  // Use set to avoid duplicates
    
    for (int i = selRow - 1; i <= selRow + 1; i++) {
        for (int j = selCol - 1; j <= selCol + 1; j++) {
            if (i >= 0 && i < GameMap::ROWS && j >= 0 && j < GameMap::COLS) {
                Tile& tile = gameMap.grid[i][j];
                if (tile.type == TOWER && tile.towerIndex >= 0) {
                    uniqueTowers.insert(tile.towerIndex);
                }
            }
        }
    }
    
    for (int idx : uniqueTowers) {
        overlappingTowers.push_back(idx);
    }
    return overlappingTowers;
}

// Helper: Check if 3x3 placement area is valid (no road/path tiles)
bool isValidPlacementLocation(GameMap& gameMap, int selRow, int selCol) {
    int startRow = selRow - 1;
    int startCol = selCol - 1;
    int endRow = selRow + 1;
    int endCol = selCol + 1;
    
    // Check boundaries (all cells must be within valid map range)
    if (startRow < 0 || endRow >= GameMap::ROWS || 
        startCol < 0 || endCol >= GameMap::COLS) {
        return false;
    }
    
    // Check each cell in 3x3 area - cannot place on PATH, BASE, TOWER, or BLOCKED
    for (int r = startRow; r <= endRow; r++) {
        for (int c = startCol; c <= endCol; c++) {
            TileType type = gameMap.grid[r][c].type;
            if (type == PATH || type == BASE || type == TOWER || type == BLOCKED) {
                return false;
            }
        }
    }
    return true;
}

// Helper: Check whether a level is unlocked for the current player
bool isLevelUnlockedForPlayer(int levelNum, const string& playerName, int maxUnlockedLevel) {
    if (playerName == "test") {
        return true;
    }
    return levelNum <= maxUnlockedLevel;
}

// Helper: Number of towers unlocked at a given level
int getUnlockedTowerCountForLevel(int levelNum, int towerCount) {
    if (levelNum <= 0 || towerCount <= 0) {
        return 0;
    }

    int unlocked = levelNum;
    if (levelNum >= 9) {
        unlocked = 9;
    }
    if (unlocked > towerCount) {
        unlocked = towerCount;
    }
    return unlocked;
}

// Helper: Check if selection fully covers one tower (all 9 cells belong to same tower)
bool checkFullTowerCoverage(GameMap& gameMap, int selRow, int selCol, int& outTowerIndex) {
    outTowerIndex = -1;
    int firstTowerIdx = -1;
    int cellCount = 0;
    int towerCellsInSelection = 0;
    
    // Count tower cells and check if all are same tower
    for (int i = selRow - 1; i <= selRow + 1; i++) {
        for (int j = selCol - 1; j <= selCol + 1; j++) {
            if (i >= 0 && i < GameMap::ROWS && j >= 0 && j < GameMap::COLS) {
                Tile& tile = gameMap.grid[i][j];
                cellCount++;
                
                if (tile.type == TOWER && tile.towerIndex >= 0) {
                    towerCellsInSelection++;
                    if (firstTowerIdx == -1) {
                        firstTowerIdx = tile.towerIndex;
                    } else if (tile.towerIndex != firstTowerIdx) {
                        return false;  // Multiple towers - not full coverage
                    }
                }
            }
        }
    }
    
    // Check if all 9 cells belong to one tower
    if (towerCellsInSelection == 9 && firstTowerIdx >= 0) {
        outTowerIndex = firstTowerIdx;
        return true;
    }
    
    return false;
}

// Display BYTE RUSH logo
void displayLogo() {
    cout << "\n\n";
    cout << "  ██████╗ ██╗   ██╗████████╗███████╗" << endl;
    cout << "  ██╔══██╗╚██╗ ██╔╝╚══██╔══╝██╔════╝" << endl;
    cout << "  ██████╔╝ ╚████╔╝    ██║   █████╗  " << endl;
    cout << "  ██╔══██╗  ╚██╔╝     ██║   ██╔══╝  " << endl;
    cout << "  ██████╔╝   ██║      ██║   ███████╗" << endl;
    cout << "  ╚═════╝    ╚═╝      ╚═╝   ╚══════╝" << endl;
    cout << "\n";
    cout << "  ██████╗ ██╗   ██╗███████╗██╗  ██╗" << endl;
    cout << "  ██╔══██╗██║   ██║██╔════╝██║  ██║" << endl;
    cout << "  ██████╔╝██║   ██║███████╗███████║" << endl;
    cout << "  ██╔══██╗██║   ██║╚════██║██╔══██║" << endl;
    cout << "  ██║  ██║╚██████╔╝███████║██║  ██║" << endl;
    cout << "  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝" << endl;
    cout << "\n";
}

// Display starting screen
string displayStartScreen() {
    clearScreen();
    
    displayLogo();
    
    cout << string(33, ' ') << "Welcome to BYTE RUSH!" << endl;
    cout << string(30, ' ') << "A Tower Defense Adventure" << endl;
    cout << string(30, ' ') << "Make Sure Your Console is Full Screen" << endl;
    cout << "\n\n";
    
    cout << string(35, ' ') << "Enter your name:" << endl;
    cout << string(35, ' ') << "> ";
    
    string playerName;
    getline(cin, playerName);
    
    if (playerName.empty()) {
        playerName = "Player";
    }
    
    return playerName;
}

// Display level selection screen
int displayLevelSelect(const string& playerName, int maxUnlockedLevel) {
    int selectedCol = 0;
    int selectedRow = 0;
    string statusMessage;
    
    while (true) {
        clearScreen();
        
        cout << "\n\n";
        cout << string(30, ' ') << "SELECT YOUR LEVEL" << endl;
        cout << string(20, ' ') << string(59, '=') << endl;
        cout << "\n";
        
        // Display level grid (5 columns x 2 rows)
        for (int row = 0; row < LEVEL_ROWS; row++) {
            cout << "\n\n";
            cout << string(10, ' ');
            
            for (int col = 0; col < LEVEL_COLS; col++) {
                int levelNum = row * LEVEL_COLS + col + 1;
                bool unlocked = isLevelUnlockedForPlayer(levelNum, playerName, maxUnlockedLevel);
                
                if (selectedRow == row && selectedCol == col) {
                    setTextColor(COLOR_GREEN);
                } else if (!unlocked) {
                    setTextColor(COLOR_RED);
                }
                cout << "  ┌─────┐  ";
                if ((selectedRow == row && selectedCol == col) || !unlocked) {
                    resetTextColor();
                }
            }
            cout << "\n" << string(10, ' ');
            
            for (int col = 0; col < LEVEL_COLS; col++) {
                int levelNum = row * LEVEL_COLS + col + 1;
                bool unlocked = isLevelUnlockedForPlayer(levelNum, playerName, maxUnlockedLevel);
                
                if (selectedRow == row && selectedCol == col) {
                    setTextColor(COLOR_GREEN);
                } else if (!unlocked) {
                    setTextColor(COLOR_RED);
                }
                if (unlocked && levelNum <= 9) {
                    cout << "  │ " << levelNum << "   │  ";
                } else if (unlocked && levelNum == 10) {
                    cout << "  │  ∞  │  ";
                } else {
                    cout << "  │LOCK │  ";
                }
                if ((selectedRow == row && selectedCol == col) || !unlocked) {
                    resetTextColor();
                }
            }
            cout << "\n" << string(10, ' ');
            
            for (int col = 0; col < LEVEL_COLS; col++) {
                int levelNum = row * LEVEL_COLS + col + 1;
                bool unlocked = isLevelUnlockedForPlayer(levelNum, playerName, maxUnlockedLevel);
                
                if (selectedRow == row && selectedCol == col) {
                    setTextColor(COLOR_GREEN);
                } else if (!unlocked) {
                    setTextColor(COLOR_RED);
                }
                cout << "  └─────┘  ";
                if ((selectedRow == row && selectedCol == col) || !unlocked) {
                    resetTextColor();
                }
            }
        }
        
        cout << "\n\n";
        cout << string(20, ' ') << "Use ← → ↑ ↓ to navigate, ENTER to select" << endl;
        if (!statusMessage.empty()) {
            setTextColor(COLOR_RED);
            cout << string(20, ' ') << statusMessage << endl;
            resetTextColor();
        }
        
        // Display selection indicator
        cout << "\n" << string(20, ' ') << "Selected: Level ";
        setTextColor(COLOR_GREEN);
        int selectedLevel = selectedRow * LEVEL_COLS + selectedCol + 1;
        if (selectedLevel <= 9) {
            cout << selectedLevel;
        } else {
            cout << "∞ (Infinite)";
        }
        resetTextColor();
        cout << endl;
        
        // Handle input
        int key = _getch();
        
        if (key == 13) { // Enter key
            if (isLevelUnlockedForPlayer(selectedLevel, playerName, maxUnlockedLevel)) {
                return selectedLevel;
            }
            statusMessage = "Level " + to_string(selectedLevel) + " is locked. Clear level 1 first.";
        } else if (key == 224) { // Extended keys (arrow keys)
            int extKey = _getch();
            
            if (extKey == 72) { // Up arrow
                if (selectedRow > 0) {
                    selectedRow--;
                }
            } else if (extKey == 80) { // Down arrow
                if (selectedRow < LEVEL_ROWS - 1) {
                    selectedRow++;
                }
            } else if (extKey == 75) { // Left arrow
                if (selectedCol > 0) {
                    selectedCol--;
                }
            } else if (extKey == 77) { // Right arrow
                if (selectedCol < LEVEL_COLS - 1) {
                    selectedCol++;
                }
            }
        }
    }
}

// Display game screen with interactive tower placement
bool displayGameScreen(string playerName, int levelSelected) {
    clearScreen();
    
    // Initialize game map manager
    GameMapManager mapManager;
    
    // Load map from file for the selected level
    mapManager.loadMapFromFile(levelSelected);
    
    // Use existing map spaces - don't create artificial paths
    vector<pair<int, int>> pathCoords;
    
    // Initialize backend array for game logic processing
    char gameStateArray[33][147];
    mapManager.loadMapToArray(levelSelected, gameStateArray);
    
    // Note: mob system will be initialized after basic player state (money, HP)
    
    // Tower placement mode
    int selRow = 16;  // Start at middle
    int selCol = 50;
    int previewTowerIndex = -1;
    bool showFlash = false;
    int flashCounter = 0;
    bool demoMode = (playerName == "demo");
    int money = demoMode ? 500 : 250;
    int baseHP = 10;

    // Initialize mob system (needs money/baseHP refs)
    MobSystemManager mobSystem(mapManager.getAllMobs(), mapManager.getGameMap(), money, baseHP, demoMode);
    mobSystem.loadLevelDesign(levelSelected);
    if (!demoMode) {
        mobSystem.findGlobalPath();
    }
    
    // Load backend paths for mob movement (using previously declared gameStateArray)
    mapManager.loadMapToArray(levelSelected, gameStateArray);
    auto backendPathMap = mapManager.loadPathsFromBackendArray(gameStateArray);
    
    // Convert backend path format to simple coordinate pairs
    map<pair<int, int>, vector<pair<pair<int,int>, pair<int,int>>>> pathData;
    for (const auto& entry : backendPathMap) {
        auto spawnCoord = entry.first;
        const auto& pathCommands = entry.second;
        vector<pair<pair<int,int>, pair<int,int>>> cmdPairs;
        for (const auto& cmd : pathCommands) {
            cmdPairs.push_back({cmd.startPos, cmd.endPos});
        }
        pathData[spawnCoord] = cmdPairs;
    }
    mobSystem.loadBackendPaths(pathData);

    // DEBUG: print navigation routes for verification
    {
        auto routes = mobSystem.getNavigationRoutes();
        cerr << "DEBUG: Navigation routes count=" << routes.size() << endl;
        for (size_t ri = 0; ri < routes.size(); ++ri) {
            cerr << "Route " << ri << " spawn=(" << routes[ri].spawnRow << "," << routes[ri].spawnCol << ") nodes=";
            for (const auto &n : routes[ri].nodes) cerr << "(" << n.first << "," << n.second << ")";
            cerr << " checkpointIndex=" << routes[ri].checkpointNodeIndex << "\n";
        }
    }

    // Also print global path for debugging
    {
        // Accessing private globalPath via a temporary helper call would be cleaner,
        // but use findGlobalPath() and then read first navigation route as fallback.
        // Print note: globalPath is internal; we report navigationRoutes[0] if exists.
        auto routes2 = mobSystem.getNavigationRoutes();
        if (!routes2.empty()) {
            cerr << "DEBUG: Using navigationRoutes[0] as globalPath, size=" << routes2[0].nodes.size() << "\n";
        } else {
            cerr << "DEBUG: No navigation routes available to represent globalPath" << endl;
        }
    }

    if (demoMode && levelSelected == 1) {
        const vector<pair<int, int>> demoTowerSpots = {
            {5, 10}, {5, 20}, {5, 30}, {5, 40}
        };
        // Place the first four tower types at demo spots for quick verification
        for (size_t ti = 0; ti < demoTowerSpots.size() && ti < 4; ++ti) {
            const auto& spot = demoTowerSpots[ti];
            int towerIndex = (int)ti; // 0..3 -> Arrow, Laser, Frost, Earthquake
            if (mapManager.canPlaceTower(spot.first, spot.second, towerIndex)) {
                mapManager.placeTowerAt(spot.first, spot.second, towerIndex);
                // Update backend array when demo tower is placed
                mapManager.updateBackendArrayTowerPlaced(gameStateArray, spot.first, spot.second);
            }
        }
    }

    int unlockedTowerCount = getUnlockedTowerCountForLevel(levelSelected, (int)mapManager.getAllTowers().size());
    
    bool placingTowers = true;

    // Tower sell system variables
    int sellModeState = 0;  // 0: no sell, 1: showing price, 2: confirmed
    int sellTowerIndex = -1;
    int sellTowerPrice = 0;
    
    // Error message display
    string errorMessage = "";
    int errorMessageDuration = 0;  // Frames to display error
    
    // Hide cursor for better appearance
    showCursor(false);
    
    // Draw static UI once
    setCursorPosition(0, 0);
    cout << "\n";
    cout << string(20, '=') << " BYTE RUSH - Level " << levelSelected << " " << string(20, '=') << endl;
    cout << string(75, '=') << endl;
    cout << "\n";
    int mapStartLine = 5;  // Line where map starts
    
    auto lastFrameTime = chrono::steady_clock::now();
    
    while (placingTowers) {
        // Use real elapsed time so speed value means tiles per second.
        auto now = chrono::steady_clock::now();
        double frameTime = chrono::duration<double>(now - lastFrameTime).count();
        lastFrameTime = now;

        // Clamp to avoid giant jumps after pauses.
        if (frameTime < 0.0) frameTime = 0.0;
        if (frameTime > 0.1) frameTime = 0.1;

        GameMap& gameMap = mapManager.getGameMap();
        vector<Tower>& towers = mapManager.getAllTowers();

        mobSystem.update(frameTime, towers);
        
        // Update error message timer
        if (errorMessageDuration > 0) {
            errorMessageDuration--;
        }
        
        // Update flash effect
        flashCounter++;
        showFlash = (flashCounter / 20) % 2 == 0;  // Flash every 20 frames (slower)

        bool redrawMapThisFrame = mobSystem.consumeDemoRenderDirty();
        
           // Update header info (also show current wave remaining/total)
           setCursorPosition(0, 1);
           int waveRemaining = mobSystem.getRemainingInCurrentWave();
           int waveTotal = mobSystem.getTotalSpawnsCurrentWave();
           cout << "Player: " << playerName << " | Money: $" << setfill(' ') << setw(5) << money 
               << " | HP: " << baseHP << "/10" << " | wave: " << waveRemaining << "/" << waveTotal << "   ";
        
        if (redrawMapThisFrame) {
            // Render game map with selection bracket at fixed position
            setCursorPosition(0, mapStartLine);
        
        const vector<NavigationRoute>& routes = mobSystem.getNavigationRoutes();
        
            // Save current position for later restoration
            int currentMapLine = mapStartLine;
            for (int i = 0; i < GameMap::ROWS; i++) {
                setCursorPosition(0, currentMapLine + i);
                for (int j = 0; j < GameMap::COLS; j++) {
                Tile& tile = gameMap.grid[i][j];
                
                // Check if in placement area (3x3 area)
                bool inPlacement = (i >= selRow - 1 && i <= selRow + 1 && 
                                   j >= selCol - 1 && j <= selCol + 1);
                
                // Check if tower preview should be shown here
                bool inPreview = inPlacement && previewTowerIndex >= 0;
                
                if (inPreview) {
                    // Check if placement is valid
                    bool isValidPlacement = isValidPlacementLocation(gameMap, selRow, selCol);
                    
                    // Show cyan for valid, red for invalid
                    setTextColor(isValidPlacement ? COLOR_CYAN : COLOR_RED);
                    
                    // Determine which part of the tower ASCII art to display
                    int relRow = i - (selRow - 1);  // 0, 1, or 2 (top, middle, bottom of 3x3)
                    int relCol = j - (selCol - 1);  // 0, 1, or 2 (left, center, right of 3x3)
                    
                    if (showFlash && previewTowerIndex < (int)towers.size() && relRow < 3) {
                        // Show ASCII art during even frames (visible)
                        if (relCol < (int)towers[previewTowerIndex].art[relRow].length()) {
                            cout << towers[previewTowerIndex].art[relRow][relCol];
                        } else {
                            cout << "█";
                        }
                    } else {
                        // Show solid square during odd frames (flash effect)
                        cout << "█";
                    }
                    resetTextColor();
                } else if (inPlacement) {
                    // Check if this is a valid placement area
                    bool isValidPlacement = isValidPlacementLocation(gameMap, selRow, selCol);
                    bool isTowerCell = (tile.type == TOWER && tile.towerIndex >= 0);
                    bool isRoadCell = (tile.type == PATH);
                    
                    if (isTowerCell) {
                        // Show overlapping tower cells in red
                        setTextColor(COLOR_RED);
                        cout << tile.displayChar;
                        resetTextColor();
                    } else if (isRoadCell || !isValidPlacement) {
                        // Show red for road cells or invalid placement areas
                        setTextColor(COLOR_RED);
                        cout << "█";
                        resetTextColor();
                    } else {
                        // Show cyan for empty valid placement area
                        setTextColor(COLOR_CYAN);
                        cout << "█";
                        resetTextColor();
                    }
                } else {
                    // Normal rendering
                    bool isCheckpoint = false;
                    for (const auto& route : routes) {
                        if (route.checkpoint.first == i && route.checkpoint.second == j) {
                            isCheckpoint = true;
                            break;
                        }
                    }

                    // Checkpoint rendering disabled - was causing mobs to stop
                    // if (isCheckpoint && tile.type == PATH) {
                    //     setTextColor(COLOR_GREEN);
                    //     cout << 'C';
                    //     resetTextColor();
                    //     continue;
                    // }

                    switch (tile.type) {
                        case PATH:
                            if (tile.displayChar == '+') setTextColor(COLOR_YELLOW);
                            else if (tile.displayChar == '-') setTextColor(COLOR_GRAY);
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
            }
        }
        
        // Render mobs on top of the map
        mobSystem.renderMobs(mapStartLine, redrawMapThisFrame);
        mobSystem.renderAttackFlashes(mapStartLine);
        
        // Display info at bottom
        int infoLine = mapStartLine + GameMap::ROWS;
        setCursorPosition(0, infoLine);
        cout << string(75, '=') << endl;
        
        setCursorPosition(0, infoLine + 1);
        cout << "Arrow Keys: Move | E: Next wave | 1-" << unlockedTowerCount << ": Tower | Enter: Plant/Sell | Q: Exit    Mobs: " 
             << mobSystem.getActiveMobCount() << "              ";
        
        // Check for overlapping towers
        vector<int> overlapping = getOverlappingTowers(gameMap, selRow, selCol);
        int coveredTowerIndex = -1;
        bool isFullCoverage = checkFullTowerCoverage(gameMap, selRow, selCol, coveredTowerIndex);
        
        setCursorPosition(0, infoLine + 2);
        if (errorMessageDuration > 0) {
            // Display error message
            setTextColor(COLOR_RED);
            cout << "ERROR: " << errorMessage;
            resetTextColor();
            cout << string(max(0, 70 - (int)(6 + errorMessage.length())), ' ');
        } else if (isFullCoverage && coveredTowerIndex >= 0) {
            // Full tower coverage - offer sell
            if (sellModeState == 0) {
                cout << "SELL TOWER: " << towers[coveredTowerIndex].name << " | Press ENTER to see price";
                cout << string(10, ' ');
            } else if (sellModeState == 1) {
                // Show price
                sellTowerPrice = towers[coveredTowerIndex].cost / 2;
                cout << "SELL FOR: $" << sellTowerPrice << " | Press ENTER to CONFIRM or move to cancel";
                sellModeState = 1;
            }
        } else if (overlapping.size() > 0) {
            // Partial overlap - can't place
            cout << "CONFLICT: " << (int)overlapping.size() << " existing tower(s) overlap - CANNOT PLACE";
            cout << string(15, ' ');
        } else if (previewTowerIndex >= 0) {
            cout << "Preview: " << towers[previewTowerIndex].name << " - Cost: $" << towers[previewTowerIndex].cost;
            if (mapManager.canPlaceTower(selRow, selCol, previewTowerIndex)) {
                cout << " [CAN PLACE]" << string(20, ' ');
            } else {
                cout << " [CANNOT PLACE - Road/Boundary]" << string(10, ' ');
            }
        } else {
            cout << "Tower Selection: Press 1-9 for towers" << string(35, ' ');
        }
        
        setCursorPosition(0, infoLine + 3);
        cout << "Position: (" << setfill(' ') << setw(2) << selRow << ", " << setw(2) << selCol << ")                                              ";
        
        // Handle input (non-blocking)
        if (_kbhit()) {
            int key = _getch();
            
            if (key == 'q' || key == 'Q') {
                // Exit tower placement
                placingTowers = false;
            } else if (key == 13) {
                // Enter key - handle plant, sell confirmation, or show sell price
                int tempTowerIdx = -1;
                bool fullCov = checkFullTowerCoverage(gameMap, selRow, selCol, tempTowerIdx);
                
                if (fullCov && tempTowerIdx >= 0) {
                    // Tower sell interaction
                    if (sellModeState == 0) {
                        // First press - show price
                        sellModeState = 1;
                        sellTowerIndex = tempTowerIdx;
                    } else if (sellModeState == 1) {
                        // Second press - confirm sell
                        sellTowerPrice = towers[tempTowerIdx].cost / 2;
                        money += sellTowerPrice;
                        
                        // Remove tower from all 9 cells
                        for (int i = selRow - 1; i <= selRow + 1; i++) {
                            for (int j = selCol - 1; j <= selCol + 1; j++) {
                                if (i >= 0 && i < GameMap::ROWS && j >= 0 && j < GameMap::COLS) {
                                    gameMap.grid[i][j].type = BUILDABLE;
                                    gameMap.grid[i][j].displayChar = '.';
                                    gameMap.grid[i][j].towerIndex = -1;
                                    gameMap.grid[i][j].towerPosRow = -1;
                                    gameMap.grid[i][j].towerPosCol = -1;
                                }
                            }
                        }
                        
                        // Update backend array immediately when tower is sold
                        mapManager.updateBackendArrayTowerSold(gameStateArray, selRow, selCol);
                        
                        sellModeState = 0;
                        sellTowerIndex = -1;
                    }
                } else {
                    // Normal tower placement
                    if (previewTowerIndex >= 0) {
                        if (!mapManager.canPlaceTower(selRow, selCol, previewTowerIndex)) {
                            // Placement is invalid
                            errorMessage = "Cannot place tower on road or boundary!";
                            errorMessageDuration = 120;  // Show for 2 seconds (120 frames at 60 FPS)
                        } else {
                            vector<int> overlapCheck = getOverlappingTowers(gameMap, selRow, selCol);
                            if (overlapCheck.size() > 0) {
                                // Overlapping with existing tower
                                errorMessage = "Cannot overlap with existing tower!";
                                errorMessageDuration = 120;
                            } else if (money < towers[previewTowerIndex].cost) {
                                // Insufficient money
                                errorMessage = "Insufficient money! Cost: $" + to_string(towers[previewTowerIndex].cost) + " | Have: $" + to_string(money);
                                errorMessageDuration = 120;
                            } else {
                                // Valid placement
                                mapManager.placeTowerAt(selRow, selCol, previewTowerIndex);
                                // Update backend array immediately when tower is placed
                                mapManager.updateBackendArrayTowerPlaced(gameStateArray, selRow, selCol);
                                money -= towers[previewTowerIndex].cost;
                                previewTowerIndex = -1;
                            }
                        }
                    }
                }
            } else if (key == 'e' || key == 'E') {
                mobSystem.startNextWave();
            } else if (key >= '1' && key <= '9') {
                // Number key - select tower for preview
                int requestedTowerIndex = (key - '1');
                if (requestedTowerIndex < unlockedTowerCount) {
                    previewTowerIndex = requestedTowerIndex;
                    sellModeState = 0;  // Reset sell mode
                } else {
                    previewTowerIndex = -1;
                    sellModeState = 0;
                    errorMessage = "Tower " + to_string(requestedTowerIndex + 1) + " is locked until level " + to_string(requestedTowerIndex + 1) + ".";
                    errorMessageDuration = 120;
                }
            } else if (key == 224) {
                // Extended keys (arrow keys)
                int extKey = _getch();
                
                if (extKey == 72) {  // Up arrow
                    if (selRow > 2) selRow--;
                } else if (extKey == 80) {  // Down arrow
                    if (selRow < GameMap::ROWS - 3) selRow++;
                } else if (extKey == 75) {  // Left arrow
                    if (selCol > 2) selCol--;
                } else if (extKey == 77) {  // Right arrow
                    if (selCol < GameMap::COLS - 3) selCol++;
                }
                // Clear preview and sell mode when moving
                previewTowerIndex = -1;
                sellModeState = 0;
            }
            else if (key == 'z' || key == 'Z') {
                mobSystem.triggerNextWave();
            }
        }
        
        // Small delay to reduce CPU usage and control frame rate (60 FPS ~ 16ms)
        Sleep(16);
        
        // UI prompts and win/lose checks (available in all modes)
        // If waiting for next wave, show prompt
        if (mobSystem.isWaitingForNextWave()) {
            setCursorPosition(0, infoLine + 4);
            setTextColor(COLOR_GREEN);
            cout << "Wave clear, press Z to spawn next wave" << string(30, ' ');
            resetTextColor();
        }

        // Check victory: all waves spawned and no active mobs
        if (mobSystem.allWavesSpawned() && mobSystem.getActiveMobCount() == 0) {
            showCursor(true);
            clearScreen();
            cout << "\n\n";
            cout << string(30, '=') << endl;
            cout << "  Level " << levelSelected << " - Victory!" << endl;
            cout << "  Player: " << playerName << endl;
            cout << string(30, '=') << endl;
            cout << "\n\nPress any key to return to level select...";
            _getch();
            return true;
        }

        // Check defeat: base HP reached 0
        if (baseHP <= 0) {
            showCursor(true);
            clearScreen();
            cout << "\n\n";
            cout << string(30, '=') << endl;
            cout << "  Level " << levelSelected << " - Defeat!" << endl;
            cout << "  Player: " << playerName << endl;
            cout << string(30, '=') << endl;
            cout << "\n\nPress any key to return to level select...";
            _getch();
            return false;
        }
    }
    
    showCursor(true);
    
    clearScreen();
    cout << "\n\n";
    cout << string(30, '=') << endl;
    cout << "  Level " << levelSelected << " - Game Complete!" << endl;
    cout << "  Player: " << playerName << endl;
    cout << "  Final Money: $" << money << endl;
    cout << string(30, '=') << endl;
    
    cout << "\n\nPress any key to return to menu...";
    _getch();
    return false;
}

// Main menu loop
void gameLoop() {
    bool playing = true;
    string playerName = displayStartScreen();
    int maxUnlockedLevel = (playerName == "test") ? 10 : 1;
    
    while (playing) {
        // Level selection
        int levelSelected = displayLevelSelect(playerName, maxUnlockedLevel);
        
        // Game screen
        bool victory = displayGameScreen(playerName, levelSelected);

        if (victory) {
            if (levelSelected == 1 && maxUnlockedLevel < 2) {
                maxUnlockedLevel = 2;
            }
            // After victory screen, directly return to level selection.
            continue;
        }
        
        // Ask if player wants to continue
        clearScreen();
        cout << "\n\n";
        cout << string(30, ' ') << "Return to menu? (Y/N)" << endl;
        cout << string(30, ' ') << "> ";
        
        char choice;
        cin >> choice;
        cin.ignore(); // Clear newline from input buffer
        
        if (choice != 'y' && choice != 'Y') {
            playing = false;
        }
    }
    
    clearScreen();
    cout << "\n\n";
    cout << string(25, ' ') << "Thanks for playing BYTE RUSH!" << endl;
    cout << string(30, ' ') << "Goodbye!" << endl;
    cout << "\n\n";
}

int main() {
    // Enable UTF-8 support in Windows console
    system("chcp 65001 > nul");
    
    // Set console title
    system("title BYTE RUSH - Tower Defense Game");
    
    // Set console window and buffer size to fit the game map
    setConsoleSize();
    
    gameLoop();
    
    return 0;
}
