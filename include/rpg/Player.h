#pragma once
#include "rpg/Character.h"
#include "rpg/Location.h"
#include <vector>

namespace rpg {

class Inventory;

class Player final : public Character {
public:
    Player(std::string name, int level, int maxHp, int hp);
    void bindInventory(Inventory* inv) { inventory_ = inv; }

    int gold() const { return gold_; }
    int exp() const { return exp_; }
    int nextLevelExp() const { return nextLevelExp_; }

    void gainGold(int amount);
    void gainExp(int amount);
    void levelUp();

    Location& location() { return location_; }
    const Location& location() const { return location_; }

    Inventory& inventory();
    const Inventory& inventory() const;

    std::string act(GameState&) override;

    std::vector<std::string> headerLines() const;

    // для загрузки
    std::string& mutableName() { return name_; }
    int& mutableLevel() { return level_; }
    int& mutableMaxHp() { return maxHp_; }
    int& mutableHp() { return hp_; }
    int& mutableGold() { return gold_; }
    int& mutableExp() { return exp_; }
    int& mutableNextLevelExp() { return nextLevelExp_; }

private:
    int gold_{0};
    int exp_{0};
    int nextLevelExp_{500};
    Location location_{};
    Inventory* inventory_{nullptr}; // non-owning
};

}
