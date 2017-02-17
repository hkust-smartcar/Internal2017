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

namespace libbase{
	namespace k60{

		Mcg::Config Mcg::GetMcgConfig(){
			Mcg::Config config;
			config.external_oscillator_khz = 50000;
			config.core_clock_khz = 150000;
			return config;
		}
	}
}

using namespace libsc;
using namespace libbase::k60;

// do initializations and definitions here -------------

//define
#define WIDTH 80
#define HEIGHT 60

//variables
k60::Ov7725* cam;
St7735r* lcd;
AlternateMotor* motor;
Servo* servo;
LcdTypewriter* typeWriter;
Led* led1;
Led* led2;
Led* led3;
Led* led4;

//functions
void init();


//main function ---------------
int main(void){
	System::Init();
	init();

	while (true){
		led1->Switch();
		led2->Switch();
		led3->Switch();
		led4->Switch();
	}

	return 0;
}



//function definition -------------

//init function
void init(){
	//init LED
	Led::Config config;
	config.id=0;
	Led led_1(config);
	config.id=1;
	Led led_2(config);
	config.id=2;
	Led led_3(config);
	config.id=3;
	Led led_4(config);
	led1 = &led_1;
	led2 = &led_2;
	led3 = &led_3;
	led4 = &led_4;

	led1->SetEnable(true);
	led2->SetEnable(false);
	led3->SetEnable(false);
	led4->SetEnable(true);

	return;

	//init MOTOR
	AlternateMotor::Config motor_config;
	motor_config.id=1;
	AlternateMotor motor1(motor_config);
	motor = &motor1;

	//init servo
	Servo::Config servo_config;
	servo_config.id=0;
	servo_config.period=40000;
	servo_config.min_pos_width=910;
	servo_config.max_pos_width=2100;
	Servo servo1(servo_config);
	servo = &servo1;

	servo->SetDegree(900);


	//init LCD
	St7735r::Config lcd_config;
	St7735r lcd1(lcd_config);
	lcd = &lcd1;
	lcd->SetRegion(Lcd::Rect(0,0,WIDTH,HEIGHT));


	//init type writer
	LcdTypewriter::Config writer_config;
	writer_config.lcd = &lcd1;
	LcdTypewriter writer(writer_config);
	typeWriter = &writer;

	//init camera
	k60::Ov7725::Config cam_config;
	cam_config.id=0;
	cam_config.w=WIDTH;
	cam_config.h=HEIGHT;
	cam_config.fps = k60::Ov7725::Config::Fps::kHigh;
	k60::Ov7725 cam1(cam_config);
	cam = &cam1;

	cam->Start();
}
