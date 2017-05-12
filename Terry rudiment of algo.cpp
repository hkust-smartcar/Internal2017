/*
 * main.cpp
 *
 * Author: ZK
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cstring>
#include <string>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/pin.h>
#include <libsc/st7735r.h>
#include <libsc/lcd.h>
#include <libsc/k60/ov7725.h>
#include "libsc/k60/ov7725_configurator.h"
#include <libsc/futaba_s3010.h>
#include <libsc/servo.h>
#include <libsc/alternate_motor.h>
#include <libsc/dir_encoder.h>
#include <libsc/lcd_typewriter.h>
#include <vector>
#include <math.h>
#include <stdlib.h>
#define CAM_W 80
#define CAM_H 60
#define KP   4.8
#define KD   2.4
#define KP_curve 5.3
#define KD_curve 2.5
#define KP_R  6
#define KD_R  2.8
#define KP_LM 0.06
#define KD_LM 0.07
#define KI_LM 0
#define KP_RM 0.06
#define KD_RM 0.07
#define KI_RM 0
#define height 29
#define focalL 40
#define pathwidth 45
#define initial_servo 790
#define expected_count 300
#define speed 2.5
#define speed_ratio 0.3
#define ratio_ratio 1
#define increase_ratio 1.1
#define L(y) y
#define R(y) y+60
#define offset 33
#define round_set 30
namespace libbase
{
	namespace k60
	{

		Mcg::Config Mcg::GetMcgConfig()
		{
			Mcg::Config config;
			config.external_oscillator_khz = 50000;
			config.core_clock_khz = 200000;
			return config;
		}

	}
}

using namespace libsc;
using namespace libbase::k60;
using namespace std;


bool camptr[CAM_W][CAM_H];

//int8_t means integer with 8 bits
//Uint means unsigned integer
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
	for(Uint i = 0; i<CAM_H ; i++){
		for (Uint j = 0; j<CAM_W; j++){
			if (--bit_pos < 0) // Update after 8 bits are read
			{
				bit_pos = 7;// to track which position in a branch of Byte(Totally 8) is being read now.
				++pos;// to track which position in Byte array is being read now.
			}
			camptr[j][i] = GET_BIT(camBuffer[pos], bit_pos);
		}
	}
}




void Filter2D(){
	for (int j = 1; j < CAM_H-1; j++){
		for (int i = 1; i < CAM_W-1; i++){
			int count = 0;
			for (int y = j-1; y < j+2; y++){
				for (int x = i-1; x < i+2; x++){
					count += (int)camptr[x][y];
				}
			}
			if (count >= 5) {
				camptr[i][j] = 1;
			} else {
				camptr[i][j] = 0;
			}
		}
	}
}

St7735r* lcdP;
AlternateMotor* L_motorP;
AlternateMotor* R_motorP;
FutabaS3010* servoP;
LcdTypewriter* writerP;



St7735r::Config getLcdConfig()
{
	St7735r::Config c;
	return c;
}

FutabaS3010::Config getServoConfig(){
	FutabaS3010::Config c;
	c.id =0;
	return c;
}

k60::Ov7725::Config getCameraConfig()
{
	k60::Ov7725::Config c;
	c.id = 0;
	c.w = CAM_W;
	c.h = CAM_H;
	c.fps = k60::Ov7725Configurator::Config::Fps::kHigh;
	return c;
}

Led::Config getLedConfig(){
	Led::Config c;
	c.id = 0;
	c.is_active_low = false;
	return c;

}


//int8_t K[60];
//int8_t J[60][80];

/*
 * Find corresponding world coordinate to the camera.
 *
 *
 *
 *
 *
 *
 *
 *
 * */
//void Change_Matrix(){
//	for(int y=0; y < 60 ;y++){
//		K[y] = int((height*(y-2.1) + height*focalL*0.446 + 40*(y-2.1))/(focalL*0.899-(y-2.1)*0.446));
//		for(int x=0; x<80 ;x++){
//			J[y][x] = (int)((x-40)*(K[y]*(0.446/focalL) + height*(0.899/focalL) +1 ));
//	}
//}
//}

struct coor{
	int x;
	int y;
};
coor L_corner;
coor R_corner;
std::vector<coor> Path;
std::vector<coor> detectline;



void Print2D(){

	for (int y=0; y<CAM_H; y++){
		for (int x=0; x<CAM_W; x++){
			lcdP->SetRegion(Lcd::Rect(x, y+CAM_H+1, 1, 1));
			if (!camptr[x][y]){
				lcdP->FillColor(0xFFFF);
			} else {
				lcdP->FillColor(0x001F);
			}
		}
	}

}
k60::Ov7725* cameraP;

int16_t L_motor_error = 0;
int16_t L_premotor_error = 0;
int16_t L_motor_errorsum=0;
int16_t L_motor_ideal = 0;

int16_t R_motor_error = 0;
int16_t R_premotor_error = 0;
int16_t R_motor_errorsum = 0;
int16_t R_motor_ideal = 0;


int32_t L_encoder =0;
int32_t R_encoder =0;
int32_t L_encoder_count = 0;
int32_t R_encoder_count = 0; // 5800 corresponding to 1 meter.  512-> 1 cycle->8.826 cm
int8_t L_power = 0;
int8_t R_power = 0;
int8_t num_check = 4;
struct set{
	int8_t edgeposition; //x-coordinate
	float slope;
	int8_t row;// y-coordinate

};
set edge[120];
bool left_bottom_exist = false;
bool right_bottom_exist = false;

bool changeworld = false;
int ServoErr = 0;
int ServoPreErr = 0;
int idealdegree = initial_servo;
bool corner_Lexist = false;
bool corner_Rexist = false;
bool L_slow = false;
bool R_slow = false;
bool crossroad = false;
bool entercross =false;
bool Rcross = false;
bool roundabout = false;
bool Round_step = false;
bool Round_step2 = false;
bool Cross_step = false;
bool motor_run = true;
bool Lcurve = false;
bool Rcurve = false;
void find_edge();//code for edge finding
void Printedge();
void printCameraImage(const Byte* image);
void find_follow(); //find the line the car following
void turningPID(); // change the angle of servo
void motorPID(int ,int);
void checkround();
void findpath();
void Printpath();
bool Lcorner(int , int );
bool Rcorner(int , int);
bool roundabout_detect(int,int);
int main(void)
{

	System::Init();

	//camera initialization
	k60::Ov7725 camera(getCameraConfig());
	camera.Start();

	// Initialization for MOTOR1+2
	AlternateMotor::Config M1_config;
	M1_config.id = 0;
	AlternateMotor L_motor(M1_config);
	L_motorP = &L_motor;
	L_motor.SetClockwise(true);
	AlternateMotor::Config M2_config;

	M1_config.id = 1;
	AlternateMotor R_motor(M1_config);
	R_motor.SetClockwise(false);
	R_motorP = &R_motor;

	/*LCD initialization
	 *
	 *
	 */
	St7735r lcd(getLcdConfig());
	lcdP = &lcd;
	LcdTypewriter::Config typeconfig;
	typeconfig.lcd = &lcd;
	/*
	 * LED initialization
	 */
	Led led(getLedConfig());


	LcdTypewriter writer(typeconfig);
	writerP = &writer;
	/*servo initialization
	 *
	 *
	 *
	 *
	 *
	 *
	 */
	FutabaS3010 servo(getServoConfig());
	servo.SetDegree(initial_servo);
	servoP = &servo;


	/*encoder initialization
	 *
	 *
	 */
	libsc::Encoder::Config Lencoder_config, Rencoder_config;
	Lencoder_config.id = 0;
	Rencoder_config.id = 1;
	DirEncoder L_Encoder(Lencoder_config);
	DirEncoder R_Encoder(Rencoder_config);

	int32_t lastTime = System::Time();
	Path.reserve(20);
	detectline.reserve(20);

	while(true)
	{


		if( ( System::Time() - lastTime ) >= 20 ){
			lastTime = System::Time();
			led.SetEnable(0);
			const Byte* cameraBuffer = camera.LockBuffer();
			//printCameraImage(cameraBuffer);
			camera.UnlockBuffer();
//
			extract_cam(cameraBuffer);
			Filter2D();
//			Print2D();

			find_edge();

			char buf[50];

			if(!roundabout && ! crossroad ){
				if(corner_Lexist && corner_Rexist && !entercross){
					checkround();
				}
				else if(entercross && corner_Lexist &&Rcross){
					crossroad = true;
				}
				else if(entercross && corner_Rexist && !Rcross){
					crossroad = true;
				}


				if(roundabout){
//					sprintf(buf, "Roundabout！");
//					lcdP->SetRegion(Lcd::Rect(0,60,80,80));
//					writer.WriteString(buf);
				}
				else if(crossroad){
//					sprintf(buf, "Cross！");
//					lcdP->SetRegion(Lcd::Rect(0,60,80,80));
//					writer.WriteString(buf);

				}
//				else{
//					sprintf(buf, "Normal!");
//					lcdP->SetRegion(Lcd::Rect(0,60,80,80));
//					writer.WriteString(buf);
//					}
			}
			else if(roundabout){
//				sprintf(buf, "Roundabout！");
//				lcdP->SetRegion(Lcd::Rect(0,60,80,80));
//				writer.WriteString(buf);
				int R = 0;
				if(!Round_step){
//					sprintf(buf, "R ready");
//					lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//					writer.WriteString(buf);
				}
				if(!Round_step&& !corner_Lexist && !corner_Rexist){
					Round_step = true;
//					sprintf(buf, "R PASS 1");
//					lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//					writer.WriteString(buf);
				}

				if(Round_step && ! Round_step2 &&corner_Lexist){
					Round_step2 = true;
//					sprintf(buf, "R PASS 2");
//					lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//					writer.WriteString(buf);
				}


				if(Round_step2){
					for(int i=0;i<40;i++){
						if(edge[R(i)].edgeposition == 78){
							R ++;
						}
					}
					if(R <=20){
						roundabout = false;
						Round_step = false;
						Round_step2 = false;
						L_slow = false;
						R_slow = false;
//						sprintf(buf, "R Done");
//						lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//						writer.WriteString(buf);
					}
				}
			}

			else if(crossroad){
//				sprintf(buf, "Cross!");
//				lcdP->SetRegion(Lcd::Rect(0,60,80,80));
//				writer.WriteString(buf);
				int L =0;
				int R = 0;
				if(!Cross_step){
//					sprintf(buf, "C ready");
//					lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//					writer.WriteString(buf);
				}
				if(!Cross_step && !camptr[1][58] && !camptr[78][58]){
					Cross_step = true;
//					sprintf(buf, "C PASS 1");
//					lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//					writer.WriteString(buf);
				}

				if(Cross_step){
					for(int i=0; i<30;i++){
						if(edge[L(i)].edgeposition <= 5){
							L++;
						}
						if(edge[R(i)].edgeposition >= 73){
							R ++;
						}
					}

//					sprintf(buf, "%d %d",L,R);
//					lcdP->SetRegion(Lcd::Rect(60,80,50,40));
//					writer.WriteString(buf);
					if(L < 5 || R < 5){
						int L=0;
						int R=0;
						crossroad = false;
						Cross_step = false;
						L_slow = false;
						R_slow = false;

						for(int row = 30; row<60;row++){
							if(edge[L(row)].edgeposition <=2){
								L++;
							}
							if(edge[R(row)].edgeposition >= 78){
								R++;
							}
						}
						if(R>L){
							Rcross= true;
						}
						else{
							Rcross =false;
						}
						entercross = !entercross;
//						sprintf(buf, "C done");
//						lcdP->SetRegion(Lcd::Rect(0,80,80,80));
//						writer.WriteString(buf);

					}



				}



			}


			if(entercross){
//				sprintf(buf, "Entering!");
//				lcdP->SetRegion(Lcd::Rect(0,120,50,40));
//				writer.WriteString(buf);
			}
			else{
//				sprintf(buf, "Out!");
//				lcdP->SetRegion(Lcd::Rect(0,120,50,40));
//				writer.WriteString(buf);
			}
			findpath();


			//Printedge();
			//Printpath();
			turningPID();
			L_Encoder.Update();
			L_encoder = L_Encoder.GetCount();
			L_encoder_count = L_encoder_count+L_encoder;
			R_Encoder.Update();
			R_encoder = R_Encoder.GetCount();
			R_encoder_count = R_encoder_count-R_encoder;
			if(motor_run && L_slow && R_slow){
				motorPID(0.0*speed*117,servoP->GetDegree());
			}
			else if(motor_run && roundabout){
				motorPID(0.6*speed*117,servoP->GetDegree());
			}
			else if(motor_run && crossroad){
				motorPID(0.6*speed*117,servoP->GetDegree());
			}
			else if(motor_run && !crossroad && entercross){
				motorPID(0.8*speed*117,servoP->GetDegree());
			}
			else if(motor_run && (Lcurve||Rcurve)){
				L_slow = false;
				R_slow = false;
				motorPID(0.7*speed*117,servoP->GetDegree());
			}
			else if(motor_run && !L_slow && !R_slow){
				motorPID(speed*117,servoP->GetDegree());
			}
			else if(motor_run && (L_slow || R_slow)){
				motorPID(speed_ratio*speed*117,servoP->GetDegree());
			}
			else{
				L_motor.SetPower(0);
				R_motor.SetPower(0);
			}


			if( (L_encoder < 10 || R_encoder > -10 ) && System::Time() > 1000){
				motor_run = false;
			}
			else{
				motor_run = true;
			}

			char buffer[50];
			float L_speed =0;
			float R_speed =0;
			L_speed = float(L_encoder*50)/5800;
			R_speed = float(-R_encoder*50)/5800;
//						sprintf(buffer, "%d , %d ",L_encoder_count,R_encoder_count);
//						lcdP->SetRegion(Lcd::Rect(0,130,120,40));
//						writerP->WriteString(buffer);

//						sprintf(buffer, "%.3f , %.3f ",L_speed,R_speed);
//						lcdP->SetRegion(Lcd::Rect(0,110,120,40));
//						writerP->WriteString(buffer);


//
//
//
//
//
//
//
//
//
//
//
//
//			//sprintf(edge_buffer," edge: %d   %d",left_edge[10],right_edge[10]);
//			//lcdP->SetRegion(Lcd::Rect(0,60,80,80));
//			//writer.WriteString(edge_buffer);
//			//Print2D();
//		    /*
//			L_encoder_count = LdirEncoder.GetCount();
//			R_encoder_count = RdirEncoder.GetCount();
//			find_motor_error(L_encoder_count,R_encoder_count);
//			angle_change = KP*difference + KD*(difference - pre_diff);
//			now_angle = now_angle + angle_change;
//			servo.SetDegree(now_angle);
//			pre_diff = difference;
//			L_power = L_power + KD_LM*L_motor_error + KD_LM*(L_motor_error - L_premotor_error);
//			R_power = R_power + KD_RM*R_motor_error + KD_RM*(R_motor_error - R_premotor_error);
//			motor1.SetPower(L_power);
//			motor2.SetPower(R_power);
//			L_premotor_error = L_motor_error;
//			R_premotor_error = R_motor_error;*/
//
		}

		}
	return 0;
	}

void motorPID(int ideal_count,int servoangle){
	float L_ideal_count =ideal_count;
	float R_ideal_count =ideal_count;



//	float input = (servoangle - 800)/273.9;
//	float R_L_ratio =(-0.03685)* pow(input,5) + 0.04041* pow(input,4) + 0.1487*pow(input,3)+0.002813 * pow(input,2) + 0.2978 * input + 1.005;
//	R_L_ratio = ratio_ratio * R_L_ratio;
//	if(servoangle > 800){
//	L_ideal_count = ideal_count * increase_ratio / R_L_ratio;
//	R_ideal_count = ideal_count*increase_ratio ;
//	}
//	else{
//		L_ideal_count =  ideal_count * increase_ratio ;
//		R_ideal_count = ideal_count * increase_ratio * R_L_ratio;
//	}


	if(servoangle >850){
		if(servoangle>=1100){
			L_ideal_count = 0.65* ideal_count*increase_ratio;
			R_ideal_count =  ideal_count * increase_ratio;
			//R_ideal_count = ideal_count / 0.65;
		}
		else if(servoangle<1100 && servoangle >= 1050){
			L_ideal_count = 0.65* ideal_count * increase_ratio;
			R_ideal_count = ideal_count *increase_ratio;
			//R_ideal_count = ideal_count / 0.715;
		}
		else if(servoangle<1050 && servoangle >= 1000){
			L_ideal_count = 0.71* ideal_count*increase_ratio;
			R_ideal_count = ideal_count *increase_ratio;
			//R_ideal_count = ideal_count / 0.715;
		}
		else if(servoangle<1000 && servoangle >= 950){
			L_ideal_count = 0.78* ideal_count*increase_ratio;
			R_ideal_count = ideal_count *increase_ratio;
			//R_ideal_count = ideal_count / 0.715;
		}
		else if(servoangle<950 && servoangle >= 900){
			L_ideal_count = 0.85* ideal_count*increase_ratio;
			R_ideal_count = ideal_count *increase_ratio;
			//R_ideal_count = ideal_count / 0.715;
		}
		else if(servoangle<900 && servoangle >= 850){
			L_ideal_count = 0.88* ideal_count*increase_ratio;
			R_ideal_count = ideal_count *increase_ratio;
			//R_ideal_count = ideal_count / 0.715;
		}
	}
	else if(servoangle < 750){
		if(servoangle <500){
			L_ideal_count = increase_ratio*ideal_count;
			R_ideal_count = increase_ratio*ideal_count/1.65;
			//L_ideal_count = ideal_count * 1.65;
		}
		else if(servoangle >=500 && servoangle <550){
			L_ideal_count = increase_ratio*ideal_count;
			R_ideal_count = increase_ratio*ideal_count/1.65;
			//L_ideal_count = ideal_count * 1.5;

		}
		else if(servoangle >=550 && servoangle <600){
			L_ideal_count = increase_ratio*ideal_count;
			R_ideal_count = increase_ratio*ideal_count/1.5;
			//L_ideal_count = ideal_count * 1.5;

		}
		else if(servoangle >=600 && servoangle <650){
			L_ideal_count = increase_ratio*ideal_count;
			R_ideal_count = increase_ratio*ideal_count/1.33;
			//L_ideal_count = ideal_count * 1.5;

		}
		else if(servoangle >=650 && servoangle < 700){
			L_ideal_count = increase_ratio*ideal_count;
			R_ideal_count = increase_ratio*ideal_count/1.23;
			//L_ideal_count = ideal_count * 1.23;
		}
		else if(servoangle >=700){
			L_ideal_count = increase_ratio*ideal_count;
			R_ideal_count = increase_ratio*ideal_count/1.13;
			//L_ideal_count = ideal_count * 1.06;
		}
	}

	L_premotor_error = L_motor_error;
	L_motor_error = L_ideal_count - L_encoder;
	L_motor_errorsum += L_motor_error;

	L_motor_ideal += KP_LM*(L_motor_error)+KI_LM*(L_motor_errorsum)+KD_LM*(L_motor_error-L_premotor_error);
	if(L_motor_ideal > 400){
		L_motor_ideal = 400;
	}
	if(L_motor_ideal <0){
		 L_motor_ideal=0;
	}
	R_premotor_error = R_motor_error;
	R_motor_error = R_ideal_count + R_encoder;
	R_motor_errorsum += R_motor_error;
	R_motor_ideal += KP_RM*(R_motor_error)+KI_RM*(R_motor_errorsum)+KD_RM*(R_motor_error-R_premotor_error);
	if(R_motor_ideal >400){
		R_motor_ideal = 400;
	}
	if(R_motor_ideal <0){
		 R_motor_ideal= 0;
	}





	L_motorP->SetPower(L_motor_ideal);
    R_motorP->SetPower(R_motor_ideal);
}
void turningPID(){
	int a = Path.size();
	double error1 = 0;
	double error2 = 0;
	int middle = 0;
	for(int row=0;row< std::min(4,a);row++){
		middle = middle+Path[row].x;
	}
	for(int row =0; row < std::min(4,a); row++){
		error1 = error1-(Path[row].x - 46);
	}
	for(int row=4; row< std::min(8,a); row++){
		error2 = error2-(Path[row].x-46);
	}
	ServoErr = (0.5 * error1) + (0.5 * error2);

	//change the ServoErr to fit the turning angle


	/*
	 *
	 *
	 *
	 */
	// Incremental PID(n) = PID(n-1) + kp * (e(n)-e(n-1)) +kd *(e(n)-2e(n-1)+e(n-2)) + ki * e(n)
	if(crossroad && !Cross_step && !entercross){
		idealdegree = initial_servo;
	}

	else if(crossroad && Cross_step){
		if(ServoErr>0){
			idealdegree =initial_servo + 50;
		}
		else{
			idealdegree = initial_servo - 50;
		}
	}
	else if(!crossroad && entercross && Rcross && ServoErr > 0){
		idealdegree = initial_servo + 30;
	}
	else if(!crossroad && entercross && !Rcross && ServoErr < 0){

			idealdegree = initial_servo - 30;

	}
	else if(crossroad && entercross && Rcross){
		idealdegree = initial_servo-60;
	}
	else if(crossroad && entercross && !Rcross){
		idealdegree = initial_servo+60;

	}

	else if(roundabout && !Round_step){
		idealdegree = initial_servo + int(KP_R*ServoErr +KD_R*(ServoErr-ServoPreErr));
		if(idealdegree <900){
			idealdegree = 900;
		}
	}
	else if(roundabout && Round_step && !Round_step2){
		idealdegree = initial_servo + int(KP_R*ServoErr +KD_R*(ServoErr-ServoPreErr));

	}
	else if(roundabout && Round_step2){
		idealdegree = initial_servo + int(KP_R*ServoErr +KD_R*(ServoErr-ServoPreErr));
		if(idealdegree <900){
			idealdegree = 900;
		}
	}
	else if(Lcurve || Rcurve){
		idealdegree = initial_servo + int(KP_curve*ServoErr +KD_curve*(ServoErr-ServoPreErr));
	}
	else{
		idealdegree = initial_servo + int(KP*ServoErr +KD*(ServoErr-ServoPreErr));
	}
	if(idealdegree > 1100){
		idealdegree = 1100;
	}
	if(idealdegree < 500){
		idealdegree = 500;
	}
//	char buffer[50];
//	sprintf(buffer,"%d %d %d",ServoErr,idealdegree, middle/4);
//	lcdP->SetRegion(Lcd::Rect(0,80,128,40));
//	writerP->WriteString(buffer);


	servoP->SetDegree(idealdegree);
	ServoPreErr = ServoErr;
}

void find_edge(){

//find the left edge at bottom line

	for(int i = 40; i > 2; i--){
		//BWW found!
		if(!camptr[i][59] && !camptr[i-1][59] && camptr[i-2][59] ){
			edge[L(0)].edgeposition = i - 1;
			edge[L(0)].row = 59;
			left_bottom_exist = true;
			break;
		}
	}
	if(left_bottom_exist == false){
		edge[L(0)].edgeposition = 1;
		edge[L(0)].row = 59;
	}
	// find the right edge at bottom line.
	for(int i = 40; i < 78 ; i++){
		//WWB found!
		if(!camptr[i][59]&&!camptr[i+1][59]&&camptr[i+2][59]){
			edge[R(0)].edgeposition = i+1;
			edge[R(0)].row = 59;
			right_bottom_exist = true;
			break;
		}
	}
	if(right_bottom_exist == false){
		edge[R(0)].edgeposition = 78;
		edge[R(0)].row = 59;
	}




		corner_Lexist = false;
		corner_Rexist = false;
		L_slow = false;
		R_slow = false;
		bool L_stop = false;
		bool R_stop = false;
	for(int row = 1; row<60; row ++){

		bool L_up = false;
		bool R_up = false;
		L_up = camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row-1];
		//upper point is still white for left edges
		if(!L_up){

			if(edge[L(row-1)].edgeposition == 1){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}

			else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row-1]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}
			else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
				edge[L(row)].row = edge[L(row-1)].row-1;

			}
			else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row+1]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
				edge[L(row)].row = edge[L(row-1)].row;

			}
			else if(camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
				edge[L(row)].row = edge[L(row-1)].row+1;
			}
			else {
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}

		}
		//upper point is black for left edges
		else{
			if(!L_stop){
				if(edge[L(row-1)].edgeposition == 78){
					L_stop = true;
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
				else if(edge[L(row-1)].edgeposition <=2 && (camptr[40][edge[L(row-1)].row-5]||camptr[40][edge[L(row-1)].row-3]) ){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
					L_stop = true;
				}
				else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row-1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
				else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
					edge[L(row)].row = edge[L(row-1)].row;

				}
				else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
					edge[L(row)].row = edge[L(row-1)].row+1;

				}
				else if(!camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row+1;
				}
				else{
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
			}
			else{
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}

		}


		//upper point is still white for right edges
		R_up = camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row-1];
		if(!R_up){
			if(edge[R(row-1)].edgeposition == 78){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}
			else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row-1]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}
			else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
				edge[R(row)].row = edge[R(row-1)].row-1;

			}
			else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row+1]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
				edge[R(row)].row = edge[R(row-1)].row;

			}
			else if(camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
				edge[R(row)].row = edge[R(row-1)].row+1;

			}
			else {
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}

		}
		//upper point is black for right edges
		else{
			if(!R_stop){
				if(edge[R(row-1)].edgeposition == 1){
					R_stop = true;
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
				if(edge[R(row-1)].edgeposition == 78 && camptr[40][edge[R(row-1)].row-5]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
					R_stop = true;
				}
				else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row-1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
				else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
					edge[R(row)].row = edge[R(row-1)].row;

				}
				else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row+1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
					edge[R(row)].row = edge[R(row-1)].row+1;

				}
				else if(!camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row+1;
				}
				else{
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
			}
			else{
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}
		}
/*
 * Trigger one L
 */
		char buf[20];
       if(row >= 12 && row % 2==0 && abs(edge[L(row)].edgeposition + edge[L(row-12)].edgeposition - 2*edge[L(row-6)].edgeposition) >= 4 && edge[L(row-6)].row >= 30 && edge[L(row-6)].edgeposition > 5  ){

//			lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
//			lcdP->FillColor(0xF800);//Red

    	   	if(Lcorner(edge[L(row-6)].row,edge[L(row-6)].edgeposition)&&edge[L(row-6)].row < 35 && !L_slow){
    	   		L_slow = true;
//    	   		lcdP->SetRegion(Lcd::Rect(0,100,100,40));
//    	   		sprintf(buf,"L corner");
//    	   		writerP->WriteString(buf);
    	   	}
    	   	else{
//    	   		lcdP->SetRegion(Lcd::Rect(0,100,100,40));
//    	   		sprintf(buf," No L ");
//    	   		writerP->WriteString(buf);
    	   	}


        	if(Lcorner(edge[L(row-6)].row,edge[L(row-6)].edgeposition) && !corner_Lexist && edge[L(row-6)].row >= 40){

				corner_Lexist = true;
				L_slow = false;
				L_corner.x = edge[L(row-6)].edgeposition;
				L_corner.y = edge[L(row-6)].row;
//				lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 4,edge[L(row-6)].row -4, 9,9));
//				lcdP->FillColor(0x0000);//Black
			}
        	if(!corner_Lexist){
//    			lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
//    			lcdP->FillColor(0xF800);//Red
        	}
		}

		/*
		 * Trigger one R
		 */

		if(row >= 12 && row % 2 == 0 && abs(edge[R(row)].edgeposition + edge[R(row-12)].edgeposition - 2*edge[R(row-6)].edgeposition )>= 4 && edge[R(row-6)].row >=30 && edge[R(row-6)].edgeposition < 74){


			if(Rcorner(edge[R(row-6)].row,edge[R(row-6)].edgeposition) && edge[R(row-6)].row < 35 && !R_slow){
				R_slow = true;
//    	   		lcdP->SetRegion(Lcd::Rect(80,100,50,40));
//    	   		sprintf(buf,"R c");
//    	   		writerP->WriteString(buf);
			}
			else{
//    	   		lcdP->SetRegion(Lcd::Rect(80,100,50,40));
//    	   		sprintf(buf,"No R ");
//    	   		writerP->WriteString(buf);
			}
			if(Rcorner(edge[R(row-6)].row,edge[R(row-6)].edgeposition) && !corner_Rexist && edge[R(row-6)].row>=40){
				corner_Rexist = true;
				R_slow = false;
				R_corner.x = edge[R(row-6)].edgeposition;
				R_corner.y = edge[R(row-6)].row;
//				lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition-4,edge[R(row-6)].row-4, 9, 9));
//				lcdP->FillColor(0xF800);//Red
			}
			if(!corner_Rexist){
//				lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition-2,edge[R(row-6)].row-2, 4, 4));
//				lcdP->FillColor(0x0000);//Red
			}
		}



	}
}

void findpath(){
	 Lcurve = false;
	 Rcurve = false;
	Path.clear();
	int Left_x[15];
	int Right_x[15];
	for(int x=0;x<15;x++){
		Left_x[x] = -1;
		Right_x[x] = -1;
	}
	for(int i=0; i< 60 ;i++){
		/*
			* Left!
		*/
		if(edge[L(i)].row == 55 && Left_x[0] == -1 ){
			Left_x[0] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 52 && Left_x[1] == -1){
			Left_x[1] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 49 && Left_x[2] == -1){
			Left_x[2] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 46 && Left_x[3] == -1){
			Left_x[3] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 43 && Left_x[4] == -1){
			Left_x[4] = edge[L(i)].edgeposition;
			continue;

		}
		if(edge[L(i)].row == 40 && Left_x[5] == -1){
			Left_x[5] = edge[L(i)].edgeposition;
			continue;
	 	}
		if(edge[L(i)].row == 37 && Left_x[6] == -1){
			Left_x[6] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 34 && Left_x[7] == -1){
			Left_x[7] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 31 && Left_x[8] == -1){
			Left_x[8] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 28 && Left_x[9] == -1){
			Left_x[9] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 25 && Left_x[10] == -1){
			Left_x[10] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 22 && Left_x[11] == -1){
			Left_x[11] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 19 && Left_x[12] == -1){
			Left_x[12] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 16 && Left_x[13] == -1){
			Left_x[13] = edge[L(i)].edgeposition;
			continue;
		}
	}
				/*
				 * Right
				 */
			for(int i=0; i< 60 ;i++){
				if(edge[R(i)].row ==55 && Right_x[0] == -1){
					Right_x[0] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==52 && Right_x[1] == -1){
					Right_x[1] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==49 && Right_x[2] == -1){
					Right_x[2] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==46 && Right_x[3] == -1){
					Right_x[3] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==43 && Right_x[4] == -1){
					Right_x[4] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==40 && Right_x[5] == -1){
					Right_x[5] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==37 && Right_x[6] == -1){
					Right_x[6] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==34 && Right_x[7] == -1){
					Right_x[7] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==31 && Right_x[8] == -1){
					Right_x[8] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==28 && Right_x[9] == -1){
					Right_x[9] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==25 && Right_x[10] == -1){
					Right_x[10] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==22 && Right_x[11] == -1){
					Right_x[11] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==19 && Right_x[12] == -1){
					Right_x[12] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==16 && Right_x[13] == -1){
					Right_x[13] = edge[R(i)].edgeposition;
					continue;
				}


			}


		if(!roundabout && !crossroad){

			/*
			 * curve
			 */
				char buffer[50];
			if( Right_x[4] == 78 &&Right_x[5] == 78 && Left_x[8]-Left_x[4] >4 ){
				Rcurve = true;

//				sprintf(buffer, "R");
//				lcdP->SetRegion(Lcd::Rect(80,0,40,40));
//				writerP->WriteString(buffer);
			}
			else if(Left_x[4] == 1 && Left_x[5] ==1 && Right_x[8] - Right_x[4] <-4 ){
				Lcurve = true;


//				sprintf(buffer, "L");
//				lcdP->SetRegion(Lcd::Rect(80,0,40,40));
//				writerP->WriteString(buffer);
			}
			else{
				Lcurve = false;
				Rcurve = false;

//				sprintf(buffer, "NO");
//				lcdP->SetRegion(Lcd::Rect(80,0,40,40));
//				writerP->WriteString(buffer);
			}



			/*
			 * Path
			 */

			for(int row = 0; row< 14;row++){
				if(Left_x[row] != -1 && Right_x[row] != -1){
					if(Lcurve){
						if(Right_x[row] > 2){
							if(Left_x[0] <= 5){
								coor temp;
								temp.x = Right_x[row] - offset - 3*row -4;
								temp.y = 58-3*(row+1);
								Path.push_back(temp);
							}
							else{
								if(Left_x[row] > 5){
									coor temp;
									temp.x = (9*Left_x[row]+ 3*Right_x[row])/12;
									temp.y = 58-3*(row+1);
									Path.push_back(temp);
								}
								else{
									coor temp;
									temp.x = Path[Path.size()-1].x -2;
								//temp.x = Right_x[row] - offset - 3*row;
									temp.y = 58-3*(row+1);
									Path.push_back(temp);
									}
								}
							}
					}
					else if(Rcurve){

						if(Left_x[row] < 75){
							if(Right_x[0] >=75){
								coor temp;
								temp.x = Left_x[row] + offset + 3*row +4;
								temp.y = 58-3*(row+1);
								Path.push_back(temp);
							}
							else{
								if(Right_x[row] < 75){
									coor temp;
									temp.x = (3*Left_x[row] + 5*Right_x[row])/8;
									temp.y = 58-3*(row+1);
									Path.push_back(temp);

								}

								else{
									coor temp;
									temp.x = Path[Path.size()-1].x + 2;
									//temp.x = Left_x[row] + offset + 3*row;
									temp.y = 58-3*(row+1);
									Path.push_back(temp);
								}
						}
					}
					}

					else if(Left_x[row] != 1 && Left_x[row] != 78 && Right_x[row] != 78 && Right_x[row] != 1){
					coor temp;
					temp.x = (Left_x[row] + Right_x[row])/2;
					temp.y = 58-3*(row+1);
					Path.push_back(temp);
					}

					else if(Left_x[row] == 1 && Right_x[row] != 78){
						coor temp;
						temp.x = Right_x[row] - offset - row;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

					else if(Left_x[row] != 1 && Right_x[row] == 78){
						coor temp;
						temp.x = Left_x[row] + offset + row;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

					else{
						coor temp;
						temp.x = (Left_x[row] + Right_x[row])/2;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

				}



			}


		}
		else if(roundabout){

			L_slow= false;
			R_slow = false;

			/*
			 * Path
			 */
			for(int row=0; row<14;row++){
				if(!Round_step){
					coor temp;
					temp.x = Left_x[row] + round_set -10;
					temp.y = 58-3*(row+1);
					Path.push_back(temp);
				}
				else if(Round_step && !Round_step2){
					if(Left_x[row] >2){
						coor temp;
						temp.x = Left_x[row] + round_set;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}
					else{
						coor temp;
						temp.x = 45 ;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

				}
				else if(Round_step2){
					coor temp;
					temp.x = Left_x[row] + 18;
					temp.y = 58-3*(row+1);
					Path.push_back(temp);
				}



			}

		}
		else if(crossroad){
			if(!Cross_step){
			}
			else{
				if(edge[L(55)].row <= 30 && edge[R(55)].row - edge[R(55)].row<=30){
					double k1 = (edge[L(52)].row-edge[L(55)].row)/(edge[L(55)].edgeposition - edge[L(52)].edgeposition);
					double k2 = (edge[R(52)].row-edge[R(55)].row)/(edge[R(55)].edgeposition - edge[R(52)].edgeposition);
					for(int row = 0; row< (60-edge[L(52)].row)/3; row++){
						int x1 = round(edge[L(52)].edgeposition - ((58-3*(row+1))-edge[L(52)].row)/k1);
						//int x2 = round(edge[R(52)].edgeposition - ((58-3*(row+1))-edge[R(52)].row)/k2);
						coor temp;
						temp.y = (58-3*(row+1));
						temp.x = x1 + 20 - row/2;
						Path.push_back(temp);

					}
				}
//				for(int row=0; row<14;row++){
//				coor temp;
//				if(Left_x[row] != -1 && Right_x[row] != -1){
//				temp.x = (Left_x[row] + Right_x[row])/2 ;
//				temp.y = 58-3*(row+1);
//				Path.push_back(temp);
//				}
//				}
			}



	}

}
bool Lcorner(int i,int j){
	double corner_count = 0.0 ;// How many black pixel
	double corner_num = 81 ;// How many pixel count
	double corner_ratio = 0.0 ;//Percentage of area of black pixel
	// 9x9 Box
	for(int n=i-4;n <= i+4;n++){
		for(int m=j-4;m <= j+4;m++){
			// 80*60
			if(camptr[m][n]){
				corner_count++;
			}
		}
	}
	corner_ratio = corner_count / corner_num;
//	char buffer[50];
//	sprintf(buffer, "%.2f",corner_count);
//	lcdP->SetRegion(Lcd::Rect(80,90,40,40));
//	writerP->WriteString(buffer);
	// If the percentage of black pixel is 20 - 30%, then it is corner.
	if(corner_ratio >= 0.2 && corner_ratio <= 0.35){
		return true;
		}

return false;
	}

bool roundabout_detect(int i,int j){
	int black_count = 0;
	for(int n = i-10; n<i;n++){
		for(int m = j-5; m<j+5;m++){
			if(camptr[m][n]){
				black_count++;}
			}
	}
	lcdP->SetRegion(Lcd::Rect(j-5,i-10,10,10));
	lcdP->FillColor(0x0000);
	if(black_count > 20){
		return true;
	}
	return false;

}




void checkround(){

	int L_count = 0;
	int R_count = 0;


//	int half_x = 0;
//	int half_y = 0;
//	int x = 0;
//	int y = 0;
//	int scale = 0;
	int middle_x = 0;
	int middle_y = 0;
	int bottom_x = 0;
	int bottom_y = 0;
	double k =0;
	middle_x = (L_corner.x + R_corner.x)/2;
	middle_y = (L_corner.y + R_corner.y)/2;
	bottom_x = (edge[L(1)].edgeposition + edge[R(1)].edgeposition)/2;
	bottom_y = (edge[L(1)].row + edge[R(1)].row)/2;
	k = double(bottom_y - middle_y)/double(middle_x - bottom_x);
	detectline.clear();
	for(int row =0; row< (middle_y)/3; row++){
		coor temp;
		temp.x = round((3*(row+1)/k) + middle_x);
		temp.y = middle_y - 3*(row+1);
		detectline.push_back(temp);
	}

	bool first = false;
	for(int row=0;row<detectline.size() -2;row++){
//		lcdP->SetRegion(Lcd::Rect(detectline[row].x,detectline[row].y, 2, 2));
//		lcdP->FillColor(0xF800);//red
		if(!first && !camptr[detectline[row].x][detectline[row].y] &&!camptr[detectline[row+1].x][detectline[row+1].y] &&camptr[detectline[row+2].x][detectline[row+2].y]){
			first = true;
		}


		if(first && camptr[detectline[row].x][detectline[row].y] &&camptr[detectline[row+1].x][detectline[row+1].y] && !camptr[detectline[row+2].x][detectline[row+2].y]){
			roundabout = true;
			crossroad = false;
			break;
		}
		else{
			crossroad = true;
			roundabout = false;
		}
	}



//	half_x = (middle_x - bottom_x)/2;
//	half_y = (middle_y - bottom_y)/2;
//	x = 2*middle_x - bottom_x;
//	y = 2*middle_y - bottom_y;
//	if(  middle_y >= 45){
//	while(y >= 35){
//		scale += 1;
//		x = (scale+4)*middle_x/2 - (scale+2)*bottom_x/2;
//		y = (scale+4)*middle_y/2 - (scale+2)*bottom_y/2;
//
//	}
//	}
//	if(roundabout_detect(y,x)){
//		roundabout = true;
//		crossroad = false;
//	}
//	else{
//		roundabout = false;
//		crossroad = true;
//	}



}


bool Rcorner(int i,int j){
	double corner_count = 0.0 ;// How many black pixel
	double corner_num =81 ;// How many pixel count
	double corner_ratio = 0.0 ;//Percentage of area of black pixel
	// 9x9 Box
	for(int n=i-4;n <= i+4;n++){
		for(int m=j-4;m <= j+4;m++){
			// 80*60
			if(camptr[m][n]){
				corner_count++;
			}
		}
	}


	corner_ratio = corner_count / corner_num;
//	char buffer[50];
//	sprintf(buffer, "%.2f",corner_count);
//	lcdP->SetRegion(Lcd::Rect(80,110,40,40));
//	writerP->WriteString(buffer);
	// If the percentage of black pixel is 20 - 30%, then it is corner.
	if(corner_ratio >= 0.2 && corner_ratio <= 0.35){
		return true;
		}

return false;
	}


void Printedge(){
	for(int j = 0; j< 60 ;j ++){
		lcdP->SetRegion(Lcd::Rect(edge[L(j)].edgeposition, edge[L(j)].row, 2, 2));
		lcdP->FillColor(0xF800);//red
		lcdP->SetRegion(Lcd::Rect(edge[R(j)].edgeposition, edge[R(j)].row, 2, 2));
		lcdP->FillColor(0X0000);//black
	}
}
void Printpath(){
	for(int i = 0; i < Path.size(); i++){
		if(Path[i].x >= 0){
		lcdP->SetRegion(Lcd::Rect(Path[i].x,Path[i].y, 2, 2));
		lcdP->FillColor(0xF800);//red
	}
	}
	char buffer[50];
	sprintf(buffer," %d ",Path.size());
	lcdP->SetRegion(Lcd::Rect(80,40,40,40));
	writerP->WriteString(buffer);
}
void printCameraImage(const Byte* image)
{
	lcdP->SetRegion(Lcd::Rect(0, 0, CAM_W, CAM_H));
	lcdP->FillBits(0x001F, 0xFFFF, image, CAM_W * CAM_H);
}

