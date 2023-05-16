#include "si5351.h"

const uint8_t Si5351_OUT_EN_CTRL = 0x03;
const uint8_t Si5351_CLK0_CTRL = 0x10;
const uint8_t Si5351_CLK1_CTRL = 0x11;
const uint8_t Si5351_CLK2_CTRL = 0x12;

const uint32_t Si5351_MIN_VCO_FREQ = 600000000;
const uint32_t Si5351_MAX_VCO_FREQ = 900000000;

const uint8_t Si5351_MIN_FMD_INT = 15;
const uint8_t Si5351_MAX_FMD_INT = 90;

const uint16_t Si5351_MAX_OMD_INT = 2048;

const uint32_t Si5351_MAX_DIVIDER_DENOMINATOR = 1048575;

static uint8_t valid_omd_int(uint32_t omd) {
    switch (omd) {
        case 4:
        case 6:
        case 8:
            return 1;
        default:
            if (omd < 8) { return 0; }
            if (omd > Si5351_MAX_OMD_INT) { return 2; }
            return 1;
    }
}

static uint8_t valid_vco(double freq) {
    if (freq < Si5351_MIN_VCO_FREQ) {
        return 0;
    } else if (freq > Si5351_MAX_VCO_FREQ) {
        return 2;
    } else {
        return 1;
    }
}

static uint8_t set_omd_sub_150(struct Si5351Config *config, double freq) {
    uint8_t tries = 0;

    config->omd_int = (int32_t)(Si5351_MIN_VCO_FREQ / freq);
    while (tries < 10) {
        switch (valid_vco(freq * config->omd_int)) {
            case 0:
                config->omd_int++;
                continue;
            case 2:
                config->omd_int--;
                continue;
        }
        switch (valid_omd_int(config->omd_int)) {
            case 0:
                config->omd_int++;
                break;
            case 1:
                return 0;
            case 2:
                config->omd_int--;
                break;
        }

        tries++;
    }

    return 1;
}

static void set_omd(struct Si5351Config *config, double freq) {
    config->omd_num = 0;
    config->omd_den = 1;

    if (freq < 150000000) {
        set_omd_sub_150(config, freq);
    } else {
        config->omd_int = 4;
    }
}

static void set_fmd(struct Si5351Config *config, double freq) {
    double fmd = (freq * config->omd_int) / config->ref_freq;
    config->fmd_int = (uint32_t)fmd;
    config->fmd_den = Si5351_MAX_DIVIDER_DENOMINATOR;

    fmd -= config->fmd_int;
    config->fmd_num = (uint32_t)(fmd * config->fmd_den);
}

double si5351_comp_freq(struct Si5351Config *config) {
    double feedback_multiplier =
        (double)config->fmd_int + ((double)config->fmd_num / (double)config->fmd_den);
    double output_divider =
        (double)config->omd_int + ((double)config->omd_num / (double)config->omd_den);

    return ((float)config->ref_freq * feedback_multiplier) / (output_divider);
}

uint8_t si5351_valid_conf(struct Si5351Config *config) {
    if (valid_omd_int(config->omd_int) != 1) { return 1; }
    if ((config->omd_int == Si5351_MAX_OMD_INT) && (config->omd_num != 0)) { return 1; }
    if (config->omd_num > Si5351_MAX_DIVIDER_DENOMINATOR) { return 1; }
    if ((config->omd_den == 0) || (config->omd_den > Si5351_MAX_DIVIDER_DENOMINATOR)) { return 1; }
    if (config->fmd_int < Si5351_MIN_FMD_INT) { return 1; }
    if (config->fmd_int > Si5351_MAX_FMD_INT) { return 1; }
    if ((config->fmd_int == Si5351_MAX_FMD_INT) && (config->fmd_num != 0)) { return 1; }
    if (config->fmd_num > Si5351_MAX_DIVIDER_DENOMINATOR) { return 1; }
    if ((config->fmd_den == 0) || (config->omd_den > Si5351_MAX_DIVIDER_DENOMINATOR)) { return 1; }

    double vco = config->ref_freq * (config->fmd_int + ((double)config->fmd_num / config->fmd_den));
    if (valid_vco(vco) != 1) { return 1; }

    return 0;
}

uint8_t si5351_gen_conf(struct Si5351Config *config, double freq) {
    set_omd(config, freq);
    set_fmd(config, freq);

    return si5351_valid_conf(config);
}

uint32_t si5351_comp_p1(struct Si5351Config *config, uint8_t fmd) {
    uint32_t a, b, c;
    if (fmd != 0) {
        a = config->fmd_int;
        b = config->fmd_num;
        c = config->fmd_den;
    } else {
        a = config->omd_int;
        b = config->omd_num;
        c = config->omd_den;
    }

    return (uint32_t)(128 * a + floor(128.0 * b / c) - 512);
}

uint32_t si5351_comp_p2(struct Si5351Config *config, uint8_t fmd) {
    uint32_t b, c;
    if (fmd != 0) {
        b = config->fmd_num;
        c = config->fmd_den;
    } else {
        b = config->omd_num;
        c = config->omd_den;
    }

    return (uint32_t)(128 * b - c * floor(128.0 * b / c));
}

uint32_t si5351_comp_p3(struct Si5351Config *config, uint8_t fmd) {
    if (fmd != 0) { return config->fmd_den; }

    return config->omd_den;
}

void si5351_comp_fmd_regs(struct Si5351Config *config, enum PLL pll, uint8_t *result) {
    uint32_t p1 = si5351_comp_p1(config, 1);
    uint32_t p2 = si5351_comp_p2(config, 1);
    uint32_t p3 = si5351_comp_p3(config, 1);

    if (pll == PLL_A) {
        result[0] = 0x1A;
    } else {
        result[0] = 0x22;
    }

    result[1] = p3 >> 8;
    result[2] = p3;
    result[3] = 0x03 & (p1 >> 16);
    result[4] = p1 >> 8;
    result[5] = p1;
    result[6] = (0xF0 & (p3 >> 12)) | (0x0F & (p2 >> 16));
    result[7] = p2 >> 8;
    result[8] = p2;
}

void si5351_comp_omd_regs(struct Si5351Config *config, enum Multisynth ms, uint8_t *result) {
    uint32_t p1 = si5351_comp_p1(config, 0);
    uint32_t p2 = si5351_comp_p2(config, 0);
    uint32_t p3 = si5351_comp_p3(config, 0);
    uint8_t div4;

    if (config->omd_int == 4) {
        div4 = 0x0C;
    } else {
        div4 = 0x00;
    }

    switch (ms) {
        case MS0:
            result[0] = 0x2A;
            break;
        case MS1:
            result[0] = 0x32;
            break;
        case MS2:
            result[0] = 0x3A;
            break;
    }

    result[1] = p3 >> 8;
    result[2] = p3;
    result[3] = div4 | (0x03 & (p1 >> 16));
    result[4] = p1 >> 8;
    result[5] = p1;
    result[6] = (0xF0 & (p3 >> 12)) | (0x0F & (p2 >> 16));
    result[7] = p2 >> 8;
    result[8] = p2;
}
