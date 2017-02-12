#include <stdio.h>
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
#include <libsc/mpu6050.h>




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

//accelerometer and gyroscope config
    	Mpu6050::Config balance_config;
    	Mpu6050 mpu6050(balance_config);

    	St7735r::Config lcd_config;
    	St7735r lcd(lcd_config);

    	LcdTypewriter::Config screen_config;
    	screen_config.lcd=&lcd;
    	LcdTypewriter screen(screen_config);



    	const Byte *LB = nullptr;
    	int acceleration[3], omega[3];
    	int n;
    	char buffer[50];
    		while(true){
    			while(t!=System::Time()){
    				t = System::Time();

    				if(t % 10 == 0){

    					for(int i = 0; i < 3; i ++)
    					{
    						mpu6050.Update(1);
    						acceleration[i] = mpu6050.GetAccel()[i];    // 16384 per gravitational acceleration
    						omega[i]= mpu6050.GetOmega()[i];
    					}
    					 n= sprintf(buffer,"Ax: %d", acceleration[0]);
    					 lcd.SetRegion(Lcd::Rect(1,1,100,100));
    					 screen.WriteString(buffer);

    					 n= sprintf(buffer,"Ay: %d", acceleration[1]);
    					 lcd.SetRegion(Lcd::Rect(1,21,100,150));
    					 screen.WriteString(buffer);

    					 n= sprintf(buffer,"Az: %d", acceleration[2]);
    					 lcd.SetRegion(Lcd::Rect(1,41,100,200));
    					 screen.WriteString(buffer);

    				     n= sprintf(buffer,"Ox: %d", omega[0]);
    				     lcd.SetRegion(Lcd::Rect(1,61,100,100));
    				     screen.WriteString(buffer);

    					 n= sprintf(buffer,"Oy: %d", omega[1]);
    					 lcd.SetRegion(Lcd::Rect(1,81,100,150));
    					 screen.WriteString(buffer);

    					 n= sprintf(buffer,"Oz: %d", omega[2]);
    				     lcd.SetRegion(Lcd::Rect(1,101,100,200));
    				     screen.WriteString(buffer);



    				}
    			}
    		}


    		return 0;


}
