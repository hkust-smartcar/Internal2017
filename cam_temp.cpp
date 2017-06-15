
#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <algorithm>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libsc/k60/ov7725.h>
#include <libsc/system.h>
#include <libsc/st7735r.h>
#include <libutil/misc.h>
#include <libsc/lcd_typewriter.h>
#include <libsc/futaba_s3010.h>
#include <libsc/servo.h>
#include <libsc/alternate_motor.h>
#include <libsc/motor.h>

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

int main(void)
{
	System::Init();
	const Byte *LB= nullptr;
	//motor config
	   	    	 AlternateMotor::Config motor1_config,motor2_config;
	   			 motor1_config.id=0;
	   	    	 AlternateMotor Lmotor(motor1_config);
	   	    	 //Lmotor.SetPower(300);
	   	    	 Lmotor.SetClockwise(true);
	   	    	 motor2_config.id=1;
	        	 AlternateMotor Rmotor(motor2_config);
	    	   	 //Rmotor.SetPower(300);
	    	   	 Rmotor.SetClockwise(false);
	    	   	 int motor1_speed;
	    	   	 int motor2_speed;
	    	     int dir=1;

	//servo config
	    	 FutabaS3010::Config servo_config;
	    	 servo_config.id=0;
	    	 FutabaS3010 servo(servo_config);
	St7735r::Config ConfigLCD;
	St7735r Lcd(ConfigLCD);
	LcdTypewriter::Config screen_config;
	screen_config.lcd=&Lcd;
	LcdTypewriter screen(screen_config);
	Ov7725::Config ConfigCam;
	ConfigCam.id = 0;
	ConfigCam.w = 80;
	ConfigCam.h = 60;
	Ov7725 Camera(ConfigCam);
	Timer::TimerInt time_img = 0;
	bool v[60][80];
	bool processed_v[4800];

	int left_edge[60]= {}, left_flag[60]= {}, right_edge[60];
	int length_difference[60];

	for(int i = 0; i < 60; i ++)
	{
		left_edge[i] = 0;
		left_flag[60] = 0;
		right_edge[i] = 79;
	}

	int index; //v for value
	int areaDiff=0;
	int range= 150;
	int t;
	int type;
	int n;
	char buffer[50];
	int cr1, cr0;
	int rn1= 0, rn0= 0;
	bool crss = 0;
	int leftcornerfound = 0;
	int rightcornerfound = 0;
	int left_gradient, right_gradient;
	int centerline[9];
	Camera.Start();
	while(1){
		//		time_img = System::Time();
		while (time_img != System::Time()){
			time_img = System::Time();
			if (time_img % 10 == 0){
				left_gradient = 0;
				right_gradient = 0;
				rn1 = 0;
				rn0 = 0;
				for(int i = 0; i < 60; i ++)
				{
					left_edge[i] = 0;
					left_flag[i] = 0;
					right_edge[i] = 79;
				}
				for(int i = 0; i < 4800; i ++)
				{
					processed_v[i]= 0;
				}
				int place = 121;
				crss = 0;
				rn1 = 0;
				rn0 = 0;
				cr1 = 0;
				cr0 = 0;
				LB= Camera.LockBuffer();
				Lcd.SetRegion(Lcd::Rect(0,0,80,60));
				Lcd.FillBits(libutil::GetRgb565(0,0,255), libutil::GetRgb565(255, 255, 255), LB,  Camera.GetBufferSize()*8);
				Camera.UnlockBuffer();
				leftcornerfound = 0;
				rightcornerfound = 0;
				areaDiff = 0;
				for(int line=0; line<60; line++){
					index=0;
					for(int i=0; i<10 ; i++){
						for(int j=7; j>=0; j--){
							if(((*(LB+line*10+i))>>j)&1==1) v[line][index++] = true;
							else v[line][index++] = false;
						}
					}

				}


				for(int line = 58; line >= 0; line=line-2)
				{
					int newline = 59;
					for(int i = 7; i >=0; i --)
					{
						int edge = 0;
						int f = 0;
						for(int j = 5*i + 4 ; j>= 5*i; j --)
						{	f = j;
						if(!v[line][j])
						{
							edge ++;
						}
						}

						if(edge!=5&& f !=0 )
						{
							if(v[line][f-1])
							{
								left_edge[line]= f;
								break;
							}
						}

					}

					if(line < 56 &&((left_edge[line]==0 && left_edge[line+4] > 2) || (abs(left_edge[line]-left_edge[line+4])>6))&& abs(line-newline)> 6)
					{
						leftcornerfound++;
						newline = line;

					}


				}
				for(int line = 58; line >=0; line=line-2)
				{
					int newline = 59;
					for(int i = 8; i < 16; i ++)
					{
						int edge = 0;
						int f = 0;
						for(int j = 5*i ; j< 5*i + 5; j ++)
						{	f = j;
						if(!v[line][j])
						{
							edge ++;
						}
						}
						if(edge!=5&& f !=79 )
						{
							if(v[line][f+1])
							{
								right_edge[line]= f;
								break;
							}
						}

					}
					if(line <56 &&(( right_edge[line]== 79 && right_edge[line+4]< 77) ||(abs(right_edge[line]-right_edge[line+4])>6)) && abs(line-newline)> 6 )
					{

						rightcornerfound++;
						newline = line;

					}

				}
				type = 0;
				//				Lcd.SetRegion(Lcd::Rect(0, 121, 100, 200));
				//				n = sprintf(buffer, "LC: %d", leftcornerfound);
				//				screen.WriteString(buffer);
				//				Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
				//				n = sprintf(buffer, "RC: %d", rightcornerfound);
				//				screen.WriteString(buffer);
				if(leftcornerfound && rightcornerfound == 0)
				{
					Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
					n = sprintf(buffer, "Left");
					screen.WriteString(buffer);
					type = 1;
				}
				else if(leftcornerfound == 0 && rightcornerfound)
				{
					Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
					n = sprintf(buffer, "Right");
					screen.WriteString(buffer);
					type = 2;
				}
				else if(leftcornerfound && rightcornerfound)
				{



					for(int line = 25; line < 40; line++)
					{
						for(int i = 26; i < 54; i++)
						{
							if(v[line][i])
								rn1++;
							else
								rn0++;
						}
					}
					if(rn1 > rn0)
					{
						Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
						n = sprintf(buffer, "Rbt");
						screen.WriteString(buffer);
						type = 4;

					}
					else
					{
						Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
						n = sprintf(buffer, "Crs");
						screen.WriteString(buffer);
						type = 3;
					}

				}
				else
				{
					for(int line = 35; line < 50; line++)
					{
							for(int i = 26; i < 54; i++)
									{
										if(v[line][i])
													rn1++;
										else
													rn0++;
									}
					}
					if(rn1 > rn0)
										{
											Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
											n = sprintf(buffer, "Rbt");
											screen.WriteString(buffer);
											type = 4;

										}
					else
					{
						Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
											n = sprintf(buffer, "Straight");
											screen.WriteString(buffer);
											type = 3;
					}

				}

				if(type !=4)
				{
					int i = 0;
					for(int line = 56; line >=32; line = line -4)
					{
						centerline[i]= (left_edge[line]+ right_edge[line])/2;
						i ++;
					}
					int turn=0;
					for(int i = 0; i < 7; i++)
					{
						turn += centerline[i]-40;
					}
                    turn *= turn/70;
					Lcd.SetRegion(Lcd::Rect(0, 121, 100, 200));
					n = sprintf(buffer, "%d", turn);
					screen.WriteString(buffer);
					servo.SetDegree(830 - (turn)*3.2);
				}
				else
				{
					servo.SetDegree(1100);
				}

				// printing ou the processed image
				for(int line = 0; line < 60; line++)
				{
					for(int i = 0; i < 80; i ++)
					{
						if((line*80 + i >= line*80 + left_edge[line]) && (line*80+i <= line*80+ right_edge[line]))
						{
							processed_v[line*80+ i] = 1;
						}
					}
					processed_v[line*80+left_edge[line]]= 1;
					processed_v[line*80+right_edge[line]]= 1;

				}
				// 800
				//1050
				//450
				//14
				//servo.SetDegree(830);
				    					//Rmotor.SetPower(300);
				Lcd.SetRegion(Lcd::Rect(0,60,80,120));
				Lcd.FillBits(libutil::GetRgb565(0,150,255), libutil::GetRgb565(255, 255, 255), processed_v,  4800);


			}
		}
	}

	return 0;
}
