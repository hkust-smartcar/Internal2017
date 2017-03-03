 
/////////////*
//////////// * motor pid.cpp
//////////// *
//////////// *  Created on: Jan 18, 2017
//////////// *      Author: lzhangbj
//////////// */
#include <cassert>
using namespace std;
#include <stdio.h>
#include <cstring>
#include <cstdint>
#include <stdint.h>
#include <inttypes.h>

#include <libbase/k60/mcg.h>
#include <libsc/system.h>
//#include <libsc/k60/ov7725.h>
#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libutil/misc.h>
#include <libsc/button.h>
#include <libsc/lcd_typewriter.h>

#include <libutil/string.h>
#include <libsc/encoder.h>
#include <libsc/alternate_motor.h>
#include <libsc/motor.h>
#include <libsc/dir_motor.h>
#include <libsc/lcd.h>
#include <libsc/tower_pro_mg995.h>

#include <libsc/dir_encoder.h>
#include <libsc/k60/uart_device.h>
#include <libbase/k60/uart.h>
#include <libsc/k60/jy_mcu_bt_106.h>
#include <libsc/mpu6050.h>
//#include "pVarManager.h"
#include "PID.h"
#include "Filter.h"
#include "FBalanceCtrl.h"
#include "SpeedCtrl.h"




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

/* ******************************     Balance control  *************************************/

/*
 * left  motor -> encoder2 ->  B ->  true  ->  1
 * right motor -> encoder1 ->  A ->  false -> -1
 *
 */

float setpoint =  60;



//  control command
bool ifusempu = true;
bool ifusefilter = true;

bool ifusenopidcontrol = false;
bool ifusepid = false;

bool ifusebalancecontrol = false;
bool ifusespeedcontrol = false;

bool ifusebluetooth = true;


//	time interval
const uint8_t mpu_time_interval = 1;					//(AngleCalculate)
const uint8_t BC_calculate_time_interval = 5;
const uint8_t SC_calculate_time_interval = 5;
const uint8_t ctrl_time_interval = 1;
const uint8_t bt_time_interval = 15;


//for filter
const float k1 = 0.05;	// Tg  under the time_interval of 5ms

//for Balance Ctrl
const float k2 = 100000;	//used to calculate acceleration
const float k3 = 0;		//used to calculate acceleration

//for Speed Ctrl
const float k4 = 0;
const float k5 = 0;


//for PID
const float pl=1;
const float pr=1;
const float i=0;
const float dl=0;
const float dr=0;

// for wheel direction
const bool LForDir = true;
const bool RForDir = false;

