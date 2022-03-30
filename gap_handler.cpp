#include "not.hpp"
#include "pretty_print.hpp"

#include "mbed.h"

void GapHandler::onAdvertisingStart(const ble::AdvertisingStartEvent &event)
{
    printf("Advertising started.\n");
}

void GapHandler::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    if (event.getStatus() != BLE_ERROR_NONE) {
        print_error(event.getStatus(), "Connection failed");
        return;
    }

    printf("Connection made with %u.\n", event.getConnectionHandle());
    printf("Please put your hand to the \"near distance\" and press the user button\n");

    queue.call_every(10ms, main_game);
}

void GapHandler::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    printf("Disconnected from %u because %u.\n", event.getConnectionHandle(), event.getReason());

    game_state = GAME_ENDED; 
}