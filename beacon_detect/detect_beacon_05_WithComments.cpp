/*
 * main.cpp
 *
 * Author: Peter
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
//#include <libsc/led.h>
//#include <libsc/button.h>
#include <libsc/k60/ov7725.h>
#include <libsc/k60/ov7725_configurator.h>
#include <libsc/lcd.h>
#include <libsc/st7735r.h>
#include <libsc/system.h>
#include <libsc/config.h>
#include <libsc/servo.h>
#include <libsc/motor.h>

#include <libsc/mpu6050.h>
#include <libsc/ak8963.h>

#include <libsc/lcd_typewriter.h>
#include <libsc/tower_pro_mg995.h>
#include <libsc/k60/jy_mcu_bt_106.h>
//#include <libutil>
//#include <libsc/config.h>
#include <cmath>


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
//using namespace libbase::k60;
using namespace k60;

struct circle
{
	int x;
	int y;
	int distance;
};

void circle_detection(int image[80][60], int length, int width);
void circle_display();

int image[80][60];
bool procArray[80];
Byte* track;

int height = 60;
int camera_width = 80;

double point_distance = 0;
int c1x = 0;
int c1y = 0;
int c2x = 0;
int c2y = 0;
char display[50];

//void imageInit();
//void extractArray(int row);
void getImage(Byte * data);
circle sphere[2];

St7735r* lcdP;
LcdTypewriter* writerP;

int main(void)
{

	int tempC1x = 0;
	int tempC1y = 0;
	int tempC2x = 0;
	int tempC2y = 0;

	int tempC1d = 0;
	int tempC2d = 0;

	int tempcAx = 0;
	int tempcAy = 0;
	int tempcAd = 0;

	// system init-----------------------------------------
	System::Init();

	// lcd init-----------------------------------------

	St7735r::Config st;
	st.fps = 100;
	st.is_revert=true;
	St7735r lcd1(st);
	lcdP = &lcd1;

	LcdTypewriter::Config config;
	config.lcd = &lcd1; // St7735r lcd from prev page
	LcdTypewriter writer(config);
	writerP = &writer;

	// camera init-----------------------------------------
	Ov7725::Config ov;
	ov.id = 0;
	ov.w = 80;
	ov.h = 60;
	ov.fps = Ov7725::Config::Fps::kMid;	//100

	Ov7725 cam1(ov);

	JyMcuBt106::Config blth;
	blth.id = 0;
	blth.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	//	blth.rx_isr = bluetoothFunc;

	JyMcuBt106 bt1(blth);

	cam1.Start();
	uint32_t last_time = System::Time();


//	circle test;
	char buf[50];

	while (true)
	{
	//	ak1.
		if ( (System::Time() - last_time) == 150)
		{
			last_time = System::Time();
	//		lcd1.Clear(0);
			lcd1.SetRegion(Lcd::Rect(0,0,80,60));
			// Collect camera input & print it on LCD-----------------------------------------
					track = (Byte*) cam1.LockBuffer();
					lcd1.FillBits(lcd1.kBlack, lcd1.kWhite, track, cam1.GetBufferSize()*8);
			// Signal proc. : Byte to 2D array -----------------------------------------
					getImage(track);
					//	camera1.UnlockBuffer();
					//	lcd1.FillBits(lcd1.kBlack, lcd1.kWhite, cam1.LockBuffer(), cam1.GetBufferSize()*8);	//8
			cam1.UnlockBuffer();

// Draw 2 squares on the screen
			lcd1.SetRegion(Lcd::Rect(tempC1x, tempC1y, tempC1d, tempC1d));
			lcd1.FillColor(lcd1.kRed);

			lcd1.SetRegion(Lcd::Rect(tempC2x, tempC2y, tempC2d, tempC2d));
			lcd1.FillColor(lcd1.kBlue);

// Detect circle
			circle_detection(image,80,60);
	//		circle_display();

// Circle detected
			if (sphere[1].distance != 0)
			{
				if (sphere[2].distance == 0)			// only get one circle
				{
					if ((tempC2x - sphere[1].x < 8)||(sphere[1].x - tempC2x < 8) )
					{									// comparing x, y coordinates with previous circle-2
						if ((tempC2y - sphere[1].y < 8)||(sphere[1].y - tempC2y < 8) )
						{								// this circle should be indeed circle-2
							tempC1x = sphere[2].x;
							tempC1y = sphere[2].y;
							tempC1d = sphere[2].distance;
						}
					}
				}
// maybe need to add 'else'
				tempC1x = sphere[1].x;
				tempC1y = sphere[1].y;
				tempC1d = sphere[1].distance;

				c1x = (sphere[1].x + sphere[1].distance)/2;		// finding centre
				c1y = (sphere[1].y + sphere[1].distance)/2;

				lcd1.SetRegion(Lcd::Rect(10,90,80,30));		// print out the centre
				//		writer.WriteString("X/ Y/ Dist");
				sprintf(buf,"X=  %d", c1x);
				writer.WriteString(buf);

				lcd1.SetRegion(Lcd::Rect(10,105,80,30));
				//		writer.WriteString(buf);
				sprintf(buf,"Y=  %d", c1y);
				//		lcd1.SetRegion(Lcd::Rect(10,100,80,60));
				writer.WriteString(buf);


/*			point_distance = sqrt((sphere[1].x)*(sphere[1].x)+(sphere[1].y)*(sphere[1].y));
			lcd1.SetRegion(Lcd::Rect(10,120,80,30));
		//	sprintf(buf,"Dist=  %d", sphere[1].distance);
			sprintf(buf,"Dist=  %d", point_distance);
			writer.WriteString(buf);
*/
				lcd1.SetRegion(Lcd::Rect(sphere[1].x, sphere[1].y, sphere[1].distance, sphere[1].distance));
				lcd1.FillColor(lcd1.kRed);				// Fill circle-1 with RED

				// Similar to circle 2
				if (sphere[2].distance > 0)
				{
					tempC2x = sphere[2].x;
					tempC2y = sphere[2].y;
					tempC2d = sphere[2].distance;

					c2x = (sphere[2].x + sphere[2].distance)/2;
					c2y = (sphere[2].y + sphere[2].distance)/2;

//					lcd1.SetRegion(Lcd::Rect(45,90,80,30));
					//		writer.WriteString("X/ Y/ Dist");
//					sprintf(buf,"X=  %d", c2x);
//					writer.WriteString(buf);

//					lcd1.SetRegion(Lcd::Rect(45,105,80,30));
					//		writer.WriteString(buf);
//					sprintf(buf,"Y=  %d", c2y);
					//		lcd1.SetRegion(Lcd::Rect(10,100,80,60));
//					writer.WriteString(buf);

					lcd1.SetRegion(Lcd::Rect(sphere[2].x, sphere[2].y, sphere[2].distance, sphere[2].distance));
					lcd1.FillColor(0x001F);

	// Distance between 2 circles
					point_distance = sqrt((c1x-c2x)*(c1x-c2x)+(c1y-c2y)*(c1y-c2y));
			//		cout << point_distance <<endl;
					lcd1.SetRegion(Lcd::Rect(10,120,80,30));
					sprintf(buf,"DsB= %f", point_distance);
					writer.WriteString(buf);
	//			}
				}
			}


		}
	}

	return 0;
}



