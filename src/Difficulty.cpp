#include "rpg/Difficulty.h"
#include <algorithm>

namespace rpg {

    const std::unordered_map<Difficulty, DifficultyModifiers, DifficultyHash>
        DifficultyManager::modifiers_ = {
            {Difficulty::Easy, DifficultyModifiers(
                0.7, 0.7, 1.3, 1.2, 1.2, 0.8, 1.3,
                "Лёгкий",
                "Враги слабее, больше наград, чаще находите предметы"
            )},
            {Difficulty::Normal, DifficultyModifiers(
                1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                "Нормальный",
                "Сбалансированный опыт для всех игроков"
            )},
            {Difficulty::Hard, DifficultyModifiers(
                1.4, 1.3, 0.8, 1.3, 1.3, 1.2, 0.8,
                "Сложный",
                "Враги опаснее, но награды выше"
            )},
            {Difficulty::Nightmare, DifficultyModifiers(
                2.0, 1.6, 0.6, 1.5, 1.5, 1.4, 0.6,
                "Кошмар",
                "Только для опытных! Враги очень опасны"
            )}
    };

    DifficultyManager& DifficultyManager::instance() {
        static DifficultyManager inst;
        return inst;
    }

    DifficultyManager::DifficultyManager() {}

    void DifficultyManager::setDifficulty(Difficulty diff) {
        currentDifficulty_ = diff;
    }

    std::string DifficultyManager::getDifficultyName() const {
        auto it = modifiers_.find(currentDifficulty_);
        return it != modifiers_.end() ? it->second.displayName : "Неизвестно";
    }

    std::string DifficultyManager::getDifficultyDescription() const {
        auto it = modifiers_.find(currentDifficulty_);
        return it != modifiers_.end() ? it->second.description : "";
    }

    int DifficultyManager::modifyEnemyHp(int baseHp) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseHp;
        return static_cast<int>(baseHp * it->second.enemyHpMultiplier);
    }

    int DifficultyManager::modifyEnemyAttack(int baseAtk) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseAtk;
        return static_cast<int>(baseAtk * it->second.enemyAttackMultiplier);
    }

    int DifficultyManager::modifyPlayerHp(int baseHp) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseHp;
        return static_cast<int>(baseHp * it->second.playerHpMultiplier);
    }

    int DifficultyManager::modifyExpReward(int baseExp) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseExp;
        return static_cast<int>(baseExp * it->second.expMultiplier);
    }

    int DifficultyManager::modifyGoldReward(int baseGold) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseGold;
        return static_cast<int>(baseGold * it->second.goldMultiplier);
    }

    double DifficultyManager::modifyEnemyChance(double baseChance) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseChance;
        return std::min(0.95, baseChance * it->second.enemyChanceMultiplier);
    }

    double DifficultyManager::modifyItemDropChance(double baseChance) const {
        auto it = modifiers_.find(currentDifficulty_);
        if (it == modifiers_.end()) return baseChance;
        return std::min(0.95, baseChance * it->second.itemDropMultiplier);
    }

    const DifficultyModifiers& DifficultyManager::getModifiers() const {
        return modifiers_.at(currentDifficulty_);
    }

    void DifficultyManager::recordEnemyKilled() {
        ++enemiesKilled_;
    }

    void DifficultyManager::recordDeath() {
        ++deaths_;
    }

} // namespace rpg