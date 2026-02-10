#pragma once
#include <string>
namespace rpg {
struct Location {
    std::string name{"Подземелье"};
    int depth{1};
    std::string description{"Сырые каменные коридоры."};
    std::string toString() const { return name + " уровня " + std::to_string(depth); }
};
}
