/*
 * magnetic.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: Mk
 */

#include <cassert>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>
#include <cstring>
#include <stdio.h>

//LED
#include <libsc/led.h>

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

//ADC
#include <libbase/k60/adc.h>

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

//Object pointer
AlternateMotor * motorPtr;
DirEncoder * encoderPtr;
JyMcuBt106 * btPtr;
FutabaS3010 * servoPtr;

//BT listener
string inputStr;
bool tune = false;
vector<double> constVector;

//Constants
double inputTargetSpeed = 0;
double powSpeedP = 0, powSpeedI = 0, powSpeedD = 0;
double diffP = 0, diffD = 0;
double sumSpeedErrLim = 10000, sumAngErrLim = 100000;
double temp = 0;

int power = 0;
int curDeg = 900;
double differential = 0;
double sumSpeedErr = 0;
bool programRun = true, tuneSpeed = false;

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

	if (data[0] == 'w') {
		servoPtr->SetDegree(900);
	}
	if (data[0] == 'a') {
		curDeg += 20;
		servoPtr->SetDegree(curDeg);
	}
	if (data[0] == 'd') {
		curDeg -= 20;
		servoPtr->SetDegree(curDeg);
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

			inputTargetSpeed = constVector[0];
			powSpeedP = constVector[1];
			powSpeedI = constVector[2];
			powSpeedD = constVector[3];
			diffP = constVector[4];
			diffD = constVector[5];
			sumSpeedErrLim = constVector[6];
			tuneSpeed = constVector[7];

			power = 0;
			sumSpeedErr = 0;
		}
	}

	return 1;
}

int main(void) {

	System::Init();

//Loop
	long startTime = System::Time();
	long prevSampleTime = System::Time();
	double dt = 0;
	long currentTime = 0;
	long loopCounter = 0;

//Speed
	bool forward = 1;

	const int speedArrSize = 10;
	double speed = 0, prevSpeed = 0;
	double targetSpeed = 0;
	int speedArrCounter = 0;
	double speedTotal = 0;
	double speedArr[speedArrSize];
	for (int i = 0; i < speedArrSize; i++){
		speedArr[i] = 0;
	}

	const int speedErrRateArrSize = 5;
	double speedErr = 0;
	double prevSpeedErr = 0;
	double speedErrRate = 0;
	int speedErrRateCounter = 0;
	double speedErrRateTotal = 0;
	double speedErrRateArr[speedErrRateArrSize];

	for (int i = 0; i < speedErrRateArrSize; i++){
		speedErrRateArr[i] = 0;
	}

//Servo

	int servoMid = 1000, servoRange = 550;

//Adc
	double midLeft = 0, midRight = 0, frontLeft = 0, frontRight = 0;
	double midDiff = 0, prevMidDiff = 0;

//Angle
//	double acc[3];
//	double accbine = 0;
//	double accAng = 0, gyroAng = 0;
//
//	double curAng = 0, prevAng = 0;
//	double tempTargetAng = 0;
//	int angCounter = 0;
//	const int angArrSize = 10;
//	double angTotal = 0;
//	double angArr[angArrSize] = {0};

//	Led::Config LedConfig;
//	LedConfig.id = 0;
//	Led led1(LedConfig);

	FutabaS3010::Config ServoConfig;
	ServoConfig.id = 0;
	FutabaS3010 servo(ServoConfig);
	servoPtr = &servo;

	JyMcuBt106::Config BluetoothConfig;
	BluetoothConfig.id = 0;
	BluetoothConfig.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	BluetoothConfig.rx_isr = &bluetoothListener;
	JyMcuBt106 bluetooth1(BluetoothConfig);
	btPtr = &bluetooth1;

//	Mpu6050::Config mpuConfig;
//	mpuConfig.gyro_range=Mpu6050::Config::Range::kSmall;
//	mpuConfig.accel_range=Mpu6050::Config::Range::kSmall;
//	Mpu6050 mpu(mpuConfig);

	AlternateMotor::Config MotorConfig;
	MotorConfig.id = 0;
	AlternateMotor motor(MotorConfig);
	motorPtr = &motor;

	DirEncoder::Config EncoderConfig;
	EncoderConfig.id = 0;
	DirEncoder encoder(EncoderConfig);
	encoderPtr = &encoder;

	Adc::Config adcConfig;
	adcConfig.resolution = Adc::Config::Resolution::k12Bit;
	adcConfig.speed = Adc::Config::SpeedMode::kTypical;
	adcConfig.avg_pass = Adc::Config::AveragePass::k8;
//	adcConfig.pin = libbase::k60::Pin::Name::kPte0;
//	Adc midLeftInductor(adcConfig);
//	adcConfig.pin = libbase::k60::Pin::Name::kPte1;
//	Adc midRightInductor(adcConfig);
//	adcConfig.pin = libbase::k60::Pin::Name::kPte2;
//	Adc frontLeftInductor(adcConfig);
//	adcConfig.pin = libbase::k60::Pin::Name::kPte3;
//	Adc frontRightInductor(adcConfig);
	adcConfig.pin = libbase::k60::Pin::Name::kPtb4;
	Adc midLeftInductor(adcConfig);
	adcConfig.pin = libbase::k60::Pin::Name::kPtb7;
	Adc midRightInductor(adcConfig);
	adcConfig.pin = libbase::k60::Pin::Name::kPtb5;
	Adc frontLeftInductor(adcConfig);
	adcConfig.pin = libbase::k60::Pin::Name::kPtb6;
	Adc frontRightInductor(adcConfig);

	motor.SetClockwise(forward);
	motor.SetPower(0);

	double temp = 0, temp1 = 0;
	servo.SetDegree(servoMid);
	System::DelayMs(2000);

	//Constant
	inputTargetSpeed = 3000;
	powSpeedP = -0.05;
	powSpeedI = -0.6;
	powSpeedD = -0.0009;
	diffP = -30;
	diffD = 0;
	sumSpeedErrLim = 10000;
	tuneSpeed = 1;

	while(1) {

		currentTime = System::Time();

		if (currentTime-startTime >= 5) {

			startTime = currentTime;
			loopCounter++;

			//time interval
			dt = (double)(System::Time()-prevSampleTime)/1000;
			prevSampleTime = System::Time();

			//speed
			encoder.Update();
			temp = -encoder.GetCount()/dt;
			if (temp>50000 || temp<-50000) {
				temp = speed;
			}
			speed = arrAvg(speedArr, speedArrSize, speedArrCounter, speedTotal, temp);
			prevSpeed = speed;

			//angle
//			mpu.Update(1);
//			acc[0] = mpu.GetAccel()[0];
//			acc[1] = mpu.GetAccel()[1];
//			acc[2] = mpu.GetAccel()[2];
//			accbine = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
//			accAng = (acos(acc[0]/accbine)*180)/3.1415;
//			gyroAng = curAng + (mpu.GetOmega()[1]/160/131)*dt;
//			prevAng = curAng;
//			curAng = arrAvg(angArr, angArrSize, angCounter, angTotal, abs((0.98*gyroAng)+(0.02*accAng)));

			if (programRun == false || System::Time()<3000) {
				motor.SetPower(0);
				continue;
			}

			//Adc
			midLeft = midLeftInductor.GetResultF()*100;
			frontLeft = frontLeftInductor.GetResultF()*100;
			frontRight = frontRightInductor.GetResultF()*100;
			midRight = midRightInductor.GetResultF()*100;

			prevMidDiff = midDiff;
			if (midLeft != 0 && midRight != 0) {
				midDiff = 1000/(midRight+frontRight) - 1000/(midLeft+frontLeft);
			}
			//filter

			curDeg = servoMid + (int)diffP*midDiff + (int)diffD*(midDiff-prevMidDiff);
			if (curDeg > servoMid+servoRange) {
				curDeg = servoMid+servoRange;
			} else if (curDeg < servoMid-servoRange) {
				curDeg = servoMid-servoRange;
			}
			servo.SetDegree(curDeg);

			targetSpeed = inputTargetSpeed;

			//power-speed PID
			prevSpeedErr = speedErr;
			speedErr = speed - targetSpeed;
			sumSpeedErr += speedErr * dt;
			if (sumSpeedErr > sumSpeedErrLim) {
				sumSpeedErr = sumSpeedErrLim;
			} else if (sumSpeedErr < -1*sumSpeedErrLim) {
				sumSpeedErr = -1*sumSpeedErrLim;
			}
			speedErrRate = arrAvg(speedErrRateArr, speedErrRateArrSize, speedErrRateCounter, speedErrRateTotal, (speedErr-prevSpeedErr) / dt);
//			speedErrRate = (speedErr-prevSpeedErr) / dt;
			power = (int)(powSpeedP * speedErr + powSpeedI * sumSpeedErr + powSpeedD * speedErrRate);
//			power = 0;

			//power limit
			if (power > 500) {
				power = 500;
			} else if (power < -500) {
				power = -500;
			}

			//set power
			if (power > 0) {
				motor.SetClockwise(!forward);
				motor.SetPower(power);
			} else {
				motor.SetClockwise(forward);
				motor.SetPower(-1*power);
			}

			//send data
			if (loopCounter%3 == 0) {
				char dataChar[15] = {};
//				sprintf(dataChar, "%.1f,%.3f\n", 1.0, midDiff);
//				sprintf(dataChar, "%.1f,%d\n", 1.0, curDeg);
				sprintf(dataChar, "%.1f,%.3f,%.3f=%.3f,%.3f,%.3f,%.3f\n", 1.0, speed, targetSpeed, midLeft, frontLeft, frontRight, midRight);
				string dataStr = dataChar;

				const Byte startByte = 85;
				bluetooth1.SendBuffer(&startByte, 1);
				bluetooth1.SendStr(dataStr);
			}
		}
	}

	return 0;
}
