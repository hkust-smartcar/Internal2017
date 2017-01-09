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
    cameraConfig.fps = k60::Ov7725Configurator::Config::Fps::kMid; // placeholder, not sure if kMid is enough
    cameraConfig.id = 0;
    cameraConfig.w = 128; // downscale the width to 128
    cameraConfig.h = 160; // downscale the height to 160
    k60::Ov7725 camera(cameraConfig);

    // initialize LCD
    St7735r::Config lcdConfig;
    lcdConfig.is_bgr = true;
    lcdConfig.fps = 10; //
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

            // lock the buffer, and retrieve the data into an array
            const Byte *pImgBuffer = camera.LockBuffer();
            const auto bufferSize = camera.GetBufferSize();
            Byte bufferArr[bufferSize];
            for (uint16_t i = 0; i < bufferSize; ++i) {
                bufferArr[i] = static_cast<Byte>(pImgBuffer[i]);
            }

            // unlock the buffer, since we got everything we need in bufferArr
            camera.UnlockBuffer();

            // convert the data into binary, and store it in a vector
            std::vector<bool> bufferVec;
            for (Byte b : bufferArr) {
                // cast the byte into a 8-length string in binary form
                std::string s = std::bitset<8>(static_cast<int>(b)).to_string();
                for (uint8_t j = 0; j < 8; ++j) {
                    // retrieve the j-th character from the string,
                    // cast it into an int, and convert it into a boolean
                    bufferVec.push_back((static_cast<uint8_t>(s.at(j)) - '0') == 1);
                }
            }

            // clear the lcd
            lcd.Clear();

            // fill the lcd
            for (uint32_t i = 0; i < bufferVec.size(); ++i) {
                // go through the pixels one by one
                lcd.SetRegion(Lcd::Rect(i/128, i%128, 1, 1));
                // fill the colors according to boolean value in vector
                // assuming true = white, false = black
                lcd.FillColor(bufferVec[i] ? Lcd::kWhite : Lcd::kBlack);
            }
        }
    }

    // clean up resources in case something goes wrong
    camera.Stop();

    return 0;
}
#pragma clang diagnostic pop
