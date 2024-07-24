#include <furi.h>
#include <lib/subghz/transmitter.h>
#include <applications/drivers/subghz/cc1101_ext/cc1101_ext_interconnect.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <flipper_format/flipper_format_i.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/devices/devices.h>

#include "subghz.h"

static void set_payload(FlipperFormat* flipper_format, ShockerChannel channel, ShockerMode mode, uint8_t strength) {
    static const uint16_t transmitter_id = 0xDDC8;
    uint8_t payload[5] = {0};
    char payloadstr[] = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

    payload[0] = transmitter_id >> 8;
    payload[1] = transmitter_id & 0xFF;
    payload[2] = (channel & 0xF) << 4 | (mode & 0xF);
    payload[3] = strength;
    payload[4] = (payload[0] + payload[1] + payload[2] + payload[3]) % 256;

    for(int i = 0; i < 5 * 8; i++) {
        payloadstr[i + i / 2] = (payload[i / 8] & (1 << (7 - i % 8))) ? 'E' : '8';
    }

    FURI_LOG_D("set_payload", "Payload: %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4]);

    FuriString* binraw_settings = furi_string_alloc_printf(
        "Protocol: BinRAW\n"
        "Bit: 256\n"
        "TE: 250\n"
        "Bit_RAW: 256\n"
        "Data_RAW: FC %s 88 00 00 00 00 00 00 00 00 00 00\n",
        payloadstr);
    Stream* stream = flipper_format_get_raw_stream(flipper_format);
    stream_clean(stream);
    stream_write_cstring(stream, furi_string_get_cstr(binraw_settings));
    stream_seek(stream, 0, StreamOffsetFromStart);
    furi_string_free(binraw_settings);
}

void send_payload(uint8_t strength) {
    uint32_t frequency = 433950000;
    const SubGhzDevice* device;

    subghz_devices_init();
    device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);

    SubGhzEnvironment* environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);
    SubGhzTransmitter* transmitter = subghz_transmitter_alloc_init(environment, "BinRAW");
    FlipperFormat* flipper_format = flipper_format_string_alloc();

    set_payload(flipper_format, CH01, Shock, strength);

    subghz_transmitter_deserialize(transmitter, flipper_format);
    subghz_devices_begin(device);
    subghz_devices_reset(device);
    subghz_devices_load_preset(device, FuriHalSubGhzPresetOok270Async, NULL);
    frequency = subghz_devices_set_frequency(device, frequency);

    // Send
    furi_hal_power_suppress_charge_enter();
    if(subghz_devices_start_async_tx(device, subghz_transmitter_yield, transmitter)) {
        while(!(subghz_devices_is_async_complete_tx(device))) {
            furi_delay_ms(100);
        }
        subghz_devices_stop_async_tx(device);
    } else {
        FURI_LOG_E("send_payload", "Failed to start async transmission.");
    }

    // Cleanup
    subghz_devices_sleep(device);
    subghz_devices_end(device);
    subghz_devices_deinit();
    furi_hal_power_suppress_charge_exit();
    flipper_format_free(flipper_format);
    subghz_transmitter_free(transmitter);
    subghz_environment_free(environment);
}
