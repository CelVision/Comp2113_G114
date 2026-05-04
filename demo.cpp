#include <windows.h>
#include <conio.h>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "src/core/GameData.h"

using namespace std;

const int COLOR_YELLOW = 14;
const int COLOR_GREEN = 10;
const int COLOR_RED = 12;
const int COLOR_CYAN = 11;
const int COLOR_WHITE = 15;
const int COLOR_GRAY = 8;

static void setTextColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

static void resetTextColor() {
    setTextColor(COLOR_WHITE);
}

struct Point {
    int row;
    int col;
};

struct SpawnEvent {
    double spawnTime;
    int spawnRow;
};

struct ArrowTower {
    int row;
    int col;
    int damage;
    double range;
    double attackInterval;
    double cooldown;
    double flashUntil;
    string art[3];
};

struct PigmanUnit {
    int spawnRow;
    int hp;
    double speed;
    bool alive;
    bool reachedBase;
    double flashUntil;
    vector<Point> route;
    size_t routeIndex;
    double posRow;
    double posCol;

    PigmanUnit()
        : spawnRow(0), hp(200), speed(2.0), alive(true), reachedBase(false),
          flashUntil(0.0), routeIndex(0), posRow(0.0), posCol(0.0) {}
};

struct ArrowTowerSpec {
    char symbol;
    string name;
    int cost;
    int damage;
    double attackSpeed;
    int hitRange;
    string art[3];
};

struct PigmanSpec {
    char symbol;
    string name;
    int hp;
    double speed;
    int gold;
};

static vector<Point> buildRouteForSpawnRow(int spawnRow);

static string trimLeftCopy(const string& text) {
    size_t index = 0;
    while (index < text.size() && isspace(static_cast<unsigned char>(text[index]))) {
        index++;
    }
    return text.substr(index);
}

static string trimCopy(const string& text) {
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && isspace(static_cast<unsigned char>(text[begin]))) begin++;
    while (end > begin && isspace(static_cast<unsigned char>(text[end - 1]))) end--;
    return text.substr(begin, end - begin);
}

static bool startsWithTrimmed(const string& line, const string& prefix) {
    return trimLeftCopy(line).find(prefix) == 0;
}

static int parseIntAfterColon(const string& line) {
    size_t colon = line.find(':');
    if (colon == string::npos) return 0;
    string value = trimCopy(line.substr(colon + 1));
    if (!value.empty() && value[0] == '$') value.erase(value.begin());
    size_t endPos = 0;
    while (endPos < value.size() && (isdigit(static_cast<unsigned char>(value[endPos])) || value[endPos] == '-')) {
        endPos++;
    }
    return stoi(value.substr(0, endPos));
}

static double parseDoubleAfterColon(const string& line) {
    size_t colon = line.find(':');
    if (colon == string::npos) return 0.0;
    string value = trimCopy(line.substr(colon + 1));
    size_t endPos = 0;
    while (endPos < value.size() && (isdigit(static_cast<unsigned char>(value[endPos])) || value[endPos] == '-' || value[endPos] == '.')) {
        endPos++;
    }
    return stod(value.substr(0, endPos));
}

static double parseDoubleWithSuffix(const string& text) {
    string value = trimCopy(text);
    size_t endPos = 0;
    while (endPos < value.size() && (isdigit(static_cast<unsigned char>(value[endPos])) || value[endPos] == '-' || value[endPos] == '.')) {
        endPos++;
    }
    return stod(value.substr(0, endPos));
}

static int parseCountBeforeToken(const string& line, const string& token) {
    size_t tokenPos = line.find(token);
    if (tokenPos == string::npos) return 1;
    size_t index = tokenPos;
    while (index > 0 && isspace(static_cast<unsigned char>(line[index - 1]))) {
        index--;
    }
    size_t end = index;
    while (index > 0 && isdigit(static_cast<unsigned char>(line[index - 1]))) {
        index--;
    }
    if (index == end) return 1;
    return stoi(line.substr(index, end - index));
}

static int parseRowNumber(const string& line) {
    size_t rowPos = line.find("row");
    if (rowPos == string::npos) return -1;
    size_t start = rowPos + 3;
    while (start < line.size() && !isdigit(static_cast<unsigned char>(line[start]))) {
        start++;
    }
    size_t end = start;
    while (end < line.size() && isdigit(static_cast<unsigned char>(line[end]))) {
        end++;
    }
    if (start >= end) return -1;
    return stoi(line.substr(start, end - start));
}

