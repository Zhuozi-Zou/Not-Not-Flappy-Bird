/**
 * @file initialize.cpp
 * @author Angela Zhu, Fillis Zou
 * @version 1.0
 *
 * @brief Initialize all sensors
 */
#include "mbed.h"
#include "not.hpp"
#include "pretty_print.hpp"

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
 * @brief Extra initialization routines after BLE is done initializing.
 *
 * @param event Information about the BLE initialization.
 */
void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error) {
        print_error(event->error, "Error during the initialisation");
        return;
    }

    // You will see this in Mbed Studio and, if all goes well, on your
    // Bluetooth Scanner
    print_mac_address();

    BLE &ble = BLE::Instance();
    auto &gap = ble.gap();

    // Setup the default phy used in connection to 2M to reduce power consumption
    if (gap.isFeatureSupported(ble::controller_supported_features_t::LE_2M_PHY)) {
        ble::phy_set_t phys(false, true, false);

        ble_error_t error = gap.setPreferredPhys(&phys, &phys);
        if (error) {
            print_error(error, "GAP::setPreferedPhys failed");
        }
    }

    // Rely on the event queue to advertise the device over BLE
    queue.call(advertise, &queue);
}

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
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
 * @brief Initialize the ToF sensors and register interrupts.
 *
 * This function raises an assertion error if the sensors
 * cannot be initialized (need to check API again for this).
 */
bool flappy_init() {

    // The BLE class is a singleton
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    // Initialize BLE and then call our own function
    ble_error_t error = ble.init(&on_init_complete);
    if (error) {
        print_error(error, "Error returned by BLE::init");
        return false;
    }

    // Setup our own listener for specific events.
    GapHandler handler;
    auto &gap = ble.gap();
    gap.setEventHandler(&handler);
    range.init_sensor(0x53);
    button.rise(&button1_rise_handler);

    return true;
}

/**
 * @brief Print (to stdout) information about the distance when
 * BUTTON1 is released. For testing purposes only.
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