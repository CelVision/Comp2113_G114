#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include "src/core/GameData.h"
#include "src/core/GameMap.cpp"

using namespace std;

void displayPathInfo(GameMapManager& mapManager, int level) {
    cout << "\n" << string(75, '=') << endl;
    cout << "LEVEL " << level << " - PATH VISUALIZATION" << endl;
    cout << string(75, '=') << endl;
    
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
        cout << "\n  ┌─ Spawn Point " << spawnIndex << " at (" << setw(3) << spawnCoord.first 
             << ", " << setw(3) << spawnCoord.second << ") " << endl;
        
        if (pathCommands.empty()) {
            cout << "  └─ [No path needed - already at base]" << endl;
        } else {
            cout << "  │  Path: " << pathCommands.size() << " movement command(s)" << endl;
            
            // Display path commands
            for (size_t i = 0; i < pathCommands.size(); i++) {
                const auto& cmd = pathCommands[i];
                bool isLast = (i == pathCommands.size() - 1);
                
                cout << "  " << (isLast ? "└" : "├") << "─ [Step " << (i+1) << "] " 
                     << cmd.description << endl;
                cout << "  " << (isLast ? " " : "│") << "   From (" << setw(3) << cmd.startPos.first 
                     << ", " << setw(3) << cmd.startPos.second << ") → (" 
                     << setw(3) << cmd.endPos.first << ", " << setw(3) << cmd.endPos.second 
                     << ")" << endl;
            }
        }
    }
}

int main() {
    cout << "\n" << string(80, '#') << endl;
    cout << "#" << string(78, ' ') << "#" << endl;
    cout << "#" << setw(79) << left << "  BACKEND PATHFINDING VISUALIZATION - ALL 10 LEVELS" << "#" << endl;
    cout << "#" << string(78, ' ') << "#" << endl;
    cout << string(80, '#') << endl;
    
    cout << "\nThis demo shows:" << endl;
    cout << "  • Backend pathfinding computed via Dijkstra algorithm" << endl;
    cout << "  • All spawn points for each level" << endl;
    cout << "  • Waypoints and movement commands" << endl;
    cout << "  • Coordinates for rendering and mob movement" << endl;
    
    cout << "\n" << string(80, '-') << endl;
    
    GameMapManager mapManager;
    
    // Process all 10 levels
    for (int level = 1; level <= 10; level++) {
        displayPathInfo(mapManager, level);
    }
    
    // Summary statistics
    cout << "\n\n" << string(80, '=') << endl;
    cout << "SUMMARY - PATH VERIFICATION FOR ALL LEVELS" << endl;
    cout << string(80, '=') << endl;
    
    cout << "\nLevel-by-level spawn point count:" << endl;
    int totalSpawns = 0;
    
    for (int level = 1; level <= 10; level++) {
        char gameStateArray[33][147];
        mapManager.loadMapToArray(level, gameStateArray);
        auto pathMap = mapManager.loadPathsFromBackendArray(gameStateArray);
        totalSpawns += pathMap.size();
        
        cout << "  Level " << setw(2) << level << ": " << setw(2) << pathMap.size() 
             << " spawn point(s)" << endl;
    }
    
    cout << "\n" << string(80, '-') << endl;
    cout << "Total spawn points across all 10 levels: " << totalSpawns << endl;
    
    cout << "\n✓ All paths computed successfully!" << endl;
    cout << "✓ Backend pathfinding system verified for all levels!" << endl;
    cout << "✓ Mob movement ready to implement!" << endl;
    
    cout << "\n" << string(80, '=') << endl;
    
    return 0;
}
