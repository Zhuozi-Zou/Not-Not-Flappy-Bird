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
tutorial_state_t tutorial_state;
Timeout timer;

instruction_state_t instruction_state = NEW_INSTRUCTION_ON;
read_input_state_t read_input_state = READ_INPUT_OFF;
GameService game_service{};

// current instruction
int instruction;
// previous instruction
int prev_instruction = -1;
// counter for how many times LEDs blinked at end of game
int end_blink = 0;
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
// print flag indicating whether instructions should be printed or not
bool print_flag = false;

// current rate, or interval between each instruction
// starts off with 3s per instruction, with a minimum
// rate of 1s per instruction
std::chrono::microseconds rate = 3000ms;
// reduce the rate when it reaches current rate
std::chrono::microseconds reduce_rate = 25ms;
// minimum rate
std::chrono::microseconds min_rate = 1100ms;

void reset_input_globals() {
    prev_input = 0;
    alter_input = 0;
    min_distance = 0;
    max_distance = 0;
    end_blink = 0;
}

void timeout_handler() {
    if (game_state == GAME_STARTED) {
        read_input_state = READ_INPUT_ENDED;
        if (rate > min_rate)
            rate -= reduce_rate;
    }
    else if (game_state == GAME_ENDING) {
        led1.write(0);
        led2.write(0);
        game_state = GAME_ENDED;
    }
}

/**
 * @brief small helper that simply blinks LED1 or 2
 */
void blinky() {
    led2 = !led2;
    if (game_state == GAME_ENDING) {
        led1 = !led1;
        thread_sleep_for(75);
    }
    else if (game_state == GAME_TUTORIAL) {
        if (tutorial_state == TUTORIAL_GAME_END) led1 = !led1;
        thread_sleep_for(100);
    }
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

    if (game_state == GAME_CALIBRATION_NEAR_PENDING) {
        printf("Now move your hand farther the sensor (move >10 cm, for best experience), and press the blue user button when you're ready.\n");
        printf("This will be recorded as your \"far\" distance.\n");
    }

    if (game_state == GAME_CALIBRATION_FAR_PENDING) {
        printf("\n\n ===== Calibration Complete! =====\n\n");

        if (far_dist <= near_dist) {
            near_dist = default_near_dist;
            far_dist = default_far_dist;
            printf("Sorry, the difference between your near and far distances are too small. \n");
            printf("We will be using the default settings instead.\n\n");
        }

        printf("Current near distance: %dmm\n", near_dist - err_value);
        printf("Current far distance: %dmm\n\n", far_dist + err_value);
        printf("Please press the user button to start the tutorial.\n\n");
    }
}

void tutorial() {
    if (tutorial_state == TUTORIAL_START) {
        if (print_flag) {
            printf("\n\n ===== Tutorial =====\n\n");
            printf("This game is simply played by moving your hand close to or far from the distance sensor according to instructions given.\n");
            printf("There are a total of 3 different basic instructions, plus the negation of those 3, making a total of 6.\n");
            printf("Instructions will be given using the two LED lights ob the board, which we will walk you through later.\n\n");
            printf("You can press the blue user button to progress through this tutorial.\n");
            printf("Now, press the button when you're ready to learn about the instructions...\n\n");
            print_flag = false;
        }
    }
    else if (tutorial_state == TUTORIAL_NEAR) {
        if (print_flag) {
            printf("1. \"Near\"\n");
            printf("   => the #instruction LED# lights up\n");
            printf("   => move your hand near the sensor\n");
            printf("   => a \"near\" distance was defined through the calibration earlier\n\n");
            print_flag = false;
        }
        led2.write(1);
    }
    else if (tutorial_state == TUTORIAL_FAR) {
        if (print_flag) {
            printf("2. \"Far\"\n");
            printf("   => the #instruction LED# stays off\n");
            printf("   => move your hand far from the sensor\n");
            printf("   => a \"far\" distance was defined through the calibration earlier\n\n");
            print_flag = false;
        }
        led2.write(0);
    }
    else if (tutorial_state == TUTORIAL_ALT) {
        if (print_flag) {
            printf("3. \"Alternate\"\n");
            printf("   => the #instruction LED# flashes\n");
            printf("   => *quickly alternate* your hand between near and far\n\n");
            print_flag = false;
        }
        blinky();
    }
    else if (tutorial_state == TUTORIAL_NOT) {
        if (print_flag) {
            printf("4. \"Not\"\n");
            printf("   => when the #not LED# lights up, along with any 3 state of the #instruction LED#\n");
            printf("   => this *negates* whatever instruction is given by the #instruction LED#, where:\n");
            printf("      -> \"not near\" = \"far\"\n");
            printf("      -> \"not far\" = \"near\"\n");
            printf("      -> \"not alternate\" = \"stay still\", do not move your hand\n\n");
            print_flag = false;
        }
        led2.write(0);
        led1.write(1);
    }
    else if (tutorial_state == TUTORIAL_PAUSE) {
        if (print_flag) {
            printf("5. Pausing the Game\n");
            printf("   => at any time of the game, you can press the user button to pause the game play, and the two LEDs will remain the same\n");
            printf("   => pressing the button again will resume the game, and a random *new instruction* will be given\n\n");
            print_flag = false;
        }
    }
    else if (tutorial_state == TUTORIAL_GAME_END) {
        if (print_flag) {
            printf("6. Game End\n");
            printf("   => for each instruction, the correct move must be made within a given timeframe\n");
            printf("   => as the game progresses, this timeframe gets shorter\n");
            printf("   => if your move does not match the given instruction, the game ends, and both LEDs would flash\n");
            printf("   => you can check your phone for your current score and high score, which is sent via bluetooth\n");
            printf("   => TIP: turn on *notify* to have live score updates! \n\n");
            printf("Once you're ready, press the user button to start playing the game! \n");
            led2.write(0);
            led1.write(0);
            print_flag = false;
        }
        blinky();
    }
}

void show_lights() {
    if (print_flag) {
        if (prev_instruction == -1)
            printf("\n\n ===== New Game Started! =====\n\n");
        else
            printf(" --- Resume Game ---\n");
        print_flag = false;
    }

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

    // printf("current instruction: %d\n", instruction);
    
    read_input_state = READ_INPUT_STARTED;
    prev_instruction = instruction;
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
        game_state = GAME_ENDING;
        instruction_state = END_INSTRUCTION_START;
    }
}

void main_game() {
    if (game_state == GAME_CALIBRATION_NEAR || game_state == GAME_CALIBRATION_FAR) {
        calibrate();
    }
    else if (game_state == GAME_TUTORIAL) {
        tutorial();
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
    else if (game_state == GAME_ENDING) {
        if (instruction_state == END_INSTRUCTION_START) {
            led1.write(0);
            led2.write(0);
            instruction_state = END_INSTRUCTION_ON;
            timer.attach(&timeout_handler, 500ms);
        }
        blinky();
    }
    else if (game_state == GAME_PAUSED) {
        game_state = GAME_PAUSED_PENDING;
        printf(" --- Game Paused ---\n");
        instruction_state = NEW_INSTRUCTION_ON;
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
    printf("\n\n ===== Game END =====\n\n");
    printf("Check your phone for your score and high score!\n");
    printf("You can press the user button again to start a new game.\n");
    
    game_service.reset_score();
    reset_input_globals();
    // near_dist = default_near_dist;
    // far_dist = default_far_dist;
    prev_instruction = -1;
}
