/*
 * global.h
 *
 *  Created on: 2017Äê4ÔÂ22ÈÕ
 *      Author: Lee Chun Hei
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

//#include <functional>

//ir_ultrasound_modules Header File--------------------------------------------------------------------------------------
#include <inc/ir_ultrasound.h>

//Looper Header File----------------------------------------------------------------------------------------------
#include <libutil/looper.h>

//Button Header File--------------------------------------------------------------------------------------------------------
#include<libsc/button.h>

//Joy Stick Header File-----------------------------------------------------------------------------------------------------
#include<libsc/joystick.h>

//led Header File-----------------------------------------------------------------------------------------------------------
#include <libsc/led.h>

//LCD Header File-----------------------------------------------------------------------------------------------------------
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>

//Camera Header File--------------------------------------------------------------------------------------------------------
#include<libsc/k60/ov7725.h>

//Servo Header File---------------------------------------------------------------------------------------------------------
#include<libsc/futaba_s3010.h>

//Motor Header File---------------------------------------------------------------------------------------------------------
#include<libsc/alternate_motor.h>
//#include<libsc/dir_motor.h>
#include<libsc/motor.h>

//Dir Encoder Header File
#include<libsc/dir_encoder.h>

#include "libsc/battery_meter.h"

#include <libsc/k60/jy_mcu_bt_106.h>
#include "../inc/bluetooth.h"

#include "../inc/util.h"

#include "../inc/camera.h"

#include "../inc/car.h"

//namespace-----------------------------------------------------------------------------------------------------------------
using namespace libsc;
using namespace libsc::k60;
using namespace libbase::k60;
using namespace libutil;
using namespace std;

struct car{
	int midAngle=800;
	int leftAngle=1050;
	int rightAngle=450;
};

//Global variable-----------------------------------------------------------------------------------------------------------
const int camHeight=60;
const int rawCamHeight=480;
const int camWidth=80;
const int rawCamWidth=128;
extern const Byte* camBuffer;
extern int leftEdge[camHeight];
extern int rightEdge[camHeight];
extern int midPt[camHeight];
extern int midPtFound;
extern int midAngle;
extern int leftAngle;
extern int rightAngle;
extern bool isRoundabout;
extern int pathError;
extern car car1;
extern car car2;

//Global Pointers------------------------------------------------------------------------------------------------------------------------------------
extern Led* led1;
extern Led* led2;
extern Led* led3;
extern Led* led4;
extern Joystick* joystick;
extern Lcd* lcd;
extern LcdConsole* console;
extern Ov7725* cam;
//extern FutabaS3010* servo;
extern AlternateMotor* motor1;
extern DirEncoder* encoder1;
extern AlternateMotor* motor2;
extern DirEncoder* encoder2;
extern BatteryMeter* batteryMeter;
extern BTComm* bluetooth;

void carInit(car carUsing);

#endif /* INC_GLOBAL_H_ */
