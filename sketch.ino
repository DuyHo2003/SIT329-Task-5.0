#include "sam.h"
#include "GPIO_DEAKIN.h"
#include "TIMER_DEAKIN.h"

// ----------------- LED Configuration -----------------
typedef struct {
    char port;   // 'A' or 'B'
    uint8_t pin; // Pin number
} LedPin;

#define NUM_LEDS 6
LedPin leds[NUM_LEDS] = {
    {'A', 10}, // LED1
    {'A', 11}, // LED2
    {'B', 10}, // LED3
    {'B', 11}, // LED4
    {'A', 20}, // LED5
    {'A', 21}  // LED6
};

#define BUTTON_PIN 6 // PA6

// ----------------- States -----------------
#define STOP  0
#define RUN   1
#define RESET 2

volatile uint8_t state = STOP;
volatile int pos = 0;
volatile int dir = 1;

// ----------------- LED Control -----------------
void update_leds(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        Write_GPIO(leds[i].port, leds[i].pin, LOW);
    }
    Write_GPIO(leds[pos].port, leds[pos].pin, HIGH);
}

void led_init(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        Config_GPIO(leds[i].port, leds[i].pin, OUTPUT);
        Write_GPIO(leds[i].port, leds[i].pin, LOW);
    }
}

// ----------------- Button ISR -----------------
void EIC_Handler(void) {
    // Cycle through states: STOP → RUN → RESET → STOP ...
    if (state == STOP) {
        state = RUN;
    } else if (state == RUN) {
        state = STOP;
    } else if (state == RESET) {
        // Perform reset now
        pos = 0;
        dir = 1;
        update_leds();
        state = STOP; // After reset, go to STOP
    }

    // Prepare for next press (if RUN→RESET)
    if (state == STOP && pos != 0) {
        state = RESET; // Next press will trigger RESET
    }

    EIC->INTFLAG.reg = (1 << 6); // Clear interrupt flag
}

// ----------------- Button Init -----------------
void button_init(void) {
    PORT->Group[0].DIRCLR.reg = (1 << BUTTON_PIN); // input
    PORT->Group[0].PINCFG[BUTTON_PIN].bit.INEN = 1;
    PORT->Group[0].PINCFG[BUTTON_PIN].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[BUTTON_PIN >> 1].bit.PMUXE = MUX_PA06A_EIC_EXTINT6;

    PM->APBAMASK.reg |= PM_APBAMASK_EIC;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);

    EIC->CONFIG[0].reg &= ~EIC_CONFIG_SENSE6_Msk;
    EIC->CONFIG[0].reg |= EIC_CONFIG_SENSE6_FALL;
    EIC->INTENSET.reg = (1 << 6);

    EIC->CTRL.bit.ENABLE = 1;
    while (EIC->STATUS.bit.SYNCBUSY);

    NVIC_EnableIRQ(EIC_IRQn);
}

// ----------------- Timer ISR -----------------
void TCC0_Handler(void) {
    if (state == RUN) {
        pos += dir;
        if (pos == NUM_LEDS - 1 || pos == 0) {
            dir = -dir;
        }
        update_leds();
    }
    TCC0->INTFLAG.reg = TCC_INTFLAG_OVF; // clear overflow flag
}

// ----------------- Main -----------------
int main(void) {
    SystemInit();
    led_init();
    button_init();
    update_leds(); // Ensure first LED starts ON
    TCC0_init(200); // 200 ms per step
    __enable_irq();

    while (1) {
        // Idle loop; ISRs manage LEDs
    }
}
