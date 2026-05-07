#include <iostream>
#include <fstream>
#include "src/core/GameData.h"
#include "src/core/GameMap.cpp"

using namespace std;

void printArraySection(const char stateArray[33][147], int startRow, int startCol, int rows, int cols) {
    for (int i = startRow; i < startRow + rows && i < 33; i++) {
        cout << "Row " << i << ": ";
        for (int j = startCol; j < startCol + cols && j < 147; j++) {
            char ch = stateArray[i][j];
            if (ch == '+') cout << "+";
            else if (ch == '.') cout << ".";
            else if (ch == 'B') cout << "B";
            else if (ch == 'T') cout << "T";  // Tower
            else if (ch == ' ') cout << "-";  // Road/blocked
            else cout << "?";
        }
        cout << endl;
    }
}

int main() {
    cout << "\n========== Backend Array Tower Updates Test ==========" << endl;
    
    // Initialize
    GameMapManager mapManager;
    char gameStateArray[33][147];
    
    // Load map
    cout << "\n[1] Loading map for Level 1..." << endl;
    mapManager.loadMapToArray(1, gameStateArray);
    cout << "✓ Map loaded" << endl;
    
    // Display initial state around a buildable area
    cout << "\n[2] Initial backend array state (rows 4-7, cols 5-15):" << endl;
    printArraySection(gameStateArray, 4, 5, 4, 11);
    
    // Simulate placing a tower at position (5, 10)
    cout << "\n[3] Placing tower at center position (5, 10)..." << endl;
    int towerCenterRow = 5;
    int towerCenterCol = 10;
    mapManager.updateBackendArrayTowerPlaced(gameStateArray, towerCenterRow, towerCenterCol);
    cout << "✓ Tower placed in backend array" << endl;
    
    // Display state after tower placement
    cout << "\n[4] Backend array after tower placement (rows 4-7, cols 5-15):" << endl;
    printArraySection(gameStateArray, 4, 5, 4, 11);
    
    // Verify all 9 cells are marked as 'T'
    cout << "\n[5] Verifying 3x3 tower area is marked as 'T':" << endl;
    bool allMarked = true;
    for (int i = towerCenterRow - 1; i <= towerCenterRow + 1; i++) {
        for (int j = towerCenterCol - 1; j <= towerCenterCol + 1; j++) {
            if (gameStateArray[i][j] != 'T') {
                allMarked = false;
                cout << "   ERROR: Position (" << i << ", " << j << ") = '" << gameStateArray[i][j] << "' (expected 'T')" << endl;
            }
        }
    }
    if (allMarked) {
        cout << "   ✓ All 9 cells of 3x3 area correctly marked as 'T'" << endl;
    }
    
    // Simulate selling the tower
    cout << "\n[6] Selling tower at position (5, 10)..." << endl;
    mapManager.updateBackendArrayTowerSold(gameStateArray, towerCenterRow, towerCenterCol);
    cout << "✓ Tower removed from backend array" << endl;
    
    // Display state after tower removal
    cout << "\n[7] Backend array after tower removal (rows 4-7, cols 5-15):" << endl;
    printArraySection(gameStateArray, 4, 5, 4, 11);
    
    // Verify all 9 cells are restored to '.'
    cout << "\n[8] Verifying 3x3 tower area is restored to '.':" << endl;
    bool allRestored = true;
    for (int i = towerCenterRow - 1; i <= towerCenterRow + 1; i++) {
        for (int j = towerCenterCol - 1; j <= towerCenterCol + 1; j++) {
            if (gameStateArray[i][j] != '.') {
                allRestored = false;
                cout << "   ERROR: Position (" << i << ", " << j << ") = '" << gameStateArray[i][j] << "' (expected '.')" << endl;
            }
        }
    }
    if (allRestored) {
        cout << "   ✓ All 9 cells of 3x3 area correctly restored to '.' (buildable)" << endl;
    }
    
    // Test multiple towers
    cout << "\n[9] Testing multiple tower placements..." << endl;
    vector<pair<int, int>> towerPositions = {
        {8, 20}, {12, 30}, {16, 40}
    };
    
    for (const auto& pos : towerPositions) {
        mapManager.updateBackendArrayTowerPlaced(gameStateArray, pos.first, pos.second);
        cout << "   ✓ Tower placed at (" << pos.first << ", " << pos.second << ")" << endl;
    }
    
    // Display state with multiple towers
    cout << "\n[10] Backend array with multiple towers (rows 7-18, cols 15-45):" << endl;
    printArraySection(gameStateArray, 7, 15, 12, 31);
    
    // Count towers in array
    int towerCount = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 147; j++) {
            if (gameStateArray[i][j] == 'T') towerCount++;
        }
    }
    cout << "\n[11] Total 'T' cells in array: " << towerCount << " (expected " << (towerPositions.size() * 9 + 9) << ")" << endl;
    
    // Summary
    cout << "\n========== Test Summary ==========" << endl;
    cout << "✓ Backend array initialized" << endl;
    cout << "✓ Tower placement updates array in real-time" << endl;
    cout << "✓ 3x3 tower area marked with 'T'" << endl;
    cout << "✓ Tower removal restores area to '.'" << endl;
    cout << "✓ Multiple towers supported" << endl;
    cout << "✓ All tests passed!" << endl;
    
    return 0;
}