static ArrowTowerSpec loadArrowTowerSpec() {
    ArrowTowerSpec spec{};
    spec.symbol = 'A';
    spec.name = "Arrow Tower";
    spec.cost = 50;
    spec.damage = 100;
    spec.attackSpeed = 1.0;
    spec.hitRange = 5;
    spec.art[0] = "<->";
    spec.art[1] = "| |";
    spec.art[2] = "===";

    ifstream file(buildTowerDataPath());
    if (!file.is_open()) {
        return spec;
    }

    string line;
    bool inArrowSection = false;
    bool readingArt = false;
    int artLine = 0;

    while (getline(file, line)) {
        string trimmed = trimLeftCopy(line);
        if (trimmed.find("Tower 1: Arrow Tower") == 0) {
            inArrowSection = true;
            readingArt = false;
            artLine = 0;
            continue;
        }
        if (inArrowSection && trimmed.find("Tower 2:") == 0) {
            break;
        }
        if (!inArrowSection) {
            continue;
        }

        if (trimmed.find("Symbol:") == 0) {
            size_t pos = trimmed.find(':');
            if (pos != string::npos && pos + 1 < trimmed.size()) {
                spec.symbol = trimLeftCopy(trimmed.substr(pos + 1))[0];
            }
        } else if (trimmed.find("Cost:") == 0) {
            spec.cost = parseIntAfterColon(trimmed);
        } else if (trimmed.find("damage:") == 0) {
            spec.damage = parseIntAfterColon(trimmed);
        } else if (trimmed.find("Attack Speed:") == 0) {
            string value = trimCopy(trimmed.substr(trimmed.find(':') + 1));
            if (!value.empty() && value.back() == 's') value.pop_back();
            spec.attackSpeed = parseDoubleWithSuffix(value);
        } else if (trimmed.find("Hit Range:") == 0) {
            spec.hitRange = parseIntAfterColon(trimmed);
        } else if (trimmed.find("ASCII Art:") == 0) {
            readingArt = true;
            artLine = 0;
        } else if (readingArt && artLine < 3) {
            spec.art[artLine] = trimLeftCopy(line);
            artLine++;
        }
    }

    return spec;
}

static PigmanSpec loadPigmanSpec() {
    PigmanSpec spec{};
    spec.symbol = 'p';
    spec.name = "Pigman";
    spec.hp = 200;
    spec.speed = 2.0;
    spec.gold = 10;

    ifstream file(buildMobDataPath());
    if (!file.is_open()) {
        return spec;
    }

    string line;
    bool inPigmanSection = false;
    while (getline(file, line)) {
        string trimmed = trimLeftCopy(line);
        if (trimmed.find("Enemy 1: Pigman") == 0) {
            inPigmanSection = true;
            continue;
        }
        if (inPigmanSection && trimmed.find("Enemy 2:") == 0) {
            break;
        }
        if (!inPigmanSection) {
            continue;
        }

        if (trimmed.find("Symbol:") == 0) {
            size_t pos = trimmed.find(':');
            if (pos != string::npos && pos + 1 < trimmed.size()) {
                spec.symbol = trimLeftCopy(trimmed.substr(pos + 1))[0];
            }
        } else if (trimmed.find("HP:") == 0) {
            spec.hp = parseIntAfterColon(trimmed);
        } else if (trimmed.find("Speed:") == 0) {
            spec.speed = parseDoubleAfterColon(trimmed);
        } else if (trimmed.find("Gold:") == 0) {
            spec.gold = parseIntAfterColon(trimmed);
        }
    }

    return spec;
}

static vector<SpawnEvent> loadLevel1Spawns() {
    vector<SpawnEvent> events;
    ifstream file(buildLevelDesignPath(1));
    if (!file.is_open()) {
        return events;
    }

    string line;
    double currentTime = 0.0;

    while (getline(file, line)) {
        string trimmed = trimLeftCopy(line);
        if (trimmed.empty() || trimmed[0] == '-' || trimmed[0] == '[') {
            continue;
        }

        if (trimmed.find("[wait") != string::npos) {
            size_t forPos = trimmed.find("for");
            if (forPos != string::npos) {
                string value = trimmed.substr(forPos + 3);
                size_t secPos = value.find("seconds");
                if (secPos != string::npos) {
                    value = value.substr(0, secPos);
                }
                currentTime += parseDoubleWithSuffix(value);
            }
            continue;
        }

        if (trimmed.find("pigman") != string::npos) {
            int spawnCount = parseCountBeforeToken(trimmed, "pigman");
            int spawnRow = parseRowNumber(trimmed);
            if (spawnRow < 0) {
                continue;
            }

            for (int i = 0; i < spawnCount; ++i) {
                SpawnEvent event;
                event.spawnTime = currentTime;
                event.spawnRow = spawnRow - 1;
                events.push_back(event);
            }
        }
    }

    sort(events.begin(), events.end(), [](const SpawnEvent& a, const SpawnEvent& b) {
        return a.spawnTime < b.spawnTime;
    });

    return events;
}

