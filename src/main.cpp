#include "rpg/GameManager.h"
#ifdef _WIN32
#include <windows.h>
#include <ctime> 
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