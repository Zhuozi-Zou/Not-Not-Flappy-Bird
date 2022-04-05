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

// Initialize the user button as interrupt input
InterruptIn button(BUTTON1);
// Initialize LED 1 and 2
DigitalOut led1(LED1);
DigitalOut led2(LED2);

// main event queue
EventQueue queue;
// // game state
// game_state_t game_state;

/**
 * @brief Interrupt handler for when the button is released.
 */
void button1_rise_handler()
{
    // set printing to true
    print_flag = true;
    // update states
    if (game_state == GAME_INITIALIZED) {
        game_state = GAME_CALIBRATION_NEAR;
    } else if (game_state == GAME_CALIBRATION_NEAR_PENDING) {
        game_state = GAME_CALIBRATION_FAR;
    } else if (game_state == GAME_CALIBRATION_FAR_PENDING) {
        game_state = GAME_TUTORIAL;
    } else if (game_state == GAME_TUTORIAL) {
        if (tutorial_state == TUTORIAL_START)
            tutorial_state = TUTORIAL_NEAR;
        else if (tutorial_state == TUTORIAL_NEAR)
            tutorial_state = TUTORIAL_FAR;
        else if (tutorial_state == TUTORIAL_FAR)
            tutorial_state = TUTORIAL_ALT;
        else if (tutorial_state == TUTORIAL_ALT)
            tutorial_state = TUTORIAL_NOT;
        else if (tutorial_state == TUTORIAL_NOT)
            tutorial_state = TUTORIAL_PAUSE;
        else if (tutorial_state == TUTORIAL_PAUSE)
            tutorial_state = TUTORIAL_GAME_END;
        else if (tutorial_state == TUTORIAL_GAME_END)
            game_state = GAME_STARTED;
    } else if (game_state == GAME_STARTED) {
        game_state = GAME_PAUSED;
    } else if (game_state == GAME_PAUSED_PENDING) {
        game_state = GAME_STARTED;
    } else if (game_state == GAME_ENDED_PENDING) {
        game_state = GAME_STARTED;
    }
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
 * @brief Initialize the ToF sensors and register interrupts.
 *
 * This function raises an assertion error if the sensors
 * cannot be initialized (need to check API again for this).
 */
bool flappy_init() {
    assert(read_player_name());
    printf("Welcome, *%s*!\n\n", player_name.c_str());
    printf("Please connect your smartphone to the board using bluetooth.\n");

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

    button.rise(queue.event(button1_rise_handler));
    game_state = GAME_INITIALIZED;
    tutorial_state = TUTORIAL_START;

    queue.dispatch_forever();

    return true;
}