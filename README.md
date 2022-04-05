# Not-Not-Flappy-Bird

## Introduction
The **Not Not Flappy Bird** is a simple game that requires the player to make gestures according to
the light instructions, aiming to stay in game as long as possible. The player must move their
hand close to or away from the sensor following the instructions light. If the not light turns on,
the player must do the opposite of what the instructions light is showing. The time limit given to
follow an instruction will become shorter as the game is played for a longer time. Failure to
follow an instruction within the limit will cause the game to fail.

## Instructions for Compiling

> This is VERY IMPORTANT please follow the steps below before trying to compile the code, or come back when compiling fails.

1. In the uploaded `.zip`, we only included the necessary `.cpp`, `.hpp`, and `.json` files. We originally thought it might be easier to zip everything due to random library issues mentioned below, but we are using a shared `mbed-os` instance when creating the project, which makes it difficult for us to just go and zip everything.
2. Install the `BSP_B-L475E-IOT01` library, using [this link](http://os.mbed.com/teams/ST/code/BSP_B-L475E-IOT01/).
3. Install the `VL53L0X` library, using [this link](https://os.mbed.com/teams/ST/code/VL53L0X/docs/tip/classVL53L0X.html) (Note that simply Googling `VL53L0X` is very likely to lead to to the _incorrect_ link).
4. Before compiling, **search** for the usage of `wait_ms` in the `VL53L0X` library code, and replace them with `thread_sleep_for`. Unfortunately `wait_ms` is deprecated, and since we use a newer version of `mbed-os`, leaving the code as is will cause compiler errors.
5. We are using NFC as well, which requires no installation of an additional library, but requires `mbed_app.json` to have a proper label added. The label added for our board specifically is the `M24SR` driver, which should be included in the json file we submitted. If this line `"target.extra_labels_add": ["M24SR"]` does not exist, the code will fail to compile.
6. Occasionaly, when we compile our code for the first time, there might be an error related to clocks, time, deprecated options, etc.. We never figured out the cause, but simply re-opening Mbed Studio and re-compiling solves the issue.

## Usage
All instructions on how to use this program will be printed in the terminal (please set `baud rate` to `115200`) once it starts running.

As a reference, here is what's expected to happen, and some notes that you need to :

1. The program will ask for your name, which needs to be sent through NFC. Please have a NFC tag writer app installed on your phone, and write a Text message to the NFC tag on the board.
2. You would then need to connect your phone to the board via bluetooth. Please have a BLE scanner app installed on your phone.
3. Once connected, you will enter user calibration. Locate the ToF distance sensor on the board and follow the instructions to record a "near" and a "far" distance.
4. After calibration, there will be a detailed tutorial section going through the different game instructions, their corresponding lights, and how to play the game.
5. When the game starts, you can see your live score on your phone (bluetooth), and the game can be paused at any time using the user button. When the game ends, your highscore will also be updated, and you can press the button to start a new game.

## Board Reference
In case it is hard to find, here are the locations for the NFC tag and the ToF sensor:
<img width="601" alt="board reference" src="https://user-images.githubusercontent.com/12402631/161864713-977ca5ba-43e1-488f-b2b8-18153b144776.png">
