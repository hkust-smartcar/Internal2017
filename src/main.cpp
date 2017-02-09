/*
 * main.cpp
 * Author: King
 */

#include <cassert>
#include <cstring>
#include <cstdint>
#include <libbase/k60/mcg.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/pin.h>
#include <libsc/system.h>
#include <libsc/lcd.h>
#include <libsc/st7735r.h>
#include <libsc/button.h>
#include <libsc/k60/ov7725.h>
#include <libsc/led.h>
#include <libsc/k60/ov7725_configurator.h>
#include <libsc/alternate_motor.h>
#include <libsc/servo.h>
#include <libsc/futaba_s3010.h>

#include <algorithm>
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

struct Coor{
	int x;
	int y;
};

#define H 60
#define W 80
#define initServoAngle 900
#define initMotorSpeed 500
uint16_t Max_ServoAngle=1800;
uint16_t Min_ServoAngle=0;
int L_sum_white = 0;
int R_sum_white = 0;
Coor Left_edge[20]; // 19 - 39
Coor Right_edge[20];

bool ext_camptr[W][H];

/*
 * Using Median filter
 */
void Med_Filter(){
	for (int j = 1; j < H-1; j++){
		for (int i = 1; i < W-1; i++){
			int count = 0;
			for (int y = j-1; y < j+2; y++){
				for (int x = i-1; x < i+2; x++){
					count += (int)ext_camptr[x][y];
				}
			}
			if (count >= 5) {
				ext_camptr[i][j] = 1;
			} else {
				ext_camptr[i][j] = 0;
			}
		}
	}
}

/*
 * A function extract the byte array
 * @ camBuffer: initial camera array in Byte
 * @ return type: bool array in 0/1 - only one bit
 * NOTE: Inside the camera, one pixel = one bit
 */
void extract_cam (const Byte* camBuffer) {

	Uint pos = 0;
	int bit_pos = 8;
	// Get 8 bits info. from byte and pass it into a new array called extract_cam
	for(Uint i = 0; i<H ; i++){
		for (Uint j = 0; j<W; j++){
			if (--bit_pos < 0) // Update after 8 bits are read
			{
				bit_pos = 7;// to track which position in a branch of Byte(Totally 8) is being read now.
				++pos;// to track which position in Byte array is being read now.
			}
			ext_camptr[i][j] = GET_BIT(camBuffer[pos], bit_pos);
		}
	}
}

/*
 * Step one: Find the edges on both sides
 * Step two: Find the "white area" on both sides according to edge
 * @Left_edge: array to store the left edge coordinate
 * @Right_edge: array to store the right edge coordinate
 * @L_sum_white: the sum of white points on the left side
 * @R_sum_white: the sum of white points on the right side
 *
 */


void Find_left_edge(){
    // To store the position of current layer, when layer=0 means y-cor=0
    int layer = 19;
    // Find the origin (y=20). (Assume the original layer is accurate every time)
    for (int x = W/2; x>0 ;x--){
  	  if (ext_camptr[layer][x]!=ext_camptr[layer][x-1]){
  		  Left_edge[layer].y = layer;
  		  Left_edge[layer].x = x;
  		  break;
  	  }
    layer++;
    // Find the rest.
    for(;layer< H-20 ;layer++){
  	  // LEFT
		  for (int x = W/2; x>0 ;x--){
			  // Tolerance = 15
			  if (ext_camptr[layer][x]!=ext_camptr[layer][x-1] && x <= Left_edge[x-1].x + 15){
				  Left_edge[layer].y = layer;
				  Left_edge[layer].x = x;
				  break;
			  }
		  }
    	}
    }
}

