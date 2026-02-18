#pragma once
#include <random>
#include "rpg/Player.h"
#include "rpg/Inventory.h"
#include "rpg/Factories.h"
#include "rpg/SaveSystem.h"
#include "rpg/Observers.h"
#include "rpg/Difficulty.h"

namespace rpg {

    class GameManager {
    public:
        explicit GameManager(unsigned int seed = 42);
        void run();

    private:
        void printHud() const;
        void printMenu() const;
        void showInventory() const;
        void useHealthPotion();
        void saveGame() const;
        void loadGame();
        void resetGame();
        void explore();
        void encounterEnemy(Enemy enemy);
        void drawBattleTable(const Player& player, const Enemy& enemy);
        void selectDifficulty();
        void printDifficultyMenu() const;

        unsigned int seed_;
        std::mt19937 rng_;
        EnemyFactory enemyFactory_;
        NPCFactory npcFactory_;
        AllyFactory allyFactory_;
        SaveManager saveManager_;
        Inventory inventory_;
        Player player_;
        ConsoleInventoryObserver invObserver_;
        Difficulty currentDifficulty_;
    };

} // namespace rpg