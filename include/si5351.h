#ifndef PICO53_SI5351_H
#define PICO53_SI5351_H

#include <math.h>
#include <stdint.h>

#define SI5351_FREQ_TOL 2

extern const uint8_t Si5351_OUT_EN_CTRL;
extern const uint8_t Si5351_CLK0_CTRL;
extern const uint8_t Si5351_CLK1_CTRL;
extern const uint8_t Si5351_CLK2_CTRL;

extern const uint32_t Si5351_MIN_VCO_FREQ;
extern const uint32_t Si5351_MAX_VCO_FREQ;

extern const uint8_t Si5351_MIN_FMD_INT;
extern const uint8_t Si5351_MAX_FMD_INT;

extern const uint16_t Si5351_MAX_OMD_INT;

extern const uint32_t Si5351_MAX_DIVIDER_DENOMINATOR;

enum RefFrequency { REF_25 = 25000000, REF_27 = 27000000 };
enum Multisynth { MS0 = 0, MS1 = 1, MS2 = 2 };
enum PLL { PLL_A, PLL_B };

struct Si5351Config {
    uint32_t fmd_int;
    uint32_t fmd_num;
    uint32_t fmd_den;
    uint32_t omd_int;
    uint32_t omd_num;
    uint32_t omd_den;
    enum RefFrequency ref_freq;
};

double si5351_comp_freq(struct Si5351Config *config);
uint8_t si5351_valid_conf(struct Si5351Config *config);
uint8_t si5351_gen_conf(struct Si5351Config *config, double freq);
uint32_t si5351_comp_p1(struct Si5351Config *config, uint8_t fmd);
uint32_t si5351_comp_p2(struct Si5351Config *config, uint8_t fmd);
uint32_t si5351_comp_p3(struct Si5351Config *config, uint8_t fmd);
void si5351_comp_fmd_regs(struct Si5351Config *config, enum PLL pll, uint8_t *result);
void si5351_comp_omd_regs(struct Si5351Config *config, enum Multisynth ms, uint8_t *result);

#endif
