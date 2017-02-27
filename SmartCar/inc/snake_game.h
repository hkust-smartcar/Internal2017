#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

//Random Number Header File-------------------------------------------------------------------------------------------------
#include<stdlib.h>

//Joy Stick Header File-----------------------------------------------------------------------------------------------------
#include<libsc/joystick.h>

//LCD Header File-----------------------------------------------------------------------------------------------------------
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>

//namespace-----------------------------------------------------------------------------------------------------------------
using namespace libsc;
using namespace libsc::k60;
using namespace libbase::k60;
using namespace std;

int snake_game(Joystick *FiveWaySwitch,St7735r *LCD,LcdConsole *Console);
