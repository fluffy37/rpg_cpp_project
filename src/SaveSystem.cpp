#include "rpg/SaveSystem.h"
#include "rpg/Player.h"
#include "rpg/Inventory.h"
#include <filesystem>
#include <fstream>
#include <sstream>

// Если в сборке есть SQLite3, CMake выставит RPG_USE_SQLITE=1
#if defined(RPG_USE_SQLITE) && RPG_USE_SQLITE
  #include <sqlite3.h>
#endif

namespace rpg {

SaveManager::SaveManager(std::string saveDir, std::string filename)
    : saveDir_(std::move(saveDir)), filename_(std::move(filename)) {}

std::string SaveManager::path() const {
#if defined(RPG_USE_SQLITE) && RPG_USE_SQLITE
    return (std::filesystem::path(saveDir_) / "saves.db").string();
#else
    return (std::filesystem::path(saveDir_) / filename_).string();
#endif
}

bool SaveManager::save(const Player& player, const Inventory& inventory) const {
    return saveSlot(1, player, inventory, "");
}

bool SaveManager::load(Player& player, Inventory& inventory) const {
    return loadSlot(1, player, inventory);
}

static std::unordered_map<std::string,std::string>
buildKv(const Player& player, const Inventory& inventory){
    std::unordered_map<std::string,std::string> kv;
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
                    const std::unordered_map<std::string,std::string>& kv){

    auto get = [&](const std::string& k, const std::string& d){
        auto it = kv.find(k); return it==kv.end()? d : it->second;
    };
    auto getInt = [&](const std::string& k, int d){
        auto it = kv.find(k);
        if (it==kv.end()) return d;
        try { return std::stoi(it->second); } catch (...) { return d; }
    };

    player.mutableName() = get("player.name","Player1");
    player.mutableLevel() = getInt("player.level",5);
    player.mutableMaxHp() = getInt("player.max_hp",60);
    player.mutableHp() = getInt("player.hp", player.maxHp());
    player.mutableGold() = getInt("player.gold",0);
    player.mutableExp() = getInt("player.exp",0);
    player.mutableNextLevelExp() = getInt("player.next_level_exp",500);

    player.location().name = get("location.name","Подземелье");
    player.location().depth = getInt("location.depth",1);
    player.location().description = get("location.description","");

    inventory.deserialize(kv, "inventory");
}

bool SaveManager::saveSlot(int slot, const Player& player, const Inventory& inventory,
                           const std::string& name) const {
    std::filesystem::create_directories(saveDir_);
    auto kv = buildKv(player, inventory);

#if defined(RPG_USE_SQLITE) && RPG_USE_SQLITE
    // SQLite: пишем одним запросом
    sqlite3* db = nullptr;
    if (sqlite3_open(path().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }

    const char* createSql =
        "CREATE TABLE IF NOT EXISTS saves ("
        "slot INTEGER PRIMARY KEY,"
        "name TEXT,"
        "timestamp TEXT,"
        "data TEXT"
        ");";
    if (sqlite3_exec(db, createSql, nullptr, nullptr, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    const char* upsertSql =
        "REPLACE INTO saves(slot, name, timestamp, data) VALUES(?, ?, datetime('now'), ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, upsertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    std::string payload = writeKeyValueString(kv);
    std::string effectiveName = name.empty() ? ("Slot " + std::to_string(slot)) : name;

    sqlite3_bind_int(stmt, 1, slot);
    sqlite3_bind_text(stmt, 2, effectiveName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, payload.c_str(), (int)payload.size(), SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
#else
    // Fallback без SQLite: файлы save_slotN.txt
    std::filesystem::path p = std::filesystem::path(saveDir_) / ("save_slot" + std::to_string(slot) + ".txt");
    return writeKeyValueFile(p.string(), kv);
#endif
}

bool SaveManager::loadSlot(int slot, Player& player, Inventory& inventory) const {
#if defined(RPG_USE_SQLITE) && RPG_USE_SQLITE
    sqlite3* db = nullptr;
    if (sqlite3_open(path().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    const char* createSql =
        "CREATE TABLE IF NOT EXISTS saves ("
        "slot INTEGER PRIMARY KEY,"
        "name TEXT,"
        "timestamp TEXT,"
        "data TEXT"
        ");";
    (void)sqlite3_exec(db, createSql, nullptr, nullptr, nullptr);

    const char* selectSql = "SELECT data FROM saves WHERE slot=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_int(stmt, 1, slot);
    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        std::string payload = txt ? reinterpret_cast<const char*>(txt) : "";
        auto kvOpt = readKeyValueString(payload);
        if (kvOpt) {
            applyKv(player, inventory, *kvOpt);
            ok = true;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
#else
    std::filesystem::path p = std::filesystem::path(saveDir_) / ("save_slot" + std::to_string(slot) + ".txt");
    auto kvOpt = readKeyValueFile(p.string());
    if (!kvOpt) return false;
    applyKv(player, inventory, *kvOpt);
    return true;
#endif
}

std::vector<SaveManager::SlotInfo> SaveManager::listSlots() const {
    std::vector<SlotInfo> out;
#if defined(RPG_USE_SQLITE) && RPG_USE_SQLITE
    sqlite3* db = nullptr;
    if (sqlite3_open(path().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return out;
    }
    const char* createSql =
        "CREATE TABLE IF NOT EXISTS saves ("
        "slot INTEGER PRIMARY KEY,"
        "name TEXT,"
        "timestamp TEXT,"
        "data TEXT"
        ");";
    (void)sqlite3_exec(db, createSql, nullptr, nullptr, nullptr);

    const char* listSql = "SELECT slot, name, timestamp FROM saves ORDER BY slot;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, listSql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return out;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SlotInfo s;
        s.slot = sqlite3_column_int(stmt, 0);
        const unsigned char* n = sqlite3_column_text(stmt, 1);
        const unsigned char* t = sqlite3_column_text(stmt, 2);
        s.name = n ? reinterpret_cast<const char*>(n) : "";
        s.timestamp = t ? reinterpret_cast<const char*>(t) : "";
        out.push_back(std::move(s));
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
#else
    // Fallback: пытаемся найти файлы save_slot*.txt
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
        std::string num = name.substr(prefix.size(), name.size() - prefix.size() - suffix.size());
        try {
            SlotInfo s;
            s.slot = std::stoi(num);
            s.name = "Slot " + std::to_string(s.slot);
            s.timestamp = "";
            out.push_back(std::move(s));
        } catch (...) {
            continue;
        }
    }
#endif
    return out;
}

bool SaveManager::deleteSlot(int slot) const {
#if defined(RPG_USE_SQLITE) && RPG_USE_SQLITE
    sqlite3* db = nullptr;
    if (sqlite3_open(path().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    const char* createSql =
        "CREATE TABLE IF NOT EXISTS saves ("
        "slot INTEGER PRIMARY KEY,"
        "name TEXT,"
        "timestamp TEXT,"
        "data TEXT"
        ");";
    (void)sqlite3_exec(db, createSql, nullptr, nullptr, nullptr);
    const char* delSql = "DELETE FROM saves WHERE slot=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, delSql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_int(stmt, 1, slot);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
#else
    std::filesystem::path p = std::filesystem::path(saveDir_) / ("save_slot" + std::to_string(slot) + ".txt");
    std::error_code ec;
    return std::filesystem::remove(p, ec);
#endif
}

bool SaveManager::writeKeyValueFile(const std::string& filePath,
                                   const std::unordered_map<std::string,std::string>& kv) {
    std::ofstream out(filePath, std::ios::binary);
    if (!out) return false;
    for (const auto& [k,v] : kv) out << k << "=" << v << "\n";
    return true;
}

std::string SaveManager::writeKeyValueString(const std::unordered_map<std::string, std::string>& kv) {
    std::ostringstream out;
    for (const auto& [k,v] : kv) out << k << "=" << v << "\n";
    return out.str();
}

std::optional<std::unordered_map<std::string,std::string>>
SaveManager::readKeyValueFile(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return std::nullopt;
    std::unordered_map<std::string,std::string> kv;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0]=='#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        kv[trim(line.substr(0,pos))] = trim(line.substr(pos+1));
    }
    return kv;
}

std::optional<std::unordered_map<std::string, std::string>>
SaveManager::readKeyValueString(const std::string& text) {
    std::istringstream in(text);
    std::unordered_map<std::string,std::string> kv;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0]=='#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        kv[trim(line.substr(0,pos))] = trim(line.substr(pos+1));
    }
    if (kv.empty()) return std::nullopt;
    return kv;
}

std::string SaveManager::trim(const std::string& s) {
    auto ws = [](unsigned char c){ return c==' '||c=='\t'||c=='\r'||c=='\n'; };
    std::size_t b=0; while (b<s.size() && ws((unsigned char)s[b])) ++b;
    std::size_t e=s.size(); while (e>b && ws((unsigned char)s[e-1])) --e;
    return s.substr(b, e-b);
}

}
