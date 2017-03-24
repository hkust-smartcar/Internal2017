#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/st7735r.h>
#include <libsc/k60/ov7725.h>
#include <libsc/futaba_s3010.h>
#include <libsc/alternate_motor.h>
#include <libsc/k60/jy_mcu_bt_106.h>
#include <libsc/led.h>
#include <libsc/dir_encoder.h>
#include <ctype.h>
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

bool BTonReceiveInstruction(const Byte *data, const size_t size);
void stopMotor();
void startMotor();
Led* led1;
JyMcuBt106* exterior_bluetooth;
const Byte tempInt = 170;
const Byte tempInt2 = 171;
const Byte tempInt3 = 169;
const Byte tempInt4 = 173;
const Byte* temp4 = &tempInt4;
const Byte* temp3 = &tempInt3;
const Byte* temp2 = &tempInt2;
const Byte* temp = &tempInt;
AlternateMotor* exterior_Lmotor;
AlternateMotor* exterior_Rmotor;
int motorPower = 100, max_speed = 450, min_speed = 0;
FutabaS3010* exterior_servo;
const Byte* camPtr;
int centerLine = 40, car_center = 40;
double intervalMs = 100;
int max_servoDeg = 45, min_servoDeg = -45;
bool inAuto = false;
double data_string[20]={};
//for pid
double Kp, Ki, I, Kd, output, err, lastInput, Input;
string var_string;
int data_string_len;
string s = "";
int k = 0, flag = 0;
FILE* fp;
char * line = NULL;
size_t len = 0;
ssize_t read;
char c[20];

void pidInit();
void PID(double input, double setPoint);
void setPidTuning(double kp, double ki, double kd);
double toDouble(char* s);
void read_data_from_file();

int main(void)
{
	System::Init();
	read_data_from_file();

	Ov7725::Config C;  //camera init;
	C.id = 0;
	C.w = 80;
	C.h = 60;
	Ov7725 cam(C);

	St7735r::Config s; //screen init;
	s.is_revert = false;
	s.is_bgr = false;
	s.fps = 100;
	St7735r screen(s);
	Timer::TimerInt t=0;

	JyMcuBt106::Config bluetooth_config;
	bluetooth_config.id = 0;
	bluetooth_config.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	bluetooth_config.rx_isr = BTonReceiveInstruction;
	JyMcuBt106 bluetooth(bluetooth_config);
	exterior_bluetooth = &bluetooth;

	FutabaS3010::Config servo_config;
	servo_config.id = 0;
	FutabaS3010 servo(servo_config);
	servo.SetDegree(900);
	exterior_servo = &servo;

	Led::Config led_config;
	led_config.id = 0;
	led_config.is_active_low = true;
	Led inner_led1(led_config);
	led1 = &inner_led1;

	AlternateMotor::Config Lmotor_config, Rmotor_config;
	Lmotor_config.id = 0; Rmotor_config.id = 1;
	AlternateMotor Lmotor(Lmotor_config), Rmotor(Rmotor_config);
	exterior_Lmotor = &Lmotor; exterior_Rmotor = &Rmotor;
	Lmotor.SetClockwise(1); Rmotor.SetClockwise(0);

	Encoder::Config Ldir_encoder_config, Rdir_encoder_config;
	Ldir_encoder_config.id = 0;
	Rdir_encoder_config.id = 1;
	DirEncoder LdirEncoder(Ldir_encoder_config);
	DirEncoder RdirEncoder(Rdir_encoder_config);

	pidInit();
	setPidTuning(0,0,0); //TBD

	cam.Start();

	while (true){
		while(t!=System::Time()){
			t = System::Time();
			if(t % (int)intervalMs == 0){

				camPtr = cam.LockBuffer();

				//for tft
				screen.SetRegion(Lcd::Rect(0,0,80,60));
				screen.FillBits(St7735r::kBlack,St7735r::kWhite,camPtr,8*cam.GetBufferSize());

				//for bluetooth
				bluetooth.SendBuffer(temp,1);
				bluetooth.SendBuffer(camPtr, cam.GetBufferSize());

				Byte isauto = 0;
				const Byte* auto_ptr = &isauto;
				if(inAuto) isauto = 1;
				else isauto = 0;
				bluetooth.SendBuffer(temp4,1);
				bluetooth.SendBuffer(auto_ptr, 1);

				LdirEncoder.Update();
				RdirEncoder.Update();

				if(inAuto) {
					PID(centerLine, car_center);
					startMotor();
				}

				cam.UnlockBuffer();
			}
		}
	}

	cam.Stop();
	return 0;
}

void stopMotor(){
	exterior_Lmotor->SetPower(0);
	exterior_Rmotor->SetPower(0);
}
void startMotor(){
	exterior_Lmotor->SetPower(motorPower);
	exterior_Rmotor->SetPower(motorPower);
}
void setMotorDir(int dir){ //forward when dir = 0
	exterior_Lmotor->SetClockwise(dir);
	exterior_Rmotor->SetClockwise(1-dir);
}
void setServoDegree(double deg){ // 0: middle, >0: right, <0: left
	exterior_servo->SetDegree((int)(deg*-10+900));
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
	Input = input;
	if(!inAuto) return;

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
void update_data_from_bt(){
	setPidTuning(data_string[0], data_string[1],data_string[2]);
	motorPower = data_string[3];
	max_servoDeg = data_string[4];
	min_servoDeg = data_string[5];
	max_speed = data_string[6];
	min_speed = data_string[7];
	intervalMs = data_string[8];
	//exterior_bluetooth->SendBuffer(temp2,1);
	//Byte test[12] = {Kp, Ki/intervalMs, Kd*intervalMs, motorPower, max_servoDeg, min_servoDeg, max_speed, min_speed, intervalMs, 0, 0, centerLine};
	//exterior_bluetooth->SendBuffer(test, 12);

}
void sendReceivedChar(Byte data){
	exterior_bluetooth->SendBuffer(temp3,1);
	exterior_bluetooth->SendBuffer(&data, 1);
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
		update_data_from_bt();
		data_string_len = 0;
		break;
	case 'c':
		centerLine = toDouble(strcpy(c, s.c_str()));
		s = "";
		break;
	default:
		s+=data[0];
		break;
	}

	return true;
}
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
void read_data_from_file(){
	char* data_from_file[20];
	int cnt = 0;
	char const* const fileName = "/Users/tina/Documents/Processing/Updated_GUI/list.txt"; /* should check that argc > 1 */
	FILE* file = fopen(fileName, "r"); /* should check the result */
	char line[256];

	while (fgets(line, sizeof(line), file)) {
		data_from_file[cnt++] = line;
	}
	fclose(file);

	Kp = toDouble(data_from_file[0]);
	Ki = toDouble(data_from_file[1]);
	Kd = toDouble(data_from_file[2]);
	motorPower = toDouble(data_from_file[3]);
	max_servoDeg = toDouble(data_from_file[4]);
	min_servoDeg = toDouble(data_from_file[5]);
	max_speed = toDouble(data_from_file[6]);
	min_speed = toDouble(data_from_file[7]);
	intervalMs = toDouble(data_from_file[8]);
}
