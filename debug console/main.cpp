/*
 * main.cpp
 *
 * Author: Gordon
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */


#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/pin.h>
#include <libsc/st7735r.h>
#include <libsc/k60/ov7725.h>
#include <libsc/lcd.h>
#include <libsc/servo.h>
#include <libsc/alternate_motor.h>
#include <libsc/motor.h>
#include <libsc/lcd_console.h>
#include <libsc/lcd_typewriter.h>
#include <libsc/ab_encoder.h>
#include <libsc/joystick.h>
//#include <algorithm>

#define WIDTH 80
#define HEIGHT 60
#define AREA_KP 1.1
#define SPEEDAREA 3

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

//===========================


//syntax sugars
//just down or start
#define START_IDLE 	0
#define DOWN_SELECT 1
#define DOWN_LEFT 	2
#define DOWN_RIGHT 	3
//just up or end5
#define END_IDLE 	4
#define UP_SELECT 	5
#define UP_LEFT 	6
#define UP_RIGHT 	7
//down for a while
#define LONG_IDLE 	8
#define LONG_SELECT 9
#define LONG_LEFT 	10
#define LONG_RIGHT 	11

#define IDLE 	0
#define SELECT 	1
#define LEFT 	2
#define RIGHT	3

#define START 	0
#define END 	1
#define DOWN 	0
#define UP		1
#define LONG 	2


#define GET_TIME 	System::Time()


class DebugConsole{

	public:
		typedef void(*Fptr)();
		class Item{
			public:

			//Item(char* n):text(n){init();}
			Item(char* text=NULL,int* value=NULL,bool readOnly=false):text(text),value(value),readOnly(readOnly){init();}
			//Item(){init();}

			//display text methods
			char* getText(){return text;}
			void setText(char* t){text=t;}

			//listener methods
			Fptr getListener(int type){return listeners[type];}
			Fptr getListeners(){return *listeners;}
			void setListener(int type, Fptr fptr){listeners[type]=fptr;}

			//value methods
			void setValuePtr(int* v){value=v;}
			void setValue(int v){if(value==NULL)return;*value=v;}
			int* getValuePtr(){return value;}
			int getValue(){if(value==NULL)return 0;return *value;}

			bool isReadOnly(){return readOnly;}
			void setReadOnly(bool isReadOnly){readOnly=isReadOnly;}

			private:
				Fptr listeners[12];
				char* text;
				int* value=NULL;
				bool readOnly;
				bool upLongExclusive;
				void init(){
					for(int i=0;i<12;i++)
						listeners[i]=NULL;
				}
		};
	public:


		DebugConsole(Joystick* joystick,St7735r* lcd, LcdTypewriter* writer):
			length(0),focus(0),topIndex(0),joystick(joystick),lcd(lcd),writer(writer),threshold(1000){
			Item item(">>exit debug<<");
			pushItem(item);
		}


		void enterDebug(){
			listItems();
			int flag=1,time_next,time_img=0;
			Joystick::State key_img;
			while(flag){
				if(System::Time()!=time_img){
					time_img=System::Time();
					if(joystick->GetState()!=Joystick::State::kIdle){
						Joystick::State key=joystick->GetState();
						Item item=items[focus];
						if (key!=key_img){
							key_img=key;
							flag = listen(key,DOWN);
							time_next=GET_TIME+threshold;
						}
						else if(time_img>time_next&&time_img%10==0){
							flag = listen(key,LONG);
						}

					}
					else{
						flag = listen(key_img,UP);
						key_img=Joystick::State::kIdle;
					}
				}



			}
			clear();
		}

		void pushItem(Item item){
			insertItem(item,length-1);
			//items[length++]=item;
		}

		void insertItem(Item item, int index=0){
			index = (index<0?0:(index>length?length:index));
			for(int i=length++;i>=index;i--)
				items[i]=items[i-1];
			items[index]=item;
		}

		void listItems(int start=0){
			clear();
			for(int i=start;i<(length<start+10?length:start+10);i++){
				printItem(i);
			}
			showFocus(0);
		}

		void printItem(int index){
			if(items[index].getValuePtr()!=NULL){
					char buff[20];
					sprintf(buff,"%s%d      ",items[index].getText(),items[index].getValue());
					printxy(1,index-topIndex,buff);
				}else
					printxy(1,index-topIndex,items[index].getText());
		}

	private:

		int length;
		int focus;
		int topIndex;
		Item items[50];
		Joystick* joystick;
		St7735r* lcd;
		LcdTypewriter* writer;
		int threshold;



		void printxy(int x, int y, char* c, int l=100){
			lcd->SetRegion(Lcd::Rect(x*10,y*15,l,15));
			writer->WriteString(c);
			//showFocus(0);
		}

		void showFocus(bool flag=1){
			if(flag) writer->WriteString(" ");
			lcd->SetRegion(Lcd::Rect(0,(focus-topIndex)*15,10,15));
			writer->WriteString(">");
		}

		void clear(){
			//lcd->SetRegion(Lcd::Rect(0,0,128,120));
			lcd->Clear();
		}

