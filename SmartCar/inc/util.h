/*
 * util.h
 *
 *  Created on: 2017��4��22��
 *      Author: Lee Chun Hei
 */

#ifndef INC_UTIL_H_
#define INC_UTIL_H_

#include "../inc/global.h"

namespace util{

bool batteryCheck();
void batteryTest();
void servoTuning();
void consoleWriteValue(float value);
void consoleWriteValue(int value);

}

#endif /* INC_UTIL_H_ */
