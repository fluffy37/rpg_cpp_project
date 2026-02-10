#include "rpg/Inventory.h"
#include "rpg/Items.h"       // у вас уже есть реализация Item и ItemRegistry
#include "rpg/Observers.h"
#include "rpg/Player.h"
#include <algorithm>

namespace rpg {

    Inventory::Inventory(std::size_t capacity) : capacity_(capacity) {}
    Inventory::~Inventory() = default;

    void Inventory::attach(IInventoryObserver* obs) {
        if (!obs) return;
        for (auto* o : observers_) if (o == obs) return;
        observers_.push_back(obs);
    }

    void Inventory::detach(IInventoryObserver* obs) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
    }

    void Inventory::notify(const std::string& event, const std::string& itemLabel,
        const std::optional<std::string>& message) {
        for (auto* obs : observers_) if (obs) obs->onInventoryEvent(event, itemLabel, message);
    }

    bool Inventory::add(std::unique_ptr<Item> item) {
        if (!item) return false;
        if (items_.size() >= capacity_) return false;
        std::string lbl = item->label();
        items_.push_back(std::move(item)); // перемещаем владение
        notify("item_added", lbl, std::nullopt);
        return true;
    }

    std::unique_ptr<Item> Inventory::remove(std::size_t index) {
        if (index >= items_.size()) return nullptr;
        std::string lbl = items_[index] ? items_[index]->label() : std::string();
        auto removed = std::move(items_[index]); // перемещение
        items_.erase(items_.begin() + static_cast<long>(index));
        notify("item_removed", lbl, std::nullopt);
        return removed;
    }

    std::optional<std::string> Inventory::useFirst(const std::string& itemType, Player& target) {
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (items_[i] && items_[i]->type() == itemType) {
                std::string lbl = items_[i]->label();
                std::string msg = items_[i]->use(target);
                // удаляем элемент после использования
                items_.erase(items_.begin() + static_cast<long>(i));
                notify("item_used", lbl, msg);
                return msg;
            }
        }
        return std::nullopt;
    }

    std::vector<std::string> Inventory::listLabels() const {
        std::vector<std::string> out;
        out.reserve(items_.size());
        for (const auto& it : items_) out.push_back(it ? it->label() : "(null)");
        return out;
    }

    void Inventory::serialize(std::unordered_map<std::string, std::string>& out, const std::string& prefix) const {
        out[prefix + ".count"] = std::to_string(items_.size());
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (!items_[i]) continue;
            items_[i]->serialize(out, prefix + "." + std::to_string(i));
        }
    }

    void Inventory::deserialize(const std::unordered_map<std::string, std::string>& in, const std::string& prefix) {
        clear();
        std::size_t count = 0;
        auto itc = in.find(prefix + ".count");
        if (itc != in.end()) {
            try { count = static_cast<std::size_t>(std::stoul(itc->second)); }
            catch (...) { count = 0; }
        }

        for (std::size_t i = 0; i < count; ++i) {
            std::string ip = prefix + "." + std::to_string(i);
            auto itType = in.find(ip + ".type");
            if (itType == in.end()) continue;
            auto item = ItemRegistry::instance().create(itType->second, in, ip);
            if (item) {
                if (!add(std::move(item))) break;
            }
        }
    }

    void Inventory::clear() {
        items_.clear();
        notify("cleared", "", std::nullopt);
    }

} // namespace rpg