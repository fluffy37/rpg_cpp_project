#include "rpg/SaveSystem.h"
#include "rpg/Player.h"
#include "rpg/Inventory.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace rpg {

    SaveManager::SaveManager(std::string saveDir, std::string filename)
        : saveDir_(std::move(saveDir)), filename_(std::move(filename)) {
    }

    std::string SaveManager::path() const {
        return (std::filesystem::path(saveDir_) / filename_).string();
    }

    static std::unordered_map<std::string, std::string>
        buildKv(const Player& player, const Inventory& inventory) {
        std::unordered_map<std::string, std::string> kv;
        kv["player.name"] = player.name();
        kv["player.level"] = std::to_string(player.level());
        kv["player.max_hp"] = std::to_string(player.maxHp());
        kv["player.hp"] = std::to_string(player.hp());
        kv["player.gold"] = std::to_string(player.gold());
        kv["player.exp"] = std::to_string(player.exp());
        kv["player.next_level_exp"] = std::to_string(player.nextLevelExp());
        kv["location.name"] = player.location().name;
        kv["location.depth"] = std::to_string(player.location().depth);
        kv["location.description"] = player.location().description;
        inventory.serialize(kv, "inventory");
        return kv;
    }

    static void applyKv(Player& player, Inventory& inventory,
        const std::unordered_map<std::string, std::string>& kv) {
        auto get = [&](const std::string& k, const std::string& d) {
            auto it = kv.find(k);
            return it == kv.end() ? d : it->second;
            };
        auto getInt = [&](const std::string& k, int d) {
            auto it = kv.find(k);
            if (it == kv.end()) return d;
            try { return std::stoi(it->second); }
            catch (...) { return d; }
            };

        player.mutableName() = get("player.name", "Player1");
        player.mutableLevel() = getInt("player.level", 1);
        player.mutableMaxHp() = getInt("player.max_hp", 60);
        player.mutableHp() = getInt("player.hp", player.maxHp());
        player.mutableGold() = getInt("player.gold", 0);
        player.mutableExp() = getInt("player.exp", 0);
        player.mutableNextLevelExp() = getInt("player.next_level_exp", 100);
        player.location().name = get("location.name", "Подземелье");
        player.location().depth = getInt("location.depth", 1);
        player.location().description = get("location.description", "");
        inventory.deserialize(kv, "inventory");
    }

    bool SaveManager::saveSlot(int slot, const Player& player, const Inventory& inventory,
        const std::string& name) const {
        std::filesystem::create_directories(saveDir_);
        auto kv = buildKv(player, inventory);
        std::filesystem::path p = std::filesystem::path(saveDir_) /
            ("save_slot" + std::to_string(slot) + ".txt");
        return writeKeyValueFile(p.string(), kv);
    }

    bool SaveManager::loadSlot(int slot, Player& player, Inventory& inventory) const {
        std::filesystem::path p = std::filesystem::path(saveDir_) /
            ("save_slot" + std::to_string(slot) + ".txt");
        auto kvOpt = readKeyValueFile(p.string());
        if (!kvOpt) return false;
        applyKv(player, inventory, *kvOpt);
        return true;
    }

    std::vector<SaveManager::SlotInfo> SaveManager::listSlots() const {
        std::vector<SlotInfo> out;
        std::error_code ec;
        if (!std::filesystem::exists(saveDir_, ec)) return out;

        for (const auto& entry : std::filesystem::directory_iterator(saveDir_, ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            auto name = entry.path().filename().string();
            const std::string prefix = "save_slot";
            const std::string suffix = ".txt";
            if (name.rfind(prefix, 0) != 0) continue;
            if (name.size() <= prefix.size() + suffix.size()) continue;
            if (name.substr(name.size() - suffix.size()) != suffix) continue;
            std::string num = name.substr(prefix.size(),
                name.size() - prefix.size() - suffix.size());
            try {
                SlotInfo s;
                s.slot = std::stoi(num);
                s.name = "Slot " + std::to_string(s.slot);
                s.timestamp = "";
                out.push_back(std::move(s));
            }
            catch (...) {
                continue;
            }
        }
        return out;
    }

    bool SaveManager::deleteSlot(int slot) const {
        std::filesystem::path p = std::filesystem::path(saveDir_) /
            ("save_slot" + std::to_string(slot) + ".txt");
        std::error_code ec;
        return std::filesystem::remove(p, ec);
    }

    bool SaveManager::writeKeyValueFile(const std::string& filePath,
        const std::unordered_map<std::string, std::string>& kv) {
        std::ofstream out(filePath);
        if (!out) return false;
        for (const auto& [k, v] : kv) {
            out << k << "=" << v << "\n";
        }
        return true;
    }

    std::optional<std::unordered_map<std::string, std::string>>
        SaveManager::readKeyValueFile(const std::string& filePath) {
        std::ifstream in(filePath);
        if (!in) return std::nullopt;

        std::unordered_map<std::string, std::string> kv;
        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            kv[trim(line.substr(0, pos))] = trim(line.substr(pos + 1));
        }
        if (kv.empty()) return std::nullopt;
        return kv;
    }

    std::string SaveManager::trim(const std::string& s) {
        auto ws = [](unsigned char c) {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
            };
        std::size_t b = 0;
        while (b < s.size() && ws(static_cast<unsigned char>(s[b]))) ++b;
        std::size_t e = s.size();
        while (e > b && ws(static_cast<unsigned char>(s[e - 1]))) --e;
        return s.substr(b, e - b);
    }

} // namespace rpg