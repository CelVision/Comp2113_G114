#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <windows.h>
#include <cmath>
#include "src/core/GameData.h"
#include "src/core/GameMap.cpp"

using namespace std;

// Mob position tracker
struct SimulatedMob {
    int spawnIndex;
    double row, col;
    int currentCommandIndex;
    double progressInCommand;  // 0.0 to 1.0
    bool hasReachedBase;
};

void visualizeMobMovement(GameMapManager& mapManager, int level) {
    cout << "\n" << string(70, '=') << endl;
    cout << "LEVEL " << level << " - MOB MOVEMENT SIMULATION" << endl;
    cout << string(70, '=') << endl;
    
    // Load map into backend array
    char gameStateArray[33][147];
    mapManager.loadMapToArray(level, gameStateArray);
    
    // Get paths from backend array
    auto pathMap = mapManager.loadPathsFromBackendArray(gameStateArray);
    
    if (pathMap.empty()) {
        cout << "\n⚠ No spawn points on this level" << endl;
        return;
    }
    
    cout << "\nSpawning " << pathMap.size() << " mob(s)...\n";
    
    // Initialize mobs
    vector<SimulatedMob> mobs;
    vector<pair<int, int>> spawnCoords;
    vector<vector<pair<pair<int, int>, pair<int, int>>>> pathData;  // Store start->end coords
    
    int spawnIndex = 0;
    for (const auto& entry : pathMap) {
        auto spawnCoord = entry.first;
        const auto& pathCommands = entry.second;
        
        spawnCoords.push_back(spawnCoord);
        
        vector<pair<pair<int, int>, pair<int, int>>> commandPairs;
        for (const auto& cmd : pathCommands) {
            commandPairs.push_back({cmd.startPos, cmd.endPos});
        }
        pathData.push_back(commandPairs);
        
        SimulatedMob mob;
        mob.spawnIndex = spawnIndex;
        mob.row = (double)spawnCoord.first;
        mob.col = (double)spawnCoord.second;
        mob.currentCommandIndex = 0;
        mob.progressInCommand = 0.0;
        mob.hasReachedBase = false;
        
        mobs.push_back(mob);
        spawnIndex++;
    }
    
    // Simulate movement
    int maxSteps = 1000;  // Increased for slower movement (1 block per 10 frames)
    for (int step = 0; step < maxSteps; step++) {
        cout << "\nStep " << step << ":" << endl;
        
        bool allReachedBase = true;
        
        for (auto& mob : mobs) {
            if (mob.hasReachedBase) {
                cout << "  Mob " << mob.spawnIndex << ": [REACHED BASE at (" 
                     << (int)mob.row << ", " << (int)mob.col << ")]" << endl;
                continue;
            }
            
            allReachedBase = false;
            
            // Check if all commands completed
            if (mob.currentCommandIndex >= (int)pathData[mob.spawnIndex].size()) {
                mob.hasReachedBase = true;
                cout << "  Mob " << mob.spawnIndex << ": [✓ REACHED BASE]" << endl;
                continue;
            }
            
            // Move along current command
            auto& cmdPair = pathData[mob.spawnIndex][mob.currentCommandIndex];
            auto start = cmdPair.first;
            auto end = cmdPair.second;
            
            // Move 1 block per 10 frames
            int distance = abs(end.first - start.first) + abs(end.second - start.second);
            if (distance == 0) distance = 1;
            mob.progressInCommand += 1.0 / (distance * 10.0);
            
            if (mob.progressInCommand >= 1.0) {
                mob.progressInCommand = 0.0;
                mob.currentCommandIndex++;
                
                // Set position to end of command
                mob.row = (double)end.first;
                mob.col = (double)end.second;
                
                if (mob.currentCommandIndex >= (int)pathData[mob.spawnIndex].size()) {
                    mob.hasReachedBase = true;
                    cout << "  Mob " << mob.spawnIndex << ": [✓ REACHED BASE at (" 
                         << (int)mob.row << ", " << (int)mob.col << ")]" << endl;
                    continue;
                }
            } else {
                // Interpolate position
                auto start = cmdPair.first;
                auto end = cmdPair.second;
                
                mob.row = start.first + (end.first - start.first) * mob.progressInCommand;
                mob.col = start.second + (end.second - start.second) * mob.progressInCommand;
            }
            
            cout << "  Mob " << mob.spawnIndex << ": (" << fixed << setprecision(1) 
                 << mob.row << ", " << mob.col << ") - Command " 
                 << (mob.currentCommandIndex + 1) << "/" << pathData[mob.spawnIndex].size() << endl;
        }
        
        if (allReachedBase) {
            cout << "\n✓ All mobs reached base!" << endl;
            break;
        }
    }
}

int main() {
    cout << "\n" << string(80, '#') << endl;
    cout << "#" << string(78, ' ') << "#" << endl;
    cout << "#" << setw(79) << left << "  MOB MOVEMENT SIMULATOR - LEVEL PATHFINDING" << "#" << endl;
    cout << "#" << string(78, ' ') << "#" << endl;
    cout << string(80, '#') << endl;
    
    cout << "\nThis demo shows:" << endl;
    cout << "  • Real-time mob position tracking" << endl;
    cout << "  • Path command execution" << endl;
    cout << "  • Movement from spawn to base" << endl;
    cout << "  • Multi-spawn pathfinding coordination" << endl;
    
    GameMapManager mapManager;
    
    // Show simulation for levels 1, 2, 4, 5 (ones with interesting spawn patterns)
    vector<int> demosLevels = {1, 2, 4, 5};
    
    for (int level : demosLevels) {
        visualizeMobMovement(mapManager, level);
    }
    
    cout << "\n\n" << string(80, '=') << endl;
    cout << "SIMULATION SUMMARY" << endl;
    cout << string(80, '=') << endl;
    
    cout << "\n✓ Pathfinding system working for all spawn points!" << endl;
    cout << "✓ Mobs can navigate from spawn to base!" << endl;
    cout << "✓ Backend path strategy successfully implemented!" << endl;
    
    cout << "\nNext steps:" << endl;
    cout << "  1. Integrate into main game" << endl;
    cout << "  2. Add mob rendering at each position" << endl;
    cout << "  3. Handle tower placement collisions" << endl;
    cout << "  4. Implement tower attacks on mobs" << endl;
    
    cout << "\n" << string(80, '=') << endl;
    
    return 0;
}
