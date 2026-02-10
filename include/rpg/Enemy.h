#pragma once
#include "rpg/Character.h"

namespace rpg {

class Enemy final : public Character {
public:
    Enemy(std::string name, int level, int maxHp, int hp, int atk, int exp, int gold);
    int attackPower() const { return attackPower_; }
    int expReward() const { return expReward_; }
    int goldReward() const { return goldReward_; }
    std::string act(GameState& state) override;

private:
    int attackPower_{8};
    int expReward_{80};
    int goldReward_{25};
};

}
