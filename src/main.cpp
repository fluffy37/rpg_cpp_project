#include "rpg/GameManager.h"
#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // Чтобы русский текст не превращался в "кракозябры" в большинстве консолей
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    rpg::GameManager gm(42);
    gm.run();
    return 0;
}