void getImage(Byte * data)
{
	int pos = 0, bit_pos = 8;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < camera_width; x++)
		{
			if (--bit_pos < 0)
			{
				bit_pos = 7;
				++pos;
			}
			if (GET_BIT(data[pos], bit_pos))
			{	image[x][y] = 1;	}		//
			else
			{	image[x][y] = 0;	}		//
		}
	}
}

void circle_detection(int image[80][60], int length, int width)
{
	sphere[1].distance = 0;
	sphere[1].x = 0;
	sphere[1].y = 0;

	sphere[2].distance = 0;
	sphere[2].x = 0;
	sphere[2].y = 0;

	int sCount = 1;
	int count = 0;
	int x_temp = 0;
	int y_temp = 0;
//	int x_match = 0;
	int y_match = 0;
	int x_check = 0;
	int y_check = 0;

//	if (found != false){
	for (int y = 0; y < width; y++)
		for (int x = 0; x < length; x++)
		{
			if (image[x][y] == 0)					// find dark pixel from 2d image
			{
				if (count == 0)
				{	x_temp = x;
					y_temp = y;}
				count++;
			}
			else									// a circle must able to form a square inside
			{
				if (( count > 2) &&(sCount < 3))			// ignore a dot
				{
					for (int j = y_temp; j < width ; j++)
					{
							if (image[x_temp][j] == 0)
							{	y_match++;	}
							else break;
					}
					if (y_match == count)			// if x = y, it is a square/circle
					{
						x_check = x_temp +count-1;
						y_check = y_temp +count-1;


						if (image[x_check][y_check]== 0)
						{
							sphere[sCount].x = x_temp;			// start pt. of x (top left corner)
							sphere[sCount].y = y_temp;			// start pt. of y
							sphere[sCount].distance = count;	// length of square
							sCount++;
						}
					}
				}
				count = 0;
				y_match = 0;
			}
	}
//	return sphere;
}

void circle_display()			// UN-USED
{
//	int cirCount =

/*for (int cCount = 1; cCount <3; cCount++)
	{
	if (sphere[cCount].distance != 0)
	{
		c1x = (sphere[cCount].x + sphere[cCount].distance)/2;
		c1y = (sphere[cCount].y + sphere[cCount].distance)/2;
		c2x = (sphere[cCount].x + sphere[cCount].distance)/2;
		c2y = (sphere[cCount].y + sphere[cCount].distance)/2;

		lcdP->SetRegion(Lcd::Rect(10,90,80,30));
//		writer.WriteString("X/ Y/ Dist");
		sprintf(display,"X=  %d", c1x);
		writerP->WriteString(display);

		lcdP->SetRegion(Lcd::Rect(10,105,80,30));
//		writer.WriteString(buf);
		sprintf(display,"Y=  %d", c1y);
//		lcd1.SetRegion(Lcd::Rect(10,100,80,60));
		writerP->WriteString(display);
/*
		lcd1.SetRegion(Lcd::Rect(10,120,80,30));
		sprintf(buf,"Dist=  %d", sphere[1].distance);
		writer.WriteString(buf);

	//	lcdP->SetRegion(Lcd::Rect(sphere[cCount].x, sphere[cCount].y, sphere[cCount].distance, sphere[cCount].distance));
	//	lcdP->FillColor(0x001F);
	}
	}
*/
	if (sphere[1].distance != 0)
	{
		c1x = (sphere[1].x + sphere[1].distance)/2;
		c1y = (sphere[1].y + sphere[1].distance)/2;

		lcdP->SetRegion(Lcd::Rect(10,90,80,30));
		//		writer.WriteString("X/ Y/ Dist");
		sprintf(display,"X=  %d", c1x);
		writerP->WriteString(display);

		lcdP->SetRegion(Lcd::Rect(10,105,80,30));
		//		writer.WriteString(buf);
		sprintf(display,"Y=  %d", c1y);
		//		lcd1.SetRegion(Lcd::Rect(10,100,80,60));
		writerP->WriteString(display);

		lcdP->SetRegion(Lcd::Rect(sphere[1].x, sphere[1].y, sphere[1].distance, sphere[1].distance));
		lcdP->FillColor(lcdP->kRed);
	}
/*
	if (sphere[2].distance != 0)
	{
		c2x = (sphere[2].x + sphere[2].distance)/2;
		c2y = (sphere[2].y + sphere[2].distance)/2;

		lcdP->SetRegion(Lcd::Rect(45,90,80,30));
		//		writer.WriteString("X/ Y/ Dist");
		sprintf(display,"X=  %d", c1x);
		writerP->WriteString(display);

		lcdP->SetRegion(Lcd::Rect(45,105,80,30));
		//		writer.WriteString(buf);
		sprintf(display,"Y=  %d", c1y);
		//		lcd1.SetRegion(Lcd::Rect(10,100,80,60));
		writerP->WriteString(display);

		lcdP->SetRegion(Lcd::Rect(sphere[2].x, sphere[2].y, sphere[2].distance, sphere[2].distance));
		lcdP->FillColor(0x001F);

		point_distance = sqrt((c1x-c2x)*(c1x-c2x)+(c1y-c2y)*(c1y-c2y));
//		cout << point_distance <<endl;
		lcdP->SetRegion(Lcd::Rect(10,120,80,30));
		sprintf(display,"Dist=  %f", point_distance);
		writerP->WriteString(display);

	}
*/
}
