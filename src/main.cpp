#include "rpg/GameManager.h"
#include <ctime>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
    rpg::GameManager gm(seed);
    gm.run();
    return 0;
}