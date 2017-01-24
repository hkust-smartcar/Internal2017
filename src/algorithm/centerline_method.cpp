/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../../inc/algorithm/centerline_method.h"

#include <libsc/alternate_motor.h>
#include <libsc/futaba_s3010.h>
#include <libsc/lcd.h>
#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>
#include <libsc/tower_pro_mg995.h>
#include <libsc/k60/ov7725.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>

#include "../../inc/tuning_constants.h"

struct {
  Uint pixel_num;
  Uint left_bound;
  Uint right_bound;
  Uint left_count;
  Uint right_count;
  Uint center_point;
} rowInfo[kCameraHeight];

using libsc::AlternateMotor;
using libsc::FutabaS3010;
using libsc::Lcd;
using libsc::Led;
using libsc::St7735r;
using libsc::System;
using libsc::Timer;
using libsc::TowerProMg995;
using libsc::k60::Ov7725;

/**
 * 2017 Smartcar Internal - Center Line Method
 *
 * Computes the center line from the image, then uses the values to commit the
 * values to motor and servo.
 *
 * @note Used in the smartcar configuration 2016_INNO during the internal
 * competition.
 * @note All smartcar components are initialized in this method.
 */
void centerLineMethod() {
  // initialize LEDs
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
  lcdConfig.fps = 50;
  lcdConfig.is_revert = true;
  St7735r lcd(lcdConfig);
  lcd.Clear();

  // initialize servo
  TowerProMg995::Config servoConfig;
  servoConfig.id = 0;
  TowerProMg995 servo(servoConfig);
  servo.SetDegree(kServoCenter);

  // initialize motors
  AlternateMotor::Config motorConfig;
  motorConfig.multiplier = kMotorPowerMultiplier;
  motorConfig.id = 0;
  AlternateMotor motor(motorConfig);
  motor.SetPower(0);
  motor.SetClockwise(false);

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
                    image2d[i + 1][j + 1])
                / 5);
          }
        }
      }

      // fill in row information to determine center point and area
      Uint totalLeftCount = 0, totalRightCount = 0;
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
        totalLeftCount += rowInfo[i].left_count;
        totalRightCount += rowInfo[i].right_count;
      }

      led2.SetEnable(false);

      // render center line on lcd
      lcd.SetRegion(Lcd::Rect(0, 0, kCameraWidth, kCameraHeight));
      lcd.FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera.GetBufferSize() * 8);
      for (Uint i = 80; i > kCameraMaxSrcHeight; --i) {
        if (!image2dMedian[i - 1][rowInfo[i - 1].center_point]) {
          lcd.SetRegion(Lcd::Rect(rowInfo[i - 1].center_point, i - 1, 1, 1));
          lcd.FillColor(Lcd::kGreen);
        }
      }

      // average the center points to construct an average direction to move in
      uint16_t steervalue = 0;
      uint8_t validCenterPointCount = 0;
      for (Uint i = 80; i > kCameraMaxSrcHeight; --i) {
        if (!image2dMedian[i - 1][rowInfo[i - 1].center_point]) {
          steervalue += rowInfo[i - 1].center_point;
          ++validCenterPointCount;
        }
      }
      steervalue /= validCenterPointCount;

      Uint prevServoDegree = servo.GetDegree();
      // use 1/3 data from previous computation cycle, 2/3 data from current computations
      Uint servoTargetDegree = steervalue != 0 ?
                               kServoCenter - ((steervalue - 32) * ServoSensitivity::kSensitivityCustom) :
                               prevServoDegree;

      // change motor speed according to servo speed
      if (servoTargetDegree > kServoLeftBound) {
        // value > left bound: higher speed for compensation
        servoTargetDegree = kServoLeftBound;
        motor.SetPower(MotorSpeed::kSpeedMid);
      } else if (servoTargetDegree < kServoRightBound) {
        // value < right bound: higher speed for compensation
        servoTargetDegree = kServoRightBound;
        motor.SetPower(MotorSpeed::kSpeedMid);
      } else if (servoTargetDegree < (kServoRightBound - kServoCenter) / 2) {
        // value < half of right steer: slower speed to decrease turning radius
        motor.SetPower(MotorSpeed::kSpeedLow);
      } else if (servoTargetDegree > (kServoCenter - kServoLeftBound) / 2) {
        // value > half of left steer: slower speed to decrease turning radius
        motor.SetPower(MotorSpeed::kSpeedLow);
      } else {
        motor.SetPower(MotorSpeed::kSpeedHigh);
      }

      servo.SetDegree(servoTargetDegree);
    }
  }
}

