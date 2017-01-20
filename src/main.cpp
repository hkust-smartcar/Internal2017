/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/main.h"

#include <libbase/k60/mcg.h>
#include <libsc/lcd.h>
#include <libsc/lcd_console.h>
#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>
#include <libsc/k60/ov7725.h>

#include <algorithm>
#include <string>

#include "../inc/alternate_algorithms/area_method.h"

struct CameraRowInfo {
    Uint pixel_num;
    Uint left_bound;
    Uint right_bound;
    Uint center_point;
} rowInfo[cameraHeight];

namespace libbase {
namespace k60 {
Mcg::Config Mcg::GetMcgConfig() {
  Mcg::Config config;
  config.external_oscillator_khz = 50000;
  config.core_clock_khz = 150000;
  return config;
}

}  // namespace k60
}  // namespace libbase

// selective includes to avoid namespace pollution
using libsc::Led;
using libsc::System;

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

using libsc::Lcd;
using libsc::LcdConsole;
using libsc::St7735r;
using libsc::Timer;
using libsc::k60::Ov7725;

int main() {
    System::Init();

    Led::Config config;
    config.is_active_low = true;
    config.id = 0;
    Led led1(config);
    config.id = 1;
    Led led2(config);
    config.id = 2;
    Led led3(config);
    config.id = 3;
    Led led4(config);

    led1.SetEnable(false);
    led2.SetEnable(false);  // buffer processing led
    led3.SetEnable(false);
    led4.SetEnable(true);  // initialization led

    // initialize camera
    Ov7725::Config cameraConfig;
    cameraConfig.id = 0;
    cameraConfig.w = cameraWidth;
    cameraConfig.h = cameraHeight;
    Ov7725 camera(cameraConfig);

    // initialize LCD
    St7735r::Config lcdConfig;
    lcdConfig.fps = 60;
    St7735r lcd(lcdConfig);
    lcd.Clear();

    // start the camera and wait until it is ready
    camera.Start();
    while (!camera.IsAvailable()) {}

    Timer::TimerInt timeImg = System::Time();  // current execution time
    const Uint kBufferSize = camera.GetBufferSize();

    led4.SetEnable(false);

    while (true) {
        if (timeImg != System::Time()) {
            timeImg = System::Time();

            led1.SetEnable(timeImg % 1000 >= 500);

            if (timeImg % 250 == 0) {
                led2.SetEnable(true);

                // copy the buffer for processing
                const Byte *pBuffer = camera.LockBuffer();
                Byte bufferArr[kBufferSize];
                for (Uint i = 0; i < kBufferSize; ++i) {
                    bufferArr[i] = pBuffer[i];
                }
                camera.UnlockBuffer();

                // 1d to 2d array
                bool image2d[cameraHeight][cameraWidth];
                led2.SetEnable(true);
                for (Uint i = 0; i < kBufferSize; ++i) {
                    std::string s =
                            std::bitset<8>(static_cast<Uint>(bufferArr[i])).to_string();
                    for (uint8_t j = 0; j < 8; ++j) {
                        image2d[i*8 / cameraWidth][(i*8 % cameraWidth) + j] =
                                (static_cast<Uint>(s.at(j) - '0') == 1);
                    }
                }

                for (Uint i = 0; i < cameraHeight; ++i) {
                    rowInfo[i].pixel_num = static_cast<Uint>(std::count(image2d[i], image2d[i] + 160, false));
                    for (Uint j = (i == 0) ? 64 : rowInfo[i-1].center_point; j < cameraWidth; ++j) {
                        if (image2d[i][j]) {
                            rowInfo[i].right_bound = j;
                            break;
                        }
                        rowInfo[i].right_bound = (i == 0) ? 128 : (rowInfo[i-1].right_bound + 128) / 2;
                    }
                    for (Uint j = (i == 0) ? 64 : rowInfo[i-1].center_point; j > 0; --j) {
                        if (image2d[i][j]) {
                            rowInfo[i].left_bound = j;
                            break;
                        }
                        rowInfo[i].left_bound = (i == 0) ? 0 : (rowInfo[i-1].left_bound) / 2;
                    }
                    rowInfo[i].center_point = (rowInfo[i].left_bound + rowInfo[i].right_bound) / 2;
                }

                led2.SetEnable(false);

                lcd.SetRegion(Lcd::Rect(0, 0, 128, 160));
                lcd.FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera.GetBufferSize() * 8);
                for (Uint i = 0; i < cameraHeight; ++i) {
                    lcd.SetRegion(Lcd::Rect(rowInfo[i].center_point, i, 1, 1));
                    lcd.FillColor(Lcd::kGreen);
                }

                // commit motor/servo changes here

                led2.SetEnable(false);
            }
        }
    }

    return 0;
}
