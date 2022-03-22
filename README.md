# Not-Not-Flappy-Bird

## Instructions for Compiling

> This is VERY IMPORTANT please follow the steps below before trying to compile the code, or come back when compiling fails.

1. In the uploaded `.zip`, we only included the necessary `.cpp`, `.hpp`, and `.json` files. We originally thought it might be easier to zip everything due to random library issues mentioned below, but we are using a shared `mbed-os` instance when creating the project, which makes it difficult for us to just go and zip everything.
2. Install the `BSP_B-L475E-IOT01` library, using [this link](http://os.mbed.com/teams/ST/code/BSP_B-L475E-IOT01/).
3. Install the `VL53L0X` library, using [this link](https://os.mbed.com/teams/ST/code/VL53L0X/docs/tip/classVL53L0X.html) (Note that simply Googling `VL53L0X` is very likely to lead to to the _incorrect_ link).
4. Before compiling, **search** for the usage of `wait_ms` in the `VL53L0X` library code, and replace them with `thread_sleep_for`. Unfortunately `wait_ms` is deprecated, and since we use a newer version of `mbed-os`, leaving the code as is will cause compiler errors.
5. Occasionaly, when we compile our code for the first time, there might be an error related to clocks, time, etc.. We never figured out the cause, but simply reopening Mbed Studio and re-compiling solves the issue.

## Introduction
The **Not Not Flappy Bird** is a simple game that requires the player to make gestures according to
the light instructions, aiming to stay in game as long as possible. The player must move their
hand close to or away from the sensor following the instructions light. If the not light turns on,
the player must do the opposite of what the instructions light is showing. The time limit given to
follow an instruction will become shorter as the game is played for a longer time. Failure to
follow an instruction within the limit will cause the game to fail.
