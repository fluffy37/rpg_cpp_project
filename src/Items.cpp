#include "rpg/Items.h"
#include "rpg/Player.h"

namespace rpg {

std::string HealthPotion::label() const {
    return "Зелье здоровья (+" + std::to_string(healAmount_) + " HP)";
}

std::string HealthPotion::use(Player& target) {
    int healed = target.heal(healAmount_);
    return "+" + std::to_string(healed) + " HP";
}

void HealthPotion::serialize(std::unordered_map<std::string,std::string>& out,
                            const std::string& prefix) const {
    out[prefix + ".type"] = type();
    out[prefix + ".heal_amount"] = std::to_string(healAmount_);
}

std::unique_ptr<Item> HealthPotion::createFromMap(
    const std::unordered_map<std::string,std::string>& in, const std::string& prefix) {
    int heal = 25;
    auto it = in.find(prefix + ".heal_amount");
    if (it != in.end()) {
        try { heal = std::stoi(it->second); } catch (...) { heal = 25; }
    }
    return std::make_unique<HealthPotion>(heal);
}

ItemRegistry& ItemRegistry::instance() {
    static ItemRegistry reg;
    return reg;
}

void ItemRegistry::registerType(const std::string& type, Creator creator) {
    creators_[type] = std::move(creator);
}

std::unique_ptr<Item> ItemRegistry::create(const std::string& type,
                                           const std::unordered_map<std::string,std::string>& in,
                                           const std::string& prefix) const {
    auto it = creators_.find(type);
    if (it == creators_.end()) return nullptr;
    return (it->second)(in, prefix);
}

void registerDefaultItems() {
    auto& reg = ItemRegistry::instance();
    reg.registerType("health_potion",
        [](const std::unordered_map<std::string,std::string>& in, const std::string& prefix) {
            return HealthPotion::createFromMap(in, prefix);
        }
    );
}

}