static vector<string> loadLevel1Map() {
    vector<string> rows;
    ifstream file(buildMapFilePath(1));
    if (!file.is_open()) {
        return rows;
    }

    string line;
    while (getline(file, line)) {
        if ((int)line.size() < GameMap::COLS) {
            line += string(GameMap::COLS - line.size(), ' ');
        }
        rows.push_back(line.substr(0, GameMap::COLS));
        if ((int)rows.size() >= GameMap::ROWS) {
            break;
        }
    }

    while ((int)rows.size() < GameMap::ROWS) {
        rows.push_back(string(GameMap::COLS, ' '));
    }

    return rows;
}

static vector<Point> buildFixedRouteForRow(int spawnRow) {
    vector<Point> route;
    int baseRow = 21;
    int baseCol = 31;
    int startCol = 0;

    int row = spawnRow;
    route.push_back({row, startCol});

    while (row != baseRow) {
        row += (row < baseRow) ? 1 : -1;
        route.push_back({row, startCol});
    }

    for (int col = startCol + 1; col <= baseCol; ++col) {
        route.push_back({baseRow, col});
    }

    return route;
}

static vector<Point> buildRouteForSpawnRow(int spawnRow) {
    if (spawnRow < 1) spawnRow = 1;
    if (spawnRow > 3) spawnRow = 3;
    return buildFixedRouteForRow(spawnRow);
}

static bool isRouteCell(int row, int col, const vector<Point>& route) {
    for (const auto& point : route) {
        if (point.row == row && point.col == col) {
            return true;
        }
    }
    return false;
}

static bool isBaseCell(int row, int col) {
    return row >= 20 && row <= 22 && col >= 30 && col <= 32;
}

static bool isSpawnCell(int row, int col) {
    return (col == 0 && row >= 1 && row <= 3);
}

static bool canPlaceTowerAt(const vector<string>& mapRows, int centerRow, int centerCol) {
    for (int r = centerRow - 1; r <= centerRow + 1; ++r) {
        for (int c = centerCol - 1; c <= centerCol + 1; ++c) {
            if (r < 0 || r >= GameMap::ROWS || c < 0 || c >= GameMap::COLS) {
                return false;
            }
            if (isBaseCell(r, c) || isSpawnCell(r, c)) {
                return false;
            }
            if (mapRows[r][c] == '#') {
                return false;
            }
        }
    }
    return true;
}

static Point chooseTowerPlacement(const vector<string>& mapRows, const vector<Point>& route) {
    // Fixed demo placement: near the vertical route so range 5 can hit pigmen.
    Point preferred{10, 5};
    if (canPlaceTowerAt(mapRows, preferred.row, preferred.col)) {
        return preferred;
    }

    for (int row = 5; row < GameMap::ROWS - 5; ++row) {
        for (int col = 4; col < 12; ++col) {
            if (canPlaceTowerAt(mapRows, row, col)) {
                double bestDist = 1e9;
                for (const auto& point : route) {
                    double dist = sqrt(pow((double)point.row - row, 2) + pow((double)point.col - col, 2));
                    bestDist = min(bestDist, dist);
                }
                if (bestDist <= 5.0) {
                    return {row, col};
                }
            }
        }
    }

    return preferred;
}

static void clearScreen() {
    system("cls");
}

static void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordinates;
    coordinates.X = x;
    coordinates.Y = y;
    SetConsoleCursorPosition(hConsole, coordinates);
}

static void showCursor(bool show) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = show;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

static void setConsoleSize() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = {160, 50};
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SMALL_RECT windowRect = {0, 0, 159, 49};
    SetConsoleWindowInfo(hConsole, TRUE, &windowRect);
}

static void printColoredChar(char ch, int color) {
    setTextColor(color);
    cout << ch;
    resetTextColor();
}

static void renderFrame(
    const vector<string>& mapRows,
    const ArrowTower& tower,
    const vector<PigmanUnit>& pigmen,
    double now,
    int baseHP,
    size_t spawnedCount,
    size_t totalSpawns
) {
    clearScreen();
    cout << "BYTE RUSH DEMO | Level 1 | demo mode" << endl;
    cout << "Arrow Tower damage: 100 | Range: 5 | Attack speed: 1/sec | Base HP: " << baseHP << endl;
    cout << "Pigmen: " << spawnedCount << "/" << totalSpawns << " spawned" << endl;
    cout << string(GameMap::COLS, '=') << endl;

    for (int row = 0; row < GameMap::ROWS; ++row) {
        for (int col = 0; col < GameMap::COLS; ++col) {
            char ch = mapRows[row][col];
            int color = COLOR_WHITE;

            if (isRouteCell(row, col, buildRouteForSpawnRow(1)) || isRouteCell(row, col, buildRouteForSpawnRow(2)) || isRouteCell(row, col, buildRouteForSpawnRow(3))) {
                if (row >= 1 && row <= 3 && col == 0) {
                    ch = '+';
                    color = COLOR_YELLOW;
                } else if (row == 21 && col == 31) {
                    ch = 'M';
                    color = COLOR_RED;
                } else {
                    if (col == 0) {
                        ch = '|';
                    } else {
                        ch = '-';
                    }
                    color = COLOR_YELLOW;
                }
            } else if (ch == '.') {
                color = COLOR_GRAY;
            } else if (ch == '#') {
                color = COLOR_WHITE;
            } else if (ch == 'M') {
                color = COLOR_RED;
            } else if (ch == '+') {
                color = COLOR_YELLOW;
            }

            if (row >= tower.row - 1 && row <= tower.row + 1 && col >= tower.col - 1 && col <= tower.col + 1) {
                int localRow = row - (tower.row - 1);
                int localCol = col - (tower.col - 1);
                char towerCh = tower.art[localRow][localCol];
                if (localRow == 0 && localCol == 1 && now < tower.flashUntil) {
                    printColoredChar('-', COLOR_YELLOW);
                } else {
                    printColoredChar(towerCh, COLOR_WHITE);
                }
                continue;
            }

            bool pigmanDrawn = false;
            for (const auto& pigman : pigmen) {
                if (!pigman.alive) {
                    continue;
                }
                int pr = (int)round(pigman.posRow);
                int pc = (int)round(pigman.posCol);
                if (pr == row && pc == col) {
                    printColoredChar('p', now < pigman.flashUntil ? COLOR_RED : COLOR_GREEN);
                    pigmanDrawn = true;
                    break;
                }
            }
            if (pigmanDrawn) {
                continue;
            }

            printColoredChar(ch, color);
        }
        cout << endl;
    }

    cout << string(GameMap::COLS, '=') << endl;
    cout << "Controls: Q to quit | Demo route is fixed, no pathfinding is used." << endl;
}

