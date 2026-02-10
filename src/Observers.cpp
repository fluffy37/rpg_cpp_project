#include "rpg/Observers.h"
#include <iostream>

namespace rpg {

void ConsoleInventoryObserver::onInventoryEvent(const std::string& event,
                                               const std::string& itemLabel,
                                               const std::optional<std::string>& message) {
    const char* p = "[Инвентарь] ";
    if (event == "item_added") {
        std::cout << p << "Добавлено: " << itemLabel << "\n";
    } else if (event == "item_removed") {
        std::cout << p << "Удалено: " << itemLabel << "\n";
    } else if (event == "item_used") {
        std::cout << p << "Использовано: " << itemLabel;
        if (message && !message->empty()) std::cout << ". " << *message;
        std::cout << "\n";
    } else if (event == "cleared") {
        std::cout << p << "Инвентарь очищен.\n";
    }
}

}
