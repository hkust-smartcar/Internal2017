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

#include <bitset>
#include <string>

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

    // initialize leds
    Led::Config config;
    config.id = 0;
    Led led1(config);
    config.id = 1;
    Led led2(config);

    // initialize camera
    k60::Ov7725::Config cameraConfig;
    cameraConfig.fps = k60::Ov7725Configurator::Config::Fps::kHigh;
    cameraConfig.id = 0;
    cameraConfig.w = 80; // downscale the width to 80
    cameraConfig.h = 60; // downscale the height to 60
    k60::Ov7725 camera(cameraConfig);

    // initialize LCD
    St7735r::Config lcdConfig;
    lcdConfig.is_bgr = true;
    lcdConfig.fps = 100;
    St7735r lcd(lcdConfig);

    // current execution cycle
    // this ensures that even if the whole retrieve->display process overruns
    // (System::Time() % 10 == 0), the next cycle will still initiate
    volatile uint32_t cycleImg = System::Time()/10;

    // start the camera before we do anything
    camera.Start();

    // wait until the camera is ready
    while (!camera.IsAvailable()) {}

    while (true) {
        // check if we're still in the same cycle
        if (cycleImg != System::Time()/10) {
            // update the cycle
            cycleImg = System::Time()/10;

            // lock the buffer and copy it
            const auto bufferSize = camera.GetBufferSize();
            const Byte* pBuffer = camera.LockBuffer();
            Byte bufferArr[bufferSize];
            for (int i = 0; i < bufferSize; ++i) {
                bufferArr[i] = pBuffer[i];
            }

            // unlock the buffer now that we have the data
            camera.UnlockBuffer();

            // clear the screen and rewrite it with new data
            lcd.Clear();
            lcd.FillBits(Lcd::kWhite, Lcd::kBlack, bufferArr, bufferSize);
        }
    }

    // clean up resources in case something goes wrong
    camera.Stop();

    return 0;
}
#pragma clang diagnostic pop
