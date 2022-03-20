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

    bool success = flappy_init();
    if (!success) return 1;

    // TODO: uncomment for testing
    // flappy_test();

    // ble_test();

    printf("[WARNING] End of program reached");
    return 0;
}
