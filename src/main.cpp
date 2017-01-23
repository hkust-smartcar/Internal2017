/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/main.h"

#include <libbase/k60/mcg.h>
#include <libsc/alternate_motor.h>
#include <libsc/futaba_s3010.h>
#include <libsc/lcd.h>
#include <libsc/lcd_console.h>
#include <libsc/led.h>
#include <libsc/tower_pro_mg995.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>
#include <libsc/k60/ov7725.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <unit_tests.h>
#include <cmath>

struct CameraRowInfo {
  Uint pixel_num;
  Uint left_bound;
  Uint right_bound;
  Uint left_count;
  Uint right_count;
  Uint center_point;
} rowInfo[kCameraHeight];

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

using libsc::AlternateMotor;
using libsc::FutabaS3010;
using libsc::Lcd;
using libsc::LcdConsole;
using libsc::Led;
using libsc::St7735r;
using libsc::System;
using libsc::Timer;
using libsc::TowerProMg995;
using libsc::k60::Ov7725;

int main() {
  System::Init();

  altMotorTest();

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

  led1.SetEnable(false);  // main loop heartbeat
  led2.SetEnable(false);  // buffer processing led
  led3.SetEnable(false);  // unused
  led4.SetEnable(true);  // initialization led

  // initialize camera
  Ov7725::Config cameraConfig;
  cameraConfig.id = 0;
  cameraConfig.w = kCameraWidth;
  cameraConfig.h = kCameraHeight;
  Ov7725 camera(cameraConfig);

  // initialize LCD
  St7735r::Config lcdConfig;
  lcdConfig.fps = 10;
  St7735r lcd(lcdConfig);
  lcd.Clear();

  // initialize servo
  FutabaS3010::Config servoConfig;
  servoConfig.id = 0;
  FutabaS3010 servo(servoConfig);
  servo.SetDegree(kServoCenter);

  // initialize motors
  AlternateMotor::Config motorConfig;
  motorConfig.multiplier = kMotorPowerMultiplier;
  motorConfig.id = 0;
  AlternateMotor motorLeft(motorConfig);
  motorConfig.id = 1;
  AlternateMotor motorRight(motorConfig);
  motorLeft.SetPower(0);
  motorLeft.SetClockwise(false);
  motorRight.SetClockwise(true);

  // start the camera and wait until it is ready
  camera.Start();
  while (!camera.IsAvailable()) {}

  Timer::TimerInt timeImg = System::Time();  // current execution time
  const Uint kBufferSize = camera.GetBufferSize();

  led4.SetEnable(false);

  while (true) {
    if (timeImg != System::Time()) {
      timeImg = System::Time();

      // heartbeat!
      led1.SetEnable(timeImg % 1000 >= 500);

      led2.SetEnable(true);

      // copy the buffer for processing
      const Byte *pBuffer = camera.LockBuffer();
      Byte bufferArr[kBufferSize];
      for (Uint i = 0; i < kBufferSize; ++i) {
        bufferArr[i] = pBuffer[i];
      }
      camera.UnlockBuffer();

      led2.SetEnable(true);

      // 1d to 2d array
      bool image2d[kCameraHeight][kCameraWidth];
      for (Uint i = 0; i < kBufferSize; ++i) {
        std::string s = std::bitset<8>(bufferArr[i]).to_string();
        for (uint8_t j = 0; j < 8; ++j) {
          image2d[i * 8 / kCameraWidth][(i * 8 % kCameraWidth) + j] =
              (static_cast<Uint>(s.at(j) - '0') == 1);
        }
      }

      // apply median filter
      bool image2dMedian[kCameraHeight][kCameraWidth];
      for (Uint i = 0; i < kCameraHeight; ++i) {
        for (Uint j = 0; j < kCameraWidth; ++j) {
          if (i == 0 || i == kCameraHeight || j == 0 || j == kCameraWidth) {
            image2dMedian[i][j] = image2d[i][j];
          } else {
            image2dMedian[i][j] = static_cast<bool>((
                image2d[i - 1][j - 1] +
                    image2d[i - 1][j] +
                    image2d[i - 1][j + 1] +
                    image2d[i][j - 1] +
                    image2d[i][j] +
                    image2d[i][j + 1] +
                    image2d[i + 1][j - 1] +
                    image2d[i + 1][j] +
                    image2d[i + 1][j + 1]
            ) / 5);
          }
        }
      }

      // fill in row information to determine center point and area
      for (Uint i = 0; i < kCameraHeight; ++i) {
        rowInfo[i].pixel_num = static_cast<Uint>(
            std::count(image2dMedian[i], image2dMedian[i] + kCameraWidth, false));
        for (Uint j = (i == 0) ? kCameraWidth / 2 : rowInfo[i - 1].center_point; j < kCameraWidth; ++j) {
          if (image2dMedian[i][j]) {
            rowInfo[i].right_bound = j;
            break;
          }
          rowInfo[i].right_bound = (i == 0) ? kCameraWidth : (rowInfo[i - 1].right_bound + kCameraWidth) / 2;
        }
        for (Uint j = (i == 0) ? kCameraWidth / 2 : rowInfo[i - 1].center_point; j > 0; --j) {
          if (image2dMedian[i][j]) {
            rowInfo[i].left_bound = j;
            break;
          }
          rowInfo[i].left_bound = (i == 0) ? 0 : (rowInfo[i - 1].left_bound) / 2;
        }
        rowInfo[i].right_count = rowInfo[i].right_bound - kCameraWidth / 2;
        rowInfo[i].left_count = kCameraWidth / 2 - rowInfo[i].left_bound;
        rowInfo[i].center_point = (rowInfo[i].left_bound + rowInfo[i].right_bound) / 2;
      }

      led2.SetEnable(false);

      // render center line on lcd
      lcd.SetRegion(Lcd::Rect(0, 0, kCameraWidth, kCameraHeight));
      lcd.FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera.GetBufferSize() * 8);
      for (Uint i = 0; i < kCameraHeight; ++i) {
        if (!image2dMedian[i][rowInfo[i].center_point]) {
          lcd.SetRegion(Lcd::Rect(rowInfo[i].center_point, i, 1, 1));
          lcd.FillColor(Lcd::kGreen);
        }
      }

      // commit servo and motor changes
      uint16_t steervalue = 0;
      uint8_t count = 0;
      for (Uint i = 80; i > 40; --i) {
        if (!image2dMedian[i][rowInfo[i].center_point]) {
          steervalue += rowInfo[i].center_point;
          ++count;
        }
      }
      steervalue /= count;
      Uint prevServoDegree = servo.GetDegree();
      // use 1/3 data from previous run, 2/3 data from current calculations
      int servoTargetDegree = steervalue != 0 ?
                              (prevServoDegree + (kServoCenter - ((steervalue - 32) * 40)) * 2) / 3 :
                              prevServoDegree;
      if (servoTargetDegree > kServoLeftBound) {
        servoTargetDegree = kServoLeftBound;
        motorLeft.SetPower(kMotorSpeed::kMid);
      } else if (servoTargetDegree < kServoRightBound) {
        servoTargetDegree = kServoRightBound;
        motorLeft.SetPower(kMotorSpeed::kMid);
      } else if (servoTargetDegree < (kServoRightBound - kServoCenter) / 2) {
        motorLeft.SetPower(kMotorSpeed::kLow);
      } else if (servoTargetDegree > (kServoCenter - kServoLeftBound) / 2) {
        motorLeft.SetPower(kMotorSpeed::kLow);
      } else {
        motorLeft.SetPower(kMotorSpeed::kHigh);
      }

      servo.SetDegree(servoTargetDegree);
    }
  }

  return 0;
}
