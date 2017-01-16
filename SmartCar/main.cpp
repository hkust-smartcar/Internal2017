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
#include<libsc/button.h>
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>
#include<libsc/k60/ov7725.h>

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
using namespace libsc::k60;
using namespace libbase::k60;

int main(void)
{
	System::Init();

	St7735r::Config ConfigLCD;
	ConfigLCD.is_revert=false;
	ConfigLCD.is_bgr=false;
	ConfigLCD.fps=60;
	St7735r LCD(ConfigLCD);

	/*
	LcdConsole::Config ConfigConsole;
	ConfigConsole.lcd=&LCD;
	ConfigConsole.region=Lcd::Rect(0,0,128,80);
	LcdConsole Console(ConfigConsole);
	*/

	Ov7725::Config ConfigCam;
	ConfigCam.id=0;
	ConfigCam.h=60;
	ConfigCam.w=80;
	Ov7725 Cam(ConfigCam);

	//LCD.SetRegion(Lcd::Rect(0,0,80,60));

	Byte *CamBufferPointer;
	int CamBuffer;
	int CamArray[80][60];

	while(1){
		Cam.Start();
		if(Cam.IsAvailable()){
			//LCD.FillBits(0x001F,0xFFFF,Cam.LockBuffer(),Cam.GetBufferSize()*8);
			CamBufferPointer=(Byte*)(Cam.LockBuffer());
			CamBuffer=(int)CamBufferPointer;
			int i=7;
			int j=0;
			int k=0;
			while(CamBuffer>0){
				if(i>-1){
					CamArray[i+k][j]=CamBuffer%2;
					CamBuffer=CamBuffer/2;
					i=i-1;
				}else if(k>72){
					i=7;
					k=k+8;
					CamBufferPointer=(Byte*)(CamBufferPointer+1);
					CamBuffer=(int)CamBufferPointer;
				}else{
					j=j+1;
					k=0;
					if(j==61){
						break;
					}
				}
			}
			for(int y=0;y<61;y++){
				for(int x=0;x<81;x++){
					LCD.SetRegion(Lcd::Rect(x,y,1,1));
					if(CamArray[x][y]==0){
						LCD.FillColor(0x001F);
					}else{
						LCD.FillColor(0xFFFF);
					}
				}
			}
			Cam.UnlockBuffer();
		}
	}
}
