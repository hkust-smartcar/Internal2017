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
#include <libsc/st7735r.h>
#include <libsc/k60/ov7725.h>



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

	Ov7725::Config C;  //camera init;
	C.id = 0;
	C.w = 80;
	C.h = 60;
	Ov7725 cam(C);

	St7735r::Config s;
	s.is_revert = false;
	s.is_bgr = false; //screen init;
	s.fps = 100;
	St7735r screen(s);
	Timer::TimerInt t=0;

	cam.Start();
	while (true)
	{
		while(t!=System::Time())
		{
			t = System::Time();
			if(t % 10 ==0)
			{
				screen.SetRegion(Lcd::Rect(0,0,80,60));
				screen.FillBits(St7735r::kBlack,St7735r::kWhite,cam.LockBuffer(),8*cam.GetBufferSize());
				cam.UnlockBuffer();
			}

		}

	}
	cam.Stop();
	return 0;
}
