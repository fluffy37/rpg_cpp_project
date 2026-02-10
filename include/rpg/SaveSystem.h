#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

namespace rpg {

class Player;
class Inventory;

class SaveManager {
public:
    struct SlotInfo {
        int slot = 0;
        std::string name;       // пользовательское имя сохранения
        std::string timestamp;  // когда сохранено (строкой)
    };

    // saveDir: папка для сохранений. Если используется SQLite — там будет лежать файл БД.
    // filename: имя файла для "старого" формата (fallback без SQLite)
    SaveManager(std::string saveDir="saves", std::string filename="save.txt");

    // Путь к файлу сохранений:
    //  - при SQLite: <saveDir>/saves.db
    //  - fallback: <saveDir>/<filename>
    std::string path() const;

    // Совместимость со старым API: это то же самое, что saveSlot(1, ...)/loadSlot(1, ...)
    bool save(const Player& player, const Inventory& inventory) const;
    bool load(Player& player, Inventory& inventory) const;

    // Мульти-слоты
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
    static std::string writeKeyValueString(const std::unordered_map<std::string, std::string>& kv);
    static std::optional<std::unordered_map<std::string, std::string>>
        readKeyValueString(const std::string& text);
    static std::string trim(const std::string& s);
};

}
