#include "rpg/Enemy.h"
#include "rpg/Player.h"
#include <iostream>
#include <filesystem>
#include <thread>

namespace rpg {

Enemy::Enemy(std::string name, int level, int maxHp, int hp, int atk, int exp, int gold)
    : Character(std::move(name), level, maxHp, hp), attackPower_(atk), expReward_(exp), goldReward_(gold) {
    std::cout << "      />" << std::endl;
    std::cout << " (====|====================>" << std::endl;
    std::cout << "      \\>" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

std::string Enemy::act(GameState& state) {
    if (!state.player) return name_ + " растерян и не атакует.";
    int dmg = state.player->takeDamage(attackPower_);

    return name_ + " атакует и наносит " + std::to_string(dmg) + " урона.";
}

}
