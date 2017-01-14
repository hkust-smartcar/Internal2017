/*
 * main.cpp
 *
 * Author: Peter
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/include_all.h"

#include <libbase/k60/mcg.h>
#include <libsc/system.h>

namespace libbase {
    namespace k60 {
        Mcg::Config Mcg::GetMcgConfig() {
          Mcg::Config config;
          config.external_oscillator_khz = 50000;
          config.core_clock_khz = 150000;
          return config;
        }

    }
}

using namespace libsc;
using namespace libbase::k60;

/**
 * Toggles the LEDs alternately on the mainboard
 */
void heartbeatTest() {
    // USING LED
    Led::Config config;
    config.id = 0;
    Led led1(config);
    config.id = 1;
    Led led2(config);
    config.id = 2;
    Led led3(config);
    config.id = 3;
    Led led4(config);

    led1.SetEnable(true);
    led2.SetEnable(false);
    led3.SetEnable(false);
    led4.SetEnable(true);
    while (true) {
        led1.Switch();
        led2.Switch();
        led3.Switch();
        led4.Switch();
        System::DelayMs(250);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main() {
    System::Init();

    Led::Config ledConfig;
    ledConfig.is_active_low = true;
    ledConfig.id = 0;
    Led led1(ledConfig);
    ledConfig.id = 1;
    Led led2(ledConfig);
    ledConfig.id = 2;
    Led led3(ledConfig);
    ledConfig.id = 3;
    Led led4(ledConfig);

    led1.SetEnable(false);
    led2.SetEnable(true);
    led3.SetEnable(false);
    led4.SetEnable(false);

    // initialize camera
    k60::Ov7725::Config cameraConfig;
    cameraConfig.id = 0;
    cameraConfig.w = 80; // downscale the width to 80
    cameraConfig.h = 60; // downscale the height to 60
    k60::Ov7725 camera(cameraConfig);

    // initialize LCD
    St7735r::Config lcdConfig;
    lcdConfig.fps = 100;
    St7735r lcd(lcdConfig);

    // current execution time
    Timer::TimerInt timeImg = System::Time();

    // start the camera before we do anything
    camera.Start();

    // wait until the camera is ready
    while (!camera.IsAvailable()) {}

    while (true) {
        // check if we're still in the same cycle
        if (timeImg != System::Time()) {
            // check if it takes shorter than 9ms to read+copy the buffer
            if ( ( System::Time() - timeImg ) <= 9) {
                // update the cycle
                timeImg = System::Time();

                // lock the buffer and copy it
                const auto bufferSize = camera.GetBufferSize();
                const Byte* pBuffer = camera.LockBuffer();
                Byte bufferArr[bufferSize];
                for (uint16_t i = 0; i < bufferSize; ++i) {
                    bufferArr[i] = pBuffer[i];
                }

                // unlock the buffer now that we have the data
                camera.UnlockBuffer();

                // rewrite it with new data
                /*lcd.SetRegion(Lcd::Rect(0, 0, 80, 60));
                lcd.FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera.GetBufferSize() * 8);*/
            } else {
                break;
            }
        }
    }

    // turn on LED4 when the process cannot keep up
    led4.SetEnable(true);

    // clean up resources in case something goes wrong
    camera.Stop();

    // infinite loop to keep the program from terminating
    while (1) {
    }

    return 0;
}
#pragma clang diagnostic pop
