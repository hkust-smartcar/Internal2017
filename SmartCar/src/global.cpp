/*
 * global.cpp
 *
 *  Created on: 2017Äê4ÔÂ22ÈÕ
 *      Author: Lee Chun Hei
 */

#include "../inc/global.h"

//Global Variables--------------------------------------------------------------------------------------------------------------------------------------------
const Byte* camBuffer=0;
int leftEdge[camHeight]={};
int rightEdge[camHeight]={};
int midPt[camHeight]={};
int midPtFound=0;
int midAngle=800;
int leftAngle=1050;
int rightAngle=450;
bool isRoundabout=false;
int pathError=0;
car car1;
car car2;

//Pointers----------------------------------------------------------------------------------------------------------------------------------------------------
Led* led1=0;
Led* led2;
Led* led3;
Led* led4;
Joystick* joystick;
Lcd* lcd;
LcdConsole* console;
Ov7725* cam;
//FutabaS3010* servo;
AlternateMotor* motor1;
DirEncoder* encoder1;
AlternateMotor* motor2;
DirEncoder* encoder2;
BatteryMeter* batteryMeter;
BTComm* bluetooth;

void carInit(car carUsing){
	leftAngle=carUsing.leftAngle;
	midAngle=carUsing.midAngle;
	rightAngle=carUsing.rightAngle;
}
