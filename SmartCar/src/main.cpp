/*
 * main.cpp
 *
 * Author: Leslie
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/main.h"

const Uint CamHeight=120;
const Uint CamWidth=160;
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

//Main Program--------------------------------------------------------------------------------------------------------------
int main(void)
{
	System::Init();

//JoyStick Configuration-------------------------------------------------------------------------------------------
	Joystick::Config ConfigJoystick1;
	ConfigJoystick1.id=0;
	ConfigJoystick1.is_active_low=true;
	Joystick FiveWaySwitch(ConfigJoystick1);

//LCD Configuration---------------------------------------------------------------------------------------------------------
	St7735r::Config ConfigLCD;
	ConfigLCD.is_revert=true;
	ConfigLCD.is_bgr=false;
	ConfigLCD.fps=60;
	St7735r LCD(ConfigLCD);

//Camera Configuration------------------------------------------------------------------------------------------------------
	Ov7725::Config ConfigCam;
	ConfigCam.id=0;
	ConfigCam.h=CamHeight;
	ConfigCam.w=CamWidth;
	Ov7725 Cam(ConfigCam);

//Servo Configuration-------------------------------------------------------------------------------------------------------
	FutabaS3010::Config ConfigServo;
	ConfigServo.id=0;
	FutabaS3010 Servo(ConfigServo);

//Motor A (Right Motor) Configuration---------------------------------------------------------------------------------------
	AlternateMotor::Config ConfigMotorA;
	ConfigMotorA.id=0;
	AlternateMotor  MotorA(ConfigMotorA);

//Encoder A (Right Motor) Configuration-------------------------------------------------------------------------------
	DirEncoder::Config ConfigEncoderA;
	ConfigEncoderA.id=0;
	DirEncoder EncoderA(ConfigEncoderA);

//Motor B (Left Motor) Configuration----------------------------------------------------------------------------------------
	AlternateMotor::Config ConfigMotorB;
	ConfigMotorB.id=1;
	AlternateMotor  MotorB(ConfigMotorB);

//Encoder B (Left Motor) Configuration-------------------------------------------------------------------------------
	DirEncoder::Config ConfigEncoderB;
	ConfigEncoderB.id=1;
	DirEncoder EncoderB(ConfigEncoderB);

	LcdConsole::Config ConfigConsole;
	ConfigConsole.lcd=&LCD;
	ConfigConsole.region=Lcd::Rect(10,128,128,30);
	LcdConsole Console(ConfigConsole);

	gui(&FiveWaySwitch,&LCD,&Console,&Cam,&Servo,&MotorA,&MotorB,&EncoderA,&EncoderB);
}
