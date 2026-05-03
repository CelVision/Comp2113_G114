#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>

using namespace std;

// Constants for UI dimensions
const int MAP_ROWS = 33;
const int MAP_COLS = 99;
const int UPPER_UI_LINES = 3;

// Tower information structure
struct Tower {
    char symbol;
    string name;
    int cost;
    string art[3];
};

// Initialize tower information
void initializeTowers(vector<Tower>& towers) {
    towers.resize(9);

    // 1. Arrow Tower
    towers[0] = {'A', "Arrow Tower", 50, {"<->", "| |", "==="}};

    // 2. Laser Tower
    towers[1] = {'L', "Laser Tower", 100, {"<o>", "| |", "^^^"}};

    // 3. Frost Tower
    towers[2] = {'F', "Frost Tower", 50, {"/*\\", "| |", "^^^"}};

    // 4. Earthquake Tower
    towers[3] = {'E', "Earthquake Tower", 200, {"<!>", "| |", "^^^"}};

    // 5. Hell Tower
    towers[4] = {'H', "Hell Tower", 200, {"<%>", "| |", "==="}};

    // 6. Thief Tower
    towers[5] = {'T', "Thief Tower", 200, {"<$>", "| |", "..."}};

    // 7. Armor Penetration Tower
    towers[6] = {'P', "Armor Penetration Tower", 150, {"<^>", "| |", "==="}};

    // 8. War Drum Tower
    towers[7] = {'D', "War Drum Tower", 150, {"[ ]", "| |", "..."}};

    // 9. Vampire Tower
    towers[8] = {'V', "Vampire Tower", 400, {"<+>", "| |", "==="}};
}

// Display the upper UI section
void displayUpperUI(string playerName, int level, int currentWave, int totalWaves, 
                    int money, int currentHP, int maxHP) {
    // Line 1: Player info
    string line1 = playerName;
    line1 += string(32 - playerName.length(), ' ');
    line1 += "Level " + to_string(level) + " --- Wave " + to_string(currentWave) + "/" + to_string(totalWaves);
    
    // Pad to make room for money and HP info
    while (line1.length() < 75) {
        line1 += " ";
    }
    line1 += "Money: " + to_string(money);
    while (line1.length() < 95) {
        line1 += " ";
    }
    line1 += "HP: " + to_string(currentHP) + "/" + to_string(maxHP);
    
    cout << line1.substr(0, MAP_COLS) << endl;
    
    // Line 2: Dividing line
    for (int i = 0; i < MAP_COLS; i++) {
        cout << "-";
    }
    cout << endl;
}

// Display the game map (placeholder)
void displayGameMap() {
    // Display 27 rows of empty map (33 - 3 upper - 3 lower)
    for (int i = 0; i < 27; i++) {
        for (int j = 0; j < MAP_COLS; j++) {
            cout << " ";
        }
        cout << endl;
    }
}

// Display the lower UI section with tower options
void displayLowerUI(vector<Tower>& towers) {
    // Dividing line
    for (int i = 0; i < MAP_COLS; i++) {
        cout << "-";
    }
    cout << endl;
    
    // Tower display line 1
    for (int i = 0; i < 9; i++) {
        cout << " " << towers[i].art[0] << "  ";
    }
    cout << "                     " << endl;
    cout << " Tab Q to check tower details" << endl;
    
    // Tower display line 2
    for (int i = 0; i < 9; i++) {
        cout << " " << towers[i].art[1] << "  ";
    }
    cout << endl;
    
    // Tower display line 3
    for (int i = 0; i < 9; i++) {
        cout << " " << towers[i].art[2] << "  ";
    }
    cout << "                     " << endl;
    cout << " Tab E to check monster details" << endl;
    
    // Tower numbers and costs
    for (int i = 0; i < 9; i++) {
        cout << "  " << (i + 1) << "   ";
    }
    cout << endl;
    
    for (int i = 0; i < 9; i++) {
        cout << " $" << setw(2) << towers[i].cost << "  ";
    }
    cout << endl;
}

// Display complete UI
void displayFullUI(string playerName, int level, int currentWave, int totalWaves,
                   int money, int currentHP, int maxHP, vector<Tower>& towers) {
    // Clear console (Windows specific)
    system("cls");
    
    displayUpperUI(playerName, level, currentWave, totalWaves, money, currentHP, maxHP);
    displayGameMap();
    displayLowerUI(towers);
}

// Main function
int main() {
    // Initialize towers
    vector<Tower> towers;
    initializeTowers(towers);
    
    // Example display with sample data
    string playerName = "Player_Name";
    int level = 1;
    int currentWave = 1;
    int totalWaves = 5;
    int money = 50000;
    int currentHP = 10;
    int maxHP = 10;
    
    // Display the UI
    displayFullUI(playerName, level, currentWave, totalWaves, money, currentHP, maxHP, towers);
    
    // Keep console open
    cout << "\nPress Enter to exit...";
    cin.get();
    
    return 0;
}
