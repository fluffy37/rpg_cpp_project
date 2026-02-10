#include "rpg/Ally.h"
#include "rpg/Player.h"

namespace rpg {

Ally::Ally(std::string name, int level, int maxHp, int hp, int supportPower)
    : Character(std::move(name), level, maxHp, hp), supportPower_(supportPower) {}

std::string Ally::act(GameState& state) {
    if (!state.player) return name_ + " не находит, кому помочь.";
    int healed = state.player->heal(supportPower_);
    return name_ + " помогает: лечит игрока на " + std::to_string(healed) + " HP.";
}

}
