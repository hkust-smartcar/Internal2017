#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

#include <functional>

//Looper Header File----------------------------------------------------------------------------------------------
#include <libutil/looper.h>

//Button Header File--------------------------------------------------------------------------------------------------------
#include<libsc/button.h>

//Joy Stick Header File-----------------------------------------------------------------------------------------------------
#include<libsc/joystick.h>

//led Header File-----------------------------------------------------------------------------------------------------------
#include <libsc/led.h>

//LCD Header File-----------------------------------------------------------------------------------------------------------
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>

//Camera Header File--------------------------------------------------------------------------------------------------------
#include<libsc/k60/ov7725.h>

//Servo Header File---------------------------------------------------------------------------------------------------------
#include<libsc/futaba_s3010.h>

//Motor Header File---------------------------------------------------------------------------------------------------------
#include<libsc/alternate_motor.h>
#include<libsc/motor.h>

//Dir Encoder Header File
#include<libsc/dir_encoder.h>

#include"../inc/smart_car.h"

#include <libsc/k60/jy_mcu_bt_106.h>

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

//GUI CPP---------------------------------------------------------------------------------------------------------
#include "../inc/gui.h"
