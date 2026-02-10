#include "rpg/GameManager.h"
#include "rpg/Items.h"
#include <iostream>
#include <conio.h>
#include <cctype>

namespace rpg {
    static char readKeylower() {
        return (char)std::tolower((unsigned char)_getch());
}

static std::string lowerAscii(std::string s){
    for (auto& ch : s) if (ch>='A' && ch<='Z') ch = char(ch - 'A' + 'a');
    return s;
}

GameManager::GameManager(unsigned int seed)
  : seed_(seed),
    rng_(seed),
    enemyFactory_(rng_),
    npcFactory_(rng_),
    allyFactory_(rng_),
    saveManager_("saves","save.txt"),
    inventory_(20),
    player_("Player1",1,60,60)
{
    resetGame();
}

void GameManager::printHud() const {
    for (const auto& l : player_.headerLines()) std::cout << l << "\n";
    std::cout << "\nТекущая локация: " << player_.location().toString() << "\n\n";
}

void GameManager::printMenu() const {
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
    for (std::size_t i=0;i<labels.size();++i)
        std::cout << "  " << (i+1) << ". " << labels[i] << "\n";
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
        try { slot = std::stoi(s); } catch (...) { slot = 1; }
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
    } else {
        std::cout << "Сохранений пока нет.\n";
    }

    std::cout << "Выберите слот для загрузки (1-3) или Enter для 1: ";
    std::string s; std::getline(std::cin, s);
    int slot = 1;
    if (!s.empty()) {
        try { slot = std::stoi(s); } catch (...) { slot = 1; }
    }
    if (slot < 1) slot = 1;
    if (slot > 3) slot = 3;

    if (saveManager_.loadSlot(slot, player_, inventory_))
        std::cout << "Игра загружена из слота " << slot << ".\n";
    else
        std::cout << "Сохранение в слоте " << slot << " не найдено.\n";
}

void GameManager::explore() {
    std::uniform_real_distribution<double> d(0.0,1.0);
    double roll = d(rng_);
    int depth = player_.location().depth;

    GameState state; state.player = &player_;

    if (roll < 0.55) {
        encounterEnemy(enemyFactory_.create(depth));
    } else if (roll < 0.60) {
        std::cout << npcFactory_.create(depth).talk() << "\n";
    } else if (roll < 0.9) {
        Ally ally = allyFactory_.create(depth);
        std::cout << "Вы встречаете дружественное существо: " << ally.name() << ".\n";
        std::cout << ally.act(state) << "\n";
    } else {
        bool ok = inventory_.add(std::make_unique<HealthPotion>(30));
        std::cout << (ok ? "Вы нашли предмет: Зелье здоровья.\n" : "Вы нашли зелье, но инвентарь переполнен.\n");
    }

    if (d(rng_) < 0.35) {
        player_.location().depth += 1;
        std::cout << "Вы углубились. Теперь: " << player_.location().toString() << "\n";
    }
}

void GameManager::encounterEnemy(Enemy enemy) {
    std::cout << "Вас атакует " << enemy.name() << "!\n";
    GameState state; state.player = &player_;
    std::uniform_real_distribution<double> d(0.0,1.0);

    while (enemy.isAlive() && player_.isAlive()) {
        std::cout << "  " << enemy.name() << ": HP " << enemy.hp() << "/" << enemy.maxHp() << "\n";
        std::cout << "  " << player_.name() << ": HP " << player_.hp() << "/" << player_.maxHp() << "\n";
        std::cout << "Действие: (A)таковать / (P)зелье / (R)бежать: ";
        char a = readKeylower();
        std::cout << a << "\n";

        if (a=='p') useHealthPotion();
        else if (a=='r') {
            if (d(rng_) < 0.5) { std::cout << "Вы успешно сбежали.\n"; return; }
            std::cout << "Не удалось сбежать!\n";
        } else {
            int dmg = std::max(1, 10 + player_.level());
            std::cout << "Вы наносите " << enemy.takeDamage(dmg) << " урона.\n";
        }

        if (enemy.isAlive()) std::cout << enemy.act(state) << "\n";
    }

    if (player_.isAlive()) {
        std::cout << enemy.name() << " повержен!\n";
        player_.gainGold(enemy.goldReward());
        player_.gainExp(enemy.expReward());
        std::cout << "Награда: +" << enemy.goldReward() << " золота, +" << enemy.expReward() << " опыта.\n";
    }
}
void GameManager::resetGame() {
    rng_.seed(seed_);

    inventory_ = Inventory(20);
    inventory_.attach(&invObserver_);

    player_ = Player("Player1", 5, 60, 45);
    player_.bindInventory(&inventory_);

    registerDefaultItems();

    player_.mutableGold() = 120;
    player_.mutableExp() = 320;
    player_.mutableNextLevelExp() = 500;

    player_.location().depth = 3;
    player_.location().description =
        "Темные коридоры с шорохами за стенами.";

    inventory_.add(std::make_unique<HealthPotion>(25));
    inventory_.add(std::make_unique<HealthPotion>(25));
}

void GameManager::run() {
    while (true) {
        printHud();
        printMenu();
        std::cout << "Выберите действие: ";
        char c = readKeylower();
        std::cout << c << "\n";

        if (c=='2'||c=='i') showInventory();
        else if (c=='1') explore();
        else if (c=='3') useHealthPotion();
        else if (c=='4'||c=='f5') saveGame();
        else if (c=='5'||c=='f9') loadGame();
        else if (c=='6'||c=='q' || c == 'exit' || c == 'quit') { std::cout << "Выход в меню.До встречи.\n"; break; }
        else std::cout << "Неизвестная команда.\n";

        std::cout << "\n" << std::string(60,'-') << "\n\n";
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
                else if (k == '2')
                {
                    loadGame();

                    if (player_.isAlive()) {
                        std::cout << "Игра загружена. \n";
                        break;
                    }
                    else if (k == '3' || k == 'q')
                    {
                        std::cout << "Неверный выбор (1/2/3): ";
                    }
                }
            }
        }
    }
}
}
