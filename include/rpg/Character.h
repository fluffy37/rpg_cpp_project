#pragma once
#include <string>
#include <algorithm>

namespace rpg {
class Player;
struct GameState { Player* player{nullptr}; };

class Character {
public:
    Character(std::string name, int level, int maxHp, int hp)
        : name_(std::move(name)), level_(level), maxHp_(maxHp), hp_(hp) {}
    virtual ~Character() = default;

    const std::string& name() const { return name_; }
    int level() const { return level_; }
    int maxHp() const { return maxHp_; }
    int hp() const { return hp_; }
    bool isAlive() const { return hp_ > 0; }

    int takeDamage(int amount) {
        int dmg = std::max(0, amount);
        hp_ = std::max(0, hp_ - dmg);
        return dmg;
    }
    int heal(int amount) {
        int inc = std::max(0, amount);
        int old = hp_;
        hp_ = std::min(maxHp_, hp_ + inc);
        return hp_ - old;
    }

    virtual std::string act(GameState& state) = 0;

protected:
    std::string name_;
    int level_{1};
    int maxHp_{10};
    int hp_{10};
};

}
