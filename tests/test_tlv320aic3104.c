#include "tlv320aic3104.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t registers[128];
    unsigned calls, fail_at, assertions, released, waited;
    bool reset, power_ready;
} fake_t;

static void defaults(fake_t *f)
{
    memset(f->registers, 0, sizeof(f->registers));
    f->registers[15] = f->registers[16] = 0x80;
    f->registers[43] = f->registers[44] = 0x80;
    f->registers[17] = f->registers[18] = 0xff;
    f->registers[19] = f->registers[21] = 0x78;
    f->registers[22] = f->registers[24] = 0x78;
}

static int write_bus(void *ctx, uint8_t reg, uint8_t value)
{
    fake_t *f = ctx;
    if (++f->calls == f->fail_at) return -1;
    assert(!f->reset && reg < 128);
    if (reg == 0) { assert(value == 0); return 0; }
    if (reg == 1 && value == 0x80) defaults(f);
    else f->registers[reg] = value;
    /* Ensure reserved registers aren't mistaken for AIC33 controls. */
    assert(reg != 20 && reg != 23 && reg != 39 && reg != 48 && reg != 52);
    return 0;
}

static int read_bus(void *ctx, uint8_t reg, uint8_t *value)
{
    fake_t *f = ctx;
    if (++f->calls == f->fail_at) return -1;
    assert(!f->reset && reg < 128);
    if (reg == 94) *value = f->power_ready ? 0xde : 0;
    else *value = f->registers[reg];
    return 0;
}

static void reset_bus(void *ctx, bool asserted)
{
    fake_t *f = ctx;
    f->reset = asserted;
    if (asserted) { ++f->assertions; defaults(f); }
    else ++f->released;
}

static void delay_bus(void *ctx, uint32_t ms) { ((fake_t *)ctx)->waited += ms; }

static void setup(aic3104_t *d, fake_t *f)
{
    memset(f, 0, sizeof(*f));
    f->power_ready = true;
    aic3104_bus_t bus = {f, write_bus, read_bus, reset_bus, delay_bus};
    assert(aic3104_attach(d, &bus) == AIC3104_OK);
    assert(!d->ready && f->calls == 0 && f->assertions == 0);
}

int main(void)
{
    aic3104_t d;
    fake_t f;
    aic3104_config_t c = aic3104_default_config();
    setup(&d, &f);
    assert(aic3104_set_dac_mute(&d, false) == AIC3104_NOT_READY);
    assert(aic3104_init(&d, &c) == AIC3104_OK);
    unsigned init_calls = f.calls;
    assert(d.ready && !f.reset && f.released == 1);
    assert(f.registers[3] == 0x10 && f.registers[101] == 1 && f.registers[102] == 2);
    assert(f.registers[7] == 0x0a && f.registers[8] == 0 && f.registers[9] == 0x20);
    assert(f.registers[19] == 4 && f.registers[22] == 4);
    assert(f.registers[17] == 0xff && f.registers[18] == 0xff);
    assert(f.registers[15] == 0 && f.registers[16] == 0);
    assert(f.registers[25] == 0 && f.registers[26] == 0 && f.registers[29] == 0);
    assert(f.registers[43] == 0x98 && f.registers[44] == 0x98); /* -12 dB, muted */
    assert(f.registers[47] == 0x80 && f.registers[64] == 0x80);
    assert(f.registers[82] == 0x80 && f.registers[92] == 0x80);
    assert(f.registers[51] == 9 && f.registers[65] == 9);
    assert(f.registers[86] == 9 && f.registers[93] == 9);
    assert(aic3104_set_dac_attenuation(&d, 0, 127) == AIC3104_OK);
    assert(f.registers[43] == 0x80 && f.registers[44] == 0xff);
    assert(aic3104_set_dac_mute(&d, false) == AIC3104_OK);
    assert(f.registers[43] == 0 && f.registers[44] == 127);
    assert(aic3104_set_dac_mute(&d, true) == AIC3104_OK);
    assert(f.registers[43] == 0x80 && f.registers[44] == 0xff);
    assert(aic3104_set_adc_gain(&d, 119, 1) == AIC3104_OK);
    assert(f.registers[15] == 119 && f.registers[16] == 1);
    uint8_t value;
    assert(aic3104_read_register(&d, 94, &value) == AIC3104_OK && value == 0xde);
    assert(aic3104_read_register(&d, 0, &value) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_read_register(&d, 110, &value) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_read_register(&d, 94, NULL) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_set_adc_gain(&d, 120, 0) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_set_dac_attenuation(&d, 128, 0) == AIC3104_INVALID_ARGUMENT);
    aic3104_shutdown(&d);
    assert(!d.ready && f.reset);
    assert(aic3104_init(&d, &c) == AIC3104_OK); /* recovery */

    /* Inject failure into every initial transaction, including page selects,
     * software reset, readiness reads, and final ADC unmute. */
    for (unsigned i = 1; i <= init_calls; ++i) {
        setup(&d, &f);
        f.fail_at = i;
        assert(aic3104_init(&d, &c) == AIC3104_IO_ERROR);
        assert(f.calls == i && f.reset && !d.ready);
        assert(aic3104_set_dac_mute(&d, false) == AIC3104_NOT_READY);
        f.fail_at = 0;
        assert(aic3104_init(&d, &c) == AIC3104_OK);
    }
    /* Includes failure after the left channel was already updated. */
    for (unsigned i = 1; i <= 8; ++i) {
        setup(&d, &f);
        assert(aic3104_init(&d, &c) == AIC3104_OK);
        f.fail_at = f.calls + i;
        assert(aic3104_set_dac_mute(&d, false) == AIC3104_IO_ERROR);
        assert(f.reset && !d.ready);
    }
    setup(&d, &f);
    f.power_ready = false;
    assert(aic3104_init(&d, &c) == AIC3104_TIMEOUT);
    assert(f.reset && !d.ready && f.waited == 2003);

    for (unsigned outputs = 0; outputs <= 3; ++outputs) {
        for (unsigned left = 0; left <= 1; ++left) {
            for (unsigned right = 0; right <= 1; ++right) {
                setup(&d, &f);
                c = aic3104_default_config();
                c.mclk_hz = 24576000;
                c.word_bits = 32;
                c.outputs = (uint8_t)outputs;
                c.left_input = (aic3104_input_t)left;
                c.right_input = (aic3104_input_t)right;
                assert(aic3104_init(&d, &c) == AIC3104_OK);
                assert(f.registers[3] == 0x20 && f.registers[9] == 0x30);
                assert(f.registers[19] == (left ? 0x7c : 4));
                assert(f.registers[22] == (right ? 0x7c : 4));
                assert(f.registers[17] == (left ? 0x0f : 0xff));
                assert(f.registers[18] == (right ? 0xf0 : 0xff));
                assert(f.registers[47] == ((outputs & 1) ? 0x80 : 0));
                assert(f.registers[82] == ((outputs & 2) ? 0x80 : 0));
            }
        }
    }
    setup(&d, &f);
    c = aic3104_default_config();
    c.word_bits = 16;
    assert(aic3104_init(&d, &c) == AIC3104_OK && f.registers[9] == 0);
    unsigned before = f.calls;
    c.sample_rate_hz = 44100;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT && f.calls == before);
    c = aic3104_default_config(); c.mclk_hz = 12000000;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT);
    c = aic3104_default_config(); c.word_bits = 20;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT);
    c = aic3104_default_config(); c.left_input = (aic3104_input_t)-1;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT);
    c = aic3104_default_config(); c.outputs = 4;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT);
    c = aic3104_default_config(); c.adc_gain_half_db[1] = 120;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT);
    c = aic3104_default_config(); c.dac_attenuation_half_db[0] = 128;
    assert(aic3104_init(&d, &c) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_init(&d, NULL) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_init(NULL, &c) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_attach(&d, &d.bus) == AIC3104_OK);
    assert(aic3104_attach(&d, NULL) == AIC3104_INVALID_ARGUMENT);
    assert(aic3104_set_dac_mute(&d, true) == AIC3104_INVALID_ARGUMENT);
    aic3104_shutdown(NULL);
    puts("TLV320AIC3104 tests passed (configuration, routes, mute, fault injection, timeout, recovery)");
    return 0;
}
