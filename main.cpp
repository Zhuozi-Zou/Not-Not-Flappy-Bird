/**
 * @file main.cpp
 * @author Angela Zhu, Fillis Zou
 * @version 1.0
 *
 * @brief not not flappy bird
 */
#include "not.hpp"

int main()
{
    printf("Running Not Not Flappy Bird\n");

    // initialization, which starts ble advertising 
    // as well as the main game loop
    bool success = flappy_init();
    if (!success) return 1;

    printf("[WARNING] End of program reached");
    return 0;
}
