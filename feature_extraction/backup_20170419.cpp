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
	if (x<0||x>=WIDTH||y<0||y>=HEIGHT) return true; //out of view is black
	return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
}
bool get_pkbit(const Byte* buff, int x, int y){
	if (x<0||x>=WIDTH||y<0||y>=HEIGHT) return true; //out of view is black
	y=HEIGHT-1-y;
	return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
}

/*
class ImageManager{
public:
	const Byte* buff;
	ImageManager(const Byte* buff):buff(buff){
	}
	bool get_bit(int x, int y){
		return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
	}
	bool get_pkbit(int x, int y){
		y=HEIGHT-1-y;
		return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
	}
};
*/

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

	int left_edge[200][2], right_edge[200][2], mid[200][2];

		while(!cam->IsAvailable());
		{
			const Byte* byte = cam->LockBuffer();

			//locate the base edge
			//locate the right first
			right_edge[0][0]=WIDTH-1;
			while(get_pkbit(byte,--right_edge[0][0],0)&&right_edge[0][0]>0);	//from right to left, the first white is right edge base
			left_edge[0][0]=0;
			while(get_pkbit(byte,++left_edge[0][0],0)&&left_edge[0][0]<WIDTH-1);		//from left to right, the first white is left edge base
			left_edge[0][1]=0;
			right_edge[0][1]=0;

			cam->UnlockBuffer();
			cam->Stop();
			cam->Start();
		}

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
				//lcd->SetRegion(Lcd::Rect(0,0,WIDTH,HEIGHT));
				//lcd->FillBits(0x0000,0xFFFF,byte,cam->GetBufferSize()*8);



				lcd->SetRegion(Lcd::Rect(0, 0, 80, 60));
				lcd->FillColor(Lcd::kBlack);

				int timeStart = System::Time();

				//int left_edge[200][2], right_edge[200][2], mid[200][2];

				int error=0;

				//locate the base edge
				//locate the right first
				if(get_pkbit(byte,left_edge[0][0],left_edge[0][1]))
									while(get_pkbit(byte,++left_edge[0][0],left_edge[0][1]));
								else
									while(!get_pkbit(byte,--left_edge[0][0]-1,left_edge[0][1]));
								if(get_pkbit(byte,right_edge[0][0],right_edge[0][1]))
									while(get_pkbit(byte,--right_edge[0][0],right_edge[0][1]));
								else
									while(!get_pkbit(byte,++right_edge[0][0]+1,right_edge[0][1]));


				//declare variables for searching in directions
				int left_from=0;
				int right_from=0;
				const int dx[8]={ 0,-1,-1,-1, 0, 1, 1, 1};
				const int dy[8]={-1,-1, 0, 1, 1, 1, 0,-1};

				//loop for finding edge, maximum find 200 points for each edge
				for (int count=0;count<200-1;count++){

					bool flag=1;//to break

					//x y are the last point of edge
					const int x=left_edge[count][0];
					const int y=left_edge[count][1];

					//paint it
					lcd->SetRegion(Lcd::Rect(x,HEIGHT-y-1,1,1));
					lcd->FillColor(Lcd::kRed);

					//search in directions, from last direction clockwise to last direction
					for (int i=left_from+1; i<left_from+9;i++){
						const int j=i%8;
						if(!get_pkbit(byte, x+dx[j],y+dy[j])){//if the point is white, it is a new point of edge
							left_edge[count+1][0]=x+dx[j];
							left_edge[count+1][1]=y+dy[j];
							flag=0;
							left_from=j-4;
							break;
						}
					}

					//x1 y1 are the last point of edge
					const int x1=right_edge[count][0];
					const int y1=right_edge[count][1];

					//paint it
					lcd->SetRegion(Lcd::Rect(x1,HEIGHT-y1-1,1,1));
					lcd->FillColor(Lcd::kPurple);

					//search in directions, from last direction anti clockwise to last direction
					for (int i=right_from+7; i>=right_from;i--){
						const int j=i%8;
						if(!get_pkbit(byte, x1+dx[j],y1+dy[j])){//if the point is white, it is a new point of edge
							right_edge[count+1][0]=x1+dx[j];
							right_edge[count+1][1]=y1+dy[j];
							flag=0;
							right_from=j-4;
							break;
						}
					}

					//find the mid point
					/*
					if (y>0&&y1>0){
						lcd->SetRegion(Lcd::Rect((x1+x)/2,HEIGHT-1-(y1+y)/2,1,1));
						mid[count][0]=(x1+x)/2;
						mid[count][1]=(y1+y)/2;
						error+=WIDTH/2-mid[count][0];
						lcd->FillColor(Lcd::kBlue);
					}*/


					//break condition: left edge right edge meet or fail to find new edge
					if(left_edge[count][0]==x1&&left_edge[count][1]==y1) break;
					if(flag)break;
				}

				//print variables
				char buff[10];
				sprintf(buff," %d,%d",System::Time()-timeStart,error);
				lcd->SetRegion(Lcd::Rect(0,HEIGHT,100,15));
				writer.WriteBuffer(buff,10);
				cam->UnlockBuffer();
				cam->Stop();
				cam->Start();
			}




		}
	}

	return 0;
}
