#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/st7735r.h>
#include <libsc/k60/ov7725.h>
#include <libsc/futaba_s3010.h>
#include <libsc/alternate_motor.h>
#include <libsc/k60/jy_mcu_bt_106.h>
#include <libsc/dir_encoder.h>
#include <libsc/lcd_typewriter.h>
#include <libbase/k60/flash.h>
#include <libsc/led.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

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

//Led* led1;
JyMcuBt106* bluetooth;
const Byte tempInt = 170;
const Byte tempInt2 = 171;
const Byte tempInt3 = 169;
const Byte tempInt4 = 173;
const Byte tempInt5 = 168;
const Byte* temp4 = &tempInt4;
const Byte* temp3 = &tempInt3;
const Byte* temp2 = &tempInt2;
const Byte* temp = &tempInt;
const Byte* temp5 = & tempInt5;
AlternateMotor* Lmotor;
AlternateMotor* Rmotor;
Flash* flash;
St7735r* screen;
int motorPower = 100, max_speed = 450, min_speed = 0, track_size = 60;
FutabaS3010* servo;
LcdTypewriter* writer;
const Byte* camPtr;
double car_center = 40;
int centerLine = 40, black_img_in_trackL[80], black_img_in_trackR[80];
double intervalMs = 100;
int max_servoDeg = 45, min_servoDeg = -45, sideL[80], sideR[80];
Byte center[80];
bool inAuto = true;
double data_string[20]={};
//for pid
double Kp, Ki, I, Kd, output, err, lastInput, Input;
string var_string;
int data_string_len, in_turn=0;
string s = "";
int k = 0, flag = 0, in_round;
size_t len = 0;
ssize_t read;
Uint camSize;
char c[20];
bool image[100][100], first_time = 1, maybe_black_img_in_track=0, find_sideL = 0;

double data[9] = {227, 0, 227, 10000, 4500, -4500, 45000, 0, 10000};

double toDouble(char* s){

    int start = 0, flag = 0;
    double cnt=1;
    if(s[0]=='-') start++;
    double d = 0;
    for(int i=start;i<strlen(s);i++){
        if(s[i]=='.'){
            flag = i;
            continue;
        }
        d*=10;
        d+=s[i]-'0';
        if(flag) cnt*=10;
    }
    if(s[0]=='-') return -d/cnt;
    return d/cnt;
}
void stopMotor(){
	Lmotor->SetPower(0);
	Rmotor->SetPower(0);
}
void startMotor(){
	Lmotor->SetPower(motorPower);
	Rmotor->SetPower(motorPower);
}
void setMotorDir(int dir){ //forward when dir = 0
	Lmotor->SetClockwise(dir);
	Rmotor->SetClockwise(1-dir);
}
void setServoDegree(double deg){ // 0: middle, >0: right, <0: left
	servo->SetDegree((int)(deg*-10+900));
}
void adjustSpeed(int speed){
	motorPower+=speed;
	if(motorPower>max_speed) motorPower = max_speed;
	else if(motorPower<min_speed) motorPower = min_speed;
}
void setIntervalMs(double newInterval){
	double ratio = newInterval/intervalMs;
	Ki *= ratio;
	Kd /= ratio;
	intervalMs = newInterval;
}
void setServoDegLimit(int maxi, int mini){
	max_servoDeg = maxi;
	min_servoDeg = mini;

	if(I>max_servoDeg) I = max_servoDeg;
	else if(I<min_servoDeg) I = min_servoDeg;
}
void pidInit(){
	lastInput = Input; //d of pid=0 now
	I = 0;
}
void setPidTuning(double kp, double ki, double kd){
	Kp = kp;
	Ki = ki*intervalMs;
	Kd = kd/intervalMs;
}
void setAuto(bool toAuto){
	if(toAuto) {
		pidInit();
		inAuto = true;
	} else{
		inAuto = false;
		stopMotor();
	}
}
void PID(double input, double setPoint){
	if(!inAuto) return;

	Input = input;
	err = setPoint-input;
	I += (Ki*err);
	if(I>max_servoDeg) I = max_servoDeg;
	else if(I<min_servoDeg) I = min_servoDeg;

	output = Kp*err + I + Kd*(input-lastInput);
	if(output>max_servoDeg) output = max_servoDeg;
	else if(output<min_servoDeg) output = min_servoDeg;
	setServoDegree(-output);

	lastInput = input;
}
void update_data(double* data, int data_len){
	if(data[8]==0) return;
	setPidTuning(data[0]/100,data[1]/100,data[2]/100);
	motorPower = data[3]/100;
	max_servoDeg = data[4]/100;
	min_servoDeg = data[5]/100;
	max_speed = data[6]/100;
	min_speed = data[7]/100;
	intervalMs = data[8]/100;

	//int hi;
	//hi = flash->Write(data, 9);
}
void sendReceivedChar(Byte data){
	bluetooth->SendBuffer(temp3,1);
	bluetooth->SendBuffer(&data, 1);
}
bool BTonReceiveInstruction(const Byte *data, const size_t size){
	switch(data[0]){
	case 0:
		inAuto = true;
		setServoDegree(0);
		break;
	case 1:
		inAuto = false;
		stopMotor();
		break;
	case 2:
		stopMotor();
		break;
	case ' ':
		inAuto = false; //press space key to enter manual mode
		stopMotor();
		setMotorDir(0);
		break;
	case 'w':
		sendReceivedChar('w');
		setServoDegree(0);
		setMotorDir(0);
		startMotor();
		break;
	case 's':
		sendReceivedChar('s');
		setServoDegree(0);
		setMotorDir(1);
		startMotor();
		break;
	case 'a':
		sendReceivedChar('a');
		setServoDegree(-20);
		startMotor();
		break;
	case 'd':
		sendReceivedChar('d');
		setServoDegree(20);
		startMotor();
		break;
	case '[':
		adjustSpeed(-20);
		break;
	case ']':
		adjustSpeed(20);
		break;
	case 'f':
		data_string[data_string_len++] = toDouble(strcpy(c, s.c_str()));
		s = "";
		break;
	case '\n':
		update_data(data_string, 9);
		data_string_len = 0;
		break;
	default:
		s+=data[0];
		break;
	}

	return true;
}
void about_centerline(const Byte* camPtr){
	int area_dif=0, x, white_area = 0, black_img_in_track_time=0;
	for(int line=0;line<60;line++){
		if(white_area>78) {
			center[line-1] = center[line]; // if cross road -> follows the last center line (so the route won't shift a lot)
		}
		find_sideL = 0;
		sideL[line] = 0; sideR[line] = 80; black_img_in_trackL[line]=0; black_img_in_trackR[line]=0, maybe_black_img_in_track=0;
		if(!image[line][0]) find_sideL = 1;
		x = 0, white_area = 0;
		for(int i=0; i<10; i++){
			for(int j=7; j>=0; j--){
				if(((*(camPtr+line*10+i))>>j)&1==1) image[line][x++] = 1;
				else image[line][x++] = 0, white_area++;

				if(x>2 && image[line][x-3] && !image[line][x-2] && !image[line][x-1]){ // BWW (B: Black, W: White)
					if(maybe_black_img_in_track){
						maybe_black_img_in_track=0;
						black_img_in_trackR[line] = x-2;
					}
					if(!find_sideL){
						sideL[line] = x-2;
						find_sideL = 1;
					}
				}
				if(x>2 && !image[line][x-3] && !image[line][x-2] && image[line][x-1]){ // WWB
					maybe_black_img_in_track = 1;
					if(!black_img_in_trackL[line]) black_img_in_trackL[line] = x-2;
					sideR[line] = x-2; // rewrite the value whenever this pattern is detected -> find the right most edge
				}
				if(line<5) { // find the area diff on the top region of the img (decide the turning direction in a roundabout)
					if(x>40) area_dif+=image[line][x-1];
					else area_dif-=image[line][x-1];
				} else{ // area_dif is done calculated
					if(!in_turn){ // save the turning dir cause the top white region may change as the car proceeds(thus affect the turniing dir)
						if(area_dif<0) in_turn=1; // turn right: 1, turn left: 2
						else in_turn=2;
					}
				}
			}
		}
		if(!image[line][x-1]) sideR[line]=80; // essential because the pattern WWB may exist and rewrite the original value(80) while it is a black img in track instead of sideR
		if(black_img_in_trackR[line]){ // if black img in track
			black_img_in_track_time++;
			// roundabout or obstacles detected
				if(sideR[line]-sideL[line] > 80*0.75){ // if track edge width > 75% of the horizontal line(TBD) -> roundabout
					if(in_turn==1){ // turn right
						sideL[line] = black_img_in_trackR[line];
					} else { // in_turn=2 -> turn left
						sideR[line] = black_img_in_trackL[line];
					}
				} else{ // -> obstacle
					if(black_img_in_trackL[line]-sideL[line] > sideR[line]- black_img_in_trackL[line]){ // if left road is wider
						sideR[line] = black_img_in_trackL[line]; // turn left
					} else{ // turn right
						sideL[line] = black_img_in_trackR[line];
					}
				}
		}
		center[line] = (sideL[line]+sideR[line])/2;

		if(line>10){ // cannot smooth the top 10 lines, otherwise array index will be negative
			if (abs(center[line-10]-center[line-9])>5){ // smooth center line
				for (int k=0;k<10;k++){
					center[line-10+k] = (center[line]-center[line-10])*k/9 + center[line-10]; // modify the center line 10 lines closer to our car(so as to turn earlier)
				}
			}
		}
	}
	if(black_img_in_track_time<5) { // if return to normal road -> finish turning
		in_turn = 0;
	}
}
void send_image_BT(const Byte* camPtr, Uint size){
	bluetooth->SendBuffer(temp,1);
	bluetooth->SendBuffer(camPtr, size);
}
void tft(const Byte* camPtr, Uint size){
	screen->SetRegion(Lcd::Rect(0,0,80,60));
	screen->FillBits(St7735r::kBlack,St7735r::kWhite,camPtr,8*size);

	for(int line=0;line<60;line++){

		screen->SetRegion(Lcd::Rect(center[line], line-1, 2, 2));
		screen->FillColor(St7735r::kRed);

		screen->SetRegion(Lcd::Rect(sideL[line], line-1, 2, 2));
		screen->FillColor(St7735r::kBlue);

		screen->SetRegion(Lcd::Rect(sideR[line], line-1, 2, 2));
		screen->FillColor(St7735r::kGreen);

	}
}
void receive_mode_instruction(){
	Byte isauto = 0;
	if(inAuto) isauto = 1;
	else isauto = 0;
	bluetooth->SendBuffer(temp4,1);
	bluetooth->SendBuffer(&isauto, 1);
}
void send_data_to_bt(int32_t Lencoder, int32_t Rencoder){
	Byte test[11] = {Kp, Ki/intervalMs, Kd*intervalMs, motorPower, max_servoDeg, min_servoDeg, max_speed, min_speed, intervalMs, Lencoder, Rencoder};
	bluetooth->SendBuffer(temp2,1);
	bluetooth->SendBuffer(test, 11);
	bluetooth->SendBuffer(center, 60);
}
void send_sys_time_to_BT(Byte time){
	bluetooth->SendBuffer(temp5,1);
	bluetooth->SendBuffer(&time, 1);
}

int main(void)
{
	System::Init();

	Ov7725::Config C;  //camera
	C.id = 0;
	C.w = 80;
	C.h = 60;
	Ov7725 cam(C);

	St7735r::Config s; //tft
	s.is_revert = false;
	s.is_bgr = false;
	s.fps = 100;
	St7735r m_screen(s);
	screen = &m_screen;

	LcdTypewriter::Config writerConfig; //tft_writer
	writerConfig.lcd = &m_screen;
	writerConfig.is_text_wrap = true;
	LcdTypewriter m_writer(writerConfig);
	writer = &m_writer;
	Timer::TimerInt t=0;

	JyMcuBt106::Config bluetooth_config;
	bluetooth_config.id = 0;
	bluetooth_config.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	bluetooth_config.rx_isr = BTonReceiveInstruction;
	JyMcuBt106 m_bluetooth(bluetooth_config);
	bluetooth = &m_bluetooth;

	FutabaS3010::Config servo_config;
	servo_config.id = 0;
	FutabaS3010 m_servo(servo_config);
	servo = &m_servo;

	Led::Config led_config;
	led_config.id = 0;
	led_config.is_active_low = true;
	Led inner_led1(led_config);
	//led1 = &inner_led1;

	AlternateMotor::Config Lmotor_config, Rmotor_config;
	Lmotor_config.id = 0; Rmotor_config.id = 1;
	AlternateMotor m_Lmotor(Lmotor_config), m_Rmotor(Rmotor_config);
	Lmotor = &m_Lmotor; Rmotor = &m_Rmotor;
	m_Lmotor.SetClockwise(1); m_Rmotor.SetClockwise(0);

	Encoder::Config Ldir_encoder_config, Rdir_encoder_config;
	Ldir_encoder_config.id = 0;
	Rdir_encoder_config.id = 1;
	DirEncoder LdirEncoder(Ldir_encoder_config);
	DirEncoder RdirEncoder(Rdir_encoder_config);

	Flash::Config flash_config;
	Flash m_flash(flash_config);
	flash = &m_flash;

	pidInit();
	//m_flash.Read(data, 9);

	update_data(data, 9);

	cam.Start();

	while (true){
		while(t!=System::Time()){
			t = System::Time();
			if(t % (int)intervalMs == 0){

				camPtr = cam.LockBuffer();
				camSize = cam.GetBufferSize();

				about_centerline(camPtr);
				tft(camPtr, camSize);

				PID(center[50], car_center); // use 50 for now
				//receive_mode_instruction();
				if(inAuto) startMotor();
				send_image_BT(camPtr, camSize);

				LdirEncoder.Update();
				RdirEncoder.Update();

				send_data_to_bt(LdirEncoder.GetCount(), RdirEncoder.GetCount());

				cam.UnlockBuffer();
			}
			send_sys_time_to_BT(System::Time()-t);
		}
	}

	cam.Stop();
	return 0;
}
