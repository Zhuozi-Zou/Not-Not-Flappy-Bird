#include "not.hpp"
#include "pretty_print.hpp"


static const ble::AdvertisingParameters advertising_params(
    // CTRL+Click on the advertising type below to see other types.
    ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
    // The interval between advertisements is a trade-off between
    // being discovered lower power usage.
    ble::adv_interval_t(ble::millisecond_t(25)),
    ble::adv_interval_t(ble::millisecond_t(50))
);

void advertise(EventQueue *queue)
{
    BLE &ble = BLE::Instance();
    auto &_gap = ble.gap();

    ble_error_t error = _gap.setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE, advertising_params);
    if (error) {
        print_error(error, "Gap::setAdvertisingParameters() failed");
        return;
    }

    ble::AdvertisingDataSimpleBuilder<ble::LEGACY_ADVERTISING_MAX_SIZE> data_builder;
    data_builder.setFlags();


    data_builder.setName("Flappy");

    error = _gap.setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE, data_builder.getAdvertisingData());
    if (error) {
        print_error(error, "Gap::setAdvertisingPayload() failed");
        return;
    }

    error = _gap.startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (error) {
        print_error(error, "Gap::startAdvertising() failed");
        return;
    }
}