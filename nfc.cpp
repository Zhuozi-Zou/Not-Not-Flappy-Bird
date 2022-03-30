/**
 * @file nfc.cpp
 * @author Angela Zhu, Fillis Zou
 * @cite https://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-nfc-EEPROM/
 * @version 1.0
 *
 * @brief nfc set up for reading player name
 *        using the official mbed-os mbed-os-example-nfc-EEPROM example
 */

#include "events/EventQueue.h"
#include "nfc/ndef/MessageBuilder.h"
 
#include "NFCEEPROM.h"
#include "EEPROMDriver.h"

#include <string>
#include "not.hpp"

using events::EventQueue;
 
using mbed::nfc::NFCEEPROM;
using mbed::nfc::NFCEEPROMDriver;
using mbed::Span;
 
using mbed::nfc::ndef::MessageBuilder;

// player's name
string player_name;

/**
 * @brief Player Info reader via NFC
 *
 * This does include both reading and writing functions.
 * This also requires a phone with a NFC tag reader/writer app installed.
 */
class PlayerReader : mbed::nfc::NFCEEPROM::Delegate
{
public:
    PlayerReader(events::EventQueue& queue, NFCEEPROMDriver& eeprom_driver) :
        _ndef_buffer(),
        _eeprom(&eeprom_driver, &queue, _ndef_buffer),
        _queue(queue)
    { }
 
    void run()
    {
        if (_eeprom.initialize() != NFC_OK) {
            printf("failed to initialise\r\n");
            _queue.break_dispatch();
        }
 
        _eeprom.set_delegate(this);
 
        _queue.call(&_eeprom, &NFCEEPROM::write_ndef_message);
        _queue.call_every(1000ms, &_eeprom, &NFCEEPROM::read_ndef_message);
    }
 
private:
    virtual void on_ndef_message_written(nfc_err_t result) {
        if (result == NFC_OK) {
            printf("Please enter your name via NFC:\r\n");
        } else {
            printf("failed to write (error: %d)\r\n", result);
        }
    }
 
    virtual void on_ndef_message_read(nfc_err_t result) {
        // if (result == NFC_OK) {
        //     printf("message read successfully\r\n");
        // } else {
        //     printf("failed to read (error: %d)\r\n", result);
        // }
    }
 
    virtual void parse_ndef_message(const Span<const uint8_t> &buffer) {        
        if (!buffer.empty()) {
            // first 7 bytes work as a "header" to indicate the type
            // i.e. what we're using is a "Text" type
            // since buffer is already a Span, it's easiest to parse
            // the actual message component by reading from byte 8
            Span<const uint8_t> result = buffer.last(buffer.size()-7);
            std::string s( result.data(), result.data()+result.size() );
            player_name = s;
            _queue.break_dispatch();
        }
    }
 
    virtual size_t build_ndef_message(const Span<uint8_t> &buffer) {
        // empty builder
        MessageBuilder builder(buffer);

        // directly return size 0 (empty buffer) 
        // works as wiping out the data in the NFC tag
        return builder.get_message().size();
    }
 
private:
    uint8_t _ndef_buffer[1024];
    NFCEEPROM _eeprom;
    EventQueue& _queue;
};
 
    

/**
 * @brief reads player name via NFC and stores in player_name.
 */
bool read_player_name()
{
    EventQueue nfc_queue;

    NFCEEPROMDriver& eeprom_driver = get_eeprom_driver(nfc_queue);
 
    PlayerReader pr(nfc_queue, eeprom_driver);
 
    pr.run();
    nfc_queue.dispatch_forever();

    return true;
}
