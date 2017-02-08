/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "util/unit_tests.h"

#include <memory>

#include "libsc/alternate_motor.h"
#include "libsc/dir_motor.h"
#include "libsc/futaba_s3010.h"
#include "libsc/led.h"
#include "libsc/st7735r.h"
#include "libsc/system.h"
#include "libsc/k60/ov7725.h"

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

void LedTest() {
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

void LcdTest() {
  St7735r::Config config;
  config.fps = 10;
  unique_ptr<St7735r> lcd(new St7735r(config));

  while (true) {
    lcd->SetRegion(Lcd::Rect(0, 0, 128, 80));
    lcd->FillColor(Lcd::kWhite);
    lcd->SetRegion(Lcd::Rect(0, 80, 128, 80));
    lcd->FillColor(Lcd::kBlack);
  }

  lcd.reset(nullptr);
}

void CameraTest() {
  // initialize camera
  Ov7725::Config camera_config;
  camera_config.id = 0;
  camera_config.w = 80;  // downscale the width to 80
  camera_config.h = 60;  // downscale the height to 60
  unique_ptr<Ov7725> camera(new Ov7725(camera_config));
  camera->Start();
  constexpr Uint kBufferSize = 80 * 60 / 8;
  if (kBufferSize != camera->GetBufferSize()) {
    return;
  }

  // initialize LCD
  St7735r::Config lcd_config;
  lcd_config.fps = 50;
  unique_ptr<St7735r> lcd(new St7735r(lcd_config));

  while (!camera->IsAvailable()) {}

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

  camera->Stop();
  camera.reset(nullptr);
  lcd.reset(nullptr);
}

void ServoTest() {
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

void AltMotorTest() {
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
