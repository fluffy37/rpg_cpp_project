#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <filesystem>

namespace rpg {

    class Player;
    class Inventory;

    class SaveManager {
    public:
        struct SlotInfo {
            int slot = 0;
            std::string name;
            std::string timestamp;
        };

        explicit SaveManager(std::string saveDir = "saves", std::string filename = "save.txt");

        std::string path() const;
        bool saveSlot(int slot, const Player& player, const Inventory& inventory,
            const std::string& name = "") const;
        bool loadSlot(int slot, Player& player, Inventory& inventory) const;
        std::vector<SlotInfo> listSlots() const;
        bool deleteSlot(int slot) const;

    private:
        std::string saveDir_;
        std::string filename_;

        static bool writeKeyValueFile(const std::string& filePath,
            const std::unordered_map<std::string, std::string>& kv);
        static std::optional<std::unordered_map<std::string, std::string>>
            readKeyValueFile(const std::string& filePath);
        static std::string trim(const std::string& s);
    };

} // namespace rpg