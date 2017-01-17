#include "../../inc/assignments/button_toggle_led.h"

#include <libsc/button.h>
#include <libsc/led.h>

using namespace libsc;

// Pointer to led1, for access to both the listener and the main function
Led* pLed1;
void B1Listener(const uint8_t);

/**
 * Lights LED up when button is held down
 */
void buttonToggleLed() {
    Led::Config configLed;
    configLed.id = 0;
    configLed.is_active_low = true;
    Led led1(configLed);

    pLed1 = &led1;

    Button::Config configBtn;
    configBtn.id = 0;
    configBtn.is_active_low = true;
    configBtn.is_use_pull_resistor = true;
    configBtn.listener = &B1Listener;
    configBtn.listener_trigger = Button::Config::Trigger::kBoth;
    Button btn1(configBtn);

    while (true) {

    }
}

/**
 * Listener for button 1
 * @param id ID of the button
 */
void B1Listener(const uint8_t id) {
    pLed1->Switch();
}
