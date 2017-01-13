 /*
 * CamToLCD.cpp
 *
 * Author: mcreng
 * Copyright (c) 2016-2017 HKUST SmartCar Team
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
	St7735r::Config ConfigLCD;
	St7735r Lcd(ConfigLCD);
	Ov7725::Config ConfigCam;
	ConfigCam.id = 0;
	ConfigCam.w = 80;
	ConfigCam.h = 60;
	Ov7725 Camera(ConfigCam);
	Timer::TimerInt time_img = 0;
	Camera.Start();
	while(1){
		while (time_img != System::Time()){
			time_img = System::Time();
			if (time_img % 10 == 0){
				Lcd.SetRegion(Lcd::Rect(0,0,80,60));
				Lcd.FillBits(libutil::GetRgb565(0,0,255), libutil::GetRgb565(255, 255, 255), Camera.LockBuffer(),  Camera.GetBufferSize()*8);
				Camera.UnlockBuffer();
			}
		}
	}

	return 0;
}
