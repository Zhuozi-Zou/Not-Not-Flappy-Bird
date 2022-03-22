/**
 * @file main.cpp
 * @author Angela Zhu, Fillis Zou
 * @version 1.0
 *
 * @brief main game functions
 */
#include "not.hpp"

instruction_state_t instruction_state = NEW_INSTRUCTION_ON;
read_input_state_t read_input_state = READ_INPUT_OFF;
game_state_t game_state = GAME_ENDED;

// current rate, or interval between each instruction
// starts off with 3s per instruction, with a minimum
// rate of 1s per instruction
int rate = 3000;
// time counter, evaluated in main_game(), updates 
// flags and new rate when it reaches current rate.
int counter = 0;

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
        printf("Range [mm]:                --\r\n");
    }
    if ((instruction == 0 || instruction == 11) && distance <= near_dist + 50)
        return true;
    if ((instruction == 1 || instruction == 10) && distance >= far_dist - 50)
        return true;
    return false;
}

void main_game() {
    int instruction;
    bool input;
    // do stuff only if currently in game
    if (game_state == GAME_STARTED) {
        if (instruction_state == NEW_INSTRUCTION_ON) {
            instruction = show_lights();
            printf("current instruction: %d\n", instruction);
        }
        // Note that alternation requires multiple input reads
        // thus 
        if (read_input_state == READ_INPUT_ON) {
            input = read_input(instruction);
            printf("result: %d\n", (int)input);
        }
    }
}

void end_game() {

}

