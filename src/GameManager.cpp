#include "rpg/GameManager.h"
#include "rpg/Items.h"
#include <iostream>
#include <conio.h>
#include <cctype>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

namespace rpg {

    static char readKeylower() {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(_getch())));
    }

    GameManager::GameManager(unsigned int seed)
        : seed_(seed),
        rng_(seed),
        enemyFactory_(rng_),
        npcFactory_(rng_),
        allyFactory_(rng_),
        saveManager_("saves", "save.txt"),
        inventory_(20),
        player_("Player1", 1, 60, 60),
        currentDifficulty_(Difficulty::Normal) {
        selectDifficulty();
        resetGame();
    }

    void GameManager::selectDifficulty() {
        std::cout << "\n╔════════════════════════════════════════╗\n";
        std::cout << "║     ВЫБОР УРОВНЯ СЛОЖНОСТИ            ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";

        printDifficultyMenu();

        while (true) {
            std::cout << "\nВыберите сложность (1-4): ";
            char c = readKeylower();
            std::cout << c << "\n";

            if (c == '1') { currentDifficulty_ = Difficulty::Easy; break; }
            else if (c == '2') { currentDifficulty_ = Difficulty::Normal; break; }
            else if (c == '3') { currentDifficulty_ = Difficulty::Hard; break; }
            else if (c == '4') { currentDifficulty_ = Difficulty::Nightmare; break; }
            else { std::cout << "Неверный выбор!\n"; }
        }

        DifficultyManager::instance().setDifficulty(currentDifficulty_);
        std::cout << "\nУстановлена сложность: "
            << DifficultyManager::instance().getDifficultyName() << "\n";
        std::cout << DifficultyManager::instance().getDifficultyDescription() << "\n";
        std::cout << "\nНажмите любую клавишу для продолжения...";
        (void)_getch();
    }

    void GameManager::printDifficultyMenu() const {
        std::cout << "  1. Лёгкий\n";
        std::cout << "     - Враги: -30% HP, -30% урон\n";
        std::cout << "     - Игрок: +30% HP\n";
        std::cout << "     - Награды: +20% опыт, +20% золото\n\n";

        std::cout << "  2. Нормальный\n";
        std::cout << "     - Сбалансированный опыт\n\n";

        std::cout << "  3. Сложный\n";
        std::cout << "     - Враги: +40% HP, +30% урон\n";
        std::cout << "     - Игрок: -20% HP\n";
        std::cout << "     - Награды: +30% опыт, +30% золото\n\n";

        std::cout << "  4. Кошмар\n";
        std::cout << "     - Враги: +100% HP, +60% урон\n";
        std::cout << "     - Игрок: -40% HP\n";
        std::cout << "     - Награды: +50% опыт, +50% золото\n\n";
    }

    void GameManager::printHud() const {
        for (const auto& l : player_.headerLines())
            std::cout << l << "\n";
        std::cout << "\nТекущая локация: " << player_.location().toString() << "\n";
        std::cout << "Сложность: " << DifficultyManager::instance().getDifficultyName() << "\n";
    }

    void GameManager::printMenu() const {
        std::cout << "   o\n";
        std::cout << "  /|\\\n";
        std::cout << "  / \\\n\n";
        std::cout << "1. Исследовать дальше\n";
        std::cout << "2. Осмотреть инвентарь (I)\n";
        std::cout << "3. Использовать зелье здоровья\n";
        std::cout << "4. Сохранить игру (F5)\n";
        std::cout << "5. Загрузить игру (F9)\n";
        std::cout << "6. Выйти в меню\n\n";
    }

    void GameManager::showInventory() const {
        auto labels = inventory_.listLabels();
        if (labels.empty()) { std::cout << "Инвентарь пуст.\n"; return; }
        std::cout << "Инвентарь:\n";
        for (std::size_t i = 0; i < labels.size(); ++i)
            std::cout << "  " << (i + 1) << ". " << labels[i] << "\n";
    }

    void GameManager::useHealthPotion() {
        auto msg = inventory_.useFirst("health_potion", player_);
        if (!msg) std::cout << "Зелья здоровья нет.\n";
        else std::cout << "Вы использовали зелье: " << *msg << "\n";
    }

    void GameManager::saveGame() const {
        std::cout << "Выберите слот для сохранения (1-3) или Enter для 1: ";
        std::string s; std::getline(std::cin, s);
        int slot = 1;
        if (!s.empty()) {
            try { slot = std::stoi(s); }
            catch (...) { slot = 1; }
        }
        if (slot < 1) slot = 1;
        if (slot > 3) slot = 3;

        std::cout << "Имя сохранения (можно пусто): ";
        std::string name; std::getline(std::cin, name);

        if (saveManager_.saveSlot(slot, player_, inventory_, name))
            std::cout << "Игра сохранена в слот " << slot << ": " << saveManager_.path() << "\n";
        else
            std::cout << "Не удалось сохранить игру.\n";
    }

    void GameManager::loadGame() {
        auto slots = saveManager_.listSlots();
        if (!slots.empty()) {
            std::cout << "Доступные сохранения:\n";
            for (const auto& si : slots) {
                std::cout << "  Слот " << si.slot << ": " << si.name;
                if (!si.timestamp.empty()) std::cout << " (" << si.timestamp << ")";
                std::cout << "\n";
            }
        }
        else {
            std::cout << "Сохранений пока нет.\n";
        }

        std::cout << "Выберите слот для загрузки (1-3) или Enter для 1: ";
        std::string s; std::getline(std::cin, s);
        int slot = 1;
        if (!s.empty()) {
            try { slot = std::stoi(s); }
            catch (...) { slot = 1; }
        }
        if (slot < 1) slot = 1;
        if (slot > 3) slot = 3;

        if (saveManager_.loadSlot(slot, player_, inventory_))
            std::cout << "Игра загружена из слота " << slot << ".\n";
        else
            std::cout << "Сохранение в слоте " << slot << " не найдено.\n";
    }

    void GameManager::explore() {
        std::uniform_real_distribution<double> d(0.0, 1.0);
        double roll = d(rng_);
        int depth = player_.location().depth;

        // Базовые шансы
        double enemyChance = 0.4 + (depth * 0.03);
        double npcChance = 0.6 - (depth * 0.02);
        double itemChance = 0.8 - (depth * 0.01);

        // Применить модификаторы сложности
        enemyChance = DifficultyManager::instance().modifyEnemyChance(enemyChance);
        itemChance = DifficultyManager::instance().modifyItemDropChance(itemChance);

        // Ограничения
        enemyChance = std::min(0.95, std::max(0.2, enemyChance));
        npcChance = std::min(0.75, std::max(0.1, npcChance));
        itemChance = std::min(0.95, std::max(0.5, itemChance));

        GameState state; state.player = &player_;

        if (roll < enemyChance) {
            encounterEnemy(enemyFactory_.create(depth));
        }
        else if (roll < npcChance) {
            std::cout << npcFactory_.create(depth).talk() << "\n";
        }
        else if (roll < itemChance) {
            Ally ally = allyFactory_.create(depth);
            std::cout << "Вы встречаете дружественное существо: " << ally.name() << ".\n";
            std::cout << ally.act(state) << "\n";
        }
        else {
            int healAmount = 20 + rng_() % 25;
            bool ok = inventory_.add(std::make_unique<HealthPotion>(healAmount));
            std::cout << (ok ? "Вы нашли предмет: Зелье здоровья.\n"
                : "Вы нашли зелье, но инвентарь переполнен.\n");
        }

        if (d(rng_) < 0.25 + (0.05 * (10 - depth))) {
            player_.location().depth += 1;
            std::cout << "Вы углубились. Теперь: " << player_.location().toString() << "\n";
        }
    }


    void GameManager::resetGame() {
        rng_.seed(seed_);
        inventory_ = Inventory(20);
        inventory_.attach(&invObserver_);

        int baseHp = 60;
        int modifiedHp = DifficultyManager::instance().modifyPlayerHp(baseHp);

        player_ = Player("Player1", 1, modifiedHp, modifiedHp);
        player_.bindInventory(&inventory_);
        registerDefaultItems();

        player_.mutableGold() = 0;
        player_.mutableExp() = 0;
        player_.mutableNextLevelExp() = 100;
        player_.location().depth = 1;
        player_.location().description = "Начальные коридоры подземелья.";

        inventory_.clear();
        inventory_.add(std::make_unique<HealthPotion>(10));
    }

    void GameManager::drawBattleTable(const Player& player, const Enemy& enemy) {
       

        // Выводим информацию об игроке перед таблицей
        for (const auto& l : player.headerLines())
            std::cout << l << "\n";
        std::cout << "\nТекущая локация: " << player.location().toString() << "\n\n";

        // Ширина колонок — подстраиваем под нужное
        const int colWidth = 20;
        // Верхняя рамка
        std::cout << "+" << std::setfill('-') << std::setw(colWidth) << ""
            << "+" << std::setw(colWidth) << "" << "+" << std::setfill(' ') << std::endl;

        // Заголовок
        std::cout << "| " << std::left << std::setw(colWidth - 2) << ("Игрок: " + player.name())
            << "| " << std::setw(colWidth - 2) << ("Враг: " + enemy.name()) << "|" << std::endl;

        // Разделитель
        std::cout << "+" << std::setfill('-') << std::setw(colWidth) << ""
            << "+" << std::setw(colWidth) << "" << "+" << std::setfill(' ') << std::endl;

        // HP
        std::string playerHP = "HP: " + std::to_string(player.hp()) + "/" + std::to_string(player.maxHp());
        std::string enemyHP = "HP: " + std::to_string(enemy.hp()) + "/" + std::to_string(enemy.maxHp());
        std::cout << "| " << std::left << std::setw(colWidth - 2) << playerHP
            << "| " << std::setw(colWidth - 2) << enemyHP << "|" << std::endl;

        // Нижняя рамка (можно добавить больше строк, напр. для инвентаря)
        std::cout << "+" << std::setfill('-') << std::setw(colWidth) << ""
            << "+" << std::setw(colWidth) << "" << "+" << std::setfill(' ') << std::endl;

        // Дополнительно: подсказка действий
        std::cout << "Действие: (A)таковать / (P)зелье / (R)бежать\n";
    }

    void GameManager::encounterEnemy(Enemy enemy) {
        std::cout << "Вас атакует " << enemy.name() << "!\n";

        
        int modifiedMaxHp = DifficultyManager::instance().modifyEnemyHp(enemy.maxHp());
        int modifiedAtk = DifficultyManager::instance().modifyEnemyAttack(enemy.attackPower());
        int modifiedExp = DifficultyManager::instance().modifyExpReward(enemy.expReward());
        int modifiedGold = DifficultyManager::instance().modifyGoldReward(enemy.goldReward());

        // Создаём нового врага с модифицированными характеристиками
        Enemy modifiedEnemy(
            enemy.name(),
            enemy.level(),
            modifiedMaxHp,
            modifiedMaxHp,  // текущее HP = макс HP
            modifiedAtk,
            modifiedExp,
            modifiedGold
        );

        GameState state;
        state.player = &player_;
        std::uniform_real_distribution<double> d(0.0, 1.0);

        drawBattleTable(player_, modifiedEnemy);

        while (player_.isAlive() && modifiedEnemy.isAlive()) {
            char a = readKeylower();
            std::cout << a << "\n";

            if (a == 'p') useHealthPotion();
            else if (a == 'r') {
                double escapeChance = 0.5 - (modifiedEnemy.level() - player_.level()) * 0.1;
                escapeChance = std::max(0.1, escapeChance);
                if (d(rng_) < escapeChance) {
                    std::cout << "Вы успешно сбежали.\n";
                    std::cout << "\nНажмите любую клавишу для продолжения...";
                    _getch();
                    return;
                }
                std::cout << "Не удалось сбежать!\n";
            }
            else {
                int baseDmg = 5 + player_.level();
                int variance = player_.level() / 2;
                int dmg = baseDmg + (rng_() % (variance * 2 + 1)) - variance;
                dmg = std::max(1, dmg);
                std::cout << "Вы наносите " << modifiedEnemy.takeDamage(dmg) << " урона.\n";
            }

            if (modifiedEnemy.isAlive()) {
                std::cout << modifiedEnemy.act(state) << "\n";
            }

            std::cout << "\nНажмите любую клавишу для продолжения...";
            (void)_getch();
            std::cout << "\n";

            if (player_.isAlive() && modifiedEnemy.isAlive()) {
                drawBattleTable(player_, modifiedEnemy);
                printHud();
            }
        }

        if (player_.isAlive()) {
            std::cout << modifiedEnemy.name() << " повержен!\n";

            // ✅ Награды уже модифицированы при создании врага
            int goldReward = modifiedEnemy.goldReward() + (rng_() % 5) - 2;
            int expReward = modifiedEnemy.expReward() + (rng_() % 3) - 1;
            goldReward = std::max(1, goldReward);
            expReward = std::max(1, expReward);

            player_.gainGold(goldReward);
            player_.gainExp(expReward);

            std::cout << "Награда: +" << goldReward << " золота, +"
                << expReward << " опыта.\n";

            // ✅ Запись статистики
            DifficultyManager::instance().recordEnemyKilled();
        }
        else {
            std::cout << "Вы проиграли...\n";
            // ✅ Запись смерти
            DifficultyManager::instance().recordDeath();
        }

        std::cout << "\nНажмите любую клавишу для продолжения...";
        (void)_getch();
    }

    // ✅ В run() добавить запись смерти при Game Over:
    void GameManager::run() {
        while (true) {
            printHud();
            printMenu();
            std::cout << "Выберите действие: ";
            char c = readKeylower();
            std::cout << c << "\n";

            if (c == '2' || c == 'i') showInventory();
            else if (c == '1') explore();
            else if (c == '3') useHealthPotion();
            else if (c == '4' || c == 'f5') saveGame();
            else if (c == '5' || c == 'f9') loadGame();
            else if (c == '6' || c == 'q') {
                std::cout << "Выход в меню. До встречи.\n";
                break;
            }
            else std::cout << "Неизвестная команда.\n";

            std::cout << "\n" << std::string(60, '-') << "\n";

            if (!player_.isAlive()) {
                std::cout << "Игрок пал. Игра окончена.\n";
                std::cout << "1 - Начать заново\n";
                std::cout << "2 - Загрузить сохранение\n";
                std::cout << "3 - Выйти\n";
                std::cout << "Выбор: ";

                while (true) {
                    char k = readKeylower();
                    std::cout << k << "\n";

                    if (k == '1') {
                        resetGame();
                        break;
                    }
                    else if (k == '2') {
                        loadGame();
                        if (player_.isAlive()) {
                            std::cout << "Игра загружена.\n";
                            break;
                        }
                    }
                    else if (k == '3' || k == 'q') {
                        std::cout << "Выход в меню. До встречи.\n";
                        return;
                    }
                    else {
                        std::cout << "Неверный выбор (1/2/3): ";
                    }
                }
            }
        }
    }

} // namespace rpg
