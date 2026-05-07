# Backend Path System Documentation

## Overview
The backend path system uses the 2D char array (147×33) to:
1. **Spawn mobs** at spawn points in adjacent walkable space
2. **Calculate shortest paths** from spawn points to base using Dijkstra's algorithm
3. **Generate detailed path commands** like "go left 10 tiles, go down 3 tiles"

This system provides a clean separation between game logic and rendering.

## Function 1: `spawnMobAtSpawnPoint()`

### Purpose
Finds an adjacent walkable space (empty cell) next to a spawn point where a mob can spawn.

### Signature
```cpp
pair<int, int> spawnMobAtSpawnPoint(const char stateArray[33][147], 
                                     int spawnRow, 
                                     int spawnCol)
```

### Parameters
- `stateArray[33][147]`: Backend array with symbols ('+', '.', 'B', ' ')
- `spawnRow`: Row coordinate of spawn point
- `spawnCol`: Column coordinate of spawn point

### Returns
- `pair<int, int>`: (row, col) of spawn position if found
- `(-1, -1)`: If no adjacent empty space available

### Implementation Details
- Checks 4 adjacent cells: up, down, left, right
- Only accepts cells with ' ' (space = walkable/road)
- Returns first available empty neighbor found

### Usage Example
```cpp
char gameStateArray[33][147];
mapManager.loadMapToArray(1, gameStateArray);

// For each spawn point (+) found in the array:
if (gameStateArray[row][col] == '+') {
    pair<int, int> mobSpawnPos = mapManager.spawnMobAtSpawnPoint(gameStateArray, row, col);
    if (mobSpawnPos.first != -1) {
        // Spawn mob at (mobSpawnPos.first, mobSpawnPos.second)
        double spawnRow = (double)mobSpawnPos.first;
        double spawnCol = (double)mobSpawnPos.second;
        // Create MobInstance with these coordinates
    }
}
```

---

## Function 2: `loadPathsFromBackendArray()`

### Purpose
Scans backend array for all spawn points and the base, then computes shortest paths using Dijkstra's algorithm.

### Signature
```cpp
map<pair<int, int>, vector<PathCommand>> loadPathsFromBackendArray(
    const char stateArray[33][147]
)
```

### Returns
- `map<pair<int,int>, vector<PathCommand>>`: Mapping from spawn point coordinates to path commands
  - Key: `(spawnRow, spawnCol)` coordinate
  - Value: Vector of PathCommand structures with detailed movement instructions

### PathCommand Structure
```cpp
struct PathCommand {
    pair<int, int> startPos;        // Starting position of this command segment
    pair<int, int> endPos;          // Ending position of this command segment
    string description;              // e.g., "go right 10 tiles", "go down 3 tiles"
};
```

### Algorithm Details
- **Walkable cells**: `' '` (space) and `'B'` (base)
- **Non-walkable**: `'+'` (spawn), `'.'` (buildable), all others
- **Dijkstra properties**:
  - Uses 4-directional movement (up, down, left, right)
  - Each move has cost of 1
  - Returns shortest path (minimum tile count)
  - Handles multiple spawn points independently

### Usage Example
```cpp
char gameStateArray[33][147];
mapManager.loadMapToArray(1, gameStateArray);

// Get all paths
map<pair<int, int>, vector<GameMapManager::PathCommand>> allPaths = 
    mapManager.loadPathsFromBackendArray(gameStateArray);

// For each spawn point
for (const auto& pathEntry : allPaths) {
    pair<int, int> spawn = pathEntry.first;
    const vector<GameMapManager::PathCommand>& commands = pathEntry.second;
    
    cout << "Spawn at (" << spawn.first << ", " << spawn.second << ")" << endl;
    for (const auto& cmd : commands) {
        cout << "  " << cmd.description << endl;
        // Use cmd.startPos and cmd.endPos for navigation
    }
}
```

---

## Function 3: `dijkstraShortestPath()` (Helper)

### Purpose
Core algorithm that computes shortest path between two points in the backend array.

### Signature
```cpp
vector<pair<int, int>> dijkstraShortestPath(const char stateArray[33][147],
                                             pair<int, int> start,
                                             pair<int, int> goal)
```

### Returns
- `vector<pair<int, int>>`: Sequence of coordinates from start to goal
- Empty vector if no path exists

### Features
- Uses priority queue for efficient exploration
- Skips already-visited nodes with worse distances
- Reconstructs path by backtracking through parent pointers

---

## Function 4: `generatePathCommands()` (Helper)

### Purpose
Converts a sequence of coordinates into high-level movement commands.

### Signature
```cpp
vector<PathCommand> generatePathCommands(const vector<pair<int, int>>& path)
```

### Returns
- `vector<PathCommand>`: Sequence of movement commands

### Command Generation Strategy
- Groups consecutive same-direction movements
- Generates descriptions like "go right 10 tiles", "go down 5 tiles"
- Merges adjacent horizontal/vertical movements into single commands

### Example
Path: `[(0,5), (0,6), (0,7), (0,8), (1,8), (1,9)]`

Generated commands:
1. "go right 3 tiles" (from (0,5) to (0,8))
2. "go down 1 tile" (from (0,8) to (1,8))
3. "go right 1 tile" (from (1,8) to (1,9))

---

## Integration with Mob System

### Step 1: Load Backend Array
```cpp
char gameStateArray[33][147];
mapManager.loadMapToArray(levelNum, gameStateArray);
```

### Step 2: Compute Paths for All Spawn Points
```cpp
auto pathMap = mapManager.loadPathsFromBackendArray(gameStateArray);
```

### Step 3: When Spawning a Mob
```cpp
// Find spawn point's adjacent position
pair<int, int> mobSpawnPos = mapManager.spawnMobAtSpawnPoint(gameStateArray, spawnRow, spawnCol);

// Create mob instance
MobInstance mob(mobType, (double)mobSpawnPos.first, (double)mobSpawnPos.second, speed, waveId);

// Assign path commands from the precomputed map
auto it = pathMap.find(make_pair(spawnRow, spawnCol));
if (it != pathMap.end()) {
    mob.pathCommands = it->second;  // Store for movement logic
}
```

### Step 4: Use Commands in Mob Movement Logic
```cpp
// In your mob update logic:
for (const auto& cmd : mob.pathCommands) {
    // Parse "go [direction] [count] tiles"
    // Move mob accordingly
}
```

---

## Test Results

Backend array for Level 1:
- **3 spawn points** at: (1,0), (2,0), (3,0)
- **1 base** at: (21,31)
- **Paths computed** using Dijkstra:
  - Spawn (1,0) → 4 commands: right 140, down 19, left 109, down 1
  - Spawn (2,0) → 4 commands: right 140, down 18, left 109, down 1
  - Spawn (3,0) → 4 commands: right 140, down 17, left 109, down 1

---

## Array Symbol Reference

| Symbol | Meaning | Walkable? |
|--------|---------|-----------|
| `+` | Spawn point | No (but adjacent space is walkable) |
| `.` | Tower buildable | No |
| `B` | Base | Yes |
| ` ` | Road/path | Yes |
| Other | Blocked | No |

---

## Notes

- **Efficiency**: Dijkstra precomputes all paths once per level load
- **Scalability**: Works for any number of spawn points up to 147×33 map size
- **Accuracy**: Finds optimal (shortest) paths to base
- **Flexibility**: Commands can be easily parsed and followed by any movement system
