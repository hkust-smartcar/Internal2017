#include <cmath>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/k60/ov7725.h>
#include <libsc/led.h>
#include <libsc/st7735r.h>
#include <libutil/misc.h>
#include <libsc/button.h>
#include <libsc/lcd_typewriter.h>
#include <libsc/futaba_s3010.h>
#include <libsc/servo.h>
#include <stdint.h>
#include <inttypes.h>
#include <libutil/string.h>
#include <libsc/encoder.h>
#include <libsc/ab_encoder.h>
#include <libsc/alternate_motor.h>
#include <libsc/motor.h>
#include <libsc/lcd.h>
#include <libsc/tower_pro_mg995.h>
#include <stdio.h>
#include <libsc/encoder.h>
#include <libsc/dir_encoder.h>
#include <libsc/k60/uart_device.h>
#include <libbase/k60/uart.h>
#include <libsc/k60/jy_mcu_bt_106.h>
/*
 servo middle: 720;
  	     right:370
  	     left:1070
 */



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

using namespace std;
using namespace libsc;
using namespace libbase::k60;
using namespace libsc::k60;

int main(void)
{
    	System::Init();

//motor config
   	    	 AlternateMotor::Config motor1_config,motor2_config;
   			 motor1_config.id=0;
   	    	 AlternateMotor Lmotor(motor1_config);
   	    	 //Lmotor.SetPower(300);
   	    	 Lmotor.SetClockwise(false);
   	    	 motor2_config.id=1;
        	 AlternateMotor Rmotor(motor2_config);
    	   	 //Rmotor.SetPower(300);
    	   	 Rmotor.SetClockwise(true);
    	   	 int motor1_speed;
    	   	 int motor2_speed;
    	     int dir=1;

//servo config
    	 FutabaS3010::Config servo_config;
    	 servo_config.id=0;
    	 FutabaS3010 servo(servo_config);


//camera config
    	Ov7725::Config cam_config;
    	cam_config.id=0;
    	cam_config.w=80;
    	cam_config.h=60;
    	Ov7725 cam(cam_config);
    	Timer::TimerInt t=0;

    	const Byte *LB = nullptr;

    	bool v[60][80];
    	int index; //v for value
    	int areaDiff=0, orgAreaDiff;
    	int pControl, dControl, power=280;
    	double Kp = 1.5, Kd = 0.5;

    		cam.Start();

    		while (true){
    			while(t!=System::Time()){
    				t = System::Time();

    				if(t % 10 == 0){
    					LB = cam.LockBuffer();

    					orgAreaDiff = areaDiff;
    					areaDiff = 0;
    					for(int line=13; line<28; line++){
    						index=0;
    						for(int i=0; i<10 ; i++){
    							for(int j=7; j>=0; j--){
    								if(((*(LB+line*10+i))>>j)&1==1) v[line][index++] = true;
    								else v[line][index++] = false;
    							}
    						}

    						for(int i=1;i<79;i++){
    							if((v[line][i-1]&1)+(v[line][i]&1)+(v[line][i+1]&1) >= 2){
    								if(i<40) {
    									areaDiff--;
    								}
    								else{
    									areaDiff++;
    								}
    							}
    						}
    					}
    					pControl = areaDiff * Kp; //to be determined
    					dControl = (areaDiff-orgAreaDiff) * Kd; //to be determined
    					if((900+pControl+dControl>1000) ||(900+pControl+dControl<800)) {
    						if(power-1>275) power--;
    					}
    					else if(power+1<288) power++;

    					Lmotor.SetPower(power);
    					Rmotor.SetPower(power);

    					areaDiff=0;
    					for(int line=45; line<60; line++){
    					    index=0;
    					    for(int i=0; i<10 ; i++){
    					    	for(int j=7; j>=0; j--){
    					    		if(((*(LB+line*10+i))>>j)&1==1) v[line][index++] = true;
    					    		else v[line][index++] = false;
    					    	}
    					    }

    					   for(int i=1;i<79;i++){
    					    	if((v[line][i-1]&1)+(v[line][i]&1)+(v[line][i+1]&1) >= 2){
    					    	if(i<40) areaDiff--;
    					    	else areaDiff++;
    					    	}
    					   }
    					}
    					pControl = areaDiff * Kp; //to be determined
    					dControl = (areaDiff-orgAreaDiff) * Kd; //to be determined
    					if((900+pControl+dControl>1000) ||(900+pControl+dControl<800)) {

    						servo.SetDegree(900+pControl+dControl);
    					}
    					else servo.SetDegree(900);

    					cam.UnlockBuffer();
    				}
    			}
    		}

    		cam.Stop();

    		return 0;


}
