#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <iomanip>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include "GameData.h"

// Note: GameMap.cpp is included as a header-like implementation file
#include "GameMap.cpp"

using namespace std;

// Constants
const int MAP_COLS = 99;
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
int displayLevelSelect() {
    int selectedCol = 0;
    int selectedRow = 0;
    
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
                
                if (selectedRow == row && selectedCol == col) {
                    setTextColor(COLOR_GREEN);
                }
                cout << "  ┌─────┐  ";
                if (selectedRow == row && selectedCol == col) {
                    resetTextColor();
                }
            }
            cout << "\n" << string(10, ' ');
            
            for (int col = 0; col < LEVEL_COLS; col++) {
                int levelNum = row * LEVEL_COLS + col + 1;
                
                if (selectedRow == row && selectedCol == col) {
                    setTextColor(COLOR_GREEN);
                }
                if (levelNum <= 9) {
                    cout << "  │ " << levelNum << "   │  ";
                } else {
                    cout << "  │ ∞   │  ";
                }
                if (selectedRow == row && selectedCol == col) {
                    resetTextColor();
                }
            }
            cout << "\n" << string(10, ' ');
            
            for (int col = 0; col < LEVEL_COLS; col++) {
                int levelNum = row * LEVEL_COLS + col + 1;
                
                if (selectedRow == row && selectedCol == col) {
                    setTextColor(COLOR_GREEN);
                }
                cout << "  └─────┘  ";
                if (selectedRow == row && selectedCol == col) {
                    resetTextColor();
                }
            }
        }
        
        cout << "\n\n";
        cout << string(20, ' ') << "Use ← → ↑ ↓ to navigate, ENTER to select" << endl;
        
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
            return selectedLevel;
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
void displayGameScreen(string playerName, int levelSelected) {
    clearScreen();
    
    // Initialize game map manager
    GameMapManager mapManager;
    
    // Create path for the selected level
    vector<pair<int, int>> pathCoords;
    mapManager.createPathForLevel(levelSelected, pathCoords);
    
    // Initialize buildable areas
    mapManager.initializeBuildableAreas();
    
    // Set base camp at the end of the map
    mapManager.setBaseCamp(16, 97);
    
    // Tower placement mode
    int selRow = 16;  // Start at middle
    int selCol = 50;
    int previewTowerIndex = -1;
    bool showFlash = false;
    int flashCounter = 0;
    int money = 10000;
    int baseHP = 10;
    
    bool placingTowers = true;
    
    // Tower sell system variables
    int sellModeState = 0;  // 0: no sell, 1: showing price, 2: confirmed
    int sellTowerIndex = -1;
    int sellTowerPrice = 0;
    
    // Hide cursor for better appearance
    showCursor(false);
    
    // Draw static UI once
    setCursorPosition(0, 0);
    cout << "\n";
    cout << string(20, '=') << " BYTE RUSH - Level " << levelSelected << " " << string(20, '=') << endl;
    cout << "Player: " << playerName << " | Money: $" << money << " | HP: " << baseHP << "/10" << endl;
    cout << string(75, '=') << endl;
    cout << "\n";
    int mapStartLine = 5;  // Line where map starts
    
    while (placingTowers) {
        // Update flash effect
        flashCounter++;
        showFlash = (flashCounter / 20) % 2 == 0;  // Flash every 20 frames (slower)
        
        // Update header info
        setCursorPosition(0, 1);
        cout << "Player: " << playerName << " | Money: $" << setfill(' ') << setw(5) << money << " | HP: " << baseHP << "/10   ";
        
        // Render game map with selection bracket at fixed position
        setCursorPosition(0, mapStartLine);
        
        // Get map and towers references
        GameMap& gameMap = mapManager.getGameMap();
        vector<Tower>& towers = mapManager.getAllTowers();
        
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
                    // Show solid cyan 3x3 square with tower ASCII art
                    setTextColor(COLOR_CYAN);
                    
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
                    // Check for tower overlaps
                    bool isTowerCell = (tile.type == TOWER && tile.towerIndex >= 0);
                    
                    if (isTowerCell) {
                        // Show overlapping tower cells in red
                        setTextColor(COLOR_RED);
                        cout << tile.displayChar;
                        resetTextColor();
                    } else {
                        // Show cyan for empty placement area
                        setTextColor(COLOR_CYAN);
                        cout << "█";
                        resetTextColor();
                    }
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
        }
        
        // Display info at bottom
        int infoLine = mapStartLine + GameMap::ROWS;
        setCursorPosition(0, infoLine);
        cout << string(75, '=') << endl;
        
        setCursorPosition(0, infoLine + 1);
        cout << "Arrow Keys: Move | 1-9: Tower | Enter: Plant/Sell | Q: Exit                     ";
        
        // Check for overlapping towers
        vector<int> overlapping = getOverlappingTowers(gameMap, selRow, selCol);
        int coveredTowerIndex = -1;
        bool isFullCoverage = checkFullTowerCoverage(gameMap, selRow, selCol, coveredTowerIndex);
        
        setCursorPosition(0, infoLine + 2);
        if (isFullCoverage && coveredTowerIndex >= 0) {
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
                cout << " [CANNOT PLACE]" << string(15, ' ');
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
                        sellModeState = 0;
                        sellTowerIndex = -1;
                    }
                } else {
                    // Normal tower placement
                    if (previewTowerIndex >= 0 && mapManager.canPlaceTower(selRow, selCol, previewTowerIndex)) {
                        vector<int> overlapCheck = getOverlappingTowers(gameMap, selRow, selCol);
                        if (overlapCheck.size() == 0) {  // No overlaps
                            if (money >= towers[previewTowerIndex].cost) {
                                mapManager.placeTowerAt(selRow, selCol, previewTowerIndex);
                                money -= towers[previewTowerIndex].cost;
                                previewTowerIndex = -1;
                            }
                        }
                    }
                }
            } else if (key >= '1' && key <= '9') {
                // Number key - select tower for preview
                previewTowerIndex = (key - '1');
                sellModeState = 0;  // Reset sell mode
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
        }
        
        // Small delay to reduce CPU usage and control frame rate (60 FPS ~ 16ms)
        Sleep(16);
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
}

// Main menu loop
void gameLoop() {
    bool playing = true;
    string playerName;
    
    while (playing) {
        // Start screen
        playerName = displayStartScreen();
        
        // Level selection
        int levelSelected = displayLevelSelect();
        
        // Game screen
        displayGameScreen(playerName, levelSelected);
        
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
    
    gameLoop();
    
    return 0;
}
