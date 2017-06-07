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

#include <cstring>
#include <cmath>
#include <vector>
#include <list>
#include <array>
#include <sstream>
#include <cstring>
#include <stdio.h>

#include <camera.h>

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
FutabaS3010 * servoPtr;

//BT listener
string inputStr;
bool tune = false;
vector<double> constVector;

//Constants
double balAngle = 34.6;
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

bool bluetoothListener(const Byte *data, const size_t size) {

//	const Byte inputByte = 171;
//	btPtr->SendBuffer(&inputByte, 1);
//	str += '\n';
//	btPtr->SendStr(str);

	if (data[0] == 'w') {
		targetAng += 0.2;
	}
	if (data[0] == 's') {
		targetAng -= 0.2;
	}
	if (data[0] == 'a') {
		differential = -0.25;
	} else if (data[0] == 'A') {
		differential = 0;
	}
	if (data[0] == 'd') {
		differential = 0.25;
	} else if (data[0] == 'D') {
		differential = 0;
	}

	if (data[0] == 'P') {
		programRun = 0;
	}

	if (data[0] == 't') {
		programRun = 1;
		tune = 1;
		inputStr = "";
	}
	if (tune) {
		unsigned int i = 0;
		while (i<size) {
			if (data[i] != 't' && data[i] != '\n') {
				inputStr += (char)data[i];
			} else if (data[i] == '\n') {
				tune = 0;
				break;
			}
			i++;
		}
		if (!tune) {
			constVector.clear();
			char * pch;
			pch = strtok(&inputStr[0], ",");
			while (pch != NULL){
				double constant;
				stringstream(pch) >> constant;
				constVector.push_back(constant);
				pch = strtok (NULL, ",");
			}

			balAngle = constVector[0];
			inputTargetSpeed = constVector[1];
			leftPowSpeedP = constVector[2];
			leftPowSpeedI = constVector[3];
			leftPowSpeedD = constVector[4];
			rightPowSpeedP = constVector[5];
			rightPowSpeedI = constVector[6];
			rightPowSpeedD = constVector[7];
			speedAngP = constVector[8];
			speedAngI = constVector[9];
			speedAngD = constVector[10];
			targetAngSpeedP = constVector[11];
			targetAngSpeedI = constVector[12];
			targetAngSpeedD = constVector[13];
			targetAngLim = constVector[14];
			targetSpeedAngK = constVector[15];
			diffP = constVector[16];
			diffD = constVector[17];
			sumAngErrLim = constVector[18];
			sumSpeedErrLim = constVector[19];
			camEnable = constVector[20];
			tuneBal = constVector[21];
			tuneSpeed = constVector[22];
			sumLeftSpeedErr = 0;
			sumRightSpeedErr = 0;
			sumAngErr = 0;
		}
	}

//	else if (data[0] == 'a') {
//		servoPtr->SetDegree(900);
//	} else if (data[0] == 'd') {
//		servoPtr->SetDegree(430);
//	} else if (data[0] == 'A' || data[0] == 'D') {
//		servoPtr->SetDegree(700);
//	}

	return 1;
}

//Forrest variable
//Point LeftEdge[100], RightEdge[100];
//uint8_t LeftEdgeNum;
//uint8_t RightEdgeNum;
//uint8_t LeftCornerOrder, RightCornerOrder;

Point LeftEdge[150];
uint8_t LeftEdgeNum;
uint8_t RightEdgeNum;

uint8_t LeftCornerOrder;
uint8_t RightCornerOrder;
Point RightEdge[150];

Point* LeftCorner[2] = {NULL};
Point* RightCorner[2] = {NULL};
Point ModLeftEdge[150];
Point ModRightEdge[150];

