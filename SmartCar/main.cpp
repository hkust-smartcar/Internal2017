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

//Button Header File
#include<libsc/button.h>

//led Header File
#include <libsc/led.h>

//LCD Header File
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>

//Camera Header File
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

const int CamHeight=60;
const int CamWidth=80;

//Change Camera Buffer from 1D array to 2D array----------------------------------------------------------------------------
//row is row number
//col is column number
//CamByte is the Byte CameraBuffer
int Cam2DArray[CamWidth][CamHeight];
void Camera2DConverter(const Byte* CameraBuffer){
	Byte CamByte;
	for(Uint row=0;row<CamHeight;row++){
		for(Uint col=0;col<CamWidth;col+=8){
			CamByte=CameraBuffer[row*CamWidth/8+col/8];
			Cam2DArray[row][col]=CamByte&0x80;
			Cam2DArray[row][col+1]=CamByte&0x40;
			Cam2DArray[row][col+2]=CamByte&0x20;
			Cam2DArray[row][col+3]=CamByte&0x10;
			Cam2DArray[row][col+4]=CamByte&0x08;
			Cam2DArray[row][col+5]=CamByte&0x04;
			Cam2DArray[row][col+6]=CamByte&0x02;
			Cam2DArray[row][col+7]=CamByte&0x01;
		}
	}
}

//image filter
void CameraFilter(){
	Uint temp;
	for(Uint row=1;row<(CamHeight-1);row++){
		for(Uint col=1;col<(CamWidth-1);col++){
			temp=temp+Cam2DArray[row-1][col-1]+Cam2DArray[row-1][col]+Cam2DArray[row-1][col+1]
					 +Cam2DArray[row][col-1]+Cam2DArray[row][col]+Cam2DArray[row][col+1]
					 +Cam2DArray[row+1][col-1]+Cam2DArray[row+1][col]+Cam2DArray[row+1][col+1];
			if(temp>4){
				Cam2DArray[row-1][col-1]=1;
				Cam2DArray[row-1][col]=1;
				Cam2DArray[row-1][col+1]=1;
				Cam2DArray[row][col-1]=1;
				Cam2DArray[row][col]=1;
				Cam2DArray[row][col+1]=1;
			    Cam2DArray[row+1][col-1]=1;
			    Cam2DArray[row+1][col]=1;
			    Cam2DArray[row+1][col+1]=1;
			}
		}
	}
}

int main(void)
{
	System::Init();

	St7735r::Config ConfigLCD;
	ConfigLCD.is_revert=false;
	ConfigLCD.is_bgr=false;
	ConfigLCD.fps=60;
	St7735r LCD(ConfigLCD);

	Ov7725::Config ConfigCam;
	ConfigCam.id=0;
	ConfigCam.h=CamHeight;
	ConfigCam.w=CamWidth;
	Ov7725 Cam(ConfigCam);

	while(1){
		Cam.Start();
		if(Cam.IsAvailable()){
			Camera2DConverter(Cam.LockBuffer());
			CameraFilter();
			//print the 2D Array on the LCD
			for(int y=0;y<CamHeight;y++){
				for(int x=0;x<CamWidth;x++){
					LCD.SetRegion(Lcd::Rect(x,y,1,1));
					if(Cam2DArray[y][x]==0){
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
