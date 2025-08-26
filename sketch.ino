#include "sam.h"
#include "GPIO_DEAKIN.h"
#include "TIMER_DEAKIN.h"

// Define LED pin structure
typedef struct {
    char port;   // 'A' or 'B'
    uint8_t pin; // Pin number
} LedPin;

// LED configuration
#define NUM_LEDS 6
LedPin leds[NUM_LEDS] = {
    {'A', 10}, // LED1
    {'A', 11}, // LED2
    {'B', 10}, // LED3
    {'B', 11}, // LED4
    {'A', 20}, // LED5
    {'A', 21}  // LED6
};

#define BUTTON_PIN 6 // PA06 for button

// Variables
volatile int state = 0;  // 0=STOP, 1=RUN, 2=RESET
volatile int pos = 0;
volatile int dir = 1;

void updateLEDs(); // Forward declaration

// ---------------- Button ISR ----------------
void EIC_Handler(void){
    state++;
    if(state > 2) state = 0; // cycle STOP/RUN/RESET
    if(state == 2){ pos = 0; dir = 1; updateLEDs(); }
    EIC->INTFLAG.reg = (1 << BUTTON_PIN); // clear interrupt flag
}

// ---------------- Timer ISR ----------------
void TCC0_Handler(void){
    if(state==1){ // RUN mode
        pos += dir;
        if(pos==NUM_LEDS-1 || pos==0) dir=-dir;
        updateLEDs();
    }
    TCC0->INTFLAG.reg = TCC_INTFLAG_OVF; // clear timer overflow flag
}

// ---------------- Helper: Update LEDs ----------------
void updateLEDs(){
    for(int i = 0; i < NUM_LEDS; i++) {
        Write_GPIO(leds[i].port, leds[i].pin, LOW);
    }
    Write_GPIO(leds[pos].port, leds[pos].pin, HIGH);
}

// ---------------- Setup ----------------
void setup(){
    SystemInit();

    // Initialize LEDs
    for(int i = 0; i < NUM_LEDS; i++){
        Config_GPIO(leds[i].port, leds[i].pin, OUTPUT);
        Write_GPIO(leds[i].port, leds[i].pin, LOW);
    }

    // Initialize button with EIC interrupt
    PORT->Group[0].DIRCLR.reg = (1 << BUTTON_PIN);
    PORT->Group[0].PINCFG[BUTTON_PIN].bit.INEN = 1;
    PORT->Group[0].PINCFG[BUTTON_PIN].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[BUTTON_PIN >> 1].bit.PMUXE = MUX_PA06A_EIC_EXTINT6;

    PM->APBAMASK.reg |= PM_APBAMASK_EIC;
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
    while(GCLK->STATUS.bit.SYNCBUSY);

    EIC->CONFIG[0].reg &= ~EIC_CONFIG_SENSE6_Msk;
    EIC->CONFIG[0].reg |= EIC_CONFIG_SENSE6_FALL;
    EIC->INTENSET.reg = (1 << BUTTON_PIN);
    EIC->CTRL.bit.ENABLE = 1;
    while(EIC->STATUS.bit.SYNCBUSY);
    NVIC_EnableIRQ(EIC_IRQn);

    // Initialize TCC0 timer for 200ms interrupts
    TCC0_init(200);

    __enable_irq();
}

// ---------------- Main Loop ----------------
void loop(){
    __WFI(); // Wait for interrupts
}
