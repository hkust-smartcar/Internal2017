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
#include "img.h"

#include "debug_console.h"

#define WIDTH 128
#define HEIGHT 480


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
JyMcuBt106* pBt = nullptr;
St7735r* pLcd = nullptr;
const Byte* CameraBuf = nullptr;

//get bit value from camerabuf using camera coordinate system
bool getBit(int i_x, int i_y){
	if (i_x<=0 || i_x>WIDTH-1 || i_y <= 0 || i_y > HEIGHT-1) return -1;
	return CameraBuf[i_y*WIDTH/8 + i_x/8] >> (7 - (i_x%8)) & 1;
}


bool getWorldBit(int w_x, int w_y){
	int i_x,i_y;
	i_x = transformMatrix[w_x][w_y][0];
	i_y = transformMatrix[w_x][w_y][1];
	return getBit(i_x,i_y);
}

void PrintWorldImage(){
	Byte temp[128/8];
	for (int i=0; i<160; i++){
		for (int j=0; j<128; j++){
			temp[j/8]<<=1;
			temp[j/8]+=getWorldBit(j,i);
			//WorldBuf[i*128/8+j/8]<<=1;
			//WorldBuf[i*128/8+j/8]+=getWorldBit(j,i);
		}
		pLcd->SetRegion(Lcd::Rect(0,i,128,1));
		pLcd->FillBits(0x0000,0xFFFF,temp,128);
		//pLcd->FillColor(getWorldBit(j,i)?Lcd::kBlack:Lcd::kWhite);
	}
	return;
}

void PrintRawImage(){
	Byte temp[128/8];
	for (int i=0; i<480; i+=3){
		for (int j=0; j<128; j++){
			temp[j/8]<<=1;
			temp[j/8]+=getBit(j,i);
			//WorldBuf[i*128/8+j/8]<<=1;
			//WorldBuf[i*128/8+j/8]+=getWorldBit(j,i);
		}
		pLcd->SetRegion(Lcd::Rect(0,i/3,128,1));
		pLcd->FillBits(0x0000,0xFFFF,temp,128);
		//pLcd->FillColor(getWorldBit(j,i)?Lcd::kBlack:Lcd::kWhite);
	}
	return;
}

void processing(){

}


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
void capture(Ov7725* cam, St7735r* lcd,Led* led1){
	while(1){
		if(System::Time()%50==0 && cam->IsAvailable()){
			lcd->SetRegion(Lcd::Rect(0,0,WIDTH,HEIGHT));
			lcd->FillBits(Lcd::kBlack,Lcd::kWhite,cam->LockBuffer(),8*cam->GetBufferSize());
			cam->UnlockBuffer();
			led1->Switch();
		}
	}
}

//test joystick
void testjoystick(Joystick* joystick, Lcd* lcd,Led* led1){
	led1->Switch();
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
	lcdConfig.is_revert = true;
	St7735r lcd(lcdConfig);
	pLcd = &lcd;

	LcdTypewriter::Config writerconfig;
	writerconfig.lcd = &lcd;
	LcdTypewriter writer(writerconfig);

	//test lcd by color fill
	//lcd.FillColor(Lcd::kYellow);

	Joystick::Config joystick_config;
	joystick_config.id = 0;
	joystick_config.is_active_low = true;
	Joystick joystick(joystick_config);

	//test joystick
	//while(1) testjoystick(&joystick,&lcd,&led1);

	k60::JyMcuBt106::Config bt_config;
	bt_config.id = 0;
	bt_config.baud_rate = Uart::Config::BaudRate::k115200;
	bt_config.tx_buf_size = 3;
	k60::JyMcuBt106 bt(bt_config);
	pBt = &bt;

	k60::Ov7725::Config cameraConfig;
	cameraConfig.id = 0;
	cameraConfig.w = WIDTH;
	cameraConfig.h = HEIGHT;
	cameraConfig.fps = k60::Ov7725Configurator::Config::Fps::kHigh;
	k60::Ov7725 camera(cameraConfig);
	camera.Start();

	//test camera by capture image then display on lcd
	//capture(&camera , &lcd, &led1);


	//FutabaS3010::Config ConfigServo;
	//ConfigServo.id = 0;
	//FutabaS3010 servo(ConfigServo);
	//pServo = &servo;

	//testservo(&joystick, &lcd, &writer);

	//test world view
/*
	while(1){
		if(System::Time()%50==0&&camera.IsAvailable()){
			CameraBuf = camera.LockBuffer();
			PrintWorldImage();
			camera.UnlockBuffer();
		}
	}*/

	Timer::TimerInt time=0;
	Timer::TimerInt sendTime=0;
	Timer::TimerInt printTime=0;
	const Byte startByte = 170;
	const Byte* camBuf;
	Joystick::State jState = Joystick::State::kIdle;
	bool view = true;

	while (true){
		while(time!=System::Time()){
			time = System::Time();
			if(time>sendTime&&camera.IsAvailable()){
				//System::DelayMs(500);
				bt.SendBuffer(&startByte, 1);
				//System::DelayMs(500);
				camBuf = camera.LockBuffer();
				bt.SendBuffer(camBuf, camera.GetBufferSize());
				//lcd.SetRegion(Lcd::Rect(0,0,80,60));
				//lcd.FillBits(0x0000,0xffff,camBuf,camera.GetBufferSize()*8);
				camera.UnlockBuffer();
				led1.Switch();
				sendTime = time + 2000;
			}
			else if(time%50==0&&camera.IsAvailable()){
				CameraBuf = camera.LockBuffer();
				if(jState!=joystick.GetState()){
					jState = joystick.GetState();
					if(jState!=Joystick::State::kIdle) view = !view;
				}
				if(view)PrintWorldImage();
				else PrintRawImage();
				camera.UnlockBuffer();
				led2.Switch();
				printTime = time+50;
			}
		}
	}

	return 0;
}