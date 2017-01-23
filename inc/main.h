/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#pragma once

#include <libbase/misc_utils.h>

const Uint kCameraWidth = 64;
const Uint kCameraHeight = 80;

const Uint kServoLeftBound = 1200;
const Uint kServoRightBound = 500;
const Uint kServoCenter = (kServoLeftBound + kServoRightBound) / 2;

const Uint kMotorPowerMultiplier = 100;
enum kMotorSpeed {
  kLow = 200,
  kMid = 275,
  kHigh = 350,
};
