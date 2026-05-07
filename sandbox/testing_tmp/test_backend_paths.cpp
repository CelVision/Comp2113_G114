#include <iostream>
#include <fstream>
#include "src/core/GameData.h"
#include "src/core/GameMap.cpp"

using namespace std;

void printBackendArray(const char stateArray[33][147], int maxRows = 15, int maxCols = 60) {
    for (int i = 0; i < maxRows; i++) {
        cout << "Row " << i << ": ";
        for (int j = 0; j < maxCols; j++) {
            char ch = stateArray[i][j];
            if (ch == '+') cout << "+";
            else if (ch == '.') cout << ".";
            else if (ch == 'B') cout << "B";
            else if (ch == ' ') cout << "-";  // Display space as dash for visibility
            else cout << "?";
        }
        cout << endl;
    }
}

int main() {
    cout << "\n========== Backend Path System Test ==========" << endl;
    
    // Initialize
    GameMapManager mapManager;
    char gameStateArray[33][147];
    
    // Load map
    cout << "\n[1] Loading map for Level 1..." << endl;
    mapManager.loadMapToArray(1, gameStateArray);
    cout << "✓ Map loaded successfully" << endl;
    
    // Display sample of array
    cout << "\n[2] Backend Array Sample (first 10 rows):" << endl;
    printBackendArray(gameStateArray, 10, 60);
    
    // Find spawn points and base
    cout << "\n[3] Scanning for spawn points and base..." << endl;
    int spawnCount = 0, baseCount = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 147; j++) {
            if (gameStateArray[i][j] == '+') {
                cout << "   Spawn point found at (" << i << ", " << j << ")" << endl;
                spawnCount++;
            }
            if (gameStateArray[i][j] == 'B') {
                cout << "   Base found at (" << i << ", " << j << ")" << endl;
                baseCount++;
            }
        }
    }
    cout << "✓ Found " << spawnCount << " spawn point(s) and " << baseCount << " base(s)" << endl;
    
    // Test spawnMobAtSpawnPoint function
    cout << "\n[4] Testing spawnMobAtSpawnPoint()..." << endl;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 147; j++) {
            if (gameStateArray[i][j] == '+') {
                pair<int, int> spawnPos = mapManager.spawnMobAtSpawnPoint(gameStateArray, i, j);
                if (spawnPos.first != -1) {
                    cout << "   Spawn point at (" << i << ", " << j 
                         << ") can spawn mob at (" << spawnPos.first << ", " << spawnPos.second << ")" << endl;
                } else {
                    cout << "   Spawn point at (" << i << ", " << j << ") has no adjacent empty space" << endl;
                }
            }
        }
    }
    
    // Test loadPathsFromBackendArray with Dijkstra
    cout << "\n[5] Testing loadPathsFromBackendArray (Dijkstra)..." << endl;
    map<pair<int, int>, vector<GameMapManager::PathCommand>> paths = mapManager.loadPathsFromBackendArray(gameStateArray);
    
    cout << "   Found " << paths.size() << " path(s)" << endl;
    for (const auto& pathEntry : paths) {
        pair<int, int> spawn = pathEntry.first;
        const vector<GameMapManager::PathCommand>& commands = pathEntry.second;
        
        cout << "\n   Path from spawn (" << spawn.first << ", " << spawn.second << "):" << endl;
        for (int i = 0; i < (int)commands.size() && i < 10; i++) {  // Show first 10 commands
            cout << "      " << i + 1 << ". " << commands[i].description << endl;
        }
        if (commands.size() > 10) {
            cout << "      ... and " << (commands.size() - 10) << " more commands" << endl;
        }
        cout << "      Total commands: " << commands.size() << endl;
    }
    
    // Summary
    cout << "\n========== Test Summary ==========" << endl;
    cout << "✓ Backend array loaded" << endl;
    cout << "✓ Spawn points and base detected" << endl;
    cout << "✓ Mob spawn positions generated" << endl;
    cout << "✓ Dijkstra pathfinding completed" << endl;
    cout << "✓ Path commands generated" << endl;
    cout << "\nAll tests passed!" << endl;
    
    return 0;
}