		int listen(Joystick::State key,int state){
			Item item=items[focus];
			switch(key){
				case Joystick::State::kDown:
					if(state!=UP){
						printxy(0,focus," ",10);
						focus=(focus+1)%length;
						if(focus-topIndex>8&&focus!=length-1){
							topIndex++;
							listItems(topIndex);
						}else if(focus==0){
							topIndex=0;
							listItems(topIndex);
						}
					}
					printItem(focus);
					break;
				case Joystick::State::kUp:
					if(state!=UP){
						printxy(0,focus," ",10);
						if (focus==0){
							focus=length-1;
							if(length>9){
								topIndex=length-10;
								listItems(topIndex);
							}
						}else{
							focus--;
							if(focus-topIndex<1&&topIndex>0){
								topIndex--;
								listItems(topIndex);
							}
						}


					}
					printItem(focus);
					break;
				case Joystick::State::kSelect:
					if(item.getListener(SELECT+state*4)!=NULL){
						item.getListener(SELECT+state*4)();
						listItems(topIndex);
					}
					else if(item.getText()==">>exit debug<<")
						return 0;
					break;
				case Joystick::State::kLeft:
					if(item.getListener(LEFT+state*4)!=NULL){
						item.getListener(LEFT+state*4)();
						listItems(topIndex);
					}
					else if(item.getValuePtr()!=NULL&&state!=UP&&!item.isReadOnly()){
						item.setValue(item.getValue()-1);
						printItem(focus);
					}

					break;
				case Joystick::State::kRight:
					if(item.getListener(RIGHT+state*4)!=NULL){
						item.getListener(RIGHT+state*4)();
						//listItems(topIndex);
					}
					else if(item.getValuePtr()!=NULL&&state!=UP&&!item.isReadOnly()){
						item.setValue(item.getValue()+1);
						printItem(focus);
					}

					break;
				default:
					return 1;
			}
			//showFocus(1);
			printxy(0,focus,">",10);
			//listItems(topIndex);
			return 1;
		}
};


//==========================
/*
void to2D(const Byte* buff);
bool median_filter(int,int);

void analysis(const Byte* buff);
bool get_bit(const Byte* buff, int x, int y){
	return buff[y*WIDTH/8+x/8] & 0x80>>x%8;
}

//distortion solver
void find_vanish(const Byte* buff);
bool get_fixedbit(const Byte* buff, int, int, int);*/
int16_t vanish_x=0,vanish_y=0;
int yL=0,yR=0;


int max(int a,int b){return (a>b?a:b);}
int min(int a,int b){return (a<b?a:b);}

k60::Ov7725* cam;
St7735r* lcd;
AlternateMotor* motor;
Servo* servo;

int servo_target=900, servo_pos=900, motor_target = 200, motor_speed = 200;

bool map[HEIGHT][WIDTH];

int main(void){
   	System::Init();

	//init LED
	Led::Config config;
	config.id=0;
	Led led1(config);
	config.id=1;
	Led led2(config);
	config.id=2;
	Led led3(config);
	config.id=3;
	Led led4(config);

	led1.SetEnable(true);
	led2.SetEnable(false);
	led3.SetEnable(false);
	led4.SetEnable(true);


	//init MOTOR
	AlternateMotor::Config motor_config;
	motor_config.id=1;
	AlternateMotor motor1(motor_config);
	motor = &motor1;

	motor->SetPower(150);


	//init servo
	Servo::Config servo_config;
	servo_config.id=0;
	servo_config.period=40000;
	servo_config.min_pos_width=910;
	servo_config.max_pos_width=2100;
	Servo servo1(servo_config);
	servo = &servo1;

	servo->SetDegree(900);


	//init LCD
	St7735r::Config lcd_config;
	St7735r lcd1(lcd_config);
	lcd = &lcd1;

	Lcd::Rect r(0,0,WIDTH,HEIGHT);
	lcd->SetRegion(r);

	uint32_t time_img=0;

	//LcdConsole::Config console_config;
	//console_config.lcd=&lcd1;
	//LcdConsole console(console_config);

	LcdTypewriter::Config writer_config;
	writer_config.lcd = &lcd1;
	LcdTypewriter writer(writer_config);

	//init camera
	k60::Ov7725::Config cam_config;
	cam_config.id=0;
	cam_config.w=WIDTH;
	cam_config.h=HEIGHT;
	cam_config.fps = k60::Ov7725::Config::Fps::kHigh;
	k60::Ov7725 cam1(cam_config);
	cam = &cam1;

	cam->Start();

	//init encoder
	//AbEncoder::Config encoder_config;
	//encoder_config.id = 0;
	//Encoder::Initializer encoder_init(encoder_config);
	//AbEncoder encoder1(encoder_config);

	//init joystick
	Joystick::Config joystick_config;
	joystick_config.id = 0;
	joystick_config.is_active_low = true;
	Joystick joystick(joystick_config);

	writer.WriteString("hi");

	DebugConsole console(&joystick,&lcd1, &writer);
	DebugConsole::Item item("first");
	console.pushItem(item);
	int i=0;
	item.setValuePtr(&i);
	item.setText("i = ");
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.pushItem(item);
	console.enterDebug();

	while(true){


		if (System::Time()!=time_img){
			time_img=System::Time();

			if (time_img%250==0){
				led1.Switch();
				led2.Switch();
				led3.Switch();
				led4.Switch();
			}

			//joystick
			lcd->SetRegion(Lcd::Rect(0,0,100,10));
			switch(joystick.GetState()){
			case Joystick::State::kUp:
				writer.WriteString("UP");
				break;
			case Joystick::State::kDown:
				writer.WriteString("DOWN");
				break;
			case Joystick::State::kIdle:
				writer.WriteString("IDLE");
				break;
			case Joystick::State::kLeft:
				writer.WriteString("LEFT");
				break;
			case Joystick::State::kRight:
				writer.WriteString("RIGHT");
				break;
			case Joystick::State::kSelect:
				writer.WriteString("SELECT");
				break;
			}

		//System::DelayMs(250);
		}
	}

	return 0;
}
