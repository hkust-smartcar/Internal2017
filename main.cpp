/*
 * main.cpp
 *
 * Author: Peter
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cstring>
#include <cstdbool>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libsc/system.h>
#include <libsc/st7735r.h>
#include <libsc/k60/ov7725.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/pin.h>
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
	bool camera[4800]={0};
	int k=0;
	const Byte *LB=nullptr;
	System::Init();
	unsigned long long ticks_img=0;
	Ov7725::Config cc;
	cc.id=0;
	cc.w=80;
	cc.h=60;
	Ov7725 can(cc);
	St7735r::Config sc;
	sc.is_bgr=false;
	sc.is_revert=false;
	sc.fps=100;
	St7735r::Rect area(1,1,80,60);
	St7735r tft(sc);
	tft.SetRegion(area);
	can.Start();
	while(1){
		if(ticks_img!=System::Time()){
			ticks_img=System::Time();
			if(ticks_img%10==0){
				LB=can.LockBuffer();
				tft.ClearRegion();
				for(Uint i=0;i<can.GetBufferSize();i++){
					for(int j=0;j<8;j++)
						camera[k++]=(*(LB+i)>>j)&1;
				}
				can.UnlockBuffer();
				tft.FillBits(St7735r::kWhite,St7735r::kBlack,camera,sizeof(camera));
			}
		}
	}
	can.Stop();



	return 0;
}
