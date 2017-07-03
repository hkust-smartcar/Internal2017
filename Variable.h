/*
 * Variable.h
 *
 *  Created on: Jul 3, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_BA_VARIABLE_H_
#define SRC_BA_VARIABLE_H_

#include "CameraHeader/FindEdge.h"
#include <libsc/led.h>

#include <libsc/k60/ov7725.h>

#include <libsc/alternate_motor.h>

#include <libsc/dir_encoder.h>

#include <libsc/button.h>

#include <libsc/joystick.h>

#include <libbase/k60/flash.h>

#include <libbase/k60/pit.h>

#include <libsc/mpu6050.h>

using namespace libsc;
using namespace libbase::k60;
using namespace libsc::k60;




//balance data
const int globalWidth = 80;
const int globalHeight = 60;

Mpu6050			* MpuPtr;
AlternateMotor 	* LeftMotorPtr;
AlternateMotor 	* RightMotorPtr;
DirEncoder 		* LeftEncoderPtr;
DirEncoder 		* RightEncoderPtr;
Led				* Led1Ptr;
Led				* Led2Ptr;
Flash			* FlashPtr;
Pit				* Pit1Ptr;
Pit				* Pit2Ptr;
Ov7725          * CameraPtr;
Joystick		* JoystickPtr;
Button			* Button1Ptr;
Button			* Button2Ptr;




float balAngle = 77;//70
double inputTargetSpeed = 0;
double leftPowSpeedP = 0, leftPowSpeedI = 0, leftPowSpeedD = 0;
double rightPowSpeedP = 0, rightPowSpeedI = 0, rightPowSpeedD = 0;
double speedAngP = 0, speedAngI = 0, speedAngD = 0;
float targetAngSpeedP = 0, targetAngSpeedI = 0, targetAngSpeedD = 0;
double targetAngLim = 0, targetSpeedAngK = 0;
float diffP = 0, diffD = 0;
double sumSpeedErrLim = 6000, sumAngErrLim = 100000;

double targetAng = balAngle;
double sumSpeedErr = 0, sumAngErr = 0, sumLeftSpeedErr = 0, sumRightSpeedErr = 0;
float targetSpeed = 0, differential = 0, curDiff = 0, prevDiff = 0;
bool programRun = true, camEnable = true, tuneBal = false, tuneSpeed = false;

long startTime = 0;
long prevSampleTime = 0;
long camTime = 0;
double dt = 0;
long loopCounter = 0;

//Camera
const Byte * camInput;
int height = globalHeight, width = globalWidth;

//Speed
int leftPow = 0, rightPow = 0;
bool leftForward = 1, rightForward = 0;

const int speedArrSize = 10;
double curSpeed = 0, prevSpeed = 0;
double leftSpeed = 0, rightSpeed = 0;
double leftTempTargetSpeed = 0, rightTempTargetSpeed = 0, tempTargetSpeed = 0;
int leftSpeedArrCounter = 0, rightSpeedArrCounter = 0;
double leftSpeedTotal = 0, rightSpeedTotal = 0;
double leftSpeedArr[speedArrSize], rightSpeedArr[speedArrSize];


const int speedErrRateArrSize = 5;
double leftSpeedErr = 0, rightSpeedErr = 0, curSpeedErr = 0;
double prevLeftSpeedErr = 0, prevRightSpeedErr = 0, prevSpeedErr = 0;
double leftSpeedErrRate = 0, rightSpeedErrRate = 0, speedErrRate = 0;
int leftSpeedErrRateCounter = 0, rightSpeedErrRateCounter = 0, speedErrRateCounter = 0;
double leftSpeedErrRateTotal = 0, rightSpeedErrRateTotal = 0, speedErrRateTotal = 0;
double leftSpeedErrRateArr[speedErrRateArrSize], rightSpeedErrRateArr[speedErrRateArrSize], speedErrRateArr[speedErrRateArrSize];


//Angle
double acc[3];
double accbine = 0;
double accAng = 0, gyroAng = 0;

double curAng = 0, prevAng = 0;
double tempTargetAng = 0;
int angCounter = 0;
const int angArrSize = 10;
double angTotal = balAngle*angArrSize;
double angArr[angArrSize];


double curAngErr = 0, prevAngErr = 0, angErrRate = 0;
int angErrRateCounter = 0;
const int angErrRateArrSize = 5;
double angErrRateArr[angErrRateArrSize];
double angErrRateTotal = 0;

double temp = 0;
double temp1 = 0;
float preDifferential = 0;

int count = 0;




Point LeftEdge[150];
Point MidPoint[150];
Point ModifiedMidPoint[150];
Point RightEdge[150];

uint8_t LeftEdgeNum;
uint8_t RightEdgeNum;
uint8_t MidPointNum;
uint8_t ModifiedMidPointNum;

uint8_t LeftCornerOrder;
uint8_t RightCornerOrder;
Point* LeftCorner[2] = {NULL};
Point* RightCorner[2] = {NULL};

TrackState Trackstate;

bool car_run = true;
bool show_camera_image = true;
bool show_data1 = false;
bool show_data2 = false;

const int FLASHNUM = 13;

float* FlashArray[FLASHNUM] = {
		&angDiff,
		&diffP,
		&diffD,
		&DiffMax,
		&bottomLine_k,
		&middleLine_k,
		&topLine_k,
		&r_bottomLine_k,
		&r_middleLine_k,
		&r_topLine_k,
		&r_bottomLine_dis,
		&r_middleLine_dis,
		&r_topLine_dis
};


float FLASHARRAY[FLASHNUM] = {
		angDiff,
		diffP,
		diffD,
		DiffMax,
		bottomLine_k,
		middleLine_k,
		topLine_k,
		r_bottomLine_k,
		r_middleLine_k,
		r_topLine_k,
		r_bottomLine_dis,
		r_middleLine_dis,
		r_topLine_dis
};

std::pair<float,char*>data[FLASHNUM] ={
		{FLASHARRAY[0],"ANG"},
		{FLASHARRAY[1],"difP"},
		{FLASHARRAY[2],"difD"},
		{FLASHARRAY[3],"DMX"},
		{FLASHARRAY[4],"b_k"},
		{FLASHARRAY[5],"m_k"},
		{FLASHARRAY[6],"t_k"},
		{FLASHARRAY[7],"r_bk"},
		{FLASHARRAY[8],"r_mk"},
		{FLASHARRAY[9],"r_tk"},
		{FLASHARRAY[10],"r_bD"},
		{FLASHARRAY[11],"r_mD"},
		{FLASHARRAY[12],"r_tD"},
};

int8_t dataIndex = 0;

bool pitState = true;



#endif /* SRC_BALANCEFILE_VARIABLE_H_ */
