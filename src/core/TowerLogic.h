#ifndef TOWERLOGIC_H
#define TOWERLOGIC_H

#include "GameData.h"
#include "MobSystem.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

// ============ TOWER LOGIC SYSTEM ============
// Each tower type has specialized attack logic based on its properties

class TowerLogicSystem {
public:
    // Utility: Check if mob is within attack range (circular)
    static bool isInRange(int mobRow, int mobCol, int towerRow, int towerCol, int rangeRadius) {
        int rowDist = abs(mobRow - towerRow);
        int colDist = abs(mobCol - towerCol);
        return rowDist <= rangeRadius && colDist <= rangeRadius;
    }

    // Utility: Check if mob is within 3-row laser range
    static bool isInLaserRange(int mobRow, int mobCol, int towerRow, int towerCol) {
        // Laser covers 3 rows: towerRow-1, towerRow, towerRow+1
        // Any column
        return mobRow >= towerRow - 1 && mobRow <= towerRow + 1;
    }

    // ============ 1. ARROW TOWER ============
    // Basic single-target tower
    // Cost: $50 | Damage: 100 | Speed: 1.0s | Range: 9x9
    // Attacks closest target within 9x9 range
    static MobInstance* selectArrowTarget(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        MobInstance* bestTarget = nullptr;
        double bestDistance = 1e18;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 4)) continue;  // 9x9 = radius 4
            
            double distance = sqrt(pow(mobRow - towerRow, 2) + pow(mobCol - towerCol, 2));
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = &mob;
            }
        }
        
        return bestTarget;
    }

    static int getArrowDamage(MobInstance& target) {
        return 100;  // Fixed damage
    }

    // ============ 2. LASER TOWER ============
    // Line attack tower - hits entire 3 rows
    // Cost: $100 | Damage: 50 | Speed: 2.0s | Range: 3x∞
    // Attacks all enemies in 3 rows it occupies
    static vector<MobInstance*> selectLaserTargets(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        vector<MobInstance*> targets;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            
            // Check if mob is in one of the 3 rows (towerRow-1, towerRow, towerRow+1)
            if (mobRow >= towerRow - 1 && mobRow <= towerRow + 1) {
                targets.push_back(&mob);
            }
        }
        
        return targets;
    }

    static int getLaserDamage(MobInstance& target) {
        return 50;  // Fixed damage per target
    }

    // ============ 3. FROST TOWER ============
    // Crowd control tower - AOE slow
    // Cost: $50 | Damage: 10 | Speed: 1.0s | Range: 7x7
    // Hits in 7x7 range, applies 50% slowdown for 3 seconds
    static vector<MobInstance*> selectFrostTargets(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        vector<MobInstance*> targets;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 3)) continue;  // 7x7 = radius 3
            
            targets.push_back(&mob);
        }
        
        return targets;
    }

    static int getFrostDamage(MobInstance& target) {
        return 10;  // Low damage - main effect is slowdown
    }

    static void applyFrostSlow(MobInstance& target, double gameTime, double slowDuration = 3.0) {
        target.modifier.speedMultiplier = 0.5;  // 50% slowdown
        target.modifier.isSlowed = true;
        target.modifier.slowedUntilTime = gameTime + slowDuration;
    }

    // ============ 4. EARTHQUAKE TOWER ============
    // AOE damage tower
    // Cost: $200 | Damage: 100 | Speed: 1.0s | Range: 7x7 AOE
    // Hits all targets in 7x7 range with fixed damage
    static vector<MobInstance*> selectEarthquakeTargets(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        vector<MobInstance*> targets;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 3)) continue;  // 7x7 = radius 3
            
            targets.push_back(&mob);
        }
        
        return targets;
    }

    static int getEarthquakeDamage(MobInstance& target) {
        return 100;  // Fixed damage
    }

    // ============ 5. ARMOR PENETRATION TOWER ============
    // Armor breaking tower
    // Cost: $150 | Damage: 150 (x2 vs armor) | Speed: 1.0s | Range: 8x8
    // Attacks closest target, deals 2x damage if target has armor
    static MobInstance* selectArmorTarget(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        MobInstance* bestTarget = nullptr;
        double bestDistance = 1e18;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 4)) continue;  // 8x8 = radius 4
            
            double distance = sqrt(pow(mobRow - towerRow, 2) + pow(mobCol - towerCol, 2));
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = &mob;
            }
        }
        
        return bestTarget;
    }

    static int getArmorPenetrationDamage(MobInstance& target) {
        int baseDamage = 150;
        // Double damage if target has armor (armor > 0)
        return (target.modifier.armorBonus > 0) ? baseDamage * 2 : baseDamage;
    }

    // ============ 6. WAR DRUM TOWER ============
    // Buff tower - boosts nearby towers
    // Cost: $150 | Damage: 0 | Speed: N/A | Range: 5x5 buff area
    // Doesn't attack, instead buffs all towers in 5x5 area with +25% attack speed
    static vector<pair<int, int>> selectBuffedTowerCenters(const vector<pair<int, int>>& allTowerCenters, 
                                                           int drumRow, int drumCol) {
        vector<pair<int, int>> buffedTowers;
        
        for (const auto& towerCenter : allTowerCenters) {
            int towerRow = towerCenter.first;
            int towerCol = towerCenter.second;
            
            // Buff towers in 5x5 range (radius 2)
            if (isInRange(towerRow, towerCol, drumRow, drumCol, 2)) {
                buffedTowers.push_back(towerCenter);
            }
        }
        
        return buffedTowers;
    }

    static double getWarDrumSpeedBonus() {
        return 1.25;  // +25% attack speed = 1.25x multiplier
    }

    // ============ 7. HELL TOWER ============
    // Percentage damage tower - targets highest HP
    // Cost: $200 | Damage: 50% of current HP | Speed: 1.0s | Range: 9x9
    // Attacks highest-HP target in 9x9 range with percentage damage
    static MobInstance* selectHellTarget(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        MobInstance* bestTarget = nullptr;
        int highestHealth = 0;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 4)) continue;  // 9x9 = radius 4
            
            if (mob.health > highestHealth) {
                highestHealth = mob.health;
                bestTarget = &mob;
            }
        }
        
        return bestTarget;
    }

    static int getHellDamage(MobInstance& target) {
        int damage = (int)(target.health * 0.5);  // 50% of current HP
        if (damage < 50) damage = 50;      // Minimum 50
        if (damage > 500) damage = 500;    // Maximum 500
        
        // Armored enemies take only 50 damage (minimum)
        if (target.modifier.armorBonus > 0) {
            damage = 50;
        }
        
        return damage;
    }

    // ============ 8. THIEF TOWER ============
    // Gold multiplication tower
    // Cost: $200 | Damage: 50 | Speed: 1.0s | Range: 5x5
    // Attacks closest target, grants +50% gold bonus on kills from this tower
    static MobInstance* selectThiefTarget(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        MobInstance* bestTarget = nullptr;
        double bestDistance = 1e18;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 2)) continue;  // 5x5 = radius 2
            
            double distance = sqrt(pow(mobRow - towerRow, 2) + pow(mobCol - towerCol, 2));
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = &mob;
            }
        }
        
        return bestTarget;
    }

    static int getThiefDamage(MobInstance& target) {
        return 50;  // Base damage
    }

    static int getThiefGoldBonus(int baseGold) {
        return (int)(baseGold * 1.5);  // +50% gold = 1.5x multiplier
    }

    // ============ 9. VAMPIRE TOWER ============
    // Lifesteal tower - heals base on kills
    // Cost: $400 | Damage: 100 | Speed: 1.0s | Range: 6x6
    // Attacks closest target, heals base 1 HP for every 10 enemies killed
    static MobInstance* selectVampireTarget(vector<MobInstance>& mobs, int towerRow, int towerCol) {
        MobInstance* bestTarget = nullptr;
        double bestDistance = 1e18;
        
        for (auto& mob : mobs) {
            if (!mob.isAlive) continue;
            
            int mobRow = (int)round(mob.posRow);
            int mobCol = (int)round(mob.posCol);
            
            if (!isInRange(mobRow, mobCol, towerRow, towerCol, 3)) continue;  // 6x6 = radius 3
            
            double distance = sqrt(pow(mobRow - towerRow, 2) + pow(mobCol - towerCol, 2));
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = &mob;
            }
        }
        
        return bestTarget;
    }

    static int getVampireDamage(MobInstance& target) {
        return 100;  // Base damage
    }

    static int getVampireHeal(int killCount) {
        return killCount / 10;  // 1 HP per 10 kills
    }

    // ============ HELPER: Get tower by name ============
    static int getTowerTypeByName(const string& name) {
        if (name.find("Arrow") != string::npos) return 0;
        if (name.find("Laser") != string::npos) return 1;
        if (name.find("Frost") != string::npos) return 2;
        if (name.find("Earthquake") != string::npos) return 3;
        if (name.find("Armor Penetration") != string::npos) return 4;
        if (name.find("War Drum") != string::npos) return 5;
        if (name.find("Hell") != string::npos) return 6;
        if (name.find("Thief") != string::npos) return 7;
        if (name.find("Vampire") != string::npos) return 8;
        return -1;  // Unknown tower
    }
};

#endif // TOWERLOGIC_H
