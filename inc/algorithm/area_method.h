/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#pragma once

#include <libsc/k60/ov7725.h>
#include <libsc/st7735r.h>

/**
 * 2017 Smartcar Internal - Area Difference Method
 *
 * Computes the area difference between left and right sides, and displays it
 * to the LCD.
 *
 * @param camera Pointer to a camera object
 * @param lcd Pointer to a LCD object
 *
 * @note This function must be put in the main loop for correct execution.
 */
void areaMethod(libsc::k60::Ov7725 *camera, libsc::St7735r *lcd);
