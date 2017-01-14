/*
 * main.cpp
 *
 * Author: Peter
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <memory>

#include <libbase/k60/mcg.h>
#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>
#include <libsc/k60/ov7725.h>

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

// selective includes to avoid namespace pollution
using libsc::Lcd;
using libsc::Led;
using libsc::St7735r;
using libsc::System;
using libsc::Timer;
using libsc::k60::Ov7725;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
/**
 * Toggles the LEDs alternately on the mainboard
 *
 * Used to test compilation, flashing, and configuration
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
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main() {
    System::Init();

    // initialize LEDs
    Led::Config ledConfig;
    ledConfig.is_active_low = true;
    ledConfig.id = 0;
    Led led1(ledConfig);  // main loop
    ledConfig.id = 1;
    Led led2(ledConfig);  // unused
    ledConfig.id = 2;
    Led led3(ledConfig);  // unused
    ledConfig.id = 3;
    Led led4(ledConfig);  // heartbeat

    led1.SetEnable(false);
    led2.SetEnable(false);
    led3.SetEnable(false);
    led4.SetEnable(false);

    // initialize camera
    Ov7725::Config cameraConfig;
    cameraConfig.id = 0;
    cameraConfig.w = 80; // downscale the width to 80
    cameraConfig.h = 60; // downscale the height to 60
    Ov7725 camera(cameraConfig);

    // initialize LCD
    St7735r::Config lcdConfig;
    lcdConfig.fps = 100;
    St7735r lcd(lcdConfig);

    // start the camera and wait until it's ready
    camera.Start();
    while (!camera.IsAvailable()) {}

    Timer::TimerInt timeImg = System::Time();  // current execution time
    Timer::TimerInt startTime;  // starting time for read+copy buffer
    const uint8_t test_ms = 10;  // testing case in ms
    const Uint bufferSize = camera.GetBufferSize(); // size of camera buffer

    led1.SetEnable(true);

    // main loop
    while (true) {
        // limit max refresh time to 1ms
        if (timeImg != System::Time()) {
            // update the cycle
            timeImg = System::Time();

            // attempt to refresh the buffer at every 10th millisecond
            if ((System::Time() % 10) == 0) {
                // record the starting time
                startTime = System::Time();

                // lock the buffer and copy it
                const Byte *pBuffer = camera.LockBuffer();
                Byte *bufferArr = new Byte[bufferSize];
                for (uint16_t i = 0; i < bufferSize; ++i) {
                    bufferArr[i] = pBuffer[i];
                }

                // unlock the buffer now that we have the data
                camera.UnlockBuffer();

                // break from loop when the read+copy process takes longer
                // than test_ms
                if (Timer::TimeDiff(System::Time(), startTime) > test_ms) {
                    // clean up resources
                    camera.Stop();
                    delete [] bufferArr;
                    bufferArr = nullptr;

                    break;
                }

                // rewrite lcd with new data
                lcd.SetRegion(Lcd::Rect(0, 0, 80, 60));
                lcd.FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera.GetBufferSize()*8);
            }

            // 0.5s heartbeat
            if ((System::Time() % 500) == 0) {
                led4.Switch();
            }
        }
    }

    // turn off led1 now we're off the main loop
    led1.SetEnable(false);

    // infinite loop to keep the program from terminating
    while (true) {}

    return 0;
}
#pragma clang diagnostic pop
