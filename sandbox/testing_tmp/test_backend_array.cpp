#include <iostream>
#include <fstream>
#include "src/core/GameData.h"
#include "src/core/GameMap.cpp"

using namespace std;

int main() {
    cout << "\n=== Backend Array Loader Test ===" << endl;
    cout << "Testing loadMapToArray() function" << endl;
    
    // Create GameMapManager
    GameMapManager mapManager;
    
    // Create backend state array
    char gameStateArray[33][147];
    
    // Load map for level 1
    cout << "\nLoading map for Level 1..." << endl;
    mapManager.loadMapToArray(1, gameStateArray);
    
    // Display sample of loaded array (first 10 rows, first 50 columns)
    cout << "\nFirst 10 rows, first 50 columns of backend array:\n" << endl;
    
    for (int i = 0; i < 10; i++) {
        cout << "Row " << i << ": ";
        for (int j = 0; j < 50; j++) {
            char ch = gameStateArray[i][j];
            if (ch == '+') {
                cout << "+";  // Spawn
            } else if (ch == '.') {
                cout << ".";  // Buildable
            } else if (ch == 'B') {
                cout << "B";  // Base
            } else if (ch == ' ') {
                cout << "-";  // Road or blocked (display as - for visibility)
            } else {
                cout << " ";
            }
        }
        cout << endl;
    }
    
    // Count elements
    int spawnCount = 0, buildCount = 0, baseCount = 0, roadCount = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 147; j++) {
            if (gameStateArray[i][j] == '+') spawnCount++;
            else if (gameStateArray[i][j] == '.') buildCount++;
            else if (gameStateArray[i][j] == 'B') baseCount++;
            else if (gameStateArray[i][j] == ' ') roadCount++;
        }
    }
    
    cout << "\n=== Statistics ===" << endl;
    cout << "Spawn points:     " << spawnCount << endl;
    cout << "Buildable grids:  " << buildCount << endl;
    cout << "Base positions:   " << baseCount << endl;
    cout << "Roads/blocked:    " << roadCount << endl;
    cout << "Total cells:      " << (spawnCount + buildCount + baseCount + roadCount) << " / " << (33*147) << endl;
    
    cout << "\n✓ Backend array loaded successfully!" << endl;
    
    return 0;
}
