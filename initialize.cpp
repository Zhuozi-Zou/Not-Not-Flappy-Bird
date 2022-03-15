/**
 * @file initialize.cpp
 * @author Angela Zhu, Fillis Zou
 * @version 1.0
 *
 * @brief Initialize all sensors
 */
#include "not.hpp"
#include "mbed.h"

// Sensors drivers present in the VL53L1X library
#include "VL53L0X.h"

// Initialize ToF device
// all details please refer to manual:
// https://www.st.com/resource/en/user_manual/um2153-discovery-kit-for-iot-node-multichannel-communication-with-stm32l4-stmicroelectronics.pdf
DevI2C devI2c(PB_11, PB_10); 
DigitalOut shutdown_pin(PC_6);
VL53L0X range(&devI2c, &shutdown_pin, PC_7);

// Initialize the user button as interrupt input, using the high level api
InterruptIn button(BUTTON1);

// Application State
bool button_released_flag = false;

/**
 * @brief Interrupt handler for when the button is released.
 */
void button1_rise_handler()
{
    button_released_flag = true;
}

/**
 * @brief Initialize the ToF sensors and register interrupts.
 *
 * This function raises an assertion error if the sensors
 * cannot be initialized (need to check API again for this).
 */
void flappy_init() {
    range.init_sensor(0x53);
    button.rise(&button1_rise_handler);
}

/**
 * @brief Read and print (to stdout) the distance.
 */
void read_tof_sensor() {
    uint32_t distance;
    int status;
    status = range.get_distance(&distance);
    if (status == VL53L0X_ERROR_NONE) {
        printf("Range [mm]:            %6d\r\n", distance);
    } else {
        printf("Range [mm]:                --\r\n");
    }
}


/**
 * @brief Print (to stdout) information about the distance when
 * BUTTON1 is released.
 *
 * This function will print a reading from both the 
 * ToF sensor once each time BUTTON1 is released.
 *
 * This function does not return.
 */
void flappy_test()
{
    while (true) {
        if (button_released_flag) {
            read_tof_sensor();
            button_released_flag = false;
        }
        // read_tof_sensor();
        thread_sleep_for(10);
    }
}
