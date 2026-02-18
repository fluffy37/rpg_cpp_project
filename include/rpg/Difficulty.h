#pragma once
#include <string>
#include <unordered_map>
#include <cstddef>  
#define NOMINMAX    

namespace rpg {

    enum class Difficulty {
        Easy = 0,
        Normal = 1,
        Hard = 2,
        Nightmare = 3
    };

    
    struct DifficultyHash {
        std::size_t operator()(Difficulty d) const {
            return static_cast<std::size_t>(d);
        }
    };

    struct DifficultyModifiers {
        double enemyHpMultiplier;
        double enemyAttackMultiplier;
        double playerHpMultiplier;
        double expMultiplier;
        double goldMultiplier;
        double enemyChanceMultiplier;
        double itemDropMultiplier;
        std::string displayName;
        std::string description;

        
        DifficultyModifiers(
            double eHp, double eAtk, double pHp,
            double exp, double gold, double eChance, double iDrop,
            const std::string& dName, const std::string& desc)
            : enemyHpMultiplier(eHp), enemyAttackMultiplier(eAtk),
            playerHpMultiplier(pHp), expMultiplier(exp),
            goldMultiplier(gold), enemyChanceMultiplier(eChance),
            itemDropMultiplier(iDrop), displayName(dName),
            description(desc) {
        }
    };

    class DifficultyManager {
    public:
        static DifficultyManager& instance();

        void setDifficulty(Difficulty diff);
        Difficulty getDifficulty() const { return currentDifficulty_; }
        std::string getDifficultyName() const;
        std::string getDifficultyDescription() const;

        int modifyEnemyHp(int baseHp) const;
        int modifyEnemyAttack(int baseAtk) const;
        int modifyPlayerHp(int baseHp) const;
        int modifyExpReward(int baseExp) const;
        int modifyGoldReward(int baseGold) const;
        double modifyEnemyChance(double baseChance) const;
        double modifyItemDropChance(double baseChance) const;

        const DifficultyModifiers& getModifiers() const;

        void recordEnemyKilled();
        void recordDeath();
        int getEnemiesKilled() const { return enemiesKilled_; }
        int getDeaths() const { return deaths_; }

    private:
        DifficultyManager();
        Difficulty currentDifficulty_ = Difficulty::Normal;
        int enemiesKilled_ = 0;
        int deaths_ = 0;

       
        static const std::unordered_map<Difficulty, DifficultyModifiers, DifficultyHash> modifiers_;
    };

} // namespace rpg