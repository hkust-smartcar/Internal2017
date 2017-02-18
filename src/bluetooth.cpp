/*
 * bluetooth.cpp
 *
 *  Created on: Feb 18, 2017
 *      Author: Mr.King
 */
#include <cassert>
#include <cstring>
#include <cstdint>
#include <libbase/k60/mcg.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/pin.h>
#include <libsc/system.h>
#include <libsc/lcd.h>
#include <libsc/st7735r.h>
#include <libsc/button.h>
#include <libsc/k60/ov7725.h>
#include <libsc/led.h>
#include <libsc/k60/ov7725_configurator.h>
#include <libsc/alternate_motor.h>
#include <libsc/servo.h>
#include <libsc/futaba_s3010.h>
#include <libsc/k60/jy_mcu_bt_106.h>

#include <algorithm>
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

#define H 60
#define W 80
#define initServoAngle 900
#define initMotorSpeed 200
uint16_t Max_ServoAngle=1800;
uint16_t Min_ServoAngle=0;

bool bt_listener(const Byte*, const size_t);
bool ext_camptr[W][H];

AlternateMotor* motor1_p;
AlternateMotor* motor2_p;
FutabaS3010* servo_p;

int main(void){
	System::Init();
	// Initialization for Bluetooth
	k60::JyMcuBt106::Config Bt_config;
	Bt_config.id = 0;
	Bt_config.baud_rate = Uart::Config::BaudRate::k115200;
	Bt_config.rx_isr = &bt_listener;
	k60::JyMcuBt106 Bluetooth(Bt_config);

	// Initialization for camera
	k60::Ov7725::Config cam_config;
	cam_config.id = 0;
	cam_config.w = W;
	cam_config.h = H;
	//cam_config.fps = 60; // shooting rate
	k60::Ov7725 camera(cam_config);

	// Initialization for MOTOR1+2
	AlternateMotor::Config M1_config;
	M1_config.id = 0;
	AlternateMotor motor1(M1_config);
	AlternateMotor::Config M2_config;
	M1_config.id = 1;
	AlternateMotor motor2(M1_config);
	// Two motor rotate in opposite dir.
	motor1_p = &motor1;
	motor2_p = &motor2;

	motor1.SetClockwise(false);
	motor2.SetClockwise(true);
	motor1.SetPower(initMotorSpeed); // 0~1000, 500 -> 50% speed
	motor2.SetPower(initMotorSpeed); // 0~1000, 500 -> 50% speed

	// Initialization for SERVO
	FutabaS3010::Config S_config;
	S_config.id = 0;
	FutabaS3010 servo(S_config);
	servo_p= &servo;
	servo.SetDegree(initServoAngle);

	//starting signal telling processing to start receiving the image
	const Byte imageByte = 170;
	const Byte * imageBytePtr = &imageByte;

	Timer::TimerInt timeImg = System::Time();
	camera.Start();
	//wait until first image is available
	while(!camera.IsAvailable()){};
	while (true){
//		if (timeImg != System::Time()){
//			timeImg = System::Time();
//		 if(timeImg %200 == 0) // Update the car every 100ms.
//		 {
//			  const Byte* camBuffer = camera.LockBuffer();
//			  // Copy CamBuffer
//			  Byte* CopyCamBuffer = new Byte [camera.GetBufferSize()];
//			  for (int i; i<camera.GetBufferSize();i++){
//				  CopyCamBuffer[i]=camBuffer[i];
//			  }
//			  camera.UnlockBuffer();
//
//			  /*
//			   * For camera displaying
//			   */
//			  // Inform processing I am sending a image
//			  Bluetooth.SendBuffer(imageBytePtr, 1);
//			  // After identification inform processing I am sending the real data.
//			  Bluetooth.SendBuffer(camBuffer, camera.GetBufferSize());
//}
//		}
//	}
//	camera.Stop();
//	return 0;
}
}


/*
 * Bluetooth listener
 */
bool bt_listener(const Byte *data, const size_t size){
	if (data[0] == 'w'){
		motor1_p->SetClockwise(false);
		motor2_p->SetClockwise(true);
	}
	else if(data[0] == 'a'){
		servo_p->SetDegree(1800);
	}
	else if(data[0] == 's'){
		motor1_p->SetClockwise(true);
		motor2_p->SetClockwise(false);
	}
	else if(data[0] == 'd'){
		servo_p->SetDegree(0);
	}
		return true;
}
