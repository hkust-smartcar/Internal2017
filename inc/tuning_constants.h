/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#pragma once

#include <libbase/misc_utils.h>

// camera calibration constants
/**
 * Maximum camera row index that the camera will use to determine the center
 * line. Higher values imply rows closer to the camera.
 *
 * Value range: [0, kCameraHeight]
 */
constexpr Uint kCameraMaxSrcHeight = 30;
/**
 * Minimum number of non-blank rows to mark center line as valid. Higher values
 * imply higher accuracy and higher resistance to change.
 *
 * Value range: [0, 80-kCameraMaxSrcHeight]
 */
const Uint kCameraMinSrcConfidence = 15;
/**
 * Minimum number of white pixels on the same row to mark row as valid. Higher
 * values imply higher accuracy and higher resistance to change.
 */
constexpr Uint kCameraMinPixelCount = 10;

// servo calibration constants
/**
 * Maximum servo index for the left side.
 */
constexpr int kServoLeftBound = 1200;
/**
 * Maximum servo index for the right side.
 */
constexpr int kServoRightBound = 550;
/**
 * Servo sensitivity constants - Used to implement pseudo-dynamic servo
 * steering
 *
 * Current implementation uses these values to reduce zig-zag patterns on
 * straight lines, and increase steering sensitivity on corners.
 */
enum ServoSensitivity {
  kSensitivityLow = 5,
  kSensitivityHigh = 30,
};

// motor calibration constants
/**
 * Motor speed constants - Used to implement pseudo-dynamic motor speeds
 *
 * Current implementation uses these values to speed up during confident
 * straight lines, and slow down during corners to prevent understeering.
 */
enum MotorSpeed {
  kSpeedLow = 250,
  kSpeedMid = 275,
  kSpeedHigh = 300,
};

// LCD constants
constexpr bool kEnableLcd = false;

// fixed constants - do not edit
/**
 * Width of camera image. [1, 128]
 */
constexpr int kCameraWidth = 64;
/**
 * Height of camera image. [1, 160]
 */
constexpr int kCameraHeight = 80;
/**
 * Size of the camera image (in bytes).
 */
constexpr int kBufferSize = kCameraWidth * kCameraHeight / 8;
/**
 * Servo center constant
 *
 * Determines the center of the servo. Can be overriden to use a fixed value,
 * or compute using left and right bounds.
 */
constexpr int kServoCenter = 900;