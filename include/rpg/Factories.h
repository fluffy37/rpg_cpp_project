#pragma once
#include <random>
#include "rpg/Enemy.h"
#include "rpg/NPC.h"
#include "rpg/Ally.h"

namespace rpg {

class EnemyFactory {
public:
    explicit EnemyFactory(std::mt19937& rng) : rng_(rng) {}
    Enemy create(int depth);
private:
    std::mt19937& rng_;
};

class NPCFactory {
public:
    explicit NPCFactory(std::mt19937& rng) : rng_(rng) {}
    NPC create(int depth);
private:
    std::mt19937& rng_;
};

class AllyFactory {
public:
    explicit AllyFactory(std::mt19937& rng) : rng_(rng) {}
    Ally create(int depth);
private:
    std::mt19937& rng_;
};

}
