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
void CameraTest();
/**
 * Tests the LCD of the mainboard
 *
 * Toggles the LCD between black and white, with the top and bottom portions of the
 * screen having alternate colors.
 *
 * @note LCD will be initialized within the function
 */
void LcdTest();
/**
 * Tests the LEDs of the mainboard
 *
 * Toggles the LEDs alternately.
 *
 * @note LEDs will be initialized within the function
 */
void LedTest();
/**
 * Tests the servo of the mainboard
 *
 * Toggles the servo between 45-degrees and 135-degrees under 2 second intervals.
 *
 * @note Servo will be initialized within the function.
 */
void ServoTest();
/**
 * Tests the alternate-motor of the mainboard
 *
 * Turns both alternate motors on at 10%.
 *
 * @note Motor will be initialized within the function.
 */
void AltMotorTest();
/**
 * Tests the direct-encoder of the mainboard
 *
 * Turns on both alternate motors on at 10%, and displays raw encoder values
 * every ~100ms.
 *
 * @note Motor, encoder and LCD will be initialized within the function.
 */
void DirEncoderTest();
/**
 * Tests the EncoderPController class
 *
 * Turns on both alternate motors, keep the motor running at 4500 val/sec,
 * and displays the encoder value (in val/sec) and motor power on the LCD.
 *
 * @note Motor, encoder, and LCD will be initialized within the function.
 */
void EncoderPControllerTest();
}  // namespace util
