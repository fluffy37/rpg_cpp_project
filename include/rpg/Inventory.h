#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace rpg {

    class Item;
    class Player;
    class IInventoryObserver;

    class Inventory {
    public:
        explicit Inventory(std::size_t capacity = 20);
        ~Inventory();

        // запретить копирование, разрешить перемещение
        Inventory(const Inventory&) = delete;
        Inventory& operator=(const Inventory&) = delete;
        Inventory(Inventory&&) = default;
        Inventory& operator=(Inventory&&) = default;

        void attach(IInventoryObserver* obs);
        void detach(IInventoryObserver* obs);

        bool add(std::unique_ptr<Item> item);
        std::unique_ptr<Item> remove(std::size_t index);

        std::optional<std::string> useFirst(const std::string& itemType, Player& target);

        std::vector<std::string> listLabels() const;

        void serialize(std::unordered_map<std::string, std::string>& out, const std::string& prefix) const;
        void deserialize(const std::unordered_map<std::string, std::string>& in, const std::string& prefix);

        void clear();

    private:
        void notify(const std::string& event, const std::string& itemLabel,
            const std::optional<std::string>& message);

        std::size_t capacity_{ 20 };
        std::vector<std::unique_ptr<Item>> items_;
        std::vector<IInventoryObserver*> observers_;
    };

} // namespace rpg