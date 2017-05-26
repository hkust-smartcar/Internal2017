/*
 * util.cpp
 *
 *  Created on: 2017Äê4ÔÂ22ÈÕ
 *      Author: Lee Chun Hei
 */

#include "../inc/util.h"

namespace util{

/*
 * @brief Check if the battery is OK for the car to run, true means voltage is fine
 */
bool batteryCheck(){
	return batteryMeter->GetVoltage()>7.4;
}

/*
 * @brief Check if the battery is OK for the car to run, show the voltage on console
 */
void batteryTest(){
	if(batteryCheck()){
		console->SetCursorRow(0);
		console->SetTextColor(Lcd::kGreen);
		console->WriteString("Battery OK\n");
		util::consoleWriteValue(batteryMeter->GetVoltage());
		console->WriteString(" V");
	}else{
		console->SetTextColor(Lcd::kRed);
		console->WriteString("Battery Low\n");
		consoleWriteValue(batteryMeter->GetVoltage());
		console->WriteString(" V");
	}
}

/*
 * @brief Tune the car servo angle
 */
//void servoTuning(){
//	int servoAngle=midAngle;
//	console->SetCursorRow(1);
//	console->WriteString("Servo Angle:");
//	console->SetCursorRow(2);
//	console->WriteString("                                  ");
//	console->SetCursorRow(2);
//	consoleWriteValue(servoAngle);
//	while(true){
//		switch(joystick->GetState()){
//		case Joystick::State::kLeft:
//			servoAngle+=5;
//			console->SetCursorRow(2);
//			console->WriteString("                                  ");
//			console->SetCursorRow(2);
//			consoleWriteValue(servoAngle);
//			System::DelayMs(250);
//			break;
//		case Joystick::State::kRight:
//			servoAngle-=5;
//			console->SetCursorRow(2);
//			console->WriteString("                                  ");
//			console->SetCursorRow(2);
//			consoleWriteValue(servoAngle);
//			System::DelayMs(250);
//			break;
//		default:
//			break;
//		}
//		servo->SetDegree(servoAngle);
//	}
//}

/*
 * @brief Write the input float on screen
 */
void consoleWriteValue(float value){
	char buffer[50];
	sprintf(buffer,"%f",value);
	console->WriteString(buffer);
}

/*
 * @brief Write the input int on screen
 */
void consoleWriteValue(int value){
	char buffer[50];
	sprintf(buffer,"%d",value);
	console->WriteString(buffer);
}

namespace gui{

int numberOfSelection=0;

void guiInit(int numSelection){
	numberOfSelection=numSelection;
}

int selectionGui(){
	int selection=0;
	while(true){
		switch(joystick->GetState()){
		case Joystick::State::kUp:
			selection--;
			if(selection<0){
				selection+=numberOfSelection;
			}
			break;
		case Joystick::State::kDown:
			selection++;
			break;
		case Joystick::State::kSelect:
			return selection%numberOfSelection;
		}
	}
}

}

}



