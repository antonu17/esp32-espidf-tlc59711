#if !defined(TLC59711_H)
#define TLC59711_H

#include <driver/spi_master.h>

typedef struct
{
    int num_drivers;
    int data_pin;
    int sclk_pin;
    uint8_t spi_mode;
    int spi_clock_speed_hz;

} tlc_config_t;

void tlc_init(tlc_config_t *config);
void tlc_set_brightness(uint8_t r, uint8_t g, uint8_t b);
void tlc_set_led(int led, uint8_t r, uint8_t g, uint8_t b);
void tlc_reset_leds();
void tlc_oneshot_write();
void tlc_queue_write();
void tlc_wait_write();

void _tlc_init_spi_driver(int data_pin, int sclk_pin, uint8_t spi_mode, int spi_clock_speed_hz);
void _tlc_allocate_spi_buffer();
uint8_t _8bit_to_7bit(uint8_t val);

#endif // TLC59711_H
