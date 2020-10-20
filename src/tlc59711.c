#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

#include "tlc59711.h"

#include <driver/spi_master.h>
#include <esp_log.h>
#include <esp_system.h>
#include <string.h>

static const char *TAG = "tlc59711";

static uint8_t _num_drivers;
static spi_device_handle_t _spi;
static uint8_t *_spi_buffer;

void IRAM_ATTR tlc_init(tlc_config_t *config) {
    _num_drivers = config->num_drivers;
    _tlc_init_spi_driver(config->data_pin, config->sclk_pin, config->spi_mode, config->spi_clock_speed_hz);
    _tlc_allocate_spi_buffer();

    tlc_set_brightness(0xFF, 0xFF, 0xFF);
    ESP_LOGD(TAG, "tlc_init() finished");
}

void IRAM_ATTR tlc_set_brightness(uint8_t r, uint8_t g, uint8_t b) {
    // Write command
    uint32_t command = 0x25;

    command <<= 5;
    // OUTTMG = 1, EXTGCK = 0, TMGRST = 0, DSPRPT = 1, BLANK = 0 -> 0x12
    command |= 0x12;

    command <<= 7;
    command |= _8bit_to_7bit(b);

    command <<= 7;
    command |= _8bit_to_7bit(g);

    command <<= 7;
    command |= _8bit_to_7bit(r);

    command = SPI_SWAP_DATA_TX(command, 32);

    for (int i = 0; i < _num_drivers; i++) {
        memcpy(_spi_buffer + i * TLC59711_BUFFER_SIZE, &command, 4);
    }
}

void IRAM_ATTR tlc_set_led(int led, uint8_t r, uint8_t g, uint8_t b) {
    ESP_LOGI(TAG, "set LED #%d: (%d, %d, %d)", led, r, g, b);

    if (led > _num_drivers * 4)
        return;

    int l = _num_drivers * 4 - led - 1;
    int d = l / 4;
    int i = l - 4 * d;
    int s = d * TLC59711_BUFFER_SIZE + 4 + i * 6;

    ESP_LOGD(TAG, "reversed LED: %d", l);
    ESP_LOGD(TAG, "driver: %d", d);
    ESP_LOGD(TAG, "LED in the driver: %d", i);

    _spi_buffer[s + 0] = b;
    _spi_buffer[s + 1] = b;
    _spi_buffer[s + 2] = g;
    _spi_buffer[s + 3] = g;
    _spi_buffer[s + 4] = r;
    _spi_buffer[s + 5] = r;

    ESP_LOGV(TAG, "SPI buffer position: %p", _spi_buffer + s);
    ESP_LOGV(TAG, "SPI buffer: ");
    ESP_LOGV(TAG, "_spi_buffer starts at %p", _spi_buffer);
    ESP_LOGV(TAG, "_spi_buffer length %d", _num_drivers * TLC59711_BUFFER_SIZE);
    ESP_LOG_BUFFER_HEXDUMP(TAG, _spi_buffer, _num_drivers * TLC59711_BUFFER_SIZE, ESP_LOG_VERBOSE);
}

void IRAM_ATTR tlc_reset_leds() {
    for (int i = 0; i < _num_drivers; i++) {
        memset(_spi_buffer + TLC59711_BUFFER_SIZE * i + 4, 0, 24);
    }
}

void IRAM_ATTR tlc_oneshot_write() {
    static spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));

    trans.tx_buffer = _spi_buffer;
    trans.length = _num_drivers * TLC59711_BUFFER_SIZE * 8;
    ESP_ERROR_CHECK(spi_device_polling_transmit(_spi, &trans));
}

void IRAM_ATTR tlc_queue_write() {
    static spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));

    trans.tx_buffer = _spi_buffer;
    trans.length = _num_drivers * TLC59711_BUFFER_SIZE * 8;
    ESP_ERROR_CHECK(spi_device_queue_trans(_spi, &trans, portMAX_DELAY));
}

void IRAM_ATTR tlc_wait_write() {
    static spi_transaction_t *rtrans;
    ESP_ERROR_CHECK(spi_device_get_trans_result(_spi, &rtrans, portMAX_DELAY));
    // We could inspect rtrans now if we received any info back. The TLC59711 is treated as write-only, though.
}

void IRAM_ATTR _tlc_init_spi_driver(int data_pin, int sclk_pin, uint8_t spi_mode, int spi_clock_speed_hz) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = data_pin,
        .miso_io_num = -1,
        .sclk_io_num = sclk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_device_interface_config_t devcfg = {
        .mode = spi_mode,
        .clock_speed_hz = spi_clock_speed_hz,
        .queue_size = 8,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &buscfg, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &devcfg, &_spi));
}

void IRAM_ATTR _tlc_allocate_spi_buffer() {
    _spi_buffer = heap_caps_calloc(_num_drivers, TLC59711_BUFFER_SIZE, MALLOC_CAP_DMA);
    assert(_spi_buffer != NULL);
    ESP_LOGD(TAG, "_spi_buffer starts at %p", _spi_buffer);
    ESP_LOGD(TAG, "_spi_buffer length %d", _num_drivers * TLC59711_BUFFER_SIZE);
}

uint8_t IRAM_ATTR _8bit_to_7bit(uint8_t val) {
    return val * 127 / 255;
}
