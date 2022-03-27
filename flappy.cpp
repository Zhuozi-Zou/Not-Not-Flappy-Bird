/**
 * @file main.cpp
 * @author Angela Zhu, Fillis Zou
 * @version 1.0
 *
 * @brief main game functions
 */
#include "not.hpp"
#include <chrono>

game_state_t game_state;
Timeout timer;

instruction_state_t instruction_state = NEW_INSTRUCTION_ON;
read_input_state_t read_input_state = READ_INPUT_OFF;
GameService game_service{};

// current rate, or interval between each instruction
// starts off with 3s per instruction, with a minimum
// rate of 1s per instruction
std::chrono::microseconds rate = 3000ms;
// reduce the rate when it reaches current rate
std::chrono::microseconds reduce_rate = 10ms;
// minimum rate
std::chrono::microseconds min_rate = 1000ms;

void timeout_handler() {
    read_input_state = READ_INPUT_ENDED;
    if (rate > min_rate)
        rate -= reduce_rate;
}

void calibrate() {
    printf("current near distance: %d\n", near_dist);
    printf("current far distance: %d\n", far_dist);
    game_state = GAME_CALIBRATION_PENDING;
}

int show_lights() {
    int not_led = rand() % 2; // 0 or 1
    int instr_led = rand() % 3; // 0, 1, or 2

    if (not_led == 1) led1.write(1);
    else led1.write(0);
    // alternation is slightly more complicated,
    // will be implemented later
    if (instr_led == 1) led2.write(1);
    else led2.write(0);

    // 0 = far, 1 = near, 2 = alternate
    // 10 = near, 11 = far, 12 = stay still
    return not_led * 10 + instr_led;
}

// there will be a big update for this 
bool read_input(int instruction) {
    uint32_t distance;
    int status;
    status = range.get_distance(&distance);
    if (status == VL53L0X_ERROR_NONE) {
        printf("Range [mm]:            %6d\r\n", distance);
    } else {
        printf("Range [mm]:            --\r\n");
    }
    if ((instruction == 0 || instruction == 11) && distance <= near_dist + 50)
        return true;
    if ((instruction == 1 || instruction == 10) && distance >= far_dist - 50)
        return true;
    return false;
}

void analize_input() {
    printf("analize input\n");

    // TODO
    if (1) {
        game_service.update_score();
        instruction_state = NEW_INSTRUCTION_ON;
    }
    else {
        game_state = GAME_ENDED;
    }
    
    read_input_state = READ_INPUT_OFF;
}

void main_game() {
    int instruction;
    bool input;

    if (game_state == GAME_CALIBRATION) {
        calibrate();
    }
    // do stuff only if currently in game
    else if (game_state == GAME_STARTED) {
        if (instruction_state == NEW_INSTRUCTION_ON) {
            instruction = show_lights();
            printf("current instruction: %d\n", instruction);
            instruction_state = NEW_INSTRUCTION_OFF;
            read_input_state = READ_INPUT_STARTED;
        }

        if (read_input_state == READ_INPUT_STARTED) {
            timer.attach(&timeout_handler, rate);
            read_input_state = READ_INPUT_ON;
        }
        // Note that alternation requires multiple input reads
        // thus 
        else if (read_input_state == READ_INPUT_ON) {
            input = read_input(instruction);
            printf("result: %d\n", (int)input);
        }
        else if (read_input_state == READ_INPUT_ENDED) {
            analize_input();
        }
    }
    else if (game_state == GAME_ENDED) {
        end_game();
    } 
    else if (game_state == GAME_RESTARTED) {
        restart_game();
    }
}

void end_game() {
    printf("game end\n");
    game_state =  GAME_ENDED_PENDING;
}

void restart_game() {
    game_service.reset_score();

    printf("game restarted\n");
    game_state = GAME_INITIALIZED;
}