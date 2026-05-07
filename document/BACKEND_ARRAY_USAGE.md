# Backend 2D Array System

## Overview
A separate 2D char array (147×33) is now available for backend game logic processing, independent from the display rendering system.

## Array Symbols

| Symbol | Meaning | Purpose |
|--------|---------|---------|
| `+` | Spawn Point | Where mobs enter the map |
| `.` | Tower Buildable | Where towers can be placed |
| `B` | Base | Player's base position |
| ` ` (space) | Road/Path | Walkable routes for mobs |
| ` ` (empty) | Blocked Terrain | Inaccessible areas |

## Function Signature

```cpp
void loadMapToArray(int level, char stateArray[33][147])
```

## Usage Example

```cpp
// In your game logic code:
GameMapManager mapManager;

// Create 2D array for backend processing
char gameStateArray[33][147];

// Load map data into array (for level 1)
mapManager.loadMapToArray(1, gameStateArray);

// Now use the array for backend calculations
// Example: Check if position is buildable
if (gameStateArray[row][col] == '.') {
    // Position is buildable
}

// Example: Check if position is spawn point
if (gameStateArray[row][col] == '+') {
    // This is a spawn point
}

// Example: Check if position is base
if (gameStateArray[row][col] == 'B') {
    // This is the base
}

// Example: Check if position is road/path
if (gameStateArray[row][col] == ' ') {
    // This is a road or blocked terrain
}
```

## Integration with Game Loop

The backend array is independent from the display-focused `GameMap` structure:
- `GameMap.grid[][]` - Contains Tile objects for rendering
- `gameStateArray[][]` - Contains char symbols for backend logic

This separation allows:
- Cleaner backend game logic (no rendering concerns)
- Faster lookups for path validation, tower placement checks
- Future implementation of collision detection, spawn validation, tower range queries

## Supported Levels
- Level 1: Demo mode
- Levels 2-10: Main game levels
- Each level loads from `data/maps/map_for_levelX.txt`

## Next Steps
With this backend array in place, you can now implement:
1. Tower placement validation functions
2. Mob spawn point detection
3. Base location queries
4. Path validation for AI movement
5. Tower range queries
