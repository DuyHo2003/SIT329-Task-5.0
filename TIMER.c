#include "sam.h"
#include "TIMER_DEAKIN.h"

void TCC0_init(uint32_t ms_tick) {
    // 1. Enable APB clock for TCC0
    PM->APBCMASK.reg |= PM_APBCMASK_TCC0;

    // 2. Configure GCLK1 = 1 kHz (approx.) for TCC0
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(48); // divide DFLL48MHz by 48
    while (GCLK->STATUS.bit.SYNCBUSY);

    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_GENEN;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // 3. Route GCLK1 to TCC0
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC0_TCC1 | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // 4. Reset TCC0
    TCC0->CTRLA.reg = TCC_CTRLA_SWRST;
    while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST);

    // 5. Set prescaler = 1
    TCC0->CTRLA.reg = TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV1_Val);
    while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_MASK);

    // 6. Normal PWM mode
    TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
    while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_WAVE);

    // 7. Set period (ms_tick)
    TCC0->PER.reg = ms_tick - 1;
    while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_PER);

    // 8. Enable overflow interrupt
    TCC0->INTFLAG.reg = TCC_INTFLAG_MASK;
    TCC0->INTENSET.reg = TCC_INTENSET_OVF;

    // 9. Enable NVIC
    NVIC_SetPriority(TCC0_IRQn, 3);
    NVIC_EnableIRQ(TCC0_IRQn);

    // 10. Enable TCC0
    TCC0->CTRLA.reg |= TCC_CTRLA_ENABLE;
    while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_ENABLE);
}
