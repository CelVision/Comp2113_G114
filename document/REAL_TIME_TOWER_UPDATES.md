# Real-Time Backend Array Tower Updates

## Overview

Towers placed on the map are now **automatically synchronized** with the backend array in real-time:
- When a tower is **placed**: The 3x3 area is immediately marked with `'T'` in the backend array
- When a tower is **sold**: The 3x3 area is immediately restored to `'.'` (buildable)

This ensures the backend game logic always has up-to-date tower information for pathfinding, collision detection, and game decisions.

---

## How It Works

### Backend Array Symbol Updates

| Event | Symbol | Meaning |
|-------|--------|---------|
| Tower placed | `'T'` | Tower occupies this cell |
| Tower sold | `'.'` | Cell returns to buildable state |
| Spawn point | `'+'` | Mob spawn location |
| Base | `'B'` | Player base location |
| Road | `' '` | Walkable path |

### Tower Placement Logic

When a player places a tower at center position **(row, col)**:

1. **Display system** updates: `GameMap.grid[][]` shows tower graphics
2. **Backend system** updates: `gameStateArray[][]` marks 3×3 area with `'T'`
   - Cells affected: (row-1,col-1) to (row+1,col+1)
   - All 9 cells marked immediately

### Tower Removal Logic

When a player sells a tower at center position **(row, col)**:

1. **Display system** updates: `GameMap.grid[][]` clears tower graphics
2. **Backend system** updates: `gameStateArray[][]` restores 3×3 area to `'.'`
   - Cells affected: (row-1,col-1) to (row+1,col+1)
   - All 9 cells restored immediately

---

## Function Signatures

### Place Tower in Backend
```cpp
void updateBackendArrayTowerPlaced(char stateArray[33][147], 
                                   int centerRow, 
                                   int centerCol)
```
- Marks the entire 3×3 area with `'T'`
- Called immediately after `placeTowerAt()` in display system
- Safe to call multiple times (overwrites existing values)

### Remove Tower from Backend
```cpp
void updateBackendArrayTowerSold(char stateArray[33][147], 
                                 int centerRow, 
                                 int centerCol)
```
- Restores the entire 3×3 area to `'.'` (buildable)
- Called when tower is sold
- Safe to call multiple times (overwrites existing values)

---

## Integration Points in Game.cpp

### 1. Initialization
```cpp
// Create backend array at start of displayGameScreen()
char gameStateArray[33][147];
mapManager.loadMapToArray(levelSelected, gameStateArray);
```

### 2. Demo Tower Placement
```cpp
// When placing demo towers
mapManager.placeTowerAt(spot.first, spot.second, 0);
mapManager.updateBackendArrayTowerPlaced(gameStateArray, spot.first, spot.second);
```

### 3. Player Tower Placement
```cpp
// When player confirms tower placement
mapManager.placeTowerAt(selRow, selCol, previewTowerIndex);
mapManager.updateBackendArrayTowerPlaced(gameStateArray, selRow, selCol);
money -= towers[previewTowerIndex].cost;
```

### 4. Tower Selling
```cpp
// When player sells a tower
money += sellTowerPrice;

// Clear display
for (int i = selRow - 1; i <= selRow + 1; i++) {
    for (int j = selCol - 1; j <= selCol + 1; j++) {
        gameMap.grid[i][j].type = BUILDABLE;
        // ...
    }
}

// Update backend immediately
mapManager.updateBackendArrayTowerSold(gameStateArray, selRow, selCol);
```

---

## Real-Time Updates Example

### Sequence of Operations

```
Initial state:
- Backend array loaded with map data
- All buildable areas marked as '.'
- Spawn points marked as '+'
- Base marked as 'B'

Player places Arrow Tower at (10, 20):
1. displayGameScreen() calls placeTowerAt(10, 20, 0)
   → Display system updates GameMap.grid[][]
2. displayGameScreen() calls updateBackendArrayTowerPlaced(gameStateArray, 10, 20)
   → Backend system updates gameStateArray[][]
   → All 9 cells in 3×3 area now marked as 'T'

Backend game logic can now query:
- if (gameStateArray[10][20] == 'T') → Tower exists here
- Can use for collision detection, pathfinding adjustments

Player sells the tower:
1. Tower removal clears GameMap.grid[][]
2. updateBackendArrayTowerSold(gameStateArray, 10, 20) called
   → All 9 cells restored to '.'
```

---

## Test Results

✅ **Backend Array Symbol Updates**
- Towers marked with `'T'` upon placement
- 3×3 area fully occupied by tower
- Immediate synchronization (no delay)

✅ **Tower Removal**
- Area restored to `'.'` when sold
- All 9 cells correctly restored
- Multiple towers handled independently

✅ **Multiple Towers**
- Each tower's 3×3 area updated separately
- No conflicts between tower areas
- Array remains consistent

✅ **Example Output**
```
Before placement: . . . . . . . . . .
After placement:  . . . T T T . . . .
                  . . . T T T . . . .
                  . . . T T T . . . .
After removal:    . . . . . . . . . .
```

---

## Benefits

### 1. **Real-Time Game Logic**
- Backend always reflects current tower positions
- No stale data or synchronization issues
- Immediate collision detection updates

### 2. **Decoupled Architecture**
- Display system (Game.cpp) independent from logic (backend array)
- Easy to add new game features without affecting renderer
- Clean separation of concerns

### 3. **Performance**
- O(9) operation per tower placement/removal
- No full array rescans needed
- Efficient for real-time updates

### 4. **Future Features**
- Tower attack range queries using backend array
- Mob pathfinding adjustments based on tower positions
- Real-time game state analysis for AI decision-making

---

## Current Tower Count in Backend

After placement/removal operations, query the backend array:

```cpp
int countTowers(const char stateArray[33][147]) {
    int count = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 147; j++) {
            if (stateArray[i][j] == 'T') count++;
        }
    }
    return count / 9;  // Each tower occupies 9 cells
}
```

---

## Next Steps

With real-time tower tracking, you can now implement:

1. **Dynamic Pathfinding**: Adjust mob paths if towers block optimal routes
2. **Tower Range Queries**: Check which mobs are in tower attack range using backend array
3. **Resource Management**: Track tower density and placement efficiency
4. **Game Analytics**: Monitor tower placement patterns
5. **Advanced AI**: Mob behavior changes based on tower positions
