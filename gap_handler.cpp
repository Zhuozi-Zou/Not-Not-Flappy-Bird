#include "not.hpp"
#include "pretty_print.hpp"

#include "mbed.h"

void GapHandler::onAdvertisingStart(const ble::AdvertisingStartEvent &event)
{
    printf("The name of the board is *Flappy*.\n");
}

void GapHandler::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    if (event.getStatus() != BLE_ERROR_NONE) {
        print_error(event.getStatus(), "Connection failed");
        return;
    }

    // printf("Connection made with %u.\n", event.getConnectionHandle());
    printf("\n\n ===== Connected! =====\n\n");
    printf("We will need the readings for a \"near\" distance and a \"far\" distance. \n\n");
    printf("Please place your hand relatively close to the sensor (>5 cm, for best experience), and press the blue user button when you're ready. \n");
    printf("This will be recorded as your \"near\" distance.\n\n");

    queue.call_every(10ms, main_game);
}

void GapHandler::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    printf("\n\n ===== Disconnected ===== \n\n");
    printf("Disconnected from %u because %u.\n\n", event.getConnectionHandle(), event.getReason());
    printf("Uh oh, bluetooth is disconnected! The game requires a restart.\n");
    printf("Please press the black reset button to restart the game.\n\n");

    game_state = GAME_ENDED; 
}