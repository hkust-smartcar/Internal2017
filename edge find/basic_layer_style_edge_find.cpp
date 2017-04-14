/*
 * main.cpp
 *
 * Author: Gordon
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

//BLACK is TRUE, WHITE IS FALSE
#define BLACK true
#define WHITE false

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
#include <libsc/lcd_typewriter.h>
#include <libsc/ab_encoder.h>
#include <libsc/joystick.h>
#include <libsc/futaba_s3010.h>
//#include <algorithm>

#define WIDTH 80
#define HEIGHT 60
#define AREA_KP 1.1
#define SPEEDAREA 3
#define LEFT_ANGLE 1150
#define RIGHT_ANGLE 450
#define CENTER_ANGLE 800

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


//void analysis(const Byte* buff);
bool get_bit(const Byte* buff, int x, int y){
	return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
}
bool get_pkbit(const Byte* buff, int x, int y){
	y=HEIGHT-1-y;
	return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
}

//void JoyStickDebugListen(Joystick::State state);


//distortion solver
//void find_vanish(const Byte* buff);
//bool get_fixedbit(const Byte* buff, int, int);
int16_t vanish_x=0,vanish_y=0;
int yL=0,yR=0;


int max(int a,int b){return (a>b?a:b);}
int min(int a,int b){return (a<b?a:b);}

k60::Ov7725* cam;
St7735r* lcd;
AlternateMotor* motor;
Servo* servo;

int servo_target=900, servo_pos=900, motor_target = 200, motor_speed = 200;

bool map[HEIGHT][WIDTH];

int main(void){
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
	led2.SetEnable(true);
	led3.SetEnable(true);
	led4.SetEnable(true);

/*
	//init MOTOR
	AlternateMotor::Config motor_config;
	motor_config.id=1;
	AlternateMotor motor1(motor_config);
	motor = &motor1;

	motor->SetPower(150);


	//init servo
	Servo::Config servo_config;
	servo_config.id=0;
	servo_config.period=40000;
	servo_config.min_pos_width=910;
	servo_config.max_pos_width=2100;
	Servo servo1(servo_config);
	servo = &servo1;

	servo->SetDegree(900);

*/
	FutabaS3010::Config ConfigServo;
	ConfigServo.id=0;
	FutabaS3010 Servo(ConfigServo);
	servo = &Servo;

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

	LcdTypewriter::Config writer_config;
	writer_config.lcd = &lcd1;
	LcdTypewriter writer(writer_config);

	//init camera
	k60::Ov7725::Config cam_config;
	cam_config.id=0;
	cam_config.w=WIDTH;
	cam_config.h=HEIGHT;
	cam_config.fps = k60::Ov7725::Config::Fps::kHigh;
	k60::Ov7725 cam1(cam_config);
	cam = &cam1;

	cam->Start();

	//init encoder
	//AbEncoder::Config encoder_config;
	//encoder_config.id = 0;
	//Encoder::Initializer encoder_init(encoder_config);
	//AbEncoder encoder1(encoder_config);

	//init joystick

	Joystick::Config joystick_config;
	joystick_config.id = 0;
	joystick_config.is_active_low = true;
	Joystick joystick(joystick_config);
	Joystick::State state;

	/*
	int left_edge[HEIGHT], right_edge[HEIGHT];
	left_edge[0]=0;
	right_edge[0]=WIDTH-1;
	while(!cam->IsAvailable());

	{
		const Byte* byte = cam->LockBuffer();
		while(get_pkbit(byte,++left_edge[0],0));
		while(get_pkbit(byte,--right_edge[0],0));
		cam->UnlockBuffer();
		cam->Stop();
		cam->Start();
	}

*/

	servo->SetDegree(1150);
	System::DelayMs(1000);
	servo->SetDegree(450);
	System::DelayMs(1000);
	servo->SetDegree(800);

	while(true){

		led1.Switch();
		led2.Switch();
		led3.Switch();
		led4.Switch();
		//System::DelayMs(250);
		//continue;

		//JoyStickDebugListen(joystick.GetState());

		if (System::Time()!=time_img){
			time_img=System::Time();

			if (cam->IsAvailable()&&time_img%50==0){
				//int start = System::Time();
				const Byte* byte = cam->LockBuffer();

				//print image
				lcd->SetRegion(Lcd::Rect(0,0,WIDTH,HEIGHT));
				lcd->FillBits(0x0000,0xFFFF,byte,cam->GetBufferSize()*8);


				int left_edge[HEIGHT], right_edge[HEIGHT], mid[HEIGHT];

				int error=0;

				for (int i=0; i<HEIGHT/2;i++){

					if(i==0){
						left_edge[i]=WIDTH/2;
						right_edge[i]=WIDTH/2;
					}
					else{
						left_edge[i]=mid[i-1];
						right_edge[i]=mid[i-1];
					}
						while(!get_pkbit(byte,--left_edge[i]-1,i)){
							if(left_edge[i]==0)break;
						}
						while(!get_pkbit(byte,++right_edge[i]+1,i)){
							if(left_edge[i]==WIDTH-1)break;
						}


						lcd->SetRegion(Lcd::Rect(left_edge[i],HEIGHT-i,1,1));
						lcd->FillColor(Lcd::kRed);
						lcd->SetRegion(Lcd::Rect(right_edge[i],HEIGHT-i,1,1));
						lcd->FillColor(Lcd::kGreen);
						mid[i] = (right_edge[i]+left_edge[i])/2;
						lcd->SetRegion(Lcd::Rect(mid[i],HEIGHT-i,1,1));
						lcd->FillColor(Lcd::kBlue);

						error += mid[i]-WIDTH/2;

				}

				error+=120;

				lcd->SetRegion(Lcd::Rect(0,HEIGHT+2,100,16));
				char c[10];
				sprintf(c,"%d",error);
				writer.WriteBuffer(c,10);


				servo->SetDegree(min(LEFT_ANGLE,max(RIGHT_ANGLE,CENTER_ANGLE-error)));


				cam->UnlockBuffer();
				cam->Stop();
				cam->Start();
			}




		}
	}

	return 0;
}
