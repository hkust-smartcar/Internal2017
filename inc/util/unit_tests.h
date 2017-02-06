/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

namespace util {

/**
 * Tests the camera of the mainboard
 *
 * Outputs the camera image onto the LCD. Requires a working St7735r panel.
 *
 * @note Camera and LCD will be initialized within the function.
 */
void cameraTest();
/**
 * Tests the LCD of the mainboard
 *
 * Toggles the LCD between black and white, with the top and bottom portions of the
 * screen having alternate colors.
 *
 * @note LCD will be initialized within the function
 */
void lcdTest();
/**
 * Tests the LEDs of the mainboard
 *
 * Toggles the LEDs alternately.
 *
 * @note LEDs will be initialized within the function
 */
void ledTest();
/**
 * Tests the servo of the mainboard
 *
 * Toggles the servo between 45-degrees and 135-degrees under 2 second intervals.
 *
 * @note Servo will be initialized within the function.
 */
void servoTest();
/**
 * Tests the alternate-motor of the mainboard
 *
 * Turns both alternate motors on at 10%.
 *
 * @note Motor will be initialized within the function.
 */
void altMotorTest();

}  // namespace util
