/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/unit_tests.h"

#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>

using libsc::Lcd;
using libsc::Led;
using libsc::St7735r;
using libsc::System;
using libsc::Timer;

/**
 * Tests the LEDs of the mainboard
 *
 * Toggles the LEDs alternately.
 *
 * @note LEDs will be initialized within the function
 */
void ledTest() {
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

/**
 * Tests the LCD of the mainboard
 *
 * Toggles the LCD between black and white, with the top and bottom portions of the
 * screen having alternate colors.
 *
 * @note LCD will be initialized within the function
 */
void lcdTest() {
  St7735r::Config lcdConfig;
  lcdConfig.fps = 10;
  St7735r lcd(lcdConfig);
  lcd.Clear();

  Timer::TimerInt ticksImg = System::Time();

  while (true) {
    if (ticksImg != System::Time()) {
      ticksImg = System::Time();
      if (ticksImg % 500 == 0) {
        lcd.SetRegion(Lcd::Rect(0, 0, 128, 80));
        lcd.FillColor(ticksImg / 500 == 0 ? Lcd::kBlack : Lcd::kWhite);
        lcd.SetRegion(Lcd::Rect(0, 80, 128, 80));
        lcd.FillColor(ticksImg / 500 == 1 ? Lcd::kBlack : Lcd::kWhite);
      }
    }
  }
}
