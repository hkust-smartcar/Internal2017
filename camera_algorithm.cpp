

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
int middle_color(const bool v[][80])
{
	int count1= 0, count0= 0;

	for(int i = 0; i < 10; i ++)
	{

		for(int j = 25; j < 55; j ++)
		{
			if(v[i][j])
			{
				count1++;
			}
			else
			{
				count0++;
			}
		}

	}

	return count1;
	if(count0>count1)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}
int ovall_color(const bool v[][80])
{
	int count1= 0, count0= 0;

	for(int i = 0; i < 15; i ++)
	{
		for(int j = 0; j < 20; j ++)
		{
			if(v[i][j]&1== 0)
			{
				count0++;
			}
			else
			{
				count1++;
			}
		}
		for(int j = 60; j < 80; j ++)
		{
			if(v[i][j]&1== 0)
			{
				count0++;
			}
			else
			{
				count1++;
			}
		}


	}
	return count1;
	if(count0>count1)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}
//int med_filter(int x, int y, int z)
//{
	//
	//    int count[3][2];
	//    count[0][0]= x;
	//    count[1][0]= y;
	//    count[2][0]= z;
	//    count[0][1]= 0;
	//    count[1][1]= 0;
	//    count[2][1]= 0;
	//    int maxc= 0;
	//    int minc= 0;
	//
	//    for(int i = 0; i < 3; i ++)
		//    {
		//    	 if(count[i][0]== max(max(x,y),z)&&maxc== 0) {
//    	                maxc= 1;
//    	                count[i][1] == 1;
//    	        }
//    	        else if(count[i][0]==min(min(x,y),z)&&minc== 0){
//    	                minc= 1;
//    	                count[i][1] == 1;
//    	        }
//    }
//    for(int i = 0; i < 3; i ++)
//    {
//        if(count[i][1]==0) return count[i][0];
//    }
//
//}

using namespace libsc;
using namespace libbase::k60;
using namespace libsc::k60;

int main(void)
{
	System::Init();
	const Byte *LB= nullptr;
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
	int index; //v for value
	int areaDiff=0;
	int range= 150;
	int t;
	int type;
	int n;
	char buffer[50];
	int cr1, cr0;
	int rn1= 0, rn0= 0;
	Camera.Start();
	while(1){
		while (time_img != System::Time()){
			time_img = System::Time();
			if (time_img % 10 == 0){
				rn1 = 0;
				rn0 = 0;
				cr1 = 0;
				cr0 = 0;
				LB= Camera.LockBuffer();
				Lcd.SetRegion(Lcd::Rect(0,0,80,60));
				Lcd.FillBits(libutil::GetRgb565(0,0,255), libutil::GetRgb565(255, 255, 255), LB,  Camera.GetBufferSize()*8);
				Camera.UnlockBuffer();

				areaDiff = 0;
				for(int line=0; line<15; line++){
					index=0;
					for(int i=0; i<10 ; i++){
						for(int j=7; j>=0; j--){
							if(((*(LB+line*10+i))>>j)&1==1) v[line][index++] = true;
							else v[line][index++] = false;
						}
					}

					for(int i=1;i<79;i++){
						//                                            v[line][i]= med_filter(v[line][i-1]&1, v[line][i]&1, v[line][i+1]&1);
						//                                                if(v[line][i]==1&&i<40) {
							//                                                    areaDiff--;
							//                                                }
						//                                                else if(v[line][i]==1 && i >=40{
						//                                                    areaDiff++;
						//                                                }
						if(v[line][i-1]+v[line][i]+v[line][i+1]>=2)
						{
							if(i < 40)
							{
								areaDiff--;
								if(i > 25)
								{
									rn1++;
								}
								else if(i < 15)
								{
									cr1++;
								}
							}
							else
							{
								areaDiff++;
								if(i < 55)
								{
									rn1++;
								}
								else if(i >= 65)
								{
									cr1++;
								}
							}
						}
						else
						{
							if(i > 25 && i < 55)	rn0++;
							else if(i < 15 || i > 65) cr0++;
						}

					}
				}

				if(abs(areaDiff)>range)
				{
					if(areaDiff>0)
					{
						type= 0;
					}
					else
					{
						type= 1;
					}

				}

				else
				{


					if(rn1>rn0)
					{
						type= 2;
					}
					else

					{	if(cr1> cr0)
					{
						type = 3;
					}

					else
					{
						type= 4;
					}

					}


				}
				if(type == 0)
				{
					n= sprintf(buffer,"Left Turn");
				}
				if(type == 1)
				{
					n= sprintf(buffer,"Right Turn");
				}
				if(type == 2)
				{
					n= sprintf(buffer,"Roundabout");
				}
				if(type == 3)
				{
					n= sprintf(buffer,"Straight");
				}
				if(type == 4)
				{
					n = sprintf(buffer,"Crossroad");
				}



				Lcd.SetRegion(Lcd::Rect(0,101,100,200));
				screen.WriteString(buffer);
				//midx = middle_color(v);
				Lcd.SetRegion(Lcd::Rect(0, 121, 100, 200));
				n = sprintf(buffer, "Rnd Det: %d", rn1);
				screen.WriteString(buffer);
				//crossroad = ovall_color(v);
				Lcd.SetRegion(Lcd::Rect(0, 141, 100, 200));
				n = sprintf(buffer, "Crs Det: %d", cr1);
				screen.WriteString(buffer);



			}
		}
	}

	return 0;
}