void Find_right_edge(){
    // To store the position of current layer, when layer=0 means y-cor=0
    int layer = 19;
    // Find the origin (y=20). (Assume the original layer is accurate every time)
    for (int x = W/2; x<W ;x++){
  	  if (ext_camptr[layer][x]!=ext_camptr[layer][x+1]){
  		  Right_edge[0].y = layer;
  		  Right_edge[0].x = x;
  		  break;
  	  }
    }
    layer++;
    // Find the rest.
    for(;layer< H-20 ;layer++){
	  // RIGHT
	  for (int x = W/2; x<W ; x++){
	  if (ext_camptr[layer][x]!=ext_camptr[layer][x+1] && x >= Right_edge[x-1].x - 15){
		  Right_edge[layer].y = layer;
		  Right_edge[layer].x = x;
		  break;
	  	  }
	  }
    }
}
void cal_L(){
    // for left side, we only focus on x-cor: 0-30 / y-cor: 20-40
	L_sum_white=0;
    for (int i = 20; i<H-20 ; i++){
  	  for (int j = Left_edge[i].x; j<W/2 ; j++){
  		  if (ext_camptr[i][j])
  			  L_sum_white++;
  	  }
    }
}
void cal_R(){
    // for right side, we only focus on x-cor: 30-60 / y-cor: 20-40
	R_sum_white=0;
    for (int i = 20; i<H-20 ; i++){
  	  for (int j = W/2; j< Right_edge[i].x ; j++){
  		  if (ext_camptr[i][j])
  			  R_sum_white++;
  	  }
    }
}
int main(void){

	uint16_t Curr_Angle_diff = 0;
	uint16_t Prev_Angle_diff = 0;
	uint16_t Angle_change;
    double Kp = 1;
    double Kd = 1;
    double Const_servo = 1;// change the int into uint16_t?????
	// Initialization for camera
	k60::Ov7725::Config cam_config;
	cam_config.id = 0;
	cam_config.w = W;
	cam_config.h = H;
	//cam_config.fps = 60; // shooting rate
	k60::Ov7725 camera(cam_config);

	// Initialization for LCD
	St7735r::Config lcd_config;
	//lcd_config.fps = 100;
	St7735r lcd(lcd_config);

	// Initialization for MOTOR1+2
	AlternateMotor::Config M1_config;
	M1_config.id = 0;
	AlternateMotor motor1(M1_config);
	AlternateMotor::Config M2_config;
	M1_config.id = 1;
	AlternateMotor motor2(M1_config);
	// Two motor rotate in opposite dir.
	motor1.SetClockwise(false);
	motor1.SetPower(initMotorSpeed); // 0~1000, 500 -> 50% speed
	motor2.SetPower(initMotorSpeed); // 0~1000, 500 -> 50% speed

	// Initialization for SERVO
	FutabaS3010::Config S_config;
	S_config.id = 0;
	FutabaS3010 servo(S_config);
	servo.SetDegree(initServoAngle);

	lcd.Clear(0); // Clear the screen to black
	//start the clock
	System::Init();
	Timer::TimerInt timeImg = System::Time();
	camera.Start();
	//wait until first image is available
	while(!camera.IsAvailable()){};
	while (true){
		if (timeImg != System::Time()){
			timeImg = System::Time();
#if timeImg %10 == 0 // Update the car every 10ms.
              const Byte* camBuffer = camera.LockBuffer();
	          // Copy CamBuffer
              Byte* CopyCamBuffer = new Byte [camera.GetBufferSize()];
              for (int i; i<camera.GetBufferSize();i++){
            	  CopyCamBuffer[i]=camBuffer[i];
              }
	          camera.UnlockBuffer();
              lcd.SetRegion(Lcd::Rect(0, 0, 80, 60));
	          lcd.FillBits(Lcd::kBlack, Lcd::kWhite, CopyCamBuffer, camera.GetBufferSize()*8);
	          extract_cam (CopyCamBuffer);
	          Med_Filter();


	          /*STEP ONE: Find the edges on both sides*/
	          Find_left_edge();
	          Find_right_edge();
	          //FOR TESTING
	          for (int y =19; y<H-60;y++){
		          lcd.SetRegion(Lcd::Rect(Left_edge[y].x, y,1,1));
		          lcd.FillColor(Lcd::kBlue);
	          }










//	          /*STEP TWO: Find the "white area" on both sides according to edge*/
//	          cal_L();
//	          cal_R();
//	          /*
//	           * PD algorithm for servo control
//	           */
//
//	          Curr_Angle_diff = (L_sum_white - R_sum_white) * Const_servo;// current error
//	          Angle_change = Kp * Curr_Angle_diff + Kd * (Curr_Angle_diff-Prev_Angle_diff);
//	          Prev_Angle_diff = Curr_Angle_diff;
//	          if (servo.GetDegree()+Angle_change > Max_ServoAngle){
//	        	  servo.SetDegree(Max_ServoAngle);
//	          }
//	          else if (servo.GetDegree()+Angle_change < Min_ServoAngle){
//	        	  servo.SetDegree(Min_ServoAngle);
//	          }
//	          else
//	        	  servo.SetDegree(servo.GetDegree()+Angle_change);




//	          /*
//	           * Error tolerance = 10
//	           * L_sum_white > R_sum_white + 10 -> TURN LEFT
//	           * L_sum_white + 10 < R_sum_white -> TURN RIGHT
//	           */
////	          if (L_sum_white >= R_sum_white + 10){
////	        	  servo.SetDegree(1800); // 0~1800, 900 -> 90 degree -> DEPENDS ON OUR SERVO!!
////	        	  System::DelayMs(500);
////	        	  }
////	          else if (L_sum_white + 10 < R_sum_white){
////	        	  servo.SetDegree(0);
////	        	  System::DelayMs(500);
////	          	  }
////	          else{
////	        	  servo.SetDegree(900);
////        	  	  System::DelayMs(500);
////	          	  }

#endif
		}
	}
	camera.Stop();

	return 0;
}
