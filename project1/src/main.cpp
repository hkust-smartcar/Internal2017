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
#include <libsc/lcd_console.h>
//#include <algorithm>

#define WIDTH 80
#define HEIGHT 60
#define AREA_KP 5

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

void to2D(const Byte* buff);
bool median_filter(int,int);

int max(int a,int b){return (a>b?a:b);}
int min(int a,int b){return (a<b?a:b);}

k60::Ov7725* cam;
St7735r* lcd;
AlternateMotor* motor;
Servo* servo;

int servo_target=900, servo_pos=900;

bool map[HEIGHT][WIDTH];

int main(void)
{
	System::Init();

	//init LED
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


	//init MOTOR
	AlternateMotor::Config motor_config;
	motor_config.id=1;
	AlternateMotor motor1(motor_config);
	motor = &motor1;

	motor->SetPower(100);


	//init servo
	Servo::Config servo_config;
	servo_config.id=0;
	servo_config.period=40000;
	servo_config.min_pos_width=910;
	servo_config.max_pos_width=2100;
	Servo servo1(servo_config);
	servo = &servo1;

	servo->SetDegree(1800);


	//init LCD
	St7735r::Config lcd_config;
	St7735r lcd1(lcd_config);
	lcd = &lcd1;

	Lcd::Rect r(0,0,WIDTH,HEIGHT);
	lcd->SetRegion(r);

	uint32_t time_img=0;

	//LcdConsole::Config console_config;
	//console_config.lcd=&lcd1;
	//LcdConsole console(console_config);


	//init camera
	k60::Ov7725::Config cam_config;
	cam_config.id=0;
	cam_config.w=WIDTH;
	cam_config.h=HEIGHT;
	cam_config.fps = k60::Ov7725::Config::Fps::kHigh;
	k60::Ov7725 cam1(cam_config);
	cam = &cam1;

	cam->Start();


	while(true){

		if (System::Time()%250==0){
			led1.Switch();
		    led2.Switch();
		    led3.Switch();
		    led4.Switch();
		}

		if (cam->IsAvailable()&&System::Time()%50==0){

			to2D(cam->LockBuffer());
			//lcd->FillBits(0x0000,0xFFFF,cam->LockBuffer(),cam->GetBufferSize()*8);
			//cam->UnlockBuffer();
		    //cam->Stop();
		    //cam->Start();
		}

		if (System::Time()!=time_img){
			time_img=System::Time();

			if (time_img==0){
				//console.Clear(true);
				//console.WriteChar(System::Time()/1000%10+48);
			}


		}

		if (System::Time()%10==0){
			//update servo position
			if (servo_pos!=servo_target){
				servo_pos+=(servo_target-servo_pos)/10;
				servo->SetDegree(servo_pos);
			}
		}

		//System::DelayMs(250);
	}

	return 0;
}

//convert camera buffer to 2D array
void to2D(const Byte* buff){
	cam->UnlockBuffer();
	lcd->FillBits(0x0000,0xFFFF,buff,cam->GetBufferSize()*8);
	cam->Stop();
	cam->Start();

	int count=0;
	int left=0,right=0;
	Byte mask=0;

	//thanks Leslie's help
	for (int i=0; i<HEIGHT; i++){
		for (int j=0; j<WIDTH; j+=8){
			mask=0x40;
			for(int k=0; k<8; k++){
				map[i][j+k] = buff[count]&mask;

				if (!map[i][j+k]){//black
					if(j<WIDTH/2){
						left+=median_filter(i,j);
					}else{
						right+=median_filter(i,j);
					}
				}
				mask>>=1;
			}
			/*
			map[i][j+0] = buff[count] & 0b10000000;
			map[i][j+1] = buff[count] & 0b1000000;
			map[i][j+2] = buff[count] & 0b100000;
			map[i][j+3] = buff[count] & 0b10000;
			map[i][j+4] = buff[count] & 0b1000;
			map[i][j+5] = buff[count] & 0b100;
			map[i][j+6] = buff[count] & 0b10;
			map[i][j+7] = buff[count] & 0b1;*/
			count++;
		}
	}
	int delta = AREA_KP*(right-left);
	delta = ((delta>900?900:delta)<-900?-900:delta);
	servo_target=900+delta;
}


bool median_filter(int i, int j){
	int sum=0,count=0;

	//sum up the boolean around (x,y)
	for (int a = -1; a<=1; a++){
		for (int b= -1; b<=1; b++){
			if (i+a>=0&&i+a<HEIGHT&&j+b>=0&&j+b<WIDTH){ //make sure the index is within range of map
				sum+=map[i+a][j+b];
				count++;
			}
		}
	}
	return sum*10/count>5;
}
