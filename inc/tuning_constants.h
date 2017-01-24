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
const Uint kCameraMaxSrcHeight = 40;
/**
 * Minimum number of non-blank rows to mark center line as valid. Higher values
 * imply higher accuracy and higher resistance to change.
 *
 * Value range: [0, kCameraMaxSrcHeight]
 */
const Uint kCameraMinSrcConfidence = 4;

// servo calibration constants
const Uint kServoLeftBound = 1275;  // Max servo index for left side
const Uint kServoRightBound = 550;  // Max servo index for right side
/**
 * Servo sensitivity constants - Used to implement pseudo-dynamic servo
 * steering
 *
 * Current implementation uses these values to reduce zig-zag patterns on
 * straight lines, and increase steering sensitivity on corners.
 */
enum ServoSensitivity {
  kSensitivityLow = 15,
  kSensitivityMid = 40,
  kSensitivityHigh = 50,
  kSensitivityCustom = 60,
};

// motor calibration constants
const Uint kMotorPowerMultiplier = 150;
/**
 * Motor speed constants - Used to implement pseudo-dynamic motor speeds
 *
 * Current implementation uses these values to speed up during confident
 * straight lines, and slow down during corners to prevent understeering.
 */
enum MotorSpeed {
  kSpeedLow = 225,
  kSpeedMid = 260,
  kSpeedHigh = 325,
};

// fixed constants - do not edit
const Uint kCameraWidth = 64;
const Uint kCameraHeight = 80;
/**
 * Servo center constant
 *
 * Determines the center of the servo. Can be overriden to use a fixed value,
 * or compute using left and right bounds.
 */
const Uint kServoCenter = (kServoLeftBound + kServoRightBound) / 2;
