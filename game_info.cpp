#include "not.hpp"

// https://github.com/ARMmbed/mbed-os-example-ble/blob/master/BLE_GattServer_CharacteristicUpdates/source/main.cpp

GameService::GameService() :
        _score(0),
        _high_score(0),
        _score_characteristic(
            "12345678-abcd-efgh-9900-flapppyscore", 
            &_score, 
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        _high_score_characteristic(
            "12345678-abcd-efgh-9900-flappppyhigh", 
            &_high_score)
{
    
    uint32_t status;

    BLE &ble = BLE::Instance();

    GattCharacteristic *characteristics[] = { 
        &_high_score_characteristic, 
        &_score_characteristic };
        
    // custom service uuid
    UUID custom_id("98765432-ijkl-mnop-1999-flapyservice");
    GattService flappy_service(
        custom_id, //GattService::UUID_XXXX_SERVICE,
        characteristics,
        sizeof(characteristics) / sizeof(characteristics[0])
    );

    ble.gattServer().addService(flappy_service);
}

void GameService::update_score()
{
    // TODO: update score
    _score++;

    // TODO: Communicate the updated _charge value over BLE
    BLE &ble = BLE::Instance();
    ble.gattServer().write(_score_characteristic.getValueHandle(), &_score, sizeof(uint8_t));
}

void GameService::update_high_score()
{
    // TODO: update high score
    if (_score > _high_score) _high_score = _score;

    // TODO: Communicate the updated _charge value over BLE
    // BLE &ble = BLE::Instance();
    // ble.gattServer().write(_high_score_characteristic.getValueHandle(), &_high_score, sizeof(uint8_t));
}
