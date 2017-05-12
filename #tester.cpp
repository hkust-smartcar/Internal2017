/*
 * tester.cpp
 *
 * Author: Dipsy
 * Copyright (c) 2016-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>

#include "libsc/alternate_motor.h"
#include "libsc/dir_encoder.h"
#include "libsc/futaba_s3010.h"
#include "libsc/k60/jy_mcu_bt_106.h"
#include "libsc/lcd_console.h"
#include "libsc/led.h"
#include "libsc/st7735r.h"
#include "libsc/system.h"
#include "libsc/k60/ov7725.h"
#include "libsc/joystick.h"

#include "debug_console.h"


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

LcdTypewriter* pWriter;


//testing functions

//test board basic and led
void blink(Led* led0,Led* led1,Led* led2,Led* led3){
	led0->SetEnable(1);
	led1->SetEnable(1);
	led2->SetEnable(1);
	led3->SetEnable(1);
	while(1){
		led0->Switch();
		led1->Switch();
		led2->Switch();
		led3->Switch();
		System::DelayMs(250);
	}
}

//test camera
void capture(Ov7725* cam, St7735r* lcd){
	while(1){
		if(System::Time()%50==0 && cam->IsAvailable()){
			lcd->SetRegion(Lcd::Rect(0,0,80,60));
			lcd->FillBits(Lcd::kBlack,Lcd::kWhite,cam->LockBuffer(),8*cam->GetBufferSize());
			cam->UnlockBuffer();
		}
	}
}

//test joystick
void testjoystick(Joystick* joystick, Lcd* lcd){
	switch(joystick->GetState()){
	case Joystick::State::kDown:
		lcd->FillColor(Lcd::kBlack);break;
	case Joystick::State::kIdle:
		lcd->FillColor(Lcd::kBlue);break;
	case Joystick::State::kLeft:
		lcd->FillColor(Lcd::kCyan);break;
	case Joystick::State::kRight:
		lcd->FillColor(Lcd::kGreen);break;
	case Joystick::State::kSelect:
		lcd->FillColor(Lcd::kPurple);break;
	case Joystick::State::kUp:
		lcd->FillColor(Lcd::kRed);break;

	}

}

//test servo
int servo_degree = 600;
FutabaS3010* pServo = nullptr;
void updateServo(){
	pServo->SetDegree(++servo_degree);
}
void testservo(Joystick* joystick, St7735r* lcd,LcdTypewriter* writer){
	DebugConsole console(joystick,lcd,writer);
	DebugConsole::Item item("set servo");
	item.setValuePtr(&servo_degree);
	item.setListener(SELECT,&updateServo);
	console.pushItem(item);
	console.enterDebug();
}



int main(void){

	System::Init();

	Led::Config ledconfig;
	ledconfig.id=0;
	Led led0(ledconfig);
	ledconfig.id=1;
	Led led1(ledconfig);
	ledconfig.id=2;
	Led led2(ledconfig);
	ledconfig.id=3;
	Led led3(ledconfig);

	//test led by blinking
	//blink(&led0,&led1,&led2,&led3);

	St7735r::Config lcdConfig;
	lcdConfig.is_revert = false;
	St7735r lcd(lcdConfig);

	LcdTypewriter::Config writerconfig;
	writerconfig.lcd = &lcd;
	LcdTypewriter writer(writerconfig);

	//test lcd by color fill
	lcd.FillColor(Lcd::kYellow);

	/*k60::Ov7725::Config cameraConfig;
	cameraConfig.id = 0;
	cameraConfig.w = 80;
	cameraConfig.h = 60;
	cameraConfig.fps = k60::Ov7725Configurator::Config::Fps::kHigh;
	k60::Ov7725 camera(cameraConfig);
	camera.Start();
*/
	//test camera by capture image then display on lcd
	//capture(&camera , &lcd);

	Joystick::Config joystick_config;
	joystick_config.id = 0;
	joystick_config.is_active_low = true;
	Joystick joystick(joystick_config);

	//test joystick
	//testjoystick();

	FutabaS3010::Config ConfigServo;
	ConfigServo.id = 0;
	FutabaS3010 servo(ConfigServo);
	pServo = &servo;

	testservo(&joystick, &lcd, &writer);


	while (true){
	}

	return 0;
}
