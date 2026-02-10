#include "rpg/Player.h"
#include "rpg/Inventory.h"

namespace rpg {

Player::Player(std::string name, int level, int maxHp, int hp)
    : Character(std::move(name), level, maxHp, hp) {}

void Player::gainGold(int amount) { gold_ = std::max(0, gold_ + amount); }

void Player::gainExp(int amount) {
    exp_ = std::max(0, exp_ + amount);
    while (exp_ >= nextLevelExp_) {
        exp_ -= nextLevelExp_;
        levelUp();
    }
}

void Player::levelUp() {
    level_ += 1;
    maxHp_ += 10;
    hp_ = maxHp_;
    nextLevelExp_ = static_cast<int>(nextLevelExp_ * 1.25);
}

Inventory& Player::inventory() { return *inventory_; }
const Inventory& Player::inventory() const { return *inventory_; }

std::string Player::act(GameState&) { return "Игрок ожидает ввода."; }

std::vector<std::string> Player::headerLines() const {
    return {
        "[Игрок: " + name_ + " | Ур. " + std::to_string(level_) +
            " | HP: " + std::to_string(hp_) + "/" + std::to_string(maxHp_) + "]",
        "[Золото: " + std::to_string(gold_) + " | Опыт: " +
            std::to_string(exp_) + "/" + std::to_string(nextLevelExp_) + "]"
    };
}

}
