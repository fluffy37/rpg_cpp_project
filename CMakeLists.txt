cmake_minimum_required(VERSION 3.16)
project(rpg LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rpg
  src/main.cpp
  src/GameManager.cpp
  src/Player.cpp
  src/Enemy.cpp
  src/Ally.cpp
  src/Observers.cpp
  src/Inventory.cpp
  src/Items.cpp
  src/Factories.cpp
  src/SaveSystem.cpp
)
target_include_directories(rpg PRIVATE include)

# --- SQLite (для слотов сохранений) ---
# Если в системе установлен SQLite3 (dev-пакет), подключаем его.
# Если не установлен — проект соберётся, но SaveSystem будет использовать fallback
# в виде файлов save_slotN.txt.
find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
  target_link_libraries(rpg PRIVATE SQLite::SQLite3)
  target_compile_definitions(rpg PRIVATE RPG_USE_SQLITE=1)
else()
  target_compile_definitions(rpg PRIVATE RPG_USE_SQLITE=0)
endif()
