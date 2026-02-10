# Консольная RPG на C++ (C++17)

- Абстрактные классы/полиморфизм: Character -> Player/Enemy/Ally, Item -> HealthPotion
- Паттерны: Observer (инвентарь), Factory (генерация врагов/NPC/союзников)
- Сохранение/загрузка: мульти-слоты (1-3)
  - Если в системе найден SQLite3 — используется база `saves/saves.db`
  - Если SQLite3 не найден — fallback на файлы `saves/save_slotN.txt`

## Сборка (CMake)
```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\rpg.exe
```

Сохранения:
- SQLite: `saves/saves.db`
- fallback: `saves/save_slot1.txt`, `saves/save_slot2.txt`, ...
