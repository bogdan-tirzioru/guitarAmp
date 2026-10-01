#ifndef TLV320AIC3104_H
#define TLV320AIC3104_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AIC3104_I2C_ADDRESS 0x18U /* 7-bit; HAL adapter shifts it once. */
#define AIC3104_OUTPUT_HEADPHONE 0x01U
#define AIC3104_OUTPUT_LINE      0x02U

typedef enum {
    AIC3104_OK = 0,
    AIC3104_INVALID_ARGUMENT,
    AIC3104_IO_ERROR,
    AIC3104_NOT_READY,
    AIC3104_TIMEOUT
} aic3104_result_t;

typedef enum { AIC3104_LINE1 = 0, AIC3104_LINE2 } aic3104_input_t;

/* All callbacks are blocking, return 0 on success, and must outlive the device.
 * reset(asserted=true) drives /RESET low. Supplies must already be stable.
 * Serialize all calls and give this module exclusive access to the codec. */
typedef struct {
    void *context;
    int (*write)(void *context, uint8_t reg, uint8_t value);
    int (*read)(void *context, uint8_t reg, uint8_t *value);
    void (*reset)(void *context, bool asserted);
    void (*delay_ms)(void *context, uint32_t milliseconds);
} aic3104_bus_t;

typedef struct {
    uint32_t sample_rate_hz; /* First implementation: 48000 only. */
    uint32_t mclk_hz;        /* 12288000 or 24576000, PLL bypass. */
    uint8_t word_bits;       /* 16, 24 or 32; slots are set separately in SAI. */
    aic3104_input_t left_input;  /* Guitar -> left ADC. Single-ended inputs. */
    aic3104_input_t right_input; /* Aux -> right ADC. No analog cross-mixing. */
    uint8_t outputs;         /* HEADPHONE and/or LINE; zero disables drivers. */
    uint8_t adc_gain_half_db[2];       /* 0..119 => 0..+59.5 dB. */
    uint8_t dac_attenuation_half_db[2]; /* 0..127 => 0..-63.5 dB. */
} aic3104_config_t;

typedef struct {
    aic3104_bus_t bus;
    bool ready;
    uint8_t outputs;
} aic3104_t;

/* Initializes memory only: no I2C, GPIO or delays. Invalid attach clears device. */
aic3104_result_t aic3104_attach(aic3104_t *device, const aic3104_bus_t *bus);
aic3104_config_t aic3104_default_config(void);
/* Hardware clocks must run before init. ADCs enabled; DACs remain MUTED.
 * IO failure/timeout asserts /RESET, clears ready, and requires re-init. */
aic3104_result_t aic3104_init(aic3104_t *device, const aic3104_config_t *config);
aic3104_result_t aic3104_set_dac_mute(aic3104_t *device, bool mute);
aic3104_result_t aic3104_set_dac_attenuation(aic3104_t *device, uint8_t left_half_db,
                                           uint8_t right_half_db);
aic3104_result_t aic3104_set_adc_gain(aic3104_t *device, uint8_t left_half_db,
                                    uint8_t right_half_db);
/* Diagnostics only, page 0 register 1..109; does not identify the chip model. */
aic3104_result_t aic3104_read_register(aic3104_t *device, uint8_t reg, uint8_t *value);
/* Hardware reset powers down all paths, including after a broken I2C bus. */
void aic3104_shutdown(aic3104_t *device);

#ifdef __cplusplus
}
#endif
#endif