/**
 * 2017 Smartcar Internal - Center Line Method (Staging)
 *
 * Computes the center line from the image, then uses the values to commit the
 * values to motor and servo.
 *
 * @note Used in the smartcar configuration 2017_INNO during internal
 * competition
 * @note All smartcar components are initialized in this method.
 */
void centerLineMethodStaging() {
  // initialize LEDs
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
  lcdConfig.fps = 50;
  lcdConfig.is_revert = true;
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
  motorRight.SetPower(0);
  motorLeft.SetClockwise(false);
  motorRight.SetClockwise(false);

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
                    image2d[i + 1][j + 1])
                / 5);
          }
        }
      }

      // fill in row information to determine center point and area
      Uint totalLeftCount = 0, totalRightCount = 0;
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
        totalLeftCount += rowInfo[i].left_count;
        totalRightCount += rowInfo[i].right_count;
      }

      led2.SetEnable(false);

      // render center line on lcd
      lcd.SetRegion(Lcd::Rect(0, 0, kCameraWidth, kCameraHeight));
      lcd.FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera.GetBufferSize() * 8);
      for (Uint i = 80; i > kCameraMaxSrcHeight; --i) {
        if (rowInfo[i - 1].pixel_num == 0) {
          break;
        }
        if (!image2dMedian[i - 1][rowInfo[i - 1].center_point]) {
          lcd.SetRegion(Lcd::Rect(rowInfo[i - 1].center_point, i - 1, 1, 1));
          lcd.FillColor(Lcd::kGreen);
        }
      }

      // average the center points to construct an average direction to move in
      uint16_t steervalue = 0;
      uint8_t validCenterPointCount = 0;
      for (Uint i = 80; i > kCameraMaxSrcHeight; --i) {
        if (rowInfo[i - 1].pixel_num == 0) {
          // leave the loop if row is not part of the track anymore
          break;
        }
        if (!image2dMedian[i - 1][rowInfo[i - 1].center_point]) {
          steervalue += rowInfo[i - 1].center_point;
          ++validCenterPointCount;
        }
      }
      steervalue /= validCenterPointCount;

      Uint prevServoDegree = servo.GetDegree();
      Uint servoTargetDegree;
      if (steervalue == 0) {
        // no valid data - use previous data
        servoTargetDegree = prevServoDegree;
      } else if (validCenterPointCount < kCameraMinSrcConfidence) {
        // insufficient data - use previous data
        servoTargetDegree = prevServoDegree;
      } else if (steervalue < kCameraWidth / 5 || steervalue > kCameraWidth * 4 / 5) {
        // edge 40% for either side - steep steering
        servoTargetDegree = kServoCenter - ((steervalue - kCameraWidth / 2) * ServoSensitivity::kSensitivityHigh);
      } else if (steervalue < kCameraWidth * 2 / 5 || steervalue > kCameraWidth * 3 / 5) {
        // center 40% for either side - medium steering
        servoTargetDegree = kServoCenter - ((steervalue - kCameraWidth / 2) * ServoSensitivity::kSensitivityMid);
      } else {
        // middle 20% for either side - adjustment
        servoTargetDegree = kServoCenter - ((steervalue - kCameraWidth / 2) * ServoSensitivity::kSensitivityLow);
      }

      // commit motor and servo changes
      if (servoTargetDegree > kServoLeftBound) {
        // value > left bound: higher speed for compensation
        servoTargetDegree = kServoLeftBound;
        motorLeft.SetPower(MotorSpeed::kSpeedMid);
        motorRight.SetPower(MotorSpeed::kSpeedMid);
      } else if (servoTargetDegree < kServoRightBound) {
        // value < right bound: higher speed for compensation
        servoTargetDegree = kServoRightBound;
        motorLeft.SetPower(MotorSpeed::kSpeedMid);
        motorRight.SetPower(MotorSpeed::kSpeedMid);
      } else if (servoTargetDegree < (kServoRightBound - kServoCenter) / 2) {
        // value < half of right steer: slower speed to decrease turning radius
        motorLeft.SetPower(MotorSpeed::kSpeedLow);
        motorRight.SetPower(MotorSpeed::kSpeedLow);
      } else if (servoTargetDegree > (kServoCenter - kServoLeftBound) / 2) {
        // value > half of left steer: slower speed to decrease turning radius
        motorLeft.SetPower(MotorSpeed::kSpeedLow);
        motorRight.SetPower(MotorSpeed::kSpeedLow);
      } else {
        motorLeft.SetPower(MotorSpeed::kSpeedHigh);
        motorRight.SetPower(MotorSpeed::kSpeedHigh);
      }

      servo.SetDegree(servoTargetDegree);
    }
  }
}
