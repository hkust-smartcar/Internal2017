
#include <cassert>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <cstdint>
//#include <stdint.h>
#include <inttypes.h>
#include <sstream>
#include <cmath>
#include <array>
#include <libsc/system.h>
#include <libbase/k60/mcg.h>
#include <libutil/string.h>
#include <libutil/misc.h>
#include <libsc/tower_pro_mg995.h>
#include <libbase/k60/pit.h>
#include<fstream>

#include <libsc/led.h>
//#include <libsc/k60/uart_device.h>
//#include <libbase/k60/uart.h>
#include <libsc/k60/ov7725.h>

#include <libsc/alternate_motor.h>

#include <libsc/dir_encoder.h>

#include <libsc/button.h>

#include <libsc/joystick.h>

//#include <libsc/k60/jy_mcu_bt_106.h>



//#include <libsc/mpu6050.h>
#include "CameraHeader/FindEdge.h"
#include "libsc/futaba_s3010.h"

namespace libbase
{
    namespace k60
    {

        Mcg::Config Mcg::GetMcgConfig()
        {
            Mcg::Config config;
            config.external_oscillator_khz = 50000;  //50000
            config.core_clock_khz = 150000;			//150000
            return config;
        }

    }
}


using namespace libsc;
using namespace libbase::k60;
using namespace libsc::k60;
using namespace std;

bool q[4800];
bool CameraBin[60][80];
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
//TrackState T;



void PrintEdges(const Point* LeftEdge, const Point* RightEdge, const uint8_t LeftEdgeNum,const uint8_t RightEdgeNum,Point* LeftCorner[], Point* RightCorner[]){
	for(int i = 0 ; i < RightEdgeNum ; i ++){
		tft->SetRegion(Lcd::Rect(RightEdge[i].x+1,RightEdge[i].y+1,1,1));
		tft->FillColor(Lcd::kBlack);
	}
	for(int i = 0 ; i < LeftEdgeNum; i ++){
		tft->SetRegion(Lcd::Rect(LeftEdge[i].x+1,LeftEdge[i].y+1,1,1));
		tft->FillColor(Lcd::kRed);
	}
	for(int i = 0 ; i < 2 ; i ++){
		if(LeftCorner[i]){
			tft->SetRegion(Lcd::Rect(LeftCorner[i]->x,LeftCorner[i]->y, 2 ,2));
			tft->FillColor(Lcd::kBlue);
		}
		if(RightCorner[i]){
			tft->SetRegion(Lcd::Rect(RightCorner[i]->x,RightCorner[i]->y, 2 ,2));
			tft->FillColor(Lcd::kBlack);
		}
	}

}








int main(){
 	System::Init();




	Ov7725::Config camera_config;
	camera_config.id = 0;
	camera_config.w = 80;
	camera_config.h = 60;
	camera_config.fps = Ov7725Configurator::Config::Fps::kHigh;
	Ov7725 CAMERA(camera_config);

	JyMcuBt106::Config bluetooth_config;
	bluetooth_config.id=0;
	bluetooth_config.baud_rate=libbase::k60::Uart::Config::BaudRate::k115200;
	JyMcuBt106 BT(bluetooth_config);
	bluetooth=&BT;

	FutabaS3010::Config servo_config;
	servo_config.id = 0;
	FutabaS3010 SERVO(servo_config);
	SERVO.SetDegree(730);  //730  1000

	Led::Config led_config;
	led_config.id=0;
	led_config.is_active_low=false;
	Led led1(led_config);
//
//	DirEncoder::Config Lconfig;
//	Lconfig.id = 0;
//	DirEncoder LENCODER(Lconfig);
//	DirEncoder::Config Rconfig;
//	Rconfig.id = 1;
//	DirEncoder RENCODER(Rconfig);


	led_config.id=1;
	led_config.is_active_low=false;
	Led led2(led_config);
//
//	St7735r::Config tft_config;
//	tft_config.is_revert = true;
//	St7735r	TFT(tft_config);
//
//	TFT.SetRegion(Lcd::Rect(1,1,80,60));
//	tft = &TFT;
//
//
//	LcdTypewriter::Config writer_config;
//	writer_config.lcd = &TFT;
//	LcdTypewriter TYPER(writer_config);
//	typer = &TYPER;

	AlternateMotor::Config Lmotor_config;
	Lmotor_config.id = 0;
	AlternateMotor::Config Rmotor_config;
	Rmotor_config.id = 1;
	AlternateMotor LMOTOR(Lmotor_config);
	AlternateMotor	RMOTOR(Rmotor_config);
	LMOTOR.SetClockwise(true);
	RMOTOR.SetClockwise(false);
	LMOTOR.SetPower(250);
	RMOTOR.SetPower(250);




	uint32_t CurTime = System::Time(), PreTime = System::Time();
	while(System::Time() - PreTime < 1500);

	CAMERA.Start();
	PreTime = System::Time();
	int count=0;
	float predif = 0;
	bool T = true;
	float dif=0;
	float kp = 0.1;
	char buffer[100];
	while(T){
		CurTime = System::Time();

		if(CurTime - PreTime >= 10){
			PreTime = CurTime;

			const Byte* image= CAMERA.LockBuffer();
			CAMERA.UnlockBuffer();

//			led1.Switch();
			led2.Switch();

//			TFT.SetRegion(Lcd::Rect(1,1,80,60));
//			TFT.FillBits(Lcd::kYellow,Lcd::kWhite,image,4800);
//			TFT.FillColor(Lcd::kBlue);
			FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
//			ModifyEdge(image,LeftEdge, RightEdge,LeftEdgeNum, RightEdgeNum,LeftCornerOrder,RightCornerOrder,LeftCorner, RightCorner);
			dif = FindPath(LeftEdge,RightEdge,ModifyEdge(image,LeftEdge, RightEdge,LeftEdgeNum, RightEdgeNum,LeftCornerOrder,RightCornerOrder,LeftCorner,RightCorner),LeftCorner,RightCorner,LeftEdgeNum, RightEdgeNum);
			int16_t servo_power;
//			PrintEdges(LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum,LeftCorner,RightCorner);
			servo_power = 700 - dif;

			if(servo_power > 1000)
				SERVO.SetDegree(1000);
			else if(servo_power > 900)
				SERVO.SetDegree(900);
			else if(servo_power > 800)
				SERVO.SetDegree(800);
			else if(servo_power > 750)
				SERVO.SetDegree(750);
			else if(servo_power > 650)
				SERVO.SetDegree(700);
			else if(servo_power < 400)
				SERVO.SetDegree(400);
			else if(servo_power < 500)
				SERVO.SetDegree(500);
			else if(servo_power < 600)
				SERVO.SetDegree(600);
			else
				SERVO.SetDegree(650);
//
			if(servo_power > 800){
				LMOTOR.SetPower(200);
				RMOTOR.SetPower(250);
			}
			else if(servo_power<600){
				LMOTOR.SetPower(250);
				RMOTOR.SetPower(200);
			}
			else{
				LMOTOR.SetPower(250);
				RMOTOR.SetPower(250);
			}


//			else
//				SERVO.SetDegree(400);
//			else
//				SERVO.SetDegree(servo_power);
//			SERVO.SetDegree(400);//1050         530   700      1030    290
//			sprintf(buffer,"%f",dif);
//			TFT.SetRegion(Lcd::Rect(50,100,50,20));
//			TYPER.WriteString(buffer);
//			bluetooth->SendStr(buffer);

//			for(int i = 47 ; i < 60; i+=2 ){
//				tft->SetRegion(Lcd::Rect(39+(59-i)*dif/12,i,2,2));
//				tft->FillColor(Lcd::kCyan);
//			}

			//  MOTOR PROTECTION
//			LENCODER.Update();
//			RENCODER.Update();
//			int L  = LENCODER.GetCount();
//			int R = RENCODER.GetCount();
//			if(L< 5 && L> -5 &&R< 5 && R> -5  ){
//				LMOTOR.SetPower(0);
//				RMOTOR.SetPower(0);
//				T = false;
//			}


			//SHOW TRACK STATE
//			tft->SetRegion(Lcd::Rect(1,90,20,10));
//			switch(T){
//			case  Normal:
//				TYPER.WriteString("Normal");
//				break;
//			case  Crossroad:
//				TYPER.WriteString("Crossroad");
//				break;
//			case Roundabout:
//				TYPER.WriteString("Roundabout");
//				break;
//			}

			}
		}
	CAMERA.Stop();


return 0;

}


















//
//
//
////Mkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
//
//#include <cassert>
//#include <libbase/k60/mcg.h>
//#include <libsc/system.h>
//
//#include <cstring>
//#include <cmath>
//#include <vector>
//#include <list>
//#include <array>
//#include <sstream>
//#include <cstring>
//#include <stdio.h>
//
//#include <CameraHeader/FindEdge.h>
//
////LED
//#include <libsc/led.h>
//
////Button
////#include <libsc/button.h>
//
////LCD
////#include <libsc/st7735r.h>
//
////Camera
//#include <libsc/k60/ov7725.h>
//
////Motor
//#include <libsc/alternate_motor.h>
//
////Encoder
//#include <libsc/dir_encoder.h>
//
////Servo
//#include <libsc/futaba_s3010.h>
//
////Bluetooth
//#include <libsc/k60/jy_mcu_bt_106.h>
//
////Accel and Gyro
//#include <libsc/mpu6050.h>
//
//namespace libbase
//{
//	namespace k60
//	{
//
//		Mcg::Config Mcg::GetMcgConfig()
//		{
//			Mcg::Config config;
//			config.external_oscillator_khz = 50000;
//			config.core_clock_khz = 150000;
//			return config;
//		}
//
//	}
//}
//
//using namespace std;
//using namespace libsc;
//using namespace libbase::k60;
//using namespace libsc::k60;
//
////program...
//
//const int globalWidth = 80;
//const int globalHeight = 60;
//
//bool pixelArray[globalHeight][globalWidth];
//bool boundaryArray[globalHeight][globalWidth];
//
////Object pointer
//AlternateMotor * leftMotorPtr;
//AlternateMotor * rightMotorPtr;
//DirEncoder * encoderLeftPtr;
//DirEncoder * encoderRightPtr;
//JyMcuBt106 * btPtr;
//FutabaS3010 * servoPtr;
//
////BT listener
//string inputStr;
//bool tune = false;
//vector<float> constVector;
//
////Constants
//float balAngle = 45;
//float forRange = 3, backRange = 2;
//float inputTargetSpeed = 0;
//float leftPowSpeedP = 0, leftPowSpeedI = 0, leftPowSpeedD = 0;
//float rightPowSpeedP = 0, rightPowSpeedI = 0, rightPowSpeedD = 0;
//float speedAngP = 0, speedAngI = 0, speedAngD = 0;
//float targetAngSpeedP = 0, targetAngSpeedI = 0, targetAngSpeedD = 0;
//float diffP = 0;
//float sumAngErrLim = 0, sumSpeedErrLim = 0;
//
//float sumAngErr = 0, sumSpeedErr = 0, sumLeftSpeedErr = 0, sumRightSpeedErr = 0;
//float targetSpeed = 0, differential = 0;
//bool programRun = true, camEnable = true;
//
//void getImage(const Byte * data) {
//
//	bool previous;
//	int pos = 0, bit_pos = 8;
//	int height = globalHeight, width = globalWidth;
//
//	for (int y = 0; y < height; y++) {
//
//		for (int x = 0; x < width; x++) {
//
//			if (--bit_pos < 0) {
//				bit_pos = 7;
//				++pos;
//			}
//			if (GET_BIT(data[pos], bit_pos)) {
//				pixelArray[y][x] = 1;
//			} else {
//				pixelArray[y][x] = 0;
//			}
//
//		}
//
//	}
//
//	for (int y = 0; y < height; y++) {
//
//		previous = pixelArray[y][0];
//
//		for (int x = 0; x < width; x++) {
//
//			if (pixelArray[y][x] != previous) {
//
//				if (pixelArray[y][x] == 0) {
//					boundaryArray[y][x] = 1;
//				} else {
//					boundaryArray[y][x-1] = 1;
//					boundaryArray[y][x] = 0;
//				}
//				previous = pixelArray[y][x];
//
//			} else {
//				boundaryArray[y][x] = 0;
//			}
//
//		}
//
//	}
//
//	for (int x = 0; x < width; x++) {
//
//		previous = pixelArray[0][x];
//
//		for (int y = 0; y < height; y++) {
//
//			if (pixelArray[y][x] != previous) {
//
//				if (pixelArray[y][x] == 0) {
//					boundaryArray[y][x] = 1;
//				} else {
//					boundaryArray[y-1][x] = 1;
//					boundaryArray[y][x] = 0;
//				}
//				previous = pixelArray[y][x];
//
//			}
//
//		}
//
//	}
//
//}
//
//float arrAvg(float arr[], int size, int& counter, float& total, float newVal) {
//
//	total -= arr[counter];
//	arr[counter] = newVal;
//	total += arr[counter];
//	counter++;
//	if (counter == size) {
//		counter = 0;
//	}
//	return total/size;
//}
//
////bool bluetoothListener(const Byte *data, const size_t size) {
////
//////	const Byte inputByte = 171;
//////	btPtr->SendBuffer(&inputByte, 1);
//////	str += '\n';
//////	btPtr->SendStr(str);
////
////	if (data[0] == 'w') {
////		targetSpeed = inputTargetSpeed;
////	} else if (data[0] == 'W') {
////		targetSpeed = 0;
////	}
////	if (data[0] == 's') {
////		targetSpeed = -inputTargetSpeed;
////	} else if (data[0] == 'S') {
////		targetSpeed = 0;
////	}
////	if (data[0] == 'a') {
////		if (targetSpeed == 0) {
////			targetSpeed = inputTargetSpeed/2;
////			differential = 1;
////		} else {
////			differential = 0.5;
////		}
////	} else if (data[0] == 'A') {
////		if (targetSpeed == inputTargetSpeed/2) {
////			targetSpeed = 0;
////		}
////		differential = 0;
////	}
////	if (data[0] == 'd') {
////		if (targetSpeed == 0) {
////			targetSpeed = inputTargetSpeed/2;
////			differential = -1;
////		} else {
////			differential = -0.5;
////		}
////	} else if (data[0] == 'D') {
////		if (targetSpeed == inputTargetSpeed/2) {
////			targetSpeed = 0;
////		}
////		differential = 0;
////	}
////
////	if (data[0] == 'P') {
////		programRun = 0;
////	}
////
////	if (data[0] == 't') {
////		programRun = 1;
////		tune = 1;
////		inputStr = "";
////	}
////	if (tune) {
////		int i = 0;
////		while (i<size) {
////			if (data[i] != 't' && data[i] != '\n') {
////				inputStr += (char)data[i];
////			} else if (data[i] == '\n') {
////				tune = 0;
////			}
////			i++;
////		}
////		if (!tune) {
////			constVector.clear();
////
////			char * pch;
////			pch = strtok(&inputStr[0], " ,");
////			while (pch != NULL){
////				float constant;
////				stringstream(pch) >> constant;
////				constVector.push_back(constant);
////				pch = strtok (NULL, " ,");
////			}
////
////			balAngle = constVector[0];
////			forRange = constVector[1];
////			backRange = constVector[2];
////			inputTargetSpeed = constVector[3];
////			leftPowSpeedP = constVector[4];
////			leftPowSpeedI = constVector[5];
////			leftPowSpeedD = constVector[6];
////			rightPowSpeedP = constVector[7];
////			rightPowSpeedI = constVector[8];
////			rightPowSpeedD = constVector[9];
////			speedAngP = constVector[10];
////			speedAngI = constVector[11];
////			speedAngD = constVector[12];
////			targetAngSpeedP = constVector[13];
////			targetAngSpeedI = constVector[14];
////			targetAngSpeedD = constVector[15];
////			diffP = constVector[16];
////			sumAngErrLim = constVector[17];
////			sumSpeedErrLim = constVector[18];
////			camEnable = constVector[19];
////			sumSpeedErr = 0;
////			sumLeftSpeedErr = 0;
////			sumRightSpeedErr = 0;
////			sumAngErr = 0;
////		}
////	}
////
//////	else if (data[0] == 'a') {
//////		servoPtr->SetDegree(900);
//////	} else if (data[0] == 'd') {
//////		servoPtr->SetDegree(430);
//////	} else if (data[0] == 'A' || data[0] == 'D') {
//////		servoPtr->SetDegree(700);
//////	}
////
////}
//
//Point LeftEdge[100], RightEdge[100];
//uint8_t LeftEdgeNum;
//uint8_t RightEdgeNum;
//uint8_t LeftCornerOrder, RightCornerOrder;
//
//
//int main(void) {
//
//	System::Init();
//
////Loop
//	long startTime = System::Time();
//	long currentTime = 0;
//	int loopCounter = 0;
//
////Camera
//	const Byte * camInput;
//	int height = globalHeight, width = globalWidth;
//
////Speed
//	long prevSampleTime = System::Time();
//	float dt = 0;
//
//	int leftPow = 0, rightPow = 0;
//	bool leftForward = 1, rightForward = 0;
//
//	float curSpeed = 0;
//	float speedErr = 0, prevSpeedErr = 0;
//	float speedErrRate = 0;
//	int speedErrRateCounter = 0, speedErrRateArrSize = 5;
//	float speedErrRateTotal = 0;
//	float speedErrRateArr[speedErrRateArrSize];
//
//	float leftSpeed = 0, rightSpeed = 0;
//	float leftTempTargetSpeed = 0, rightTempTargetSpeed = 0, tempTargetSpeed = 0;
//	int leftSpeedArrCounter = 0, rightSpeedArrCounter = 0, speedArrSize = 10;
//	float leftSpeedTotal = 0, rightSpeedTotal = 0;
//	float leftSpeedArr[speedArrSize], rightSpeedArr[speedArrSize];
//	for (int i = 0; i < speedArrSize; i++){
//		leftSpeedArr[i] = 0;
//		rightSpeedArr[i] = 0;
//	}
//
//	float leftSpeedErr = 0, prevLeftSpeedErr = 0;
//	float rightSpeedErr = 0, prevRightSpeedErr = 0;
//	float leftSpeedErrRate = 0, rightSpeedErrRate = 0;
//	int leftSpeedErrRateCounter = 0, rightSpeedErrRateCounter = 0;
//	float leftSpeedErrRateTotal = 0, rightSpeedErrRateTotal = 0;
//	float leftSpeedErrRateArr[speedErrRateArrSize], rightSpeedErrRateArr[speedErrRateArrSize];
//
//	for (int i = 0; i < speedArrSize; i++){
//		leftSpeedErrRateArr[i] = 0;
//		rightSpeedErrRateArr[i] = 0;
//		speedErrRateArr[i] = 0;
//	}
//
////Angle
//	float acc[3];
//	float accbine = 0;
//	float accAng = 0, gyroAng = 0;
//
//	float curAng = 0;
//	float targetAng = balAngle;
//	int angCounter = 0, angArrSize = 10;
//	float angTotal = balAngle*angArrSize;
//	float angArr[angArrSize];
//	for (int i = 0; i < angArrSize; i++){
//		angArr[i] = balAngle;
//	}
//
//	float curAngErr = 0, prevAngErr = 0, angErrRate = 0;
//	int angErrRateCounter = 0, angErrRateArrSize = 5;
//	float angErrRateArr[angErrRateArrSize];
//	float angErrRateTotal = 0;
//	for (int i = 0; i < angErrRateArrSize; i++){
//		angErrRateArr[i] = 0;
//	}
//
////	Led::Config LedConfig;
////	LedConfig.id = 0;
////	Led led1(LedConfig);
//
////	St7735r::Config LcdConfig;
////	LcdConfig.is_revert = true;
////	St7735r Lcd1(LcdConfig);
//
////	FutabaS3010::Config ServoConfig;
////	ServoConfig.id = 0;
////	FutabaS3010 servo1(ServoConfig);
////	servoPtr = &servo1;
//
//	Ov7725::Config CamConfig;
//	CamConfig.id = 0;
//	CamConfig.w = width;
//	CamConfig.h = height;
//	CamConfig.fps = Ov7725Configurator::Config::Fps::kHigh;
//	Ov7725 Cam1(CamConfig);
//
//	JyMcuBt106::Config BluetoothConfig;
//	BluetoothConfig.id = 0;
//	BluetoothConfig.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
////	BluetoothConfig.rx_isr = &bluetoothListener;
//	JyMcuBt106 bluetooth1(BluetoothConfig);
//	btPtr = &bluetooth1;
////	b = btPtr;
//
//	Mpu6050::Config mpuConfig;
//	mpuConfig.gyro_range=Mpu6050::Config::Range::kSmall;
//	mpuConfig.accel_range=Mpu6050::Config::Range::kSmall;
//	Mpu6050 mpu(mpuConfig);
//
//	AlternateMotor::Config LeftMotorConfig;
//	LeftMotorConfig.id = 1;
//	AlternateMotor motorLeft(LeftMotorConfig);
//	leftMotorPtr = &motorLeft;
//	AlternateMotor::Config RightMotorConfig;
//	RightMotorConfig.id = 0;
//	AlternateMotor motorRight(RightMotorConfig);
//	rightMotorPtr = &motorRight;
//
//	DirEncoder::Config LeftEncoderConfig;
//	LeftEncoderConfig.id = 0;
//	DirEncoder encoderLeft(LeftEncoderConfig);
//	encoderLeftPtr = &encoderLeft;
//	DirEncoder::Config RightEncoderConfig;
//	RightEncoderConfig.id = 1;
//	DirEncoder encoderRight(RightEncoderConfig);
//	encoderRightPtr = &encoderRight;
//
////	Cam1.Start();
//	motorLeft.SetClockwise(leftForward);
//	motorRight.SetClockwise(rightForward);
//	motorLeft.SetPower(0);
//	motorRight.SetPower(0);
//
//	Cam1.Start();
////	float temp = 0;
//
//	balAngle = 44;
//	forRange = 2;
//	backRange = 1;
//	inputTargetSpeed = 6000;
//	leftPowSpeedP = 0.001;
//	leftPowSpeedI = 0.001;
//	leftPowSpeedD = 0.0001;
//	rightPowSpeedP = 0.001;
//	rightPowSpeedI = 0.001;
//	rightPowSpeedD = 0.0002;
//	speedAngP = 20000;
//	speedAngI = 1200;
//	speedAngD = 400;
//	targetAngSpeedP = -0.000015;
//	targetAngSpeedI = 0;
//	targetAngSpeedD = 0;
//	diffP = 1;
//	sumAngErrLim = 6;
//	sumSpeedErrLim = 500000;
//	camEnable = 1;
//	System::DelayMs(3000);
//
//	while(1) {
//
//		currentTime = System::Time();
//
//		if (currentTime-startTime >= 5) {
//			startTime = currentTime;/////////////////////////////
//			loopCounter++;                  ///////////////////////////////
//			const Byte* image = Cam1.LockBuffer();
//			Cam1.UnlockBuffer();
//			if (camEnable) {
//				FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
//				differential = FindPath(LeftEdge, RightEdge,ModifyEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum,LeftCornerOrder,RightCornerOrder),LeftCornerOrder,RightCornerOrder);
//				differential = diffP*(differential-0.5);
//			}
//
//			if (programRun == false) {
//				motorLeft.SetPower(0);
//				motorRight.SetPower(0);
//				continue;
//			}
//
//
//
//			dt = (float)(System::Time()-prevSampleTime)/1000;
//			prevSampleTime = System::Time();
//
//			//Speed
//			encoderLeft.Update();
//			leftSpeed = arrAvg(leftSpeedArr, speedArrSize, leftSpeedArrCounter, leftSpeedTotal, 1*encoderLeft.GetCount()/dt);
//			encoderRight.Update();
//			rightSpeed = arrAvg(rightSpeedArr, speedArrSize, rightSpeedArrCounter, rightSpeedTotal, -1*encoderRight.GetCount()/dt);
//			curSpeed = (leftSpeed+rightSpeed)/2;
//
//			//Angle
//			mpu.Update(1);
//			acc[0] = mpu.GetAccel()[0];
//			acc[1] = mpu.GetAccel()[1];
//			acc[2] = mpu.GetAccel()[2];
//			accbine = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
//			accAng = (acos(acc[0]/accbine)*180)/3.1415;
//			gyroAng = curAng + (mpu.GetOmega()[1]/160/131)*dt;
//			curAng = arrAvg(angArr, angArrSize, angCounter, angTotal, abs((0.98*gyroAng)+(0.02*accAng)));
////			if (System::Time() >= 5000 && (curAng >= balAngle+20 || curAng <= balAngle-10)) {
////				break;
////			}
//
//			//TargetAng-speed PID
//			speedErr = curSpeed - targetSpeed;
//			sumSpeedErr += speedErr * dt;
//			if (sumSpeedErr > 2) {
//				sumSpeedErr = 2;
//			} else if (sumSpeedErr < -1*2) {
//				sumSpeedErr = -1*2;
//			}
//			speedErrRate = arrAvg(speedErrRateArr, speedErrRateArrSize, speedErrRateCounter, speedErrRateTotal, (speedErr-prevSpeedErr) / dt);
//			prevSpeedErr = speedErr;
//			targetAng += targetAngSpeedP * speedErr + targetAngSpeedI * sumSpeedErr + targetAngSpeedD * speedErrRate;
//			if (targetAng-balAngle > forRange) {
//				targetAng = balAngle + forRange;
//			} else if (targetAng-balAngle < -backRange) {
//				targetAng = balAngle - backRange;
//			}
//
////			targetAng = balAngle;
//
//			//tempSpeed-angle PID
//			curAngErr = curAng - targetAng;
//			sumAngErr += curAngErr * dt;
//			if (sumAngErr > sumAngErrLim) {
//				sumAngErr = sumAngErrLim;
//			} else if (sumAngErr < -1*sumAngErrLim) {
//				sumAngErr = -1*sumAngErrLim;
//			}
//			angErrRate = arrAvg(angErrRateArr, angErrRateArrSize, angErrRateCounter, angErrRateTotal, (curAngErr-prevAngErr) / dt);
//			prevAngErr = curAngErr;
//			tempTargetSpeed = speedAngP * curAngErr + speedAngI * sumAngErr + speedAngD * angErrRate;
//			leftTempTargetSpeed = tempTargetSpeed * (1+differential);
//			rightTempTargetSpeed = tempTargetSpeed * (1-differential);
//
////			leftTempTargetSpeed = rightTempTargetSpeed = 5000*sin(temp);
////			temp += 0.02;
////			leftTempTargetSpeed = rightTempTargetSpeed = inputTargetSpeed;
//
//			//Power-speed PID
//			leftSpeedErr = leftSpeed - leftTempTargetSpeed;
//			sumLeftSpeedErr += leftSpeedErr * dt;
//			if (sumLeftSpeedErr > sumSpeedErrLim) {
//				sumLeftSpeedErr = sumSpeedErrLim;
//			} else if (sumLeftSpeedErr < -1*sumSpeedErrLim) {
//				sumLeftSpeedErr = -1*sumSpeedErrLim;
//			}
//			leftSpeedErrRate = arrAvg(leftSpeedErrRateArr, speedErrRateArrSize, leftSpeedErrRateCounter, leftSpeedErrRateTotal, (leftSpeedErr-prevLeftSpeedErr) / dt);
//			prevLeftSpeedErr = leftSpeedErr;
//			leftPow += leftPowSpeedP * leftSpeedErr + leftPowSpeedI * sumLeftSpeedErr + leftPowSpeedD * leftSpeedErrRate;
//
//			rightSpeedErr = rightSpeed - rightTempTargetSpeed;
//			sumRightSpeedErr += rightSpeedErr * dt;
//			if (sumRightSpeedErr > sumSpeedErrLim) {
//				sumRightSpeedErr = sumSpeedErrLim;
//			} else if (sumRightSpeedErr < -1*sumSpeedErrLim) {
//				sumRightSpeedErr = -1*sumSpeedErrLim;
//			}
//			rightSpeedErrRate = arrAvg(rightSpeedErrRateArr, speedErrRateArrSize, rightSpeedErrRateCounter, rightSpeedErrRateTotal, (rightSpeedErr-prevRightSpeedErr) / dt);
//			prevRightSpeedErr = rightSpeedErr;
//			rightPow += rightPowSpeedP * rightSpeedErr + rightPowSpeedI * sumRightSpeedErr + rightPowSpeedD * rightSpeedErrRate;
//
//			if (leftPow > 500) {
//				leftPow = 500;
//			} else if (leftPow < -500) {
//				leftPow = -500;
//			}
//			if (rightPow > 500) {
//				rightPow = 500;
//			} else if (rightPow < -500) {
//				rightPow = -500;
//			}
//
////			leftPow = 0;
////			rightPow = 0;
//
//			if (leftPow > 0) {
//				motorLeft.SetClockwise(!leftForward);
//				motorLeft.SetPower(leftPow);
//			} else {
//				motorLeft.SetClockwise(leftForward);
//				motorLeft.SetPower(-1*leftPow);
//			}
//			if (rightPow > 0) {
//				motorRight.SetClockwise(!rightForward);
//				motorRight.SetPower(rightPow);
//			} else {
//				motorRight.SetClockwise(rightForward);
//				motorRight.SetPower(-1*rightPow);
//			}
//
////			motorLeft.SetClockwise(leftForward);
////			motorLeft.SetPower(300);
////			motorRight.SetClockwise(rightForward);
////			motorRight.SetPower(300);
//
////			if (loopCounter%3 == 0) {
////				char speedChar[15] = {};
////				sprintf(speedChar, "%.1f,%.2f,%.2f,%.2f=%.2f,%.2f\n", 1.0, leftSpeed, rightSpeed, leftTempTargetSpeed, curAng, targetAng);
////				string speedStr = speedChar;
////
////				const Byte speedByte = 85;
////				bluetooth1.SendBuffer(&speedByte, 1);
////				bluetooth1.SendStr(speedStr);
////			}
//
////			if (Cam1.IsAvailable()) {
////
////				camInput = Cam1.LockBuffer();
////
////				getImage(camInput);
////
////				Cam1.Start();
////				Cam1.UnlockBuffer();
////
////			}
////
////			if (loopCounter%40 == 0) {
////				loopCounter = 0;
////
////				const Byte imageByte = 170;
////				bluetooth1.SendBuffer(&imageByte, 1);
////				bluetooth1.SendBuffer(camInput, Cam1.GetBufferSize());
////			}
//
//		}
//
//	}
//	Cam1.Stop();
//	return 0;
//}
