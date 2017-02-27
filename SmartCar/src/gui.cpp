/*
 * main.cpp
 *
 * Author: Leslie
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/gui.h"

//Global variable-----------------------------------------------------------------------------------------------------------
int NumberToSelect=2;

//GUI Program--------------------------------------------------------------------------------------------------------------
void gui(Joystick *FiveWaySwitch,St7735r *LCD,LcdConsole *Console,Ov7725 *Cam,FutabaS3010 *Servo,AlternateMotor *MotorA,AlternateMotor *MotorB,DirEncoder *EncoderA,DirEncoder *EncoderB)
{
	int selection=1;
		Console->Clear(true);
		Console->WriteString("Smart Car");
	Looper looper;

	bool BreakLoop=false;

	std::function<void(const Timer::TimerInt, const Timer::TimerInt)> select = [&](const Timer::TimerInt request, const Timer::TimerInt){
		if(FiveWaySwitch->GetState()==Joystick::State::kUp){
			selection=selection-1;
			if(selection<1){
				selection=NumberToSelect;
			}
			if(selection==1){
				Console->Clear(true);
				Console->WriteString("Smart Car");
			}else if(selection==2){
				Console->Clear(true);
				Console->WriteString("Snake Game");
			}
		}else if(FiveWaySwitch->GetState()==Joystick::State::kDown){
			selection=selection+1;
			if(selection>NumberToSelect){
				selection=1;
			}
			if(selection==1){
				Console->Clear(true);
				Console->WriteString("Smart Car");
			}else if(selection==2){
				Console->Clear(true);
				Console->WriteString("Snake Game");
			}
		}
		if(FiveWaySwitch->GetState()==Joystick::State::kSelect){
			BreakLoop=true;
			looper.Break();
		}
		looper.RunAfter(request, select);
	};
	looper.RunAfter(200, select);
	looper.ResetTiming();
	while(true){
		if(BreakLoop==true){
			break;
		}
		looper.Once();
	}

	if(selection==1){
		Console->Clear(true);
		smart_car(FiveWaySwitch,LCD,Console,Cam,Servo,MotorA,MotorB,EncoderA,EncoderB);
	}else if(selection==2){
		Console->Clear(true);
		snake_game(FiveWaySwitch,LCD,Console);
	}

}