static void runDemo() {
    auto mapRows = loadLevel1Map();
    auto spawns = loadLevel1Spawns();
    ArrowTowerSpec towerSpec = loadArrowTowerSpec();
    PigmanSpec pigmanSpec = loadPigmanSpec();

    vector<vector<Point>> routes;
    routes.push_back(buildRouteForSpawnRow(1));
    routes.push_back(buildRouteForSpawnRow(2));
    routes.push_back(buildRouteForSpawnRow(3));

    Point towerPos = chooseTowerPlacement(mapRows, routes[0]);
    ArrowTower tower;
    tower.row = towerPos.row;
    tower.col = towerPos.col;
    tower.damage = towerSpec.damage;
    tower.range = 5.0;
    tower.attackInterval = 1.0;
    tower.cooldown = 0.0;
    tower.flashUntil = 0.0;
    tower.art[0] = towerSpec.art[0];
    tower.art[1] = towerSpec.art[1];
    tower.art[2] = towerSpec.art[2];

    vector<PigmanUnit> pigmen;
    size_t nextSpawnIndex = 0;
    int baseHP = 10;
    auto start = chrono::steady_clock::now();
    auto last = start;
    bool running = true;

    showCursor(false);
    while (running) {
        auto nowInstant = chrono::steady_clock::now();
        double now = chrono::duration<double>(nowInstant - start).count();
        double dt = chrono::duration<double>(nowInstant - last).count();
        last = nowInstant;
        if (dt < 0.0) dt = 0.0;
        if (dt > 0.1) dt = 0.1;

        while (nextSpawnIndex < spawns.size() && spawns[nextSpawnIndex].spawnTime <= now) {
            PigmanUnit pigman;
            pigman.spawnRow = spawns[nextSpawnIndex].spawnRow;
            pigman.hp = pigmanSpec.hp;
            pigman.speed = pigmanSpec.speed;
            pigman.route = routes[min(max(pigman.spawnRow, 1), 3) - 1];
            pigman.routeIndex = 0;
            pigman.posRow = (double)pigman.route[0].row;
            pigman.posCol = (double)pigman.route[0].col;
            pigmen.push_back(pigman);
            nextSpawnIndex++;
        }

        for (auto& pigman : pigmen) {
            if (!pigman.alive || pigman.reachedBase) {
                continue;
            }
            double remaining = pigman.speed * dt;
            while (remaining > 0.0 && pigman.routeIndex + 1 < pigman.route.size()) {
                Point target = pigman.route[pigman.routeIndex + 1];
                double dr = (double)target.row - pigman.posRow;
                double dc = (double)target.col - pigman.posCol;
                double dist = sqrt(dr * dr + dc * dc);
                if (dist <= 0.0001) {
                    pigman.routeIndex++;
                    continue;
                }
                if (remaining >= dist) {
                    pigman.posRow = (double)target.row;
                    pigman.posCol = (double)target.col;
                    pigman.routeIndex++;
                    remaining -= dist;
                } else {
                    pigman.posRow += dr / dist * remaining;
                    pigman.posCol += dc / dist * remaining;
                    remaining = 0.0;
                }
            }

            if (pigman.routeIndex + 1 >= pigman.route.size()) {
                pigman.alive = false;
                pigman.reachedBase = true;
                baseHP = max(0, baseHP - 1);
            }
        }

        if (tower.cooldown > 0.0) {
            tower.cooldown -= dt;
            if (tower.cooldown < 0.0) tower.cooldown = 0.0;
        }

        PigmanUnit* target = nullptr;
        double bestDistance = 1e9;
        for (auto& pigman : pigmen) {
            if (!pigman.alive || pigman.reachedBase) {
                continue;
            }
            double dr = pigman.posRow - tower.row;
            double dc = pigman.posCol - tower.col;
            double dist = sqrt(dr * dr + dc * dc);
            if (dist <= tower.range && dist < bestDistance) {
                bestDistance = dist;
                target = &pigman;
            }
        }

        if (target != nullptr && tower.cooldown <= 0.0) {
            target->hp -= tower.damage;
            target->flashUntil = now + 0.18;
            tower.flashUntil = now + 0.18;
            tower.cooldown = tower.attackInterval;
            if (target->hp <= 0) {
                target->alive = false;
            }
        }

        bool allDone = nextSpawnIndex >= spawns.size();
        for (const auto& pigman : pigmen) {
            if (pigman.alive && !pigman.reachedBase) {
                allDone = false;
                break;
            }
        }

        renderFrame(mapRows, tower, pigmen, now, baseHP, nextSpawnIndex, spawns.size());

        if (allDone) {
            cout << "Demo complete. Pigman reached base or was defeated." << endl;
            cout << "Press any key to exit..." << endl;
            _getch();
            break;
        }

        if (_kbhit()) {
            int key = _getch();
            if (key == 'q' || key == 'Q' || key == 27) {
                running = false;
            }
        }

        Sleep(16);
    }

    showCursor(true);
}

static void verifyDemoData() {
    auto mapRows = loadLevel1Map();
    auto spawns = loadLevel1Spawns();
    ArrowTowerSpec towerSpec = loadArrowTowerSpec();
    PigmanSpec pigmanSpec = loadPigmanSpec();
    cout << "mapRows=" << mapRows.size() << endl;
    cout << "spawns=" << spawns.size() << endl;
    cout << "tower=" << towerSpec.name << ", damage=" << towerSpec.damage << ", range=" << towerSpec.hitRange << endl;
    cout << "pigman=" << pigmanSpec.name << ", hp=" << pigmanSpec.hp << ", speed=" << pigmanSpec.speed << endl;
}

int main(int argc, char** argv) {
    system("chcp 65001 > nul");
    system("title BYTE RUSH DEMO");
    setConsoleSize();

    if (argc > 1 && string(argv[1]) == "--verify") {
        verifyDemoData();
        return 0;
    }

    runDemo();
    return 0;
}
