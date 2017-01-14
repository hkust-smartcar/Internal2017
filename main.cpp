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

#include <libsc/system.h> //timer.h is also included in system.h
#include <libsc/k60/ov7725.h>
#include <libsc/st7735r.h> //lcd.h is also included in st7735r.h

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

int main(void)
{
	System::Init();

	// start config
	Ov7725::Config imgConfig;
	imgConfig.id = 0;
	imgConfig.w = 80;
	imgConfig.h = 60;

	St7735r::Config lcdConfig;
	lcdConfig.is_bgr = false;
	lcdConfig.is_revert = false;
	lcdConfig.fps = 100; // update rate: 100 times/second = 1 time per 10ms
	//end of config

	//declare
	Ov7725 camera(imgConfig);
	St7735r tft(lcdConfig);
	Timer::TimerInt currentTime;
	//end of declare

	//main
	camera.Start();

	while(1){
		if(currentTime!=System::Time()){
			currentTime = System::Time();
			if(currentTime % 10 == 0){
				tft.SetRegion(Lcd::Rect(0,0,80,60)); //In this case, func setRegion is the same as clearRegion
				tft.FillBits(St7735r::kBlack,St7735r::kWhite,camera.LockBuffer(), camera.GetBufferSize()*8);
				camera.UnlockBuffer(); //change the buffer
			}
		}
	}

	camera.Stop();

	return 0;
}
