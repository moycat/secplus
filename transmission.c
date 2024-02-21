#include <lib/subghz/transmitter.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/subghz/devices/devices.h>
#include "secplus.h"
#include "transmission.h"

extern const SubGhzProtocolRegistry subghz_protocol_registry;

void secplus_app_set_format(
    FlipperFormat* flipper_format,
    uint32_t fixed_code,
    uint32_t rolling_code) {
    uint32_t bits = 42;
    uint32_t repeat = 4;
    char data[8];
    data[0] = fixed_code >> 24;
    data[1] = fixed_code >> 16;
    data[2] = fixed_code >> 8;
    data[3] = fixed_code;
    data[4] = rolling_code >> 24;
    data[5] = rolling_code >> 16;
    data[6] = rolling_code >> 8;
    data[7] = rolling_code;
    flipper_format_insert_or_update_string_cstr(flipper_format, "Protocol", "Security+ 1.0");
    flipper_format_insert_or_update_uint32(flipper_format, "Bit", &bits, 1);
    flipper_format_insert_or_update_hex(flipper_format, "Key", (void*)data, 8);
    flipper_format_insert_or_update_uint32(flipper_format, "Repeat", &repeat, 1);
    flipper_format_rewind(flipper_format);
}

void secplus_app_transmit(SecPlusApp* app, uint32_t fixed_code, uint32_t rolling_code) {
    UNUSED(app);
    FURI_LOG_I(TAG, "Emitting fixed code: %lu, rolling code: %lu", fixed_code, rolling_code);
    uint32_t frequency = 315000000;
    if(!furi_hal_region_is_frequency_allowed(frequency)) {
        FURI_LOG_E(TAG, "Frequency %lu is not allowed in this region.", frequency);
        return;
    }
    subghz_devices_init();
    const SubGhzDevice* device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
    SubGhzEnvironment* environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);
    SubGhzTransmitter* transmitter = subghz_transmitter_alloc_init(environment, "Security+ 1.0");
    FlipperFormat* flipper_format = flipper_format_string_alloc();
    secplus_app_set_format(flipper_format, fixed_code, rolling_code);
    SubGhzProtocolStatus status = subghz_transmitter_deserialize(transmitter, flipper_format);
    furi_assert(status == SubGhzProtocolStatusOk);
    subghz_devices_begin(device);
    subghz_devices_reset(device);
    subghz_devices_load_preset(device, FuriHalSubGhzPresetOok650Async, NULL);
    frequency = subghz_devices_set_frequency(device, frequency);
    if(subghz_devices_start_async_tx(device, subghz_transmitter_yield, transmitter)) {
        while(!(subghz_devices_is_async_complete_tx(device))) {
            furi_delay_ms(500);
        }
        subghz_devices_stop_async_tx(device);
    }
    subghz_devices_sleep(device);
    subghz_devices_end(device);
    subghz_devices_deinit();
    flipper_format_free(flipper_format);
    subghz_transmitter_free(transmitter);
    subghz_environment_free(environment);
}
