#ifndef FLAPPY_HPP
#define FLAPPY_HPP

#include <string>
#include "mbed.h"  // for printf
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "VL53L0X.h"

#define err_value 50
#define default_near_dist 150
#define default_far_dist 250

/**
 * @brief Whether a new instruction needs to be generated.
 */
typedef enum {
    NEW_INSTRUCTION_ON,
    NEW_INSTRUCTION_OFF,
    ALTER_INSTRUCTION_ON,
    END_INSTRUCTION_START,
    END_INSTRUCTION_ON
} instruction_state_t;

/**
 * @brief Whether the data of the tof sensor needs to be read.
 */
typedef enum {
    READ_INPUT_STARTED,
    READ_INPUT_ON,
    READ_INPUT_ENDED,
    READ_INPUT_OFF
} read_input_state_t;

/**
 * @brief Determine the current game state.
 */
typedef enum {
    GAME_INITIALIZED,
    GAME_CALIBRATION_NEAR,
    GAME_CALIBRATION_NEAR_PENDING,
    GAME_CALIBRATION_FAR,
    GAME_CALIBRATION_FAR_PENDING,
    GAME_TUTORIAL,
    GAME_STARTED,
    GAME_PAUSED,
    GAME_PAUSED_PENDING,
    GAME_ENDING,
    GAME_ENDED,
    GAME_ENDED_PENDING
} game_state_t;

/**
 * @brief Determine the current tutorial state.
 *        Essentially an extention of the game state, 
 *        but kept separate for easier use and organization.
 */
typedef enum {
    TUTORIAL_START,
    TUTORIAL_NEAR,
    TUTORIAL_FAR,
    TUTORIAL_ALT,
    TUTORIAL_NOT,
    TUTORIAL_PAUSE,
    TUTORIAL_GAME_END
} tutorial_state_t;

// shared varaibles across files
extern DevI2C devI2c; 
extern DigitalOut shutdown_pin; 
extern VL53L0X range; 
extern InterruptIn button;
extern EventQueue queue;
extern DigitalOut led1;
extern DigitalOut led2;
extern game_state_t game_state;
extern tutorial_state_t tutorial_state;
extern string player_name;
extern bool print_flag;

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
     * @brief Reset current score.
     */
    void reset_score();

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
 * @brief Initialization (ToF sensor & Bluetooth).
 */
bool flappy_init();

/**
 * @brief User calibration
 */
void calibrate();

/**
 * @brief Tutorial that's just reading, looking at lights, and pressing button.
 */
void tutorial();

/**
 * @brief Main game - turn on LED lights according to instruction
 *        and set current instrunction
 */
void show_lights();

/**
 * @brief Main game - read from ToF sensor.
 *        return the current input if read succeed, or 0 otherwise.
 */
uint32_t read_input();

/**
 * @brief Main game - analyzes input by comparing reader results
 *        with the current instruction.
 */
void analyze_input();

/**
 * @brief Main game - main loop.
 *        calls other corresponding functions.
 */
void main_game();

/**
 * @brief End of game
 */
void end_game();

/**
 * @brief reads player name via NFC and stores in player_name.
 */
bool read_player_name();


#endif
