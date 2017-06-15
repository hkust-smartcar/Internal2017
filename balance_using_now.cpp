/*
 * main.cpp
 *
 * Author: Peter
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libbase/k60/pit.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <list>
#include <array>
#include <sstream>
#include <cstring>
#include <stdio.h>

#include <CameraHeader/FindEdge.h>

//LED
#include <libsc/led.h>

//Button
//#include <libsc/button.h>

//LCD
//#include <libsc/st7735r.h>

//Camera
#include <libsc/k60/ov7725.h>

//Motor
#include <libsc/alternate_motor.h>

//Encoder
#include <libsc/dir_encoder.h>

//Servo
#include <libsc/futaba_s3010.h>

//Bluetooth
#include <libsc/k60/jy_mcu_bt_106.h>

//Accel and Gyro
#include <libsc/mpu6050.h>

namespace libbase
{
	namespace k60
	{

		Mcg::Config Mcg::GetMcgConfig()
		{
			Mcg::Config config;
			config.external_oscillator_khz = 50000;
			config.core_clock_khz = 150000;
			return config;
		}

	}
}

using namespace std;
using namespace libsc;
using namespace libbase::k60;
using namespace libsc::k60;

//program...

const int globalWidth = 80;
const int globalHeight = 60;

//Object pointer
AlternateMotor * leftMotorPtr;
AlternateMotor * rightMotorPtr;
DirEncoder * encoderLeftPtr;
DirEncoder * encoderRightPtr;
JyMcuBt106 * btPtr;
Mpu6050 * mpuPtr;


//BT listener
string inputStr;
bool tune = false;
vector<double> constVector;

double arrAvg(double arr[], int size, int& counter, double& total, double newVal) {

	total -= arr[counter];
	arr[counter] = newVal;
	total += arr[counter];
	counter++;
	if (counter == size) {
		counter = 0;
	}
	return round((total/size)*100)/100.0;
}

//Forrest variable
Point LeftEdge[150];
uint8_t LeftEdgeNum;
uint8_t RightEdgeNum;
uint8_t MidPointNum;

uint8_t LeftCornerOrder;
uint8_t RightCornerOrder;
Point RightEdge[150];

Point* LeftCorner[2] = {NULL};
Point* RightCorner[2] = {NULL};
Point ModLeftEdge[150];
Point ModRightEdge[150];
Point MidPoint[150];


Led* led;


double balAngle = 78;
double inputTargetSpeed = 0;
double leftPowSpeedP = 0, leftPowSpeedI = 0, leftPowSpeedD = 0;
double rightPowSpeedP = 0, rightPowSpeedI = 0, rightPowSpeedD = 0;
double speedAngP = 0, speedAngI = 0, speedAngD = 0;
double targetAngSpeedP = 0, targetAngSpeedI = 0, targetAngSpeedD = 0;
double targetAngLim = 0, targetSpeedAngK = 0;
double diffP = 0, diffD = 0;
double sumSpeedErrLim = 10000, sumAngErrLim = 100000;

double targetAng = balAngle;
double sumSpeedErr = 0, sumAngErr = 0, sumLeftSpeedErr = 0, sumRightSpeedErr = 0;
double targetSpeed = 0, differential = 0, curDiff = 0, prevDiff = 0;
bool programRun = true, camEnable = true, tuneBal = false, tuneSpeed = false;

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
	double angArr[angArrSize] = {balAngle,balAngle,balAngle,balAngle,balAngle,balAngle,balAngle,balAngle,balAngle,balAngle};

	double curAngErr = 0, prevAngErr = 0, angErrRate = 0;
	int angErrRateCounter = 0;
	const int angErrRateArrSize = 5;
	double angErrRateArr[angErrRateArrSize];
	double angErrRateTotal = 0;

	float dt;

	int32_t loopCounter = 0;
void PitInterrupt(Pit* pit){
	loopCounter ++;
	led->Switch();
	//Time interval
	dt = 0.005;

	float temp;

	//speed
	encoderLeftPtr->Update();
	temp = encoderLeftPtr->GetCount()/dt;
	if (temp>50000 || temp<-50000) {
		temp = leftSpeed;
	}
	leftSpeed = arrAvg(leftSpeedArr, speedArrSize, leftSpeedArrCounter, leftSpeedTotal, temp);
	encoderRightPtr->Update();
	temp = -encoderRightPtr->GetCount()/dt;
	if (temp>50000 || temp<-50000) {
		temp = rightSpeed;
	}
	rightSpeed = arrAvg(rightSpeedArr, speedArrSize, rightSpeedArrCounter, rightSpeedTotal, temp);
	prevSpeed = curSpeed;
	curSpeed = (leftSpeed+rightSpeed)/2;

	//angle
	mpuPtr->Update(1);
	acc[0] = mpuPtr->GetAccel()[0];
	acc[1] =mpuPtr->GetAccel()[1];
	acc[2] =mpuPtr->GetAccel()[2];
	accbine = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
	accAng = (acos(acc[0]/accbine)*180)/3.1415;
	gyroAng = curAng + (mpuPtr->GetOmega()[1]/160/131)*dt;
	prevAng = curAng;
	curAng = arrAvg(angArr, angArrSize, angCounter, angTotal, abs((0.98*gyroAng)+(0.02*accAng)));
//			if (System::Time() >= 5000 && (curAng >= balAngle+20 || curAng <= balAngle-10)) {
//				break;
//			}

//	if (programRun == false || System::Time()<3000) {
//		leftMotorPtr->SetPower(0);
//		rightMotorPtr->SetPower(0);
////		continue;
//	}
	if (System::Time()<3000) {
		targetAngLim = 0;
	}

	targetSpeed = targetSpeedAngK * (targetAng-balAngle);

	//targetAng-speedDiff PID
	prevSpeedErr = curSpeedErr;
	curSpeedErr = curSpeed - targetSpeed;
	sumSpeedErr += curSpeedErr * dt;
	if (sumSpeedErr > sumSpeedErrLim) {
		sumSpeedErr = sumSpeedErrLim;
	} else if (sumSpeedErr < -1*sumSpeedErrLim) {
		sumSpeedErr = -1*sumSpeedErrLim;
	}
	speedErrRate = arrAvg(speedErrRateArr, speedErrRateArrSize, speedErrRateCounter, speedErrRateTotal, (curSpeedErr - prevSpeedErr) / dt);
	tempTargetAng = targetAng + targetAngSpeedP * curSpeedErr + targetAngSpeedI * sumSpeedErr + targetAngSpeedD * speedErrRate;
	if (tempTargetAng > targetAng+targetAngLim) {
		tempTargetAng = targetAng+targetAngLim;
	} else if (tempTargetAng < targetAng-targetAngLim) {
		tempTargetAng = targetAng-targetAngLim;
	}

	//tune balance
	if (tuneBal) {
		tempTargetAng = balAngle;
	}

//			tempTargetAng = targetAng;

	//tempSpeed-angle PID
	prevAngErr = curAngErr;
	curAngErr = curAng - tempTargetAng;
	sumAngErr += curAngErr * dt;
	if (sumAngErr > sumAngErrLim) {
		sumAngErr = sumAngErrLim;
	} else if (sumAngErr < -1*sumAngErrLim) {
		sumAngErr = -1*sumAngErrLim;
	}
	angErrRate = arrAvg(angErrRateArr, angErrRateArrSize, angErrRateCounter, angErrRateTotal, (curAngErr-prevAngErr) / dt);
//			angErrRate = (curAngErr-prevAngErr) / dt;
	tempTargetSpeed = curSpeed + speedAngP * curAngErr + speedAngI * sumAngErr + speedAngD * angErrRate;
	leftTempTargetSpeed = tempTargetSpeed * (1+differential);
	rightTempTargetSpeed = tempTargetSpeed * (1-differential);

	//tune speed
	if (tuneSpeed) {
		leftTempTargetSpeed = inputTargetSpeed;
		rightTempTargetSpeed = inputTargetSpeed;
	}

	//power-speed PID
	prevLeftSpeedErr = leftSpeedErr;
	leftSpeedErr = leftSpeed - leftTempTargetSpeed;
	sumLeftSpeedErr += leftSpeedErr * dt;
	if (sumLeftSpeedErr > sumSpeedErrLim) {
		sumLeftSpeedErr = sumSpeedErrLim;
	} else if (sumLeftSpeedErr < -1*sumSpeedErrLim) {
		sumLeftSpeedErr = -1*sumSpeedErrLim;
	}
	leftSpeedErrRate = arrAvg(leftSpeedErrRateArr, speedErrRateArrSize, leftSpeedErrRateCounter, leftSpeedErrRateTotal, (leftSpeedErr-prevLeftSpeedErr) / dt);
//			leftSpeedErrRate = (leftSpeedErr-prevLeftSpeedErr) / dt;
	leftPow += (int)(leftPowSpeedP * leftSpeedErr + leftPowSpeedI * sumLeftSpeedErr + leftPowSpeedD * leftSpeedErrRate);

	prevRightSpeedErr = rightSpeedErr;
	rightSpeedErr = rightSpeed - rightTempTargetSpeed;
	sumRightSpeedErr += rightSpeedErr * dt;
	if (sumRightSpeedErr > sumSpeedErrLim) {
		sumRightSpeedErr = sumSpeedErrLim;
	} else if (sumRightSpeedErr < -1*sumSpeedErrLim) {
		sumRightSpeedErr = -1*sumSpeedErrLim;
	}
	rightSpeedErrRate = arrAvg(rightSpeedErrRateArr, speedErrRateArrSize, rightSpeedErrRateCounter, rightSpeedErrRateTotal, (rightSpeedErr-prevRightSpeedErr) / dt);
//			rightSpeedErrRate = (rightSpeedErr-prevRightSpeedErr) / dt;
	rightPow += (int)(rightPowSpeedP * rightSpeedErr + rightPowSpeedI * sumRightSpeedErr + rightPowSpeedD * rightSpeedErrRate);

	//power limit
	if (leftPow > 500) {
		leftPow = 500;
	} else if (leftPow < -500) {
		leftPow = -500;
	}
	if (rightPow > 500) {
		rightPow = 500;
	} else if (rightPow < -500) {
		rightPow = -500;
	}

	//set power
	if (leftPow > 0) {
		leftMotorPtr->SetClockwise(!leftForward);
		leftMotorPtr->SetPower(leftPow);
	} else {
		leftMotorPtr->SetClockwise(leftForward);
		leftMotorPtr->SetPower(-1*leftPow);
	}
	if (rightPow > 0) {
		rightMotorPtr->SetClockwise(!rightForward);
		rightMotorPtr->SetPower(rightPow);
	} else {
		rightMotorPtr->SetClockwise(rightForward);
		rightMotorPtr->SetPower(-1*rightPow);
	}

	//send data
	if(loopCounter == 100)  loopCounter =0;
	if (loopCounter%10 == 0) {
		char speedChar[15] = {};
//		sprintf(speedChar, "%.1f,%.3f,%.4f,%.4f,%.3f,%.4f,%.4f\n", 1.0, leftPowSpeedP, leftPowSpeedI, leftPowSpeedD, rightPowSpeedP, rightPowSpeedI, rightPowSpeedD);
//		sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, targetAng, tempTargetAng);
//		sprintf(speedChar, "%.1f,%.2f\n", 1.0, encoderRightPtr->GetCount()/dt);
//		sprintf(speedChar, "%.1f,%.2f\n", 1.0, encoderLeftPtr->GetCount()/dt);
		sprintf(speedChar, "%.1f,%.2f\n", 1.0, curSpeed);
//		sprintf(speedChar, "%.1f,%.3f\n", 1.0, dt);
		string speedStr = speedChar;

		const Byte speedByte = 85;
		btPtr->SendBuffer(&speedByte, 1);
		btPtr->SendStr(speedStr);
	}
}



int main(void) {

	System::Init();

	Led::Config LedConfig;
	LedConfig.id = 0;
	Led led1(LedConfig);
	led = &led1;

//	St7735r::Config LcdConfig;
//	LcdConfig.is_revert = true;
//	St7735r Lcd1(LcdConfig);

//	FutabaS3010::Config ServoConfig;
//	ServoConfig.id = 0;
//	FutabaS3010 servo1(ServoConfig);
//	servoPtr = &servo1;
	long startTime = System::Time();
	long currentTime = System::Time();
//
	Ov7725::Config CamConfig;
	CamConfig.id = 0;
	CamConfig.w = width;
	CamConfig.h = height;
	CamConfig.fps = Ov7725Configurator::Config::Fps(2);
	Ov7725 Cam1(CamConfig);

	JyMcuBt106::Config BluetoothConfig;
	BluetoothConfig.id = 0;
	BluetoothConfig.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
//	BluetoothConfig.rx_isr = &bluetoothListener;
	JyMcuBt106 bluetooth1(BluetoothConfig);
	btPtr = &bluetooth1;
//	b = btPtr;

	Mpu6050::Config mpuConfig;
	mpuConfig.gyro_range=Mpu6050::Config::Range::kSmall;
	mpuConfig.accel_range=Mpu6050::Config::Range::kSmall;
	Mpu6050 mpu(mpuConfig);
	mpuPtr = &mpu;

	AlternateMotor::Config LeftMotorConfig;
	LeftMotorConfig.id = 1;
	AlternateMotor motorLeft(LeftMotorConfig);
	leftMotorPtr = &motorLeft;
	AlternateMotor::Config RightMotorConfig;
	RightMotorConfig.id = 0;
	AlternateMotor motorRight(RightMotorConfig);
	rightMotorPtr = &motorRight;

	DirEncoder::Config LeftEncoderConfig;
	LeftEncoderConfig.id = 0;
	DirEncoder encoderLeft(LeftEncoderConfig);
	encoderLeftPtr = &encoderLeft;
	DirEncoder::Config RightEncoderConfig;
	RightEncoderConfig.id = 1;
	DirEncoder encoderRight(RightEncoderConfig);
	encoderRightPtr = &encoderRight;

	motorLeft.SetClockwise(leftForward);
	motorRight.SetClockwise(rightForward);
	motorLeft.SetPower(0);
	motorRight.SetPower(0);

	Pit::Config pit_config;
	pit_config.channel = 0;
	pit_config.count   = 187500;
	pit_config.is_enable = true;
	pit_config.isr = &PitInterrupt;

	Pit  pit(pit_config);


	Cam1.Start();

//	System::DelayMs(3000);	//Constant
	targetAng = 68;	//Angle to run
	balAngle = 72;
	inputTargetSpeed = 8000;
	leftPowSpeedP = 0.002;
	leftPowSpeedI = 0.0001;
	leftPowSpeedD = 0.0001;
	rightPowSpeedP = 0.002;
	rightPowSpeedI = 0.0001;
	rightPowSpeedD = 0.0001;
//	leftPowSpeedP = 0.08;
//	leftPowSpeedI = 0.08;
//	leftPowSpeedD = 0.0001;
//	rightPowSpeedP = 0.08;//0.08
//	rightPowSpeedI = 0.08;//0.08
//	rightPowSpeedD = 0.0001;
	speedAngP = 10000;
	speedAngI = 0;
	speedAngD = 40;
	targetAngSpeedP = -0.0004;
	targetAngSpeedI = 0;
	targetAngSpeedD = 0.000012;
	targetAngLim = 8;
	targetSpeedAngK = 1200;
	diffP = 0.0065;
	diffD = 0.005;
	sumAngErrLim = 100000;
	sumSpeedErrLim = 10000;
	camEnable = 1;
	tuneBal = 0;
	tuneSpeed = 0;
	bool T = true;

	while(T) {

		currentTime = System::Time();

		if (currentTime - startTime >= 5) {

			led1.Switch();

//			const Byte speedByte = 85;
//			btPtr->SendBuffer(&speedByte, 1);
////			btPtr->SendStr(speedStr);
//			float dt = currentTime - startTime;
			startTime = currentTime;
//			sprintf(buffer,"%d,%f\n",1.0,dt);
//			btPtr->SendStr(buffer);

			//Camera algorithm, change differential
			const Byte* image = Cam1.LockBuffer();
			Cam1.UnlockBuffer();
			if (camEnable ) {
				FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
				prevDiff = curDiff;
//				FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
	//			ModifyEdge(image,LeftEdge, RightEdge,LeftEdgeNum, RightEdgeNum,LeftCornerOrder,RightCornerOrder,LeftCorner, RightCorner);
				curDiff = FindPath(LeftEdge,RightEdge,MidPoint,MidPointNum,ModifyEdge(image,LeftEdge, RightEdge,MidPoint,LeftEdgeNum, RightEdgeNum,MidPointNum,LeftCornerOrder,RightCornerOrder,LeftCorner,RightCorner),LeftCorner,RightCorner,LeftEdgeNum, RightEdgeNum);
//				curDiff = FindPath(LeftEdge, RightEdge,ModifyEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum,LeftCornerOrder,RightCornerOrder),LeftCornerOrder,RightCornerOrder);

				differential = -(diffP*(curDiff) + diffD*(curDiff-prevDiff));
//				differential = diffP*(curDiff);


				if(differential > 0.25)  differential = 0.25;
				else if(differential < -0.25)  differential = -0.25;
				sprintf(buffer,"%f\n",differential);


				btPtr->SendStr(buffer);
				if(carbegin[1]){
					leftMotorPtr->SetPower(0);
					rightMotorPtr->SetPower(0);
					T = false;
				}

//				differential = 0;
			}



		}

	}
	Cam1.Stop();
	return 0;
}
