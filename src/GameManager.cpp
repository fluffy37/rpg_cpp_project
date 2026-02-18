#include "rpg/GameManager.h"
#include "rpg/Items.h"
#include <iostream>
#include <conio.h>
#include <cctype>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <thread>

namespace rpg {
    static char readKeylower() {
        return (char)std::tolower((unsigned char)_getch());
    }

    static std::string lowerAscii(std::string s) {
        for (auto& ch : s) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return s;
    }

    GameManager::GameManager(unsigned int seed)
        : seed_(seed),
        rng_(seed),
        enemyFactory_(rng_),
        npcFactory_(rng_),
        allyFactory_(rng_),
        saveManager_("saves", "save.txt"),
        inventory_(20),
        player_("Player1", 1, 60, 60)
    {
        resetGame();
    }

    void GameManager::printHud() const {
        for (const auto& l : player_.headerLines()) std::cout << l << "\n";
        std::cout << "\nТекущая локация: " << player_.location().toString() << "\n\n";
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

       
        double enemyChance = 0.4 + (depth * 0.03); // Становится опаснее с глубиной
        double npcChance = 0.6 - (depth * 0.02); // Меньше NPC на глубине
        double itemChance = 0.8 - (depth * 0.01); // Чуть реже находить предметы на глубине

        // Ограничиваем вероятности от 0 до 1
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
            // Случайное количество здоровья для зелья
            int healAmount = 20 + rng_() % 25;
            bool ok = inventory_.add(std::make_unique<HealthPotion>(healAmount));
            std::cout << (ok ? "Вы нашли предмет: Зелье здоровья.\n" : "Вы нашли зелье, но инвентарь переполнен.\n");
        }

        // Шанс углубиться зависит от текущей глубины
        if (d(rng_) < 0.25 + (0.05 * (10 - depth))) {
            player_.location().depth += 1;
            std::cout << "Вы углубились. Теперь: " << player_.location().toString() << "\n";
        }
    }

    void GameManager::resetGame() {
        rng_.seed(seed_);
        inventory_ = Inventory(20);
        inventory_.attach(&invObserver_);
        player_ = Player("Player1", 1, 60, 60); // ИЗМЕНЕНО: 5 -> 1, 60/45 -> 30/30
        player_.bindInventory(&inventory_);
        registerDefaultItems();
        player_.mutableGold() = 0; // Начинаем с 0 золота
        player_.mutableExp() = 0; // Начинаем с 0 опыта
        player_.mutableNextLevelExp() = 100; // Начальный порог опыта
        player_.location().depth = 1; // Начинаем с 1 уровня подземелья
        player_.location().description =
            "Начальные коридоры подземелья.";
        inventory_.clear(); // Очищаем инвентарь
        inventory_.add(std::make_unique<HealthPotion>(10)); // Одно зелье для новичка
    }

    void GameManager::drawBattleTable(const Player& player, const Enemy& enemy) {
        // УДАЛЕНО: std::system("cls");

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
        std::cout << "Вас атакует " << enemy.name() << "!\n";  // Начальное сообщение
        GameState state; state.player = &player_;
        std::uniform_real_distribution<double> d(0.0, 1.0);

        // Начальная таблица
        drawBattleTable(player_, enemy);

        // Цикл боя
        while (player_.isAlive() && enemy.isAlive()) {
            char a = readKeylower();
            std::cout << a << "\n";
            if (a == 'p') useHealthPotion();
            else if (a == 'r') {
                // Шанс побега зависит от разницы уровней
                double escapeChance = 0.5 - (enemy.level() - player_.level()) * 0.1;
                escapeChance = std::max(0.1, escapeChance); // Минимум 10% шанс
                if (d(rng_) < escapeChance) {
                    std::cout << "Вы успешно сбежали.\n";
                    std::cout << "\nНажмите любую клавишу для продолжения...";
                    _getch();
                    return;
                }
                std::cout << "Не удалось сбежать!\n";
            }
            else {
                // Случайный урон с учетом уровня игрока
                int baseDmg = 5 + player_.level();
                int variance = player_.level() / 2;
                int dmg = baseDmg + (rng_() % (variance * 2 + 1)) - variance;
                dmg = std::max(1, dmg);
                std::cout << "Вы наносите " << enemy.takeDamage(dmg) << " урона.\n";
            }

            if (enemy.isAlive()) std::cout << enemy.act(state) << "\n";

            // Пауза для чтения сообщений
            std::cout << "\nНажмите любую клавишу для продолжения...";
            _getch();
            std::cout << "\n";

            // Обнови таблицу после хода
            if (player_.isAlive() && enemy.isAlive()) {
                drawBattleTable(player_, enemy);
                printHud();
            }
        }

        // Конец боя: сообщение о победе/поражении
        if (player_.isAlive()) {
            std::cout << enemy.name() << " повержен!\n";
            // Случайная награда с небольшим разбросом
            int goldReward = enemy.goldReward() + (rng_() % 5) - 2;
            int expReward = enemy.expReward() + (rng_() % 3) - 1;
            goldReward = std::max(1, goldReward);
            expReward = std::max(1, expReward);

            player_.gainGold(goldReward);
            player_.gainExp(expReward);
            std::cout << "Награда: +" << goldReward << " золота, +" << expReward << " опыта.\n";
        }
        else {
            std::cout << "Вы проиграли...\n";
        }

        std::cout << "\nНажмите любую клавишу для продолжения...";
        _getch();
    }

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
            else if (c == '6' || c == 'q' || c == 'exit' || c == 'quit') { std::cout << "Выход в меню.До встречи.\n"; break; }
            else std::cout << "Неизвестная команда.\n";

            std::cout << "\n" << std::string(60, '-') << "\n\n";
            if (!player_.isAlive()) {
                std::cout << "Игрок пал. Игра окончена. \n\n";
                std::cout << "1 - Начать заново \n";
                std::cout << "2 - Загрузить сохранение \n";
                std::cout << "3 - Выйти \n";
                std::cout << "Выбор: \n";

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
                            std::cout << "Игра загружена. \n";
                            break;
                        }
                        else {
                            std::cout << "Загруженный игрок не жив. Выберите другой слот.\n";
                        }
                    }
                    else if (k == '3' || k == 'q') {
                        std::cout << "Выход в меню.До встречи.\n";
                        return;
                    }
                    else {
                        std::cout << "Неверный выбор (1/2/3): ";
                    }
                }
            }
        }
    }
}