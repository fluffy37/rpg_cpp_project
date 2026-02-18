#include "rpg/Factories.h"
#include "rpg/Difficulty.h" 
#include <vector>
#include <algorithm>

namespace rpg {

    Enemy EnemyFactory::create(int depth) {
        struct T { const char* n; int atk; int exp; int gold; };
        const std::vector<T> t = {
            {"Скелет", 6, 70, 20},
            {"Гоблин", 7, 80, 25},
            {"Культист", 8, 90, 30},
            {"Тень", 9, 110, 40}
        };

        std::uniform_int_distribution<int> pick(0, (int)t.size() - 1);
        std::uniform_int_distribution<int> jitter(-1, 1);
        auto tpl = t[pick(rng_)];

        int lvl = std::max(1, depth + jitter(rng_));
        int maxHp = 25 + lvl * 7;
        int atk = tpl.atk + lvl;
        int exp = tpl.exp + lvl * 15;
        int gold = tpl.gold + lvl * 5;

   

        return Enemy(std::string(tpl.n) + " (ур. " + std::to_string(lvl) + ")",
            lvl, maxHp, maxHp, atk, exp, gold);
    }

NPC NPCFactory::create(int) {
    const std::vector<std::string> names = {"Арвин","Лира","Борн","Селена","Краг"};
    const std::vector<std::string> roles = {"торговец","исследователь","пленник","монах","наемник"};
    std::uniform_int_distribution<int> n(0, (int)names.size()-1);
    std::uniform_int_distribution<int> r(0, (int)roles.size()-1);
    return NPC(names[n(rng_)], roles[r(rng_)]);
}

Ally AllyFactory::create(int depth) {
    const std::vector<std::string> names = {"Фея","Дух","Голем-страж","Лесной кот"};
    std::uniform_int_distribution<int> n(0, (int)names.size()-1);
    std::uniform_int_distribution<int> jitter(-1, 1);
    int lvl = std::max(1, depth + jitter(rng_));
    int maxHp = 20 + lvl * 5;
    int sup = 4 + lvl;
    return Ally(names[n(rng_)], lvl, maxHp, maxHp, sup);
}

}
