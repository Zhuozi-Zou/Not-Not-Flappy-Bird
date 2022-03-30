/**
 * @file main.cpp
 * @author Angela Zhu, Fillis Zou
 * @version 1.0
 *
 * @brief main game functions
 */
#include "not.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iterator>

// shared variables
game_state_t game_state;
Timeout timer;

instruction_state_t instruction_state = NEW_INSTRUCTION_ON;
read_input_state_t read_input_state = READ_INPUT_OFF;
GameService game_service{};

// current instruction
int instruction;
// previous instruction
int prev_instruction = -1;
// previous input
uint32_t prev_input = 0;
// number of alternations (far to near/near to far) during the current read input period
int alter_input = 0;
// minimum distance during the current read input period
uint32_t min_distance = 0;
// maximum distance during the current read input period
uint32_t max_distance = 0;
// near distance
uint32_t near_dist = default_near_dist;
// far distance
uint32_t far_dist = default_far_dist;

// current rate, or interval between each instruction
// starts off with 3s per instruction, with a minimum
// rate of 1s per instruction
std::chrono::microseconds rate = 3000ms;
// reduce the rate when it reaches current rate
std::chrono::microseconds reduce_rate = 10ms;
// minimum rate
std::chrono::microseconds min_rate = 1000ms;

void reset_input_globals() {
    prev_input = 0;
    alter_input = 0;
    min_distance = 0;
    max_distance = 0;
}

void timeout_handler() {
    read_input_state = READ_INPUT_ENDED;
    if (rate > min_rate)
        rate -= reduce_rate;
}

void calibrate() {
    if (game_state == GAME_CALIBRATION_NEAR)
        game_state = GAME_CALIBRATION_NEAR_PENDING;
    else 
        game_state = GAME_CALIBRATION_FAR_PENDING;
    
    uint32_t distance = 0;
    int count = 0;

    for (int i = 0; i < 10; i++) {
        uint32_t input = read_input();
        distance += input;

        if (input > 0)
            count++;
    }

    if (count != 0) { // has valid inputs
        distance /= (count * 1.0);

        if (game_state == GAME_CALIBRATION_NEAR_PENDING) 
            near_dist = distance + err_value;
        else 
            far_dist = distance - err_value;
        
    }

    if (game_state == GAME_CALIBRATION_NEAR_PENDING) 
        printf("Please put your hand to the \"far distance\" and press the user button\n");

    if (game_state == GAME_CALIBRATION_FAR_PENDING) {
        if (far_dist <= near_dist) {
            near_dist = default_near_dist;
            far_dist = default_far_dist;
            printf("invalid calibration, use default distances instead\n");
        }

        printf("current near distance: %d\n", near_dist);
        printf("current far distance: %d\n", far_dist);
    }
}

void show_lights() {
    instruction_state = NEW_INSTRUCTION_OFF;
    reset_input_globals();

    int not_led = rand() % 2; // 0 or 1
    int instr_led = rand() % 3; // 0, 1, or 2
    // 0 = far, 1 = near, 2 = alternate
    // 10 = near, 11 = far, 12 = stay still
    instruction = not_led * 10 + instr_led;

    // "stay still" instruction cannot be first one or right after alternate
    while (instruction == 12 && (prev_instruction == -1 || prev_instruction == 2)) {
        not_led = rand() % 2;
        instr_led = rand() % 3;
        instruction = not_led * 10 + instr_led;
    }

    if (not_led == 1) led1.write(1);
    else led1.write(0);
    
    if (instr_led == 1) led2.write(1);
    else if (instr_led == 0) led2.write(0);
    else instruction_state = ALTER_INSTRUCTION_ON;

    printf("current instruction: %d\n", instruction);
    
    read_input_state = READ_INPUT_STARTED;
    prev_instruction = instruction;
}

void blinky() {
    led2 = !led2;
}

uint32_t read_input() {
    uint32_t distance;
    int status;
    status = range.get_distance(&distance);

    if (status == VL53L0X_ERROR_NONE) {
        if (prev_input != 0) {
            // from near to far or from far to near
            if ((prev_input <= near_dist && distance >= far_dist) ||
                (prev_input >= far_dist && distance <= near_dist))
                alter_input++;
            if (distance < min_distance)
                min_distance = distance;
            if (distance > max_distance)
                max_distance = distance;
        } 
        else {
            min_distance = distance;
            max_distance = distance;
        }

        prev_input = distance;
        return distance;
    }
    
    return 0;
}

void analyze_input() {
    read_input_state = READ_INPUT_OFF;

    bool input_correct = false;
    
    if (((instruction == 0 || instruction == 11) && prev_input >= far_dist) || 
        ((instruction == 1 || instruction == 10) && prev_input <= near_dist) ||
        (instruction == 2 && alter_input >= 3) ||
        (instruction == 12 && (max_distance - min_distance <= err_value * 0.8)))
        input_correct = true;

    if (input_correct) {
        game_service.update_score();
        instruction_state = NEW_INSTRUCTION_ON;
    } else {
        game_state = GAME_ENDED;
    }
}

void main_game() {
    if (game_state == GAME_CALIBRATION_NEAR || game_state == GAME_CALIBRATION_FAR) {
        calibrate();
    }
    // Do stuff only if currently in game
    else if (game_state == GAME_STARTED) {
        // New turn
        if (instruction_state == NEW_INSTRUCTION_ON) {
            show_lights();
        }
        else if (instruction_state == ALTER_INSTRUCTION_ON) {
            blinky();
        }

        if (read_input_state == READ_INPUT_STARTED) {
            read_input_state = READ_INPUT_ON;
            timer.attach(&timeout_handler, rate);
        }
        // Note that alternation requires multiple input reads
        // thus needs change later.
        else if (read_input_state == READ_INPUT_ON) {
            read_input();
        }
        else if (read_input_state == READ_INPUT_ENDED) {
            analyze_input();
        }
    }
    else if (game_state == GAME_ENDED) {
        end_game();
    } 
}

void end_game() {
    instruction_state = NEW_INSTRUCTION_ON;
    read_input_state = READ_INPUT_OFF;
    game_state =  GAME_ENDED_PENDING;

    game_service.update_high_score();
    printf("game end\n");
    
    game_service.reset_score();
    reset_input_globals();
    near_dist = default_near_dist;
    far_dist = default_far_dist;
}