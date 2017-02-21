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
#include <libsc/futaba_s3010.h>
#include <libsc/alternate_motor.h>
#include <libsc/k60/jy_mcu_bt_106.h>
#include <libsc/led.h>
#include <libsc/dir_encoder.h>

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

bool BTonReceiveInstruction(const Byte *data, const size_t size);
void stopMotor();
void startMotor();
Led* led1;
JyMcuBt106* exterior_bluetooth;
const Byte tempInt2 = 171;
const Byte tempInt3 = 172;
const Byte tempInt4 = 169;
const Byte* temp4 = &tempInt4;
const Byte* temp3 = &tempInt3;
const Byte* temp2 = &tempInt2;
AlternateMotor* exterior_Lmotor;
AlternateMotor* exterior_Rmotor;
int motorPower = 0;
FutabaS3010* exterior_servo;

int main(void)
{
	System::Init();

	Ov7725::Config C;  //camera init;
	C.id = 0;
	C.w = 80;
	C.h = 60;
	Ov7725 cam(C);

	St7735r::Config s; //screen init;
	s.is_revert = false;
	s.is_bgr = false;
	s.fps = 100;
	St7735r screen(s);
	Timer::TimerInt t=0;

	JyMcuBt106::Config bluetooth_config;
	bluetooth_config.id = 0;
	bluetooth_config.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	bluetooth_config.rx_isr = BTonReceiveInstruction;
	JyMcuBt106 bluetooth(bluetooth_config);
	exterior_bluetooth = &bluetooth;

	FutabaS3010::Config servo_config;
	servo_config.id = 0;
	FutabaS3010 servo(servo_config);
	servo.SetDegree(900);
	exterior_servo = &servo;

	Led::Config led_config;
	led_config.id = 0;
	led_config.is_active_low = true;
	Led inner_led1(led_config);
	led1 = &inner_led1;

	AlternateMotor::Config Lmotor_config, Rmotor_config;
	Lmotor_config.id = 0; Rmotor_config.id = 1;
	AlternateMotor Lmotor(Lmotor_config), Rmotor(Rmotor_config);
	exterior_Lmotor = &Lmotor; exterior_Rmotor = &Rmotor;
	Lmotor.SetClockwise(0); Rmotor.SetClockwise(1);
	Lmotor.SetPower(motorPower); Rmotor.SetPower(motorPower);
	Byte motorPower_byte;
	const Byte* motorPowerPtr;

	DirEncoder::Config dir_encoder_config;
	dir_encoder_config.id = 0;
	DirEncoder dirEncoder(dir_encoder_config);
	const Byte* encoder_count;
	Byte encoder_count_int;

	const Byte* camPtr;
	const Byte tempInt = 170;
	const Byte* temp = &tempInt;

	cam.Start();

	while (true){
		while(t!=System::Time()){
			t = System::Time();
			if(t % 100 == 0){

				camPtr = cam.LockBuffer();

				//for tft
				screen.SetRegion(Lcd::Rect(0,0,80,60));
				screen.FillBits(St7735r::kBlack,St7735r::kWhite,camPtr,8*cam.GetBufferSize());

				//for bluetooth
				bluetooth.SendBuffer(temp,1);
				bluetooth.SendBuffer(camPtr, cam.GetBufferSize());
				//control the car by bt upon pressing key

				bluetooth.SendBuffer(temp3,1);
				encoder_count_int = dirEncoder.GetCount();
				encoder_count = &encoder_count_int;
				bluetooth.SendBuffer(encoder_count,1);
				
				bluetooth.SendBuffer(temp4,1);
				motorPower_byte = motorPower;
				motorPowerPtr = &motorPower_byte;
				bluetooth.SendBuffer(motorPower_byte,1);

				cam.UnlockBuffer();
			}
		}
	}

	cam.Stop();
	return 0;
}

void stopMotor(){
	exterior_Lmotor->SetPower(0);
	exterior_Rmotor->SetPower(0);
}
void startMotor(){
	exterior_Lmotor->SetPower(motorPower);
	exterior_Rmotor->SetPower(motorPower);
}

bool BTonReceiveInstruction(const Byte *data, const size_t size){
	exterior_bluetooth->SendBuffer(temp2,1);
	exterior_bluetooth->SendBuffer(&data[0], 1);
	switch(data[0]){
	case ' ':
		stopMotor();
		exterior_Lmotor->SetClockwise(0);
		exterior_Rmotor->SetClockwise(1);
		break;
	case 'w':
		exterior_servo->SetDegree(900);
		exterior_Lmotor->SetClockwise(0);
		exterior_Rmotor->SetClockwise(1);
		startMotor();
		break;
	case 's':
		exterior_servo->SetDegree(900);
		exterior_Lmotor->SetClockwise(1);
		exterior_Rmotor->SetClockwise(0);
		startMotor();
		break;
	case 'd':
		exterior_servo->SetDegree(700);
		startMotor();
		break;
	case 'a':
		exterior_servo->SetDegree(1100);
		startMotor();
		break;
	case 'r':
		stopMotor();
		break;
	case ',':
		if(motorPower-20>=0) motorPower-=20;
		break;
	case '.':
		if(motorPower+20<=450) motorPower+=20;
		break;
	}


	return true;
}
