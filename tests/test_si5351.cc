#include "gtest/gtest.h"

extern "C" {
#include "si5351.h"
}

class TestSi5351 : public testing::Test {
   protected:
    Si5351Config config = Si5351Config();

    void SetUp() override { config.ref_freq = REF_25; }
};

TEST_F(TestSi5351, CalcFreq) {
    config = {
        .fmd_int = 24,
        .fmd_num = 0,
        .fmd_den = 1,
        .omd_int = 8,
        .omd_num = 0,
        .omd_den = 1,
        .ref_freq = REF_25};
    ASSERT_EQ(75e6, si5351_comp_freq(&config));

    config.fmd_num = 6;
    config.fmd_den = 10;
    config.omd_int = 4;
    ASSERT_EQ(153.75e6, si5351_comp_freq(&config));

    config.fmd_num = 482;
    config.fmd_den = 1000;
    ASSERT_EQ(153.0125e6, si5351_comp_freq(&config));
}

TEST_F(TestSi5351, GenConf) {
    double target_freq = 28.567e6;

    ASSERT_EQ(0, si5351_gen_conf(&config, target_freq));
    ASSERT_NEAR(si5351_comp_freq(&config), target_freq, SI5351_FREQ_TOL);

    target_freq = 153.0125e6;
    ASSERT_EQ(0, si5351_gen_conf(&config, target_freq));
    ASSERT_NEAR(si5351_comp_freq(&config), target_freq, SI5351_FREQ_TOL);

    target_freq = 10.7e6;
    ASSERT_EQ(0, si5351_gen_conf(&config, target_freq));
    ASSERT_NEAR(si5351_comp_freq(&config), target_freq, SI5351_FREQ_TOL);

    target_freq = 130e6;
    ASSERT_EQ(0, si5351_gen_conf(&config, target_freq));
    ASSERT_NEAR(si5351_comp_freq(&config), target_freq, 5);
}