int main()
{

	System::Init();


	/*
	 * 											led config
	 */
	Led::Config led_config;
	led_config.id=0;
	led_config.is_active_low=false;
	Led led(led_config);


	/*
	 * 											bluetooth config
	*/
	JyMcuBt106::Config bluetooth_config;
	bluetooth_config.id=0;
	bluetooth_config.baud_rate=libbase::k60::Uart::Config::BaudRate::k115200;
//	bluetooth_config.tx_buf_size = 200;
	JyMcuBt106 bluetooth(bluetooth_config);


	Button::Config button_config;
	button_config.id=0;
	button_config.is_active_low=true;
	Button button1(button_config);
	button_config.id=1;
	Button button2(button_config);


	/*
	 * 											motor config
	 */
	AlternateMotor::Config lmotor_config,rmotor_config;
	lmotor_config.id=1;
	AlternateMotor LMotor(lmotor_config);
	rmotor_config.id=0;
	AlternateMotor RMotor(rmotor_config);




	/*
	 * 											mpu6050 config
	 */

	Mpu6050::Config mpu_config;
	mpu_config.gyro_range=Mpu6050::Config::Range::kLarge;
	mpu_config.accel_range=Mpu6050::Config::Range::kLarge;
	Mpu6050 mpu(mpu_config);




	/*
	 * 										encoder config
	 */
	DirEncoder::Config encoder_config;
	encoder_config.id=1;
	DirEncoder LEncoder(encoder_config);
	encoder_config.id=0;
	DirEncoder REncoder(encoder_config);
	led.SetEnable(false);

	/*
	 * 										Filter
	 */
	Filter ilter;
	Filter*filter=&ilter;
	filter->Config(&mpu, 0.05, mpu_time_interval,0);


	/*
	 * 										PID
	 */



										PID	Lpid;
										PID* LPID=&Lpid;
										LPID->ConfigCo(pl, i, dl,1);
										LPID->ConfigIMaxMin(0,0);
										LPID->ConfigOutMaxMin(999,1);
	LMotor.SetClockwise(LForDir);		LPID->SetMotorDir(LForDir);
//										LPID->SetWheelSign(1);
	LMotor.SetPower(0);					LPID->SetPoint(0);					float LPower=0;

										PID rpid;
										PID* RPID=&rpid;
										RPID->ConfigCo(pr, i, dr,-1);
										RPID->ConfigIMaxMin(0,0);
										RPID->ConfigOutMaxMin(999,1);
	RMotor.SetClockwise(RForDir);		RPID->SetMotorDir(RForDir);
//										RPID->SetWheelSign(-1);
	RMotor.SetPower(0);					RPID->SetPoint(0);					float RPower=0;



	/*
	 * 										Balance control
	 */
	BalanceCtrl bc;
	BalanceCtrl* BC=&bc;
	BC->SetBSC(k2, k3);
	BC->Config(filter, LPID, RPID, BC_calculate_time_interval,ctrl_time_interval);

	/*
	 *										Speed Control
	 */
	SpeedCtrl sc;
	SpeedCtrl* SC=&sc;
	SC->Config(0,k4,k5,SC_calculate_time_interval, ctrl_time_interval);


	char LBuffer[100];

	//  get some accel sample values
	uint64_t prepare_time = System::Time();
	while(System::Time() - prepare_time < 500){
		mpu.Update();
		filter->UpdateAccelAng(1);
		filter->StoreHistory();
	}

	uint64_t mpu_pre_time=System::Time();
	uint64_t mpu_cur_time=0;

	uint64_t BC_pre_time = System::Time();
	uint64_t BC_cur_time = 0;

	uint64_t SC_pre_time = System::Time();
	uint64_t SC_cur_time = 0;


	uint64_t ctrl_pre_time = System::Time();
	uint64_t ctrl_cur_time = 0;


	uint64_t bt_pre_time=System::Time();
	uint64_t bt_cur_time=0;
//
	float left_count;
	float right_count;


	while(1){


//   1.  use mpu to get filter angle
//		 use balancectrl to get motor speed;
//		 use led test
//   class:  mpu  filter  balancectrl  led
//
					mpu_cur_time=System::Time();
					if(  mpu_cur_time - mpu_pre_time == mpu_time_interval){
						mpu_pre_time=mpu_cur_time;
						led.Switch();

						mpu.Update();
						filter->UpdateAccelAng(3);
						filter->StoreHistory();
						filter->FilterAccelAng();
						filter->UpdateGyroAngSpeed();
						filter->UpdateFilterAng();
					}
//	2.use Balance Ctrl to get AngleOutput
//	class: FBalanceCtrl
					BC_cur_time = System::Time();
					if(BC_cur_time - BC_pre_time == BC_calculate_time_interval){
						BC->Update();
					}

// 3.use Speed Ctrl to get SpeedOut
//	class: SpeedCtrl



					SC_cur_time = System::Time();
					if(SC_cur_time - SC_pre_time == SC_calculate_time_interval){
						SC->GetCount((int32_t)left_count,(int32_t)right_count);
						SC->Update();
					}

					ctrl_cur_time = System::Time();
					if(ctrl_cur_time - ctrl_pre_time == ctrl_time_interval){

	// directly control motor
						if(ifusenopidcontrol){
						LPower = BC->GetCtrlPoint() - SC->GetSpeedOut();
						RPower = BC->GetCtrlPoint() - SC->GetSpeedOut();

						if( LPower > 0 )  LMotor.SetClockwise(LForDir);
						if( LPower < 0 ) { LMotor.SetClockwise(!LForDir);	LPower = -LPower;	}
						if(	RPower > 0 )  RMotor.SetClockwise(RForDir);
						if(	RPower < 0 ) { RMotor.SetClockwise(!RForDir);	RPower = -RPower;   }
						}
//	4.	use pid to approach the calculated speed
//	class: pid  motor
						left_count=LEncoder.GetCount();
						right_count=REncoder.GetCount();

						if(ifusepid){
						LPID->InputValue(left_count,LPower);
						RPID->InputValue(right_count,RPower);

						LPID->SetPoint(BC->GetCtrlPoint()- SC->GetSpeedOut());     RPID->SetPoint(BC->GetCtrlPoint() - SC->GetSpeedOut());

						LPID->Update();						       RPID->Update();

						LMotor.SetClockwise(LPID->GetMotorDir());  RMotor.SetClockwise(RPID->GetMotorDir());
						LPower=LPID->GetOutPut();                  RPower=RPID->GetOutPut();
						LMotor.SetPower(RPower);       			   RMotor.SetPower(RPower);
						}

						LEncoder.Update();                         REncoder.Update();
					}

//	5. use bluetooth to send data
					bt_cur_time=System::Time();
					if(bt_cur_time - bt_pre_time == bt_time_interval){
						bt_pre_time = bt_cur_time;

						float setpoint = BC->GetLeftSetPoint();
						std::array<float ,3>accelangle = mpu.GetAccel();
						std::array<float, 3>angle = filter->GetFilterAng();
						sprintf(LBuffer, "%.1lf,%.2lf,%.2lf\n",1.0,angle[2],accelangle[2]);
						const Byte speedByte = 85;
						bluetooth.SendBuffer(&speedByte, 1);
						bluetooth.SendStr(LBuffer);
					}

	}
	return 0;
}

