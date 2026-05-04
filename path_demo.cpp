#include <iostream>
#include <vector>
#include <windows.h>
#include <cstdlib>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

// Map dimensions
const int ROWS = 33;
const int COLS = 147;

void clearScreen() {
    system("cls");
}

void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordinates;
    coordinates.X = x;
    coordinates.Y = y;
    SetConsoleCursorPosition(hConsole, coordinates);
}

int main() {
    system("chcp 65001 > nul");
    system("title Pigman Path Demo");
    
    // Create 2D array for the map
    vector<vector<char>> map(ROWS, vector<char>(COLS, '.'));
    
    // Set spawn points (rows 1, 2, 3)
    int spawnRow = 1;
    int spawnCol = 1;  // Start from column 1 (1 unit right of spawn marker at col 0)
    
    map[spawnRow][spawnCol] = '+';
    
    // Build the fixed path
    vector<pair<int, int>> path;
    
    int currentRow = spawnRow;
    int currentCol = spawnCol;
    
    // Go right 140 units
    for (int step = 0; step <= 140; step++) {
        path.push_back({currentRow, currentCol});
        currentCol++;
        if (currentCol >= COLS) break;
    }
    
    // Go down 19 units
    for (int step = 1; step <= 19; step++) {
        currentRow++;
        if (currentRow >= ROWS) break;
        path.push_back({currentRow, currentCol});
    }
    
    // Go left 111 units
    for (int step = 1; step <= 111; step++) {
        currentCol--;
        if (currentCol < 0) break;
        path.push_back({currentRow, currentCol});
    }
    
    cout << "\n\nPigman Path Demonstration\n";
    cout << "Spawn Point: (" << spawnRow << ", " << spawnCol << ")\n";
    cout << "Total Path Length: " << path.size() << " tiles\n";
    cout << "Speed: 2 tiles per second\n";
    cout << "\nPath sequence:\n";
    cout << "  1. Right 140 units\n";
    cout << "  2. Down 19 units\n";
    cout << "  3. Left 111 units\n";
    cout << "\n";
    cout << "End Point: (" << path.back().first << ", " << path.back().second << ")\n\n";
    
    cout << "Press any key to start animation...";
    cin.get();
    
    clearScreen();
    
    // Animation loop
    const double speed = 2.0;  // tiles per second
    const double timePerFrame = 1.0 / 60.0;  // 60 FPS
    
    double pigmanPosition = 0.0;  // Position along the path (in tiles)
    int frame = 0;
    
    while (pigmanPosition < path.size() - 1) {
        // Clear previous pigman position
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (map[i][j] == 'p') {
                    map[i][j] = '.';
                }
            }
        }
        
        // Update pigman position
        pigmanPosition += speed * timePerFrame;
        
        // Get the current path segment
        int pathIndex = (int)pigmanPosition;
        if (pathIndex >= (int)path.size()) {
            pathIndex = (int)path.size() - 1;
        }
        
        int newRow = path[pathIndex].first;
        int newCol = path[pathIndex].second;
        
        // Place pigman on map
        map[newRow][newCol] = 'p';
        
        // Render map
        setCursorPosition(0, 0);
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                cout << map[i][j];
            }
            cout << "\n";
        }
        
        // Show info
        setCursorPosition(0, ROWS + 1);
        cout << "Frame: " << frame << " | Position: (" << newRow << ", " << newCol << ") | Path Progress: " 
             << (int)pigmanPosition << "/" << (int)path.size() << "   ";
        
        frame++;
        
        // Sleep to maintain 60 FPS
        Sleep(16);
    }
    
    // Final frame showing pigman at end
    setCursorPosition(0, ROWS + 2);
    cout << "Pigman reached the end! Final position: (" << path.back().first << ", " 
         << path.back().second << ")\n";
    cout << "Press any key to exit...";
    cin.get();
    
    clearScreen();
    
    return 0;
}
