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

bool pixelArray[globalHeight][globalWidth];
bool boundaryArray[globalHeight][globalWidth];

//Object pointer
AlternateMotor * leftMotorPtr;
AlternateMotor * rightMotorPtr;
DirEncoder * encoderLeftPtr;
DirEncoder * encoderRightPtr;
JyMcuBt106 * btPtr;
FutabaS3010 * servoPtr;

int state = 0;

//BT listener
string inputStr;
bool tune = false;
vector<double> constVector;

//Constants
double balAngle = 45;
double powSpeedP = 0, powSpeedI = 0, powSpeedD = 0;
double speedAngP = 0, speedAngI = 0, speedAngD = 0;
double targetAngSpeedP = 0, targetAngSpeedI = 0, targetAngSpeedD = 0;

double sumAngErr = 0, sumSpeedErr = 0, sumLeftSpeedErr = 0, sumRightSpeedErr = 0;
double targetSpeed = 0;

void getImage(const Byte * data) {

	bool previous;
	int pos = 0, bit_pos = 8;
	int height = globalHeight, width = globalWidth;

	for (int y = 0; y < height; y++) {

		for (int x = 0; x < width; x++) {

			if (--bit_pos < 0) {
				bit_pos = 7;
				++pos;
			}
			if (GET_BIT(data[pos], bit_pos)) {
				pixelArray[y][x] = 1;
			} else {
				pixelArray[y][x] = 0;
			}

		}

	}

	for (int y = 0; y < height; y++) {

		previous = pixelArray[y][0];

		for (int x = 0; x < width; x++) {

			if (pixelArray[y][x] != previous) {

				if (pixelArray[y][x] == 0) {
					boundaryArray[y][x] = 1;
				} else {
					boundaryArray[y][x-1] = 1;
					boundaryArray[y][x] = 0;
				}
				previous = pixelArray[y][x];

			} else {
				boundaryArray[y][x] = 0;
			}

		}

	}

	for (int x = 0; x < width; x++) {

		previous = pixelArray[0][x];

		for (int y = 0; y < height; y++) {

			if (pixelArray[y][x] != previous) {

				if (pixelArray[y][x] == 0) {
					boundaryArray[y][x] = 1;
				} else {
					boundaryArray[y-1][x] = 1;
					boundaryArray[y][x] = 0;
				}
				previous = pixelArray[y][x];

			}

		}

	}

}

double arrAvg(double arr[], int size, int& counter, double& total, double newVal) {

	total -= arr[counter];
	arr[counter] = newVal;
	total += arr[counter];
	counter++;
	if (counter == size) {
		counter = 0;
	}
	return total/size;
}