int main(void) {

	System::Init();

//Loop
	long startTime = System::Time();
	long prevSampleTime = System::Time();
	double dt = 0;
	long currentTime = 0;
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
	for (int i = 0; i < speedArrSize; i++){
		leftSpeedArr[i] = 0;
		rightSpeedArr[i] = 0;
	}

	const int speedErrRateArrSize = 5;
	double leftSpeedErr = 0, rightSpeedErr = 0, curSpeedErr = 0;
	double prevLeftSpeedErr = 0, prevRightSpeedErr = 0, prevSpeedErr = 0;
	double leftSpeedErrRate = 0, rightSpeedErrRate = 0, speedErrRate = 0;
	int leftSpeedErrRateCounter = 0, rightSpeedErrRateCounter = 0, speedErrRateCounter = 0;
	double leftSpeedErrRateTotal = 0, rightSpeedErrRateTotal = 0, speedErrRateTotal = 0;
	double leftSpeedErrRateArr[speedErrRateArrSize], rightSpeedErrRateArr[speedErrRateArrSize], speedErrRateArr[speedErrRateArrSize];

	for (int i = 0; i < speedErrRateArrSize; i++){
		leftSpeedErrRateArr[i] = 0;
		rightSpeedErrRateArr[i] = 0;
		speedErrRateArr[i] = 0;
	}

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
	for (int i = 0; i < angArrSize; i++){
		angArr[i] = balAngle;
	}

	double curAngErr = 0, prevAngErr = 0, angErrRate = 0;
	int angErrRateCounter = 0;
	const int angErrRateArrSize = 5;
	double angErrRateArr[angErrRateArrSize];
	double angErrRateTotal = 0;
	for (int i = 0; i < angErrRateArrSize; i++){
		angErrRateArr[i] = 0;
	}

//	Led::Config LedConfig;
//	LedConfig.id = 0;
//	Led led1(LedConfig);

//	St7735r::Config LcdConfig;
//	LcdConfig.is_revert = true;
//	St7735r Lcd1(LcdConfig);

//	FutabaS3010::Config ServoConfig;
//	ServoConfig.id = 0;
//	FutabaS3010 servo1(ServoConfig);
//	servoPtr = &servo1;

	Ov7725::Config CamConfig;
	CamConfig.id = 0;
	CamConfig.w = width;
	CamConfig.h = height;
	CamConfig.fps = Ov7725Configurator::Config::Fps(2);
	Ov7725 Cam1(CamConfig);

	JyMcuBt106::Config BluetoothConfig;
	BluetoothConfig.id = 0;
	BluetoothConfig.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	BluetoothConfig.rx_isr = &bluetoothListener;
	JyMcuBt106 bluetooth1(BluetoothConfig);
	btPtr = &bluetooth1;
//	b = btPtr;

	Mpu6050::Config mpuConfig;
	mpuConfig.gyro_range=Mpu6050::Config::Range::kSmall;
	mpuConfig.accel_range=Mpu6050::Config::Range::kSmall;
	Mpu6050 mpu(mpuConfig);

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

	Cam1.Start();
	double temp = 0;
	System::DelayMs(2000);

	//Constant
	targetAng = 36;	//Angle to run
	balAngle = 36;
	inputTargetSpeed = 8000;
//	leftPowSpeedP = 0.002;
//	leftPowSpeedI = 0.0001;
//	leftPowSpeedD = 0.0001;
//	rightPowSpeedP = 0.002;
//	rightPowSpeedI = 0.0001;
//	rightPowSpeedD = 0.0001;
	leftPowSpeedP = 0.11;
	leftPowSpeedI = 0.1;
	leftPowSpeedD = 0.0001;
	rightPowSpeedP = 0.08;
	rightPowSpeedI = 0.08;
	rightPowSpeedD = 0.0001;
	speedAngP = 6000;
	speedAngI = 0;
	speedAngD = 80;
	targetAngSpeedP = -0.0003;
	targetAngSpeedI = 0;
	targetAngSpeedD = 0.00002;
	targetAngLim = 0;
	targetSpeedAngK = 0;
	diffP = 0.004;
	diffD = 0.005;
	sumAngErrLim = 100000;
	sumSpeedErrLim = 10000;
	camEnable = 1;
	tuneBal = 0;
	tuneSpeed = 0;

	while(1) {

		currentTime = System::Time();

		if (currentTime-startTime >= 5) {

			startTime = currentTime;
			loopCounter++;

			//camera algorithm, change differential
			const Byte* image = Cam1.LockBuffer();
			Cam1.UnlockBuffer();
			if (camEnable && loopCounter%2==0) {
				FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
				prevDiff = curDiff;
				FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
				curDiff = FindPath(LeftEdge,RightEdge,ModifyEdge(image,LeftEdge, RightEdge,LeftEdgeNum, RightEdgeNum,LeftCornerOrder,RightCornerOrder,LeftCorner,RightCorner),LeftCorner,RightCorner,LeftEdgeNum, RightEdgeNum);

				differential = diffP*(curDiff) + diffD*(curDiff-prevDiff);

				if(differential > 0.2)  differential = 0.2;
				else if(differential < -0.2)  differential = -0.2;
			}

			//time interval
			dt = (double)(System::Time()-prevSampleTime)/1000;
			prevSampleTime = System::Time();

			//speed
			encoderLeft.Update();
			temp = encoderLeft.GetCount()/dt;
			if (temp>50000 || temp<-50000) {
				temp = leftSpeed;
			}
			leftSpeed = arrAvg(leftSpeedArr, speedArrSize, leftSpeedArrCounter, leftSpeedTotal, temp);
			encoderRight.Update();
			temp = -encoderRight.GetCount()/dt;
			if (temp>50000 || temp<-50000) {
				temp = rightSpeed;
			}
			rightSpeed = arrAvg(rightSpeedArr, speedArrSize, rightSpeedArrCounter, rightSpeedTotal, temp);
			prevSpeed = curSpeed;
			curSpeed = (leftSpeed+rightSpeed)/2;

			//angle
			mpu.Update(1);
			acc[0] = mpu.GetAccel()[0];
			acc[1] = mpu.GetAccel()[1];
			acc[2] = mpu.GetAccel()[2];
			accbine = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
			accAng = (acos(acc[0]/accbine)*180)/3.1415;
			gyroAng = curAng + (mpu.GetOmega()[1]/160/131)*dt;
			prevAng = curAng;
			curAng = arrAvg(angArr, angArrSize, angCounter, angTotal, abs((0.98*gyroAng)+(0.02*accAng)));
//			if (System::Time() >= 5000 && (curAng >= balAngle+20 || curAng <= balAngle-10)) {
//				break;
//			}

			if (programRun == false || System::Time()<3000) {
				motorLeft.SetPower(0);
				motorRight.SetPower(0);
				continue;
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
			leftPow = (int)(leftPowSpeedP * leftSpeedErr + leftPowSpeedI * sumLeftSpeedErr + leftPowSpeedD * leftSpeedErrRate);

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
			rightPow = (int)(rightPowSpeedP * rightSpeedErr + rightPowSpeedI * sumRightSpeedErr + rightPowSpeedD * rightSpeedErrRate);

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
				motorLeft.SetClockwise(!leftForward);
				motorLeft.SetPower(leftPow);
			} else {
				motorLeft.SetClockwise(leftForward);
				motorLeft.SetPower(-1*leftPow);
			}
			if (rightPow > 0) {
				motorRight.SetClockwise(!rightForward);
				motorRight.SetPower(rightPow);
			} else {
				motorRight.SetClockwise(rightForward);
				motorRight.SetPower(-1*rightPow);
			}

			//send data
			if (loopCounter%10 == 0) {
				char speedChar[15] = {};
//				sprintf(speedChar, "%.1f,%.3f,%.4f,%.4f,%.3f,%.4f,%.4f\n", 1.0, leftPowSpeedP, leftPowSpeedI, leftPowSpeedD, rightPowSpeedP, rightPowSpeedI, rightPowSpeedD);
//				sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, targetAng, tempTargetAng);
//				sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, leftSpeed, leftTempTargetSpeed);
//				sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, rightSpeed, rightTempTargetSpeed);
				sprintf(speedChar, "%.1f,%.2f,%.2f,%.2f,%.2f\n", 1.0, targetAng, tempTargetAng, curSpeed, targetSpeed);
//				sprintf(speedChar, "%.1f,%.2f\n", 1.0, differential);
				string speedStr = speedChar;

				const Byte speedByte = 85;
				bluetooth1.SendBuffer(&speedByte, 1);
				bluetooth1.SendStr(speedStr);
			}

//			if (Cam1.IsAvailable()) {
//
//				camInput = Cam1.LockBuffer();
//
//				getImage(camInput);
//
//				Cam1.Start();
//				Cam1.UnlockBuffer();
//
//			}
//
//			if (loopCounter%60 == 0) {
//				loopCounter = 0;
//
//				const Byte imageByte = 170;
//				bluetooth1.SendBuffer(image, 1);
//				bluetooth1.SendBuffer(camInput, Cam1.GetBufferSize());
//			}

		}

	}
	Cam1.Stop();
	return 0;
}

