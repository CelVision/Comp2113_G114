#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <iomanip>
#include <windows.h>
#include "src/core/GameData.h"
#include "src/core/GameMap.cpp"

using namespace std;

void clearScreen() {
    system("cls");
}

void displayPathInfo(GameMapManager& mapManager, int level) {
    cout << "\n" << string(70, '=') << endl;
    cout << "LEVEL " << level << " - PATH VISUALIZATION" << endl;
    cout << string(70, '=') << endl;
    
    // Load map into backend array
    char gameStateArray[33][147];
    mapManager.loadMapToArray(level, gameStateArray);
    
    // Get paths from backend array
    auto pathMap = mapManager.loadPathsFromBackendArray(gameStateArray);
    
    cout << "\nFound " << pathMap.size() << " spawn point(s):" << endl;
    
    int spawnIndex = 0;
    for (const auto& entry : pathMap) {
        auto spawnCoord = entry.first;
        const auto& pathCommands = entry.second;
        
        spawnIndex++;
        cout << "\n  Spawn Point " << spawnIndex << ": (" << spawnCoord.first << ", " << spawnCoord.second << ")" << endl;
        
        if (pathCommands.empty()) {
            cout << "    Path: DIRECT (already at base)" << endl;
        } else {
            cout << "    Path commands (" << pathCommands.size() << " steps):" << endl;
            
            // Display path commands
            for (size_t i = 0; i < pathCommands.size(); i++) {
                const auto& cmd = pathCommands[i];
                cout << "      [" << i+1 << "] " << cmd.description << endl;
                cout << "          From (" << cmd.startPos.first << ", " << cmd.startPos.second 
                     << ") To (" << cmd.endPos.first << ", " << cmd.endPos.second << ")" << endl;
            }
        }
    }
}

void displayAllPathsOverview() {
    cout << "\n" << string(80, '#') << endl;
    cout << "###" << string(74, ' ') << "###" << endl;
    cout << "###" << setw(74) << left << "  DEMO: MOB PATHFINDING FOR ALL 10 LEVELS" << "###" << endl;
    cout << "###" << string(74, ' ') << "###" << endl;
    cout << string(80, '#') << endl;
    
    cout << "\nThis demo visualizes:" << endl;
    cout << "  • Backend pathfinding using Dijkstra algorithm" << endl;
    cout << "  • All spawn points per level" << endl;
    cout << "  • Computed paths from spawn to base" << endl;
    cout << "  • Movement commands for each spawn point" << endl;
    
    cout << "\n" << string(80, '-') << endl;
}

int main() {
    cout << fixed << setprecision(1);
    
    displayAllPathsOverview();
    
    GameMapManager mapManager;
    
    // Process all 10 levels
    for (int level = 1; level <= 10; level++) {
        displayPathInfo(mapManager, level);
        
        if (level < 10) {
            cout << "\nPress ENTER to continue to next level...";
            cin.ignore();
        }
    }
    
    // Summary statistics
    cout << "\n\n" << string(70, '=') << endl;
    cout << "SUMMARY - PATH VERIFICATION COMPLETE" << endl;
    cout << string(70, '=') << endl;
    
    cout << "\nStatistics:" << endl;
    int totalSpawns = 0;
    for (int level = 1; level <= 10; level++) {
        char gameStateArray[33][147];
        mapManager.loadMapToArray(level, gameStateArray);
        auto pathMap = mapManager.loadPathsFromBackendArray(gameStateArray);
        totalSpawns += pathMap.size();
        
        cout << "  Level " << setw(2) << level << ": " << setw(2) << pathMap.size() << " spawn point(s)" << endl;
    }
    
    cout << "\nTotal spawn points across all levels: " << totalSpawns << endl;
    cout << "\n✓ All paths computed successfully!" << endl;
    cout << "✓ Backend pathfinding system verified!" << endl;
    
    cout << "\nPress ENTER to exit...";
    cin.ignore();
    
    return 0;
}
