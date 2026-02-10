#pragma once
#include "rpg/Character.h"

namespace rpg {

class Ally final : public Character {
public:
    Ally(std::string name, int level, int maxHp, int hp, int supportPower);
    std::string act(GameState& state) override;

private:
    int supportPower_{6};
};

}
