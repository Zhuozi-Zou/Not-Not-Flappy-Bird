#ifndef FLAPPY_HPP
#define FLAPPY_HPP

#include <string>
#include "mbed.h"  // for printf
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "VL53L0X.h"

#define near_dist 100
#define far_dist 300

// shared varaibles across files
extern DevI2C devI2c; 
extern DigitalOut shutdown_pin; 
extern VL53L0X range; 
extern InterruptIn button;
extern EventQueue queue;
extern DigitalOut led1;
extern DigitalOut led2;

/**
 * @brief Setup the device by advertising it to other devices.
 *
 * Advertising setup configures how the device can be used via BLE.
 * The configuration may setup the device as a specific service.
 *
 * @param queue A queue to launch future events on.
 *
 * Precondition: BLE has been initialized.
 */
void advertise(EventQueue *queue);

/**
 * @brief A simple listener for some BLE events.
 */
class GapHandler : private mbed::NonCopyable<GapHandler>, public ble::Gap::EventHandler
{
public:
    /**
     * @brief Called when the device starts advertising itself to others.
     */
    void onAdvertisingStart(const ble::AdvertisingStartEvent &event) override;

    /**
     * @brief Called when another device connects to ours.
     */
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    /**
     * @brief Called when another connected evice disconnects from ours.
     */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;
};

/**
 * @brief A BLE game Service.
 *
 * This transmits data to the phone.
 */
class GameService
{
public:
    /**
     * @brief Construct a new game service object
     */
    GameService();

    /**
     * @brief Update current score.
     */
    void update_score();

    /**
     * @brief Update high score.
     */
    void update_high_score();

private:
    /**
     * @brief The current score.
     */
    uint8_t _score;

    /**
     * @brief The all time high score.
     */
    uint8_t _high_score;

    /**
     * @brief The GATT Characteristic that communicates the current score.
     */
    ReadOnlyGattCharacteristic<uint8_t> _score_characteristic;

    /**
     * @brief The GATT Characteristic that communicates the high score.
     */
    ReadOnlyGattCharacteristic<uint8_t> _high_score_characteristic;
};

/**
 * @brief Initialization (ToF sensor & Bluetooth).
 */
bool flappy_init();

/**
 * @brief User calibration
 */
void calibrate();

/**
 * @brief Main game - turn on LED lights according to instruction
 *        and return current instrunction
 */
int show_lights();

/**
 * @brief Main game - read from ToF sensor.
 *        return true on correct input and false otherwise.
 */
bool read_input(int instruction);

void main_game();

/**
 * @brief End of game
 */
void end_game();


#endif
