/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "util/unit_tests.h"

#include <libsc/alternate_motor.h>
#include <libsc/dir_motor.h>
#include <libsc/futaba_s3010.h>
#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>
#include <libsc/k60/ov7725.h>

#include <memory>

using libsc::AlternateMotor;
using libsc::DirMotor;
using libsc::FutabaS3010;
using libsc::Lcd;
using libsc::Led;
using libsc::St7735r;
using libsc::System;
using libsc::Timer;
using libsc::k60::Ov7725;
using std::unique_ptr;

namespace util {

/**
 * Tests the LEDs of the mainboard
 *
 * Toggles the LEDs alternately.
 *
 * @note LEDs will be initialized within the function
 */
void ledTest() {
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
  St7735r::Config config;
  config.fps = 10;
  unique_ptr<St7735r> lcd(new St7735r(config));

  while (true) {
    lcd->SetRegion(Lcd::Rect(0, 0, 128, 80));
    lcd->FillColor(Lcd::kWhite);
    lcd->SetRegion(Lcd::Rect(0, 80, 128, 80));
    lcd->FillColor(Lcd::kBlack);
  }
}

/**
 * Tests the camera of the mainboard
 *
 * Outputs the camera image onto the LCD. Requires a working St7735r panel.
 *
 * @note Camera and LCD will be initialized within the function.
 */
void cameraTest() {
  // initialize camera
  Ov7725::Config camera_config;
  camera_config.id = 0;
  camera_config.w = 80;  // downscale the width to 80
  camera_config.h = 60;  // downscale the height to 60
  unique_ptr<Ov7725> camera(new Ov7725(camera_config));
  camera->Start();

  // initialize LCD
  St7735r::Config lcd_config;
  lcd_config.fps = 50;
  unique_ptr<St7735r> lcd(new St7735r(lcd_config));

  while (!camera->IsAvailable()) {}

  const Uint kBufferSize = camera->GetBufferSize();  // size of camera buffer

  while (true) {
    const Byte *pBuffer = camera->LockBuffer();
    Byte bufferArr[kBufferSize];
    for (uint16_t i = 0; i < kBufferSize; ++i) {
      bufferArr[i] = pBuffer[i];
    }

    camera->UnlockBuffer();
    lcd->SetRegion(Lcd::Rect(0, 0, 80, 60));
    lcd->FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, kBufferSize * 8);
  }

}

/**
 * Tests the servo of the mainboard
 *
 * Toggles the servo between 45-degrees and 135-degrees under 2 second intervals.
 *
 * @note Servo will be initialized within the function.
 */
void servoTest() {
  FutabaS3010::Config config;
  config.id = 0;
  FutabaS3010 servo(config);

  while (true) {
    servo.SetDegree(450);
    System::DelayMs(2000);
    servo.SetDegree(1350);
    System::DelayMs(2000);
  }
}

/**
 * Tests the alternate-motor of the mainboard
 *
 * Turns both alternate motors on at 10%.
 *
 * @note Motor will be initialized within the function.
 */
void altMotorTest() {
  AlternateMotor::Config config;
  config.multiplier = 100;
  config.id = 0;
  AlternateMotor motorLeft(config);
  config.id = 1;
  AlternateMotor motorRight(config);

  motorLeft.SetClockwise(true);
  motorRight.SetClockwise(true);

  while (true) {
    motorLeft.SetPower(100);
    motorRight.SetPower(100);
  }
}

}  // namespace util
