#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

namespace rpg {

class Player;

class Item {
public:
    virtual ~Item() = default;
    virtual std::string type() const = 0;
    virtual std::string label() const = 0;
    virtual std::string use(Player& target) = 0;
    virtual void serialize(std::unordered_map<std::string,std::string>& out,
                           const std::string& prefix) const = 0;
};

class HealthPotion final : public Item {
public:
    explicit HealthPotion(int healAmount=25) : healAmount_(healAmount) {}
    std::string type() const override { return "health_potion"; }
    std::string label() const override;
    std::string use(Player& target) override;
    void serialize(std::unordered_map<std::string,std::string>& out,
                   const std::string& prefix) const override;
    static std::unique_ptr<Item> createFromMap(
        const std::unordered_map<std::string,std::string>& in, const std::string& prefix);
private:
    int healAmount_{25};
};

class ItemRegistry {
public:
    using Creator = std::function<std::unique_ptr<Item>(
        const std::unordered_map<std::string,std::string>&, const std::string&)>;
    static ItemRegistry& instance();
    void registerType(const std::string& type, Creator creator);
    std::unique_ptr<Item> create(const std::string& type,
                                 const std::unordered_map<std::string,std::string>& in,
                                 const std::string& prefix) const;
private:
    std::unordered_map<std::string, Creator> creators_;
};

void registerDefaultItems();

}
