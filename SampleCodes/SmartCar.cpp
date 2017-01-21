/*
 * main.cpp
 *
 * Author: Peter
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libsc/k60/ov7725.h>
#include <libsc/system.h>
#include <libsc/st7735r.h>
#include <libutil/misc.h>
#include <libsc/alternate_motor.h>
#include <libsc/futaba_s3010.h>


#define CameraW 80
#define CameraH 60

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

using namespace libsc;
using namespace libbase::k60;
using namespace libsc::k60;

bool Buffer2D[CameraW][CameraH];
St7735r* LCDptr;
int ServoAngle = 700;
int newServoAngle;
int RoadArea[2] = {0, 0};
int RoadAreaDiff = 0;
int MotorDiff = 0;

void Buffer1Dto2D(const Byte* Buffer1D){

	for (int i = 0; i < CameraW * CameraH / 8; i++){
		for (int j = 0; j < 8; j++){
			Buffer2D[(i*8+j)%CameraW][i/(CameraW/8)] = (Buffer1D[i] >> (7-j)) & 1;
		}
	}

}

void Print2D(void){

	for (int y=0; y<CameraH; y++){
		for (int x=0; x<CameraW; x++){
			LCDptr->SetRegion(Lcd::Rect(x, y+CameraH+1, 1, 1));
			if (!Buffer2D[x][y]){
				LCDptr->FillColor(0xFFFF);
			} else {
				LCDptr->FillColor(0x001F);
			}
		}
	}

}

void Filter2D(void){
	//Filter Bottom Only
	for (int j = CameraH-30; j < CameraH-20; j++){
		for (int i = 1; i < CameraW-1; i++){
			int cnt = 0;
			for (int y = j-1; y < j+2; y++){
				for (int x = i-1; x < i+2; x++){
					cnt += (int)Buffer2D[x][y];
				}
			}
			if (cnt >= 5) {
				Buffer2D[i][j] = 1;
			} else {
				Buffer2D[i][j] = 0;
			}
		}
	}

}

void calcRoadArea(void){
	//Left: 0 to CameraW/2-1; Right: CameraW/2 to CameraW, assume 1 = road
	//[W][H]
	//Count Bottom only
	for (int i = 0; i < CameraW/2; i++){
		for (int j = CameraH-40; j < CameraH -20; j++){
			if (!Buffer2D[i][j]){
				RoadArea[0]++;
			}
		}
	}
	for (int i = CameraW/2; i < CameraW; i++){
		for (int j = CameraH-40; j < CameraH -20; j++){
			if (!Buffer2D[i][j]){
				RoadArea[1]++;
			}
		}
	}
}

int main(void)
{
	/* Inititation of Objects */
	System::Init();
	St7735r::Config ConfigLCD;
	ConfigLCD.is_revert=true;
	St7735r Lcd(ConfigLCD);
	LCDptr = &Lcd;

	Ov7725::Config ConfigCam;
	ConfigCam.id = 0;
	ConfigCam.w = CameraW;
	ConfigCam.h = CameraH;
	Ov7725 Camera(ConfigCam);

	FutabaS3010::Config ConfigServo;
	ConfigServo.id = 0;
	FutabaS3010 Servo(ConfigServo);

	AlternateMotor::Config ConfigMotor;
	ConfigMotor.id = 0;
	AlternateMotor Motor1(ConfigMotor);
	ConfigMotor.id = 1;
	AlternateMotor Motor2(ConfigMotor);

	Motor1.SetPower(250);

	Motor2.SetPower(250);
	Motor1.SetClockwise(false);
	Servo.SetDegree(ServoAngle);

	//const Byte* BufferTemp;
	Timer::TimerInt time_img = 0;

	Camera.Start();

	/* Main Loop */
	while(1){
		while (time_img != System::Time()){
			time_img = System::Time();

			if (time_img % 10 == 0){
				//Lcd.SetRegion(Lcd::Rect(0,0,80,60));
				//Lcd.FillBits(libutil::GetRgb565(0,0,255), libutil::GetRgb565(255, 255, 255), Camera.LockBuffer(), Camera.GetBufferSize()*8);
				Buffer1Dto2D(Camera.LockBuffer());
				Filter2D();
				//Print2D();
				calcRoadArea();
				RoadAreaDiff = RoadArea[0] - RoadArea[1];
				newServoAngle = ServoAngle + RoadAreaDiff * 1.24 ;
				if (newServoAngle > 900){
					newServoAngle = 900;
				} else if (newServoAngle < 430){
					newServoAngle = 430;
				}
				Servo.SetDegree(newServoAngle);
				MotorDiff = (int) (RoadAreaDiff * 0.08);
				Motor2.SetPower(275 - MotorDiff);
				Motor1.SetPower(275 + MotorDiff);




				//ServoAngle = Servo.GetDegree();

				RoadArea[0] = 0;
				RoadArea[1] = 0;
				Camera.UnlockBuffer();
			}
		}
	}

	return 0;
}


