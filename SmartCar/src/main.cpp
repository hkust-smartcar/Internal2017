/*
 * main.cpp
 *
 * Author: Leslie
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/main.h"

const Uint CamHeight=60;
const Uint CamWidth=80;
//160*120

//Required DON'T DELETE IT !!!----------------------------------------------------------------------------------------------
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

void distanceUpdate(Gpi *gpi){
	if(gpi->Get()){

	}else{

	}
}

//Main Program--------------------------------------------------------------------------------------------------------------
int main(void)
{
	System::Init();

//ir_ultrasound_modules Configuration---------------------------------------------------------------------------------------------------
	IRUltrasoundSensor distanceSensor(Pin::Name::kPtb0);

//JoyStick Configuration-------------------------------------------------------------------------------------------
	Joystick::Config ConfigJoystick1;
	ConfigJoystick1.id=0;
	ConfigJoystick1.is_active_low=true;
	Joystick joystickA(ConfigJoystick1);
	joystick=&joystickA;

//LCD Configuration---------------------------------------------------------------------------------------------------------
	St7735r::Config ConfigLCD;
	ConfigLCD.is_revert=true;
	ConfigLCD.is_bgr=false;
	ConfigLCD.fps=60;
	St7735r LCD(ConfigLCD);
	lcd=&LCD;

//LCD Console Configuration---------------------------------------------------------------------------------------------------------
	LcdConsole::Config ConfigConsole;
	ConfigConsole.lcd=&LCD;
	ConfigConsole.region=Lcd::Rect(4,4,124,124);
	LcdConsole Console(ConfigConsole);
	console=&Console;

//Camera Configuration------------------------------------------------------------------------------------------------------
	Ov7725::Config ConfigCam;
	ConfigCam.id=0;
	ConfigCam.h=camHeight;
	ConfigCam.w=camWidth;
	Ov7725 Cam(ConfigCam);
	cam=&Cam;

//Servo Configuration-------------------------------------------------------------------------------------------------------
//	FutabaS3010::Config ConfigServo;
//	ConfigServo.id=0;
//	FutabaS3010 Servo(ConfigServo);
//	servo=&Servo;

//Motor 1 (Right Motor) Configuration---------------------------------------------------------------------------------------
	AlternateMotor::Config ConfigMotor1;
	ConfigMotor1.id=0;
	AlternateMotor Motor1(ConfigMotor1);
	motor1=&Motor1;

//Encoder 1 (Right Motor) Configuration-------------------------------------------------------------------------------
	DirEncoder::Config ConfigEncoderA;
	ConfigEncoderA.id=0;
	DirEncoder Encoder1(ConfigEncoderA);
	encoder1=&Encoder1;

//Motor 2 (Left Motor) Configuration----------------------------------------------------------------------------------------
	AlternateMotor::Config ConfigMotor2;
	ConfigMotor2.id=1;
	AlternateMotor  Motor2(ConfigMotor2);
	motor2=&Motor2;

//Encoder 2 (Left Motor) Configuration-------------------------------------------------------------------------------
	DirEncoder::Config ConfigEncoderB;
	ConfigEncoderB.id=1;
	DirEncoder Encoder2(ConfigEncoderB);
	encoder2=&Encoder2;

//Bluetooth Configuration----------------------------------------------------------------------------------------------------------------------------
	k60::JyMcuBt106::Config bt_config;
	bt_config.id = 0;
	bt_config.baud_rate = Uart::Config::BaudRate::k115200;
	BTComm bt(bt_config);
	bluetooth=&bt;

//Battery Meter Configuration---------------------------------------------------------------------------------------------------------------
	BatteryMeter::Config ConfigBM;
	ConfigBM.voltage_ratio=0.4;
	BatteryMeter BM(ConfigBM);
	batteryMeter=&BM;

//Cars Configuration---------------------------------------------------------------------------------------------------------------------------------
	car2.leftAngle=1050;
	car2.midAngle=800;
	car2.rightAngle=450;

	car1.leftAngle=980;
	car1.midAngle=720;
	car1.rightAngle=390;

	util::batteryTest();

//	util::servoTuning();

	while(true){
		util::consoleWriteValue(distanceSensor.getDistance());
	}

	car1Main();
}
