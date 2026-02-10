#pragma once
#include <string>
#include <optional>

namespace rpg {

class IInventoryObserver {
public:
    virtual ~IInventoryObserver() = default;
    virtual void onInventoryEvent(const std::string& event,
                                  const std::string& itemLabel,
                                  const std::optional<std::string>& message) = 0;
};

class ConsoleInventoryObserver final : public IInventoryObserver {
public:
    void onInventoryEvent(const std::string& event,
                          const std::string& itemLabel,
                          const std::optional<std::string>& message) override;
};

}
