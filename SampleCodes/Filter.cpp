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

void Buffer1Dto2D(const Byte* Buffer1D){

	for (int i = 0; i < CameraW * CameraH / 8; i++){
		for (int j = 0; j < 8; j++){
			Buffer2D[(i*8+j)%CameraW][i/(CameraW/8)] = (Buffer1D[i] >> (7-j)) & 1;
		}
	}

}

void Print2D(){

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

void Filter2D(){
	for (int j = 1; j < CameraH-1; j++){
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

int main(void)
{
	System::Init();
	St7735r::Config ConfigLCD;
	St7735r Lcd(ConfigLCD);
	LCDptr = &Lcd;
	Ov7725::Config ConfigCam;
	ConfigCam.id = 0;
	ConfigCam.w = CameraW;
	ConfigCam.h = CameraH;
	Ov7725 Camera(ConfigCam);
	const Byte* BufferTemp;
	Timer::TimerInt time_img = 0;
	Camera.Start();
	while(1){
		while (time_img != System::Time()){
			time_img = System::Time();
			/*if (time_img % 10 == 0){
				Lcd.SetRegion(Lcd::Rect(0,0,80,60));
				BufferTemp = Camera.LockBuffer();
				Lcd.FillBits(libutil::GetRgb565(255,255,255), libutil::GetRgb565(0, 0, 255), BufferTemp, Camera.GetBufferSize()*8);
				Camera.UnlockBuffer();
			}*/
			if (time_img % 100 == 43){
				Lcd.SetRegion(Lcd::Rect(0,0,80,60));
				Lcd.FillBits(libutil::GetRgb565(255,255,255), libutil::GetRgb565(0, 0, 255), Camera.LockBuffer(), Camera.GetBufferSize()*8);
				Buffer1Dto2D(Camera.LockBuffer());
				Filter2D();
				Print2D();
				Camera.UnlockBuffer();
			}
		}
	}

	return 0;
}


