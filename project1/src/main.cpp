/*
 * main.cpp
 *
 * Author: Gordon
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/pin.h>
#include <libsc/st7735r.h>
#include <libsc/k60/ov7725.h>
#include <libsc/lcd.h>
#include <libsc/servo.h>
#include <libsc/alternate_motor.h>
#include <libsc/motor.h>

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

int main(void)
{
	System::Init();

	Led::Config config;
	config.id=0;
	Led led1(config);
	config.id=1;
	Led led2(config);
	config.id=2;
	Led led3(config);
	config.id=3;
	Led led4(config);

	led1.SetEnable(true);
	led2.SetEnable(false);
	led3.SetEnable(false);
	led4.SetEnable(true);


	AlternateMotor::Config motor_config;
	motor_config.id=1;
	AlternateMotor motor(motor_config);

	motor.SetPower(500);


	Servo::Config servo_config;
	servo_config.id=0;
	servo_config.period=40000;
	servo_config.min_pos_width=900;
	servo_config.max_pos_width=1500;
	Servo servo(servo_config);

	servo.SetDegree(1800);


	St7735r::Config lcd_config;
	St7735r lcd(lcd_config);

	k60::Ov7725::Config cam_config;
	cam_config.id=0;
	cam_config.w=120;
	cam_config.h=80;
	cam_config.fps = k60::Ov7725::Config::Fps::kHigh;
	k60::Ov7725 cam(cam_config);



	cam.Start();



	Lcd::Rect r(0,0,120,80);
	lcd.SetRegion(r);

	uint32_t time_img=0;
	while(true){
		if (System::Time()%250==0){
			led1.Switch();
		    led2.Switch();
		    led3.Switch();
		    led4.Switch();
		}

		if (cam.IsAvailable()&&
				System::Time()>time_img){
		    lcd.FillBits(0x0000,0xFFFF,cam.LockBuffer(),cam.GetBufferSize()*8);
		    time_img=System::Time()+50;
		    cam.UnlockBuffer();
		    cam.Stop();
		    cam.Start();
		}

		//System::DelayMs(250);
	}

	return 0;
}
