/*
 * magnetic.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: Mk
 */

#include <cassert>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

#include <algorithm>
#include <cstring>
#include <cmath>
#include <sstream>
#include <stdio.h>
#include <vector>

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
double turnP = 0, turnD = 0;
double straightBound = 0;
double straightP = 0, straightD = 0;
double frontDiffPow = 0, midStraightBound = 0;
double frontCrossBoundary = 0, frontRoundBoundary = 0, midRoundBoundary = 0;
double frontRoundDiv = 0, midRoundDiv = 0;
double backMid = 0, backMidFront = 0, outRound = 0;
double detectedPeriod = 0;
double midMin = 0;
double speedDecP = 0, speedDecP2 = 0, speedDecP3 = 0, speedDecLim = 0;
double roundSpeedDec = 0;
double sumSpeedErrLim = 10000;
double temp = 0;

int power = 0;
int curDeg = 900;
int servoMid = 950, servoRange = 550;
double sumSpeedErr = 0;
bool programRun = true;

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
		curDeg = servoMid;
		servoPtr->SetDegree(curDeg);
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
			turnP = constVector[4];
			turnD = constVector[5];
			straightBound = constVector[6];
			straightP = constVector[7];
			straightD = constVector[8];
			frontDiffPow = constVector[9];
			midStraightBound = constVector[10];
			frontCrossBoundary = constVector[11];
			frontRoundBoundary = constVector[12];
			midRoundBoundary = constVector[13];
			frontRoundDiv = constVector[14];
			midRoundDiv = constVector[15];
			backMid = constVector[16];
			backMidFront = constVector[17];
			outRound = constVector[18];
			midMin = constVector[19];
			detectedPeriod = constVector[20];
			speedDecP = constVector[21];
			speedDecP2 = constVector[22];
			speedDecP3 = constVector[23];
			speedDecLim = constVector[24];
			roundSpeedDec = constVector[25];
			sumSpeedErrLim = constVector[26];

			power = 0;
			sumSpeedErr = 0;
		}
	}

	return 1;
}

int main(void) {

	System::Init();

//loop
	long startTime = System::Time();
	long prevSampleTime = System::Time();
	double dt = 0;
	long currentTime = 0;
	long loopCounter = 0;

//speed
	bool forward = 1;

	const int speedArrSize = 10;
	double speed = 0;
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

//turning

	double midLeft = 0, midRight = 0, frontLeft = 0, frontRight = 0;

	bool carOut;

	int roundState = 0;
	bool crossDetected = 0;
	long detectedTime = 0, backMidTime = 0, lastRound = 0;

	double diff = 0, prevDiff = 0, diffRate = 0;
	double midDiff = 0, frontDiff = 0;
	double midSum = 0, frontDiv = 0;

	const int diffRateArrSize = 3;
	int diffRateArrCounter = 0;
	double diffRateTotal = 0;
	double diffRateArr[diffRateArrSize];
	for (int i = 0; i < diffRateArrSize; i++){
		diffRateArr[i] = 0;
	}

	const int frontDivArrSize = 20;
	int frontDivArrCounter = 0;
	double frontDivTotal = 0;
	double frontDivArr[frontDivArrSize];
	for (int i = 0; i < frontDivArrSize; i++){
		frontDivArr[i] = 0;
	}

	const int midSumArrSize = 20;
	int midSumArrCounter = 0;
	double midSumTotal = 0;
	double midSumArr[midSumArrSize];
	for (int i = 0; i < midSumArrSize; i++){
		midSumArr[i] = 0;
	}

	double deg = 0, prevDeg = 0;
	const int degArrSize = 20;
	int degArrCounter = 0;
	double degTotal = 0;
	double degArr[degArrSize];
	for (int i = 0; i < degArrSize; i++){
		degArr[i] = 0;
	}

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
	adcConfig.avg_pass = Adc::Config::AveragePass::k32;
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
	System::DelayMs(1000);

	//Constant
	inputTargetSpeed = 8000;
	powSpeedP = -0.025;
	powSpeedI = -0.2;
	powSpeedD = -0.001;
	turnP = -25;
	turnD = -20;
	straightBound = 40;
	straightP = -8;
	straightD = -10;
	frontDiffPow = 0.2;
	midStraightBound = 20;
	frontCrossBoundary = 160;
	frontRoundBoundary = 60;
	midRoundBoundary = 150;
	frontRoundDiv = 30;
	midRoundDiv = 50;
	backMid = 100;
	backMidFront = 80;
	outRound = 200;
	midMin = 50;
	detectedPeriod = 500;
	speedDecP = 0.0;
	speedDecP2 = 0;
	speedDecP3 = 0;
	speedDecLim = 0.3;
	roundSpeedDec = 0.5;
	sumSpeedErrLim = 10000;

	while(1) {

		currentTime = System::Time();

		if (currentTime-startTime >= 8) {

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

			//stop car
			if (programRun == false || System::Time()<3000) {
				motor.SetPower(0);
				continue;
			}

			//adc
			midLeft = midLeftInductor.GetResultF()*100;
			midRight = midRightInductor.GetResultF()*100;
			frontLeft = frontLeftInductor.GetResultF()*100;
			frontRight = frontRightInductor.GetResultF()*100;
			midSum = arrAvg(midSumArr, midSumArrSize, midSumArrCounter, midSumTotal, midLeft + midRight);
			frontDiv = arrAvg(frontDivArr, frontDivArrSize, frontDivArrCounter, frontDivTotal, fabs(frontLeft-frontRight));

			//cross
			if (crossDetected == 0 && roundState==0 && frontLeft+frontRight > frontCrossBoundary) {
				crossDetected  = true;
				detectedTime = System::Time();
			}
			if (crossDetected) {
				if ((System::Time()-detectedTime)>detectedPeriod) {
					crossDetected = false;
				}
			}

			//round
			if (crossDetected == 0 && roundState==0 && System::Time()-lastRound > detectedPeriod) {
				temp = sqrt(frontLeft*frontLeft+midLeft*midLeft);
				temp1 = sqrt(frontRight*frontRight+midRight*midRight);
//				if (frontLeft > frontRoundBoundary && frontRight > frontRoundBoundary && (midLeft+midRight) < midRoundBoundary && fabs(frontLeft-frontRight)<frontRoundDiv && fabs(midLeft-midRight)<midRoundDiv && (midLeft+midRight)>(frontLeft+frontRight) && fabs(curDeg-servoMid)<300) {
				if ((frontLeft+frontRight) > frontRoundBoundary && (midLeft+midRight) < midRoundBoundary && fabs(temp-temp1)<10 && (midLeft+midRight)>(frontLeft+frontRight) && fabs(curDeg-servoMid)<300) {
					roundState = 1;
					detectedTime = System::Time();
				}
			}
			if (roundState > 0) {
				midLeft *= 2;
				midRight *= 2;
				frontLeft *= 2;
				frontRight *= 2;
			}
			if (roundState == 1) {
				if (midLeft + midRight > backMid && frontLeft + frontRight < backMidFront) {
					backMidTime = System::Time()-detectedTime;
					roundState = 2;
				}
			}
			if (roundState == 2 && frontLeft+frontRight>outRound) {
				roundState = 3;
				lastRound = System::Time();
			}
			if (roundState == 3) {
				if (fabs(1000/midRight - 1000/midLeft)<20 && midLeft>120 && midRight>120) {
					roundState = 0;
				}
			}

			//diff
			if (midLeft!=0 && frontLeft!=0 && frontRight!=0 && midRight!=0) {
				prevDiff = diff;
				if ((midLeft>midMin || midRight>midMin) && roundState == 0) {
					midDiff = 1000/midRight - 1000/midLeft;
					frontDiff = frontLeft - frontRight;
				} else if ((midLeft>70 || midRight>70) && roundState == 2) {
					midDiff = 1000/midRight - 1000/midLeft;
					frontDiff = frontLeft - frontRight;
				}
//				temp = (midLeft+midRight)/fabs(frontDiff);
//				diff = (midRatio*midDiff + frontRatio*frontDiff)*sqrt(fabs(frontDiff))/(midLeft+midRight);
//				diff = (midRatio*midDiff + frontRatio*frontDiff)*sqrt(fabs(frontDiff));
//				diff = midDiff*pow(fabs(midDiff), frontDiffPowPow);
//				if (crossDetected || fabs(frontDiff)<straightBound || fabs(midDiff)<midStraightBound) {
//					diff = midDiff;
//				} else {
//					diff = midDiff*pow(fabs(frontDiff), frontDiffPow);
//				}
				diff = midDiff;
				diffRate = arrAvg(diffRateArr, diffRateArrSize, diffRateArrCounter, diffRateTotal, diff-prevDiff);
			}


//			temp = (diffRate-(straight*deg*deg)) * (diffRate-(straight*deg*deg));
//			curDeg = servoMid + (int)(turnP*(1+temp)*diff + turnD*diffRate);

			//deg-diff PID
			prevDeg = curDeg;
			if (roundState == 1 || roundState == 3) {
				curDeg = servoMid + servoRange;
			} else if (fabs(frontDiff)>straightBound || fabs(midDiff)>midStraightBound) {
				curDeg = servoMid + (int)(turnP*diff + turnD*diffRate);
			} else if (crossDetected) {
				curDeg = servoMid + (int)(straightP*diff + straightD*diffRate);
			} else {
				curDeg = servoMid + (int)(straightP*diff + straightD*diffRate);
			}
			if (curDeg > servoMid+servoRange) {
				curDeg = servoMid+servoRange;
			} else if (curDeg < servoMid-servoRange) {
				curDeg = servoMid-servoRange;
			}
			servo.SetDegree(curDeg);
			deg = arrAvg(degArr, degArrSize, degArrCounter, degTotal, curDeg-servoMid);

			//decelerate
//			targetSpeed = inputTargetSpeed * (1-speedDecP*pow(max(((curDeg-servoMid)*(curDeg-servoMid) - (prevDeg-servoMid)*(prevDeg-servoMid)), 0.0), 3));
//			targetSpeed = inputTargetSpeed - speedDecP*max(((curDeg-servoMid)*(curDeg-servoMid) - (prevDeg-servoMid)*(prevDeg-servoMid)), 0.0);
			targetSpeed = inputTargetSpeed;
			if (roundState == 1 || roundState == 3) {
				targetSpeed *= roundSpeedDec;
			}
			targetSpeed *= (1 - speedDecP*max(((curDeg-servoMid)*(curDeg-servoMid) - deg*deg), 0.0) - speedDecP2*fabs(min((midLeft+midRight - midSum), 0.0)) - speedDecP3*max(fabs(frontLeft-frontRight)-frontDiv, 0.0));
			if (targetSpeed < inputTargetSpeed*speedDecLim) {
				targetSpeed = inputTargetSpeed*speedDecLim;
			}

//			targetSpeed = inputTargetSpeed;

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
				sprintf(dataChar, "%.1f,%.3f,%.3f=%.3f,%.3f,%.3f,%.3f\n", 1.0, (float)curDeg-servoMid, (float)roundState*1000, frontLeft, midLeft, midRight, frontRight);
//				sprintf(dataChar, "%.1f,%.3f,%.3f=%.3f,%.3f,%.3f,%.3f\n", 1.0, (float)midLeft, (float)midRight, frontLeft, midLeft, midRight, frontRight);
				sprintf(dataChar, "%.1f,%.3f,%.3f=%.3f,%.3f,%.3f,%.3f\n", 1.0, (float)diff, (float)roundState*1000, frontLeft, midLeft, midRight, frontRight);
//				temp = sqrt(frontLeft*frontLeft+midLeft*midLeft);
//				temp1 = sqrt(frontRight*frontRight+midRight*midRight);
//				sprintf(dataChar, "%.1f,%.3f,%.3f=%.3f,%.3f,%.3f,%.3f\n", 1.0, (float)temp, (float)temp1, frontLeft, midLeft, midRight, frontRight);
				string dataStr = dataChar;

				const Byte startByte = 85;
				bluetooth1.SendBuffer(&startByte, 1);
				bluetooth1.SendStr(dataStr);
			}
		}
	}

	return 0;
}
