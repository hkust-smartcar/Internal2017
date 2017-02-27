#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <functional>

//Looper Header File----------------------------------------------------------------------------------------------
#include <libutil/looper.h>

//Joy Stick Header File-----------------------------------------------------------------------------------------------------
#include<libsc/joystick.h>

//led Header File-----------------------------------------------------------------------------------------------------------
#include <libsc/led.h>

//LCD Header File-----------------------------------------------------------------------------------------------------------
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>

//namespace-----------------------------------------------------------------------------------------------------------------
using namespace libsc;
using namespace libsc::k60;
using namespace libbase::k60;
using namespace libutil;
using namespace std;

//Smart Car CPP---------------------------------------------------------------------------------------------------
#include"../inc/smart_car.h"

//Snake Game CPP--------------------------------------------------------------------------------------------------
#include "../inc/snake_game.h"

void guiSelectionArrayCreate(int NumberToSelect);
void guiSelectionInsert(int number,int selection,string name);
void gui(Joystick *FiveWaySwitch,St7735r *LCD,LcdConsole *Console,Ov7725 *Cam,FutabaS3010 *Servo,AlternateMotor *MotorA,AlternateMotor *MotorB,DirEncoder *EncoderA,DirEncoder *EncoderB);
void guiDeleteAllSelection();