bool bluetoothListener(const Byte *data, const size_t size) {

//	const Byte inputByte = 171;
//	btPtr->SendBuffer(&inputByte, 1);
//	str += '\n';
//	btPtr->SendStr(str);

	if (data[0] == 'w') {
		targetSpeed = 3000;
	} else if (data[0] == 'W') {
		targetSpeed = 0;
	}
	if (data[0] == 's') {
		targetSpeed = -3000;
	} else if (data[0] == 'S') {
		targetSpeed = 0;
	}

	if (data[0] == 't') {
		tune = 1;
		inputStr = "";
	}
	if (tune) {
		int i = 0;
		while (i<size) {
			if (data[i] != 't' && data[i] != '\n') {
				inputStr += (char)data[i];
			} else if (data[i] == '\n') {
				tune = 0;
			}
			i++;
		}
		if (!tune) {
			constVector.clear();

			char * pch;
			pch = strtok(&inputStr[0], " ,");
			while (pch != NULL){
				double constant;
				stringstream(pch) >> constant;
				constVector.push_back(constant);
				pch = strtok (NULL, " ,");
			}

			balAngle = constVector[0];
			powSpeedP = constVector[1];
			powSpeedI = constVector[2];
			powSpeedD = constVector[3];
			speedAngP = constVector[4];
			speedAngI = constVector[5];
			speedAngD = constVector[6];
			targetAngSpeedP = constVector[7];
			targetAngSpeedI = constVector[8];
			targetAngSpeedD = constVector[9];
			sumSpeedErr = 0;
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

}

int main(void) {

	System::Init();

//Loop
	long startTime = System::Time();
	long currentTime = 0;
	int loopCounter = 0;

//Camera
	const Byte * camInput;
	int height = globalHeight, width = globalWidth;

//Speed
	long prevSampleTime = System::Time();
	double dt = 0;

	int leftPow = 0, rightPow = 0, accDir = 1;
	double realFactor = 0;	// acc:1,dec:-1
	bool leftForward = 1, rightForward = 1;

	double curSpeed = 0;
	double speedErr = 0, prevSpeedErr = 0;
	double speedErrRate = 0;
	int speedErrRateCounter = 0, speedErrRateArrSize = 5;
	double speedErrRateTotal = 0;
	double speedErrRateArr[speedErrRateArrSize];

	double leftSpeed = 0, rightSpeed = 0;
	double leftTempTargetSpeed = 0, rightTempTargetSpeed = 0;
	int leftSpeedArrCounter = 0, rightSpeedArrCounter = 0, speedArrSize = 10;
	double leftSpeedTotal = 0, rightSpeedTotal = 0;
	double leftSpeedArr[speedArrSize], rightSpeedArr[speedArrSize];
	for (int i = 0; i < speedArrSize; i++){
		leftSpeedArr[i] = 0;
		rightSpeedArr[i] = 0;
	}

	double leftSpeedErr = 0, prevLeftSpeedErr = 0;
	double rightSpeedErr = 0, prevRightSpeedErr = 0;
	double leftSpeedErrRate = 0, rightSpeedErrRate = 0;
	int leftSpeedErrRateCounter = 0, rightSpeedErrRateCounter = 0;
	double leftSpeedErrRateTotal = 0, rightSpeedErrRateTotal = 0;
	double leftSpeedErrRateArr[speedErrRateArrSize], rightSpeedErrRateArr[speedErrRateArrSize];

	for (int i = 0; i < speedArrSize; i++){
		leftSpeedErrRateArr[i] = 0;
		rightSpeedErrRateArr[i] = 0;
		speedErrRateArr[i] = 0;
	}

//Angle
	double acc[3];
	double accbine = 0;
	double accAng = 0, gyroAng = 0;

	double curAng = 0;
	double targetAng = balAngle;
	int angCounter = 0, angArrSize = 10;
	double angTotal = balAngle*angArrSize;
	double angArr[angArrSize];
	for (int i = 0; i < angArrSize; i++){
		angArr[i] = balAngle;
	}

	double curAngErr = 0, prevAngErr = 0, angErrRate = 0;
	int angErrRateCounter = 0, angErrRateArrSize = 5;
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

//	Ov7725::Config CamConfig;
//	CamConfig.id = 0;
//	CamConfig.w = width;
//	CamConfig.h = height;
//	CamConfig.fps = Ov7725Configurator::Config::Fps(0);
//	Ov7725 Cam1(CamConfig);

	JyMcuBt106::Config BluetoothConfig;
	BluetoothConfig.id = 0;
	BluetoothConfig.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	BluetoothConfig.rx_isr = &bluetoothListener;
	JyMcuBt106 bluetooth1(BluetoothConfig);
	btPtr = &bluetooth1;

	Mpu6050::Config mpuConfig;
	mpuConfig.gyro_range=Mpu6050::Config::Range::kSmall;
	mpuConfig.accel_range=Mpu6050::Config::Range::kSmall;
	Mpu6050 mpu(mpuConfig);

	AlternateMotor::Config LeftMotorConfig;
	LeftMotorConfig.id = 0;
	AlternateMotor motorLeft(LeftMotorConfig);
	leftMotorPtr = &motorLeft;
	AlternateMotor::Config RightMotorConfig;
	RightMotorConfig.id = 1;
	AlternateMotor motorRight(RightMotorConfig);
	rightMotorPtr = &motorRight;

	DirEncoder::Config LeftEncoderConfig;
	LeftEncoderConfig.id = 1;
	DirEncoder encoderLeft(LeftEncoderConfig);
	encoderLeftPtr = &encoderLeft;
	DirEncoder::Config RightEncoderConfig;
	RightEncoderConfig.id = 0;
	DirEncoder encoderRight(RightEncoderConfig);
	encoderRightPtr = &encoderRight;

//	Cam1.Start();
	motorLeft.SetClockwise(leftForward);
	motorRight.SetClockwise(rightForward);
	motorLeft.SetPower(0);
//	motorRight.SetPower(200);

	while(1) {

		currentTime = System::Time();

		if (currentTime-startTime >= 5) {

			startTime = currentTime;
			loopCounter++;

			dt = (float)(System::Time()-prevSampleTime)/1000;
			prevSampleTime = System::Time();

			//Speed
			encoderLeft.Update();
			leftSpeed = arrAvg(leftSpeedArr, speedArrSize, leftSpeedArrCounter, leftSpeedTotal, encoderLeft.GetCount()/dt);
			encoderRight.Update();
			rightSpeed = arrAvg(rightSpeedArr, speedArrSize, rightSpeedArrCounter, rightSpeedTotal, encoderRight.GetCount()/dt);
			curSpeed = (leftSpeed+rightSpeed)/2;

			//Angle
			mpu.Update(1);
			acc[0] = mpu.GetAccel()[0];
			acc[1] = mpu.GetAccel()[1];
			acc[2] = mpu.GetAccel()[2];
			accbine = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
			accAng = (acos(acc[0]/accbine)*180)/3.1415;
			gyroAng = curAng + (mpu.GetOmega()[1]/160/131)*dt;
			curAng = arrAvg(angArr, angArrSize, angCounter, angTotal, abs((0.98*gyroAng)+(0.02*accAng)));
//			if (curAng >= balAngle+20 || curAng <= balAngle-10) {
//				break;
//			}

//			if (state == 0) {
//				accDir = (curSpeed>targetSpeed)?-1:1;
//				realFactor = speedAngFactor*(curSpeed-targetSpeed);
//				if (realFactor > 1) {
//					targetAng = balAngle - 1;
//				} else if (realFactor < -1) {
//					targetAng = balAngle + 1;
//				} else {
//					targetAng = balAngle - speedAngFactor*(curSpeed-targetSpeed);
//				}
//				state++;
//			}
//			if (state == 1) {
//				if (curSpeed >= targetSpeed-preSpeedDiff*abs(realFactor)-500 && curSpeed <= targetSpeed+preSpeedDiff*abs(realFactor)+500) {
//					targetAng = balAngle;
//					state++;
//				}
//			}
//			if (state == 2) {
//				if (curAng >= balAngle*(1-errorRange) && curAng <= balAngle*(1+errorRange)) {
//					state = 0;
//				}
//			}

			//TargetAng-speed PID
			speedErr = curSpeed - targetSpeed;
			sumSpeedErr += speedErr * dt;
			speedErrRate = arrAvg(speedErrRateArr, speedErrRateArrSize, speedErrRateCounter, speedErrRateTotal, (speedErr-prevSpeedErr) / dt);
			prevSpeedErr = speedErr;
			targetAng = balAngle + targetAngSpeedP * speedErr + targetAngSpeedI * sumSpeedErr + targetAngSpeedD * speedErrRate;
			if (targetAng-balAngle > 1) {
				targetAng = balAngle + 1;
			} else if (targetAng-balAngle < -1) {
				targetAng = balAngle - 1;
			}

			//tempSpeed-angle PID
			curAngErr = curAng - targetAng;
			sumAngErr += curAngErr * dt;
			angErrRate = arrAvg(angErrRateArr, angErrRateArrSize, angErrRateCounter, angErrRateTotal, (curAngErr-prevAngErr) / dt);
			prevAngErr = curAngErr;
			leftTempTargetSpeed = speedAngP * curAngErr + speedAngI * sumAngErr + speedAngD * angErrRate;

			//Power-speed PID
			leftSpeedErr = leftSpeed - leftTempTargetSpeed;
			sumLeftSpeedErr += leftSpeedErr * dt;
			leftSpeedErrRate = arrAvg(leftSpeedErrRateArr, speedErrRateArrSize, leftSpeedErrRateCounter, leftSpeedErrRateTotal, (leftSpeedErr-prevLeftSpeedErr) / dt);
			prevLeftSpeedErr = leftSpeedErr;
			leftPow += powSpeedP * leftSpeedErr + powSpeedI * sumLeftSpeedErr + powSpeedD * leftSpeedErrRate;

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

//			leftPow = 0;
//			rightPow = 0;

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

			if (loopCounter%3 == 0) {
				char speedChar[15] = {};
				sprintf(speedChar, "%.1f,%.2f,%.2f=%.2f,%.2f\n", 1.0, leftSpeed, targetSpeed, curAng, targetAng);
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
//			if (loopCounter%40 == 0) {
//				loopCounter = 0;
//
//				const Byte imageByte = 170;
//				bluetooth1.SendBuffer(&imageByte, 1);
//				bluetooth1.SendBuffer(camInput, Cam1.GetBufferSize());
//			}

		}

	}

	return 0;
}
