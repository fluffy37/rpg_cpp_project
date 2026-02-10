#pragma once
#include <string>

namespace rpg {

class NPC {
public:
    NPC(std::string name, std::string role) : name_(std::move(name)), role_(std::move(role)) {}
    std::string talk() const {
        return name_ + " (" + role_ + "): «Будь осторожен в глубине подземелья…»";
    }
private:
    std::string name_;
    std::string role_;
};

}
