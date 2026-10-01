/* Independent implementation from TI TLV320AIC3104 SLAS510G, chapter 10.
 * Linux sound/soc/codecs/tlv320aic3x.c was reviewed as a behavior reference;
 * no Linux source, regmap, ALSA, DAPM or register-cache code is included.
 * See docs/tlv320aic3104.md for scope, sources and hardware prerequisites. */
#include "tlv320aic3104.h"
#include <stddef.h>
#include <string.h>

enum {
    REG_PAGE = 0, REG_RESET = 1, REG_RATE = 2, REG_PLL_A = 3,
    REG_PATH = 7, REG_SERIAL_A = 8, REG_SERIAL_B = 9, REG_OFFSET = 10,
    REG_ADC_L = 15, REG_ADC_R = 16, REG_LINE2_L = 17, REG_LINE2_R = 18,
    REG_LINE1_L = 19, REG_CROSS_R_L = 21, REG_LINE1_R = 22, REG_CROSS_L_R = 24,
    REG_MICBIAS = 25, REG_AGC_L = 26, REG_AGC_R = 29,
    REG_DAC_POWER = 37, REG_HP_CONTROL = 38, REG_COMMON_MODE = 40,
    REG_DAC_SWITCH = 41, REG_POP = 42, REG_DAC_L = 43, REG_DAC_R = 44,
    REG_DAC_HP_L = 47, REG_HP_L = 51, REG_DAC_HP_R = 64, REG_HP_R = 65,
    REG_DAC_LINE_L = 82, REG_LINE_L = 86, REG_DAC_LINE_R = 92, REG_LINE_R = 93,
    REG_POWER_STATUS = 94, REG_CLOCK = 101, REG_CLOCK_SOURCE = 102
};

static bool attached(const aic3104_t *d)
{
    return d != NULL && d->bus.write != NULL && d->bus.read != NULL &&
           d->bus.reset != NULL && d->bus.delay_ms != NULL;
}

void aic3104_shutdown(aic3104_t *d)
{
    if (attached(d)) {
        d->bus.reset(d->bus.context, true);
        d->ready = false;
    }
}

static aic3104_result_t fail(aic3104_t *d, aic3104_result_t result)
{
    aic3104_shutdown(d);
    return result;
}

/* Select page on every transaction: no cache can become stale after reset.
 * Caller must serialize page selection + access; no interrupt-time calls. */
static aic3104_result_t write_reg(aic3104_t *d, uint8_t reg, uint8_t value)
{
    if (d->bus.write(d->bus.context, REG_PAGE, 0) != 0 ||
        d->bus.write(d->bus.context, reg, value) != 0)
        return fail(d, AIC3104_IO_ERROR);
    return AIC3104_OK;
}

static aic3104_result_t read_reg(aic3104_t *d, uint8_t reg, uint8_t *value)
{
    if (d->bus.write(d->bus.context, REG_PAGE, 0) != 0 ||
        d->bus.read(d->bus.context, reg, value) != 0)
        return fail(d, AIC3104_IO_ERROR);
    return AIC3104_OK;
}

static aic3104_result_t update_reg(aic3104_t *d, uint8_t reg, uint8_t mask, uint8_t value)
{
    uint8_t previous;
    aic3104_result_t result = read_reg(d, reg, &previous);
    if (result != AIC3104_OK) return result;
    return write_reg(d, reg, (uint8_t)((previous & (uint8_t)~mask) | (value & mask)));
}

aic3104_result_t aic3104_attach(aic3104_t *d, const aic3104_bus_t *bus)
{
    if (d == NULL) return AIC3104_INVALID_ARGUMENT;
    /* Allow a caller to pass &device.bus. */
    aic3104_bus_t copy = {0};
    if (bus != NULL) copy = *bus;
    memset(d, 0, sizeof(*d));
    if (copy.write == NULL || copy.read == NULL || copy.reset == NULL || copy.delay_ms == NULL)
        return AIC3104_INVALID_ARGUMENT;
    d->bus = copy;
    return AIC3104_OK;
}

aic3104_config_t aic3104_default_config(void)
{
    const aic3104_config_t config = {
        .sample_rate_hz = 48000, .mclk_hz = 12288000, .word_bits = 24,
        .left_input = AIC3104_LINE1, .right_input = AIC3104_LINE1,
        .outputs = AIC3104_OUTPUT_HEADPHONE | AIC3104_OUTPUT_LINE,
        .adc_gain_half_db = {0, 0}, .dac_attenuation_half_db = {24, 24}
    };
    return config;
}

static bool valid_config(const aic3104_config_t *c)
{
    return c != NULL && c->sample_rate_hz == 48000 &&
           (c->mclk_hz == 12288000 || c->mclk_hz == 24576000) &&
           (c->word_bits == 16 || c->word_bits == 24 || c->word_bits == 32) &&
           (c->left_input == AIC3104_LINE1 || c->left_input == AIC3104_LINE2) &&
           (c->right_input == AIC3104_LINE1 || c->right_input == AIC3104_LINE2) &&
           (c->outputs & (uint8_t)~3U) == 0 &&
           c->adc_gain_half_db[0] <= 119 && c->adc_gain_half_db[1] <= 119 &&
           c->dac_attenuation_half_db[0] <= 127 && c->dac_attenuation_half_db[1] <= 127;
}

aic3104_result_t aic3104_init(aic3104_t *d, const aic3104_config_t *c)
{
    if (!attached(d) || !valid_config(c)) return AIC3104_INVALID_ARGUMENT;
    d->ready = false;
    d->outputs = c->outputs;
    d->bus.reset(d->bus.context, true);
    d->bus.delay_ms(d->bus.context, 1); /* TI requires >=10 ns after supplies settle. */
    d->bus.reset(d->bus.context, false);
    d->bus.delay_ms(d->bus.context, 1);
    aic3104_result_t result = write_reg(d, REG_RESET, 0x80);
    if (result != AIC3104_OK) return result;
    d->bus.delay_ms(d->bus.context, 1);

    const bool hp = (c->outputs & AIC3104_OUTPUT_HEADPHONE) != 0;
    const bool line = (c->outputs & AIC3104_OUTPUT_LINE) != 0;
    /* fs = MCLK/(128*Q), Q=2 or 4; PLL disabled; MCLK clock input.
     * 24-bit I2S samples may occupy 32-bit SAI slots (64 BCLK/frame). */
    const uint8_t sequence[][2] = {
        {REG_PLL_A, c->mclk_hz == 12288000 ? 0x10 : 0x20},
        {REG_CLOCK_SOURCE, 0x02}, {REG_CLOCK, 0x01}, {REG_RATE, 0x00},
        {REG_PATH, 0x0a}, {REG_SERIAL_A, 0x00},
        {REG_SERIAL_B, c->word_bits == 16 ? 0x00 : (c->word_bits == 24 ? 0x20 : 0x30)},
        {REG_OFFSET, 0x00}, {REG_MICBIAS, 0x00},
        {REG_AGC_L, 0x00}, {REG_AGC_R, 0x00},
        {REG_ADC_L, 0x80}, {REG_ADC_R, 0x80},
        {REG_CROSS_R_L, 0x78}, {REG_CROSS_L_R, 0x78},
        {REG_LINE2_L, c->left_input == AIC3104_LINE2 ? 0x0f : 0xff},
        {REG_LINE2_R, c->right_input == AIC3104_LINE2 ? 0xf0 : 0xff},
        {REG_LINE1_L, c->left_input == AIC3104_LINE1 ? 0x04 : 0x7c},
        {REG_LINE1_R, c->right_input == AIC3104_LINE1 ? 0x04 : 0x7c},
        {REG_DAC_L, (uint8_t)(0x80 | c->dac_attenuation_half_db[0])},
        {REG_DAC_R, (uint8_t)(0x80 | c->dac_attenuation_half_db[1])},
        {REG_COMMON_MODE, 0x00}, /* 1.35 V; verify against final analog schematic. */
        {REG_DAC_SWITCH, 0x00}, /* DAC_L1/R1, independent digital volume. */
        {REG_HP_CONTROL, 0x06}, /* Short protection with automatic power-down. */
        {REG_POP, 0x98}, /* 800 ms driver delay, 2 ms ramp step. */
        {REG_DAC_HP_L, hp ? 0x80 : 0x00}, {REG_DAC_HP_R, hp ? 0x80 : 0x00},
        {REG_DAC_LINE_L, line ? 0x80 : 0x00}, {REG_DAC_LINE_R, line ? 0x80 : 0x00},
        {REG_DAC_POWER, 0xc0},
        {REG_HP_L, hp ? 0x09 : 0x04}, {REG_HP_R, hp ? 0x09 : 0x04},
        {REG_LINE_L, line ? 0x09 : 0x00}, {REG_LINE_R, line ? 0x09 : 0x00}
    };
    for (size_t i = 0; i < sizeof(sequence) / sizeof(sequence[0]); ++i) {
        result = write_reg(d, sequence[i][0], sequence[i][1]);
        if (result != AIC3104_OK) return result;
    }
    /* Bounded polling, rather than assuming a fixed delay proves readiness. */
    uint8_t expected = (uint8_t)(0xc0 | (hp ? 0x06 : 0) | (line ? 0x18 : 0));
    for (unsigned elapsed = 0; elapsed <= 2000; elapsed += 10) {
        uint8_t status;
        result = read_reg(d, REG_POWER_STATUS, &status);
        if (result != AIC3104_OK) return result;
        if ((status & expected) == expected) {
            result = write_reg(d, REG_ADC_L, c->adc_gain_half_db[0]);
            if (result != AIC3104_OK) return result;
            result = write_reg(d, REG_ADC_R, c->adc_gain_half_db[1]);
            if (result != AIC3104_OK) return result;
            d->ready = true;
            return AIC3104_OK;
        }
        if (elapsed < 2000) d->bus.delay_ms(d->bus.context, 10);
    }
    return fail(d, AIC3104_TIMEOUT);
}

static aic3104_result_t ready(const aic3104_t *d)
{
    if (!attached(d)) return AIC3104_INVALID_ARGUMENT;
    return d->ready ? AIC3104_OK : AIC3104_NOT_READY;
}

static aic3104_result_t update_pair(aic3104_t *d, uint8_t left_reg, uint8_t right_reg,
                                   uint8_t mask, uint8_t left, uint8_t right)
{
    aic3104_result_t result = ready(d);
    if (result != AIC3104_OK) return result;
    result = update_reg(d, left_reg, mask, left);
    if (result != AIC3104_OK) return result;
    return update_reg(d, right_reg, mask, right);
}

aic3104_result_t aic3104_set_dac_mute(aic3104_t *d, bool mute)
{
    return update_pair(d, REG_DAC_L, REG_DAC_R, 0x80, mute ? 0x80 : 0, mute ? 0x80 : 0);
}

aic3104_result_t aic3104_set_dac_attenuation(aic3104_t *d, uint8_t left, uint8_t right)
{
    if (left > 127 || right > 127) return AIC3104_INVALID_ARGUMENT;
    return update_pair(d, REG_DAC_L, REG_DAC_R, 0x7f, left, right);
}

aic3104_result_t aic3104_set_adc_gain(aic3104_t *d, uint8_t left, uint8_t right)
{
    if (left > 119 || right > 119) return AIC3104_INVALID_ARGUMENT;
    return update_pair(d, REG_ADC_L, REG_ADC_R, 0x7f, left, right);
}

aic3104_result_t aic3104_read_register(aic3104_t *d, uint8_t reg, uint8_t *value)
{
    if (value == NULL || reg == 0 || reg > 109) return AIC3104_INVALID_ARGUMENT;
    aic3104_result_t result = ready(d);
    if (result != AIC3104_OK) return result;
    return read_reg(d, reg, value);
}
