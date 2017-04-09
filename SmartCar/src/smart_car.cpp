/*
 * main.cpp
 *
 * Author: Leslie
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/smart_car.h"

#include <libsc/k60/jy_mcu_bt_106.h>

//Global variable-----------------------------------------------------------------------------------------------------------
const Uint CamHeight=60;
const Uint CamWidth=80;
const Uint MotorPower=25;
const Uint MotorStSpeed=25;
const Uint MotorSlowDownPower=25;
#define ServoConstant 1.2;
#define MotorConstant 0.001;
extern Uint Cam2DArray[CamHeight][CamWidth];

//Uint Cam2DArray[CamHeight][CamWidth];

//Speed PID control---------------------------------------------------------------------------------------------------------
void SpeedPID(AlternateMotor *motorA,AlternateMotor *motorB,DirEncoder *encoderA,DirEncoder *encoderB,int speedA,int speedB,LcdConsole *console){

#define Kp 0.02
	bool breakA=false;
	bool breakB=false;
	Timer::TimerInt t1=0;
	Timer::TimerInt t2=0;
	int TimeDiff=0;

	while(1){
		encoderA->Update();
		encoderB->Update();
		t2=System::Time();
		TimeDiff=t2-t1;
		t1=t2;
		int errorA=std::abs(encoderA->GetCount())*1000/TimeDiff;
		int errorB=std::abs(encoderB->GetCount())*1000/TimeDiff;
		char buff[20];
		char buff1[20];
		sprintf(buff,"%d ",errorA);
		sprintf(buff1,"%d ",errorB);
		console->WriteString(buff);
		console->WriteString(buff1);
		if(errorA==speedA){
			breakA=true;
		}
		if(errorB==speedB){
			breakB=true;
		}
		if(breakA==true&&breakB==true){
			break;
		}
		errorA=(speedA-errorA)*Kp;
		errorB=(speedB-errorB)*Kp;
		motorA->AddPower(errorA);
		motorB->AddPower(errorB);
	}

//	int breakA=false;
//	int breakB=false;
//
//	Looper looper;
//
//	std::function<void(const Timer::TimerInt, const Timer::TimerInt)> pid =
//			[&](const Timer::TimerInt request, const Timer::TimerInt)
//		{
//			encoderA->Update();
//			encoderB->Update();
//			if(std::abs(encoderA->GetCount())==speedA){
//				breakA=true;
//			}
//			if(std::abs(encoderB->GetCount())==speedB){
//				breakB=true;
//			}
//			if(breakA==true&&breakB==true){
//				looper.Break();
//				if(looper.IsBreak()==false){
//					looper.Break();
//				}
//			}
//			int errorA=0;
//			int errorB=0;
//			errorA=speedA-std::abs(encoderA->GetCount());
//			errorA=0.25*errorA;
//			errorB=speedB-std::abs(encoderB->GetCount());
//			errorB=0.25*errorB;
//			motorA->AddPower(errorA);
//			motorB->AddPower(errorB);
//			looper.RunAfter(request,pid);
//		};
//	looper.RunAfter(10, pid);
//
//	looper.ResetTiming();
//
//	while(true){
//		if(breakA==true&&breakB==true){
//			break;
//		}
//		looper.Once();
//	}
}

//Turn the used Camera Buffer as area, using the area different to contol the car, turning servo and two motor speed with
//area different, working fine----------------------------------------------------------------------------------------------
//void Camera2DConverterWithCarControl(const Byte* CameraBuffer,FutabaS3010 *servo,AlternateMotor *motorA,
//									 AlternateMotor *motorB,DirEncoder *encoderA,DirEncoder *encoderB){
//	Byte CamByte;
//	Uint LeftArea=0;
//	Uint RightArea=0;
//	Uint degree=700;
//	int RoadAreaDiff=0;
//	int MotorDiff=0;
//	bool IsStraightLine=false;
//	for(Uint row=(CamHeight-40);row<(CamHeight-20);row++){
//		for(Uint col=0;col<CamWidth/2;col+=8){
//			CamByte=CameraBuffer[row*CamWidth/8+col/8];
//			LeftArea=LeftArea+!(CamByte&0x80)+!(CamByte&0x40)+!(CamByte&0x20)+!(CamByte&0x10)
//							 +!(CamByte&0x08)+!(CamByte&0x04)+!(CamByte&0x02)+!(CamByte&0x01);
//		}
//		for(Uint col=(CamWidth/2);col<CamWidth;col+=8){
//			CamByte=CameraBuffer[row*CamWidth/8+col/8];
//			RightArea=RightArea+!(CamByte&0x80)+!(CamByte&0x40)+!(CamByte&0x20)+!(CamByte&0x10)
//							 +!(CamByte&0x08)+!(CamByte&0x04)+!(CamByte&0x02)+!(CamByte&0x01);
//		}
//	}
//	RoadAreaDiff=LeftArea-RightArea;
//	degree=degree+RoadAreaDiff*ServoConstant;
//	if(degree>900){
//		degree=900;
//	}else if(degree<430){
//		degree=430;
//	}
//	servo->SetDegree(degree);
//	MotorDiff=RoadAreaDiff*MotorConstant;
//	if(RoadAreaDiff>45||RoadAreaDiff<-45){
//		SpeedPID(motorA,motorB,encoderA,encoderB,MotorPower+MotorDiff,MotorPower-MotorDiff);
//	}else{
//		SpeedPID(motorA,motorB,encoderA,encoderB,MotorStSpeed,MotorStSpeed);
//		IsStraightLine=true;
//	}
//	if(IsStraightLine==true){
//		Uint UpperLeftArea=0;
//		Uint UpperRightArea=0;
//		for(Uint row=CamHeight-60;row<CamHeight-40;row++){
//			for(Uint col=0;col<(CamWidth/2);col++){
//				CamByte=CameraBuffer[row*CamWidth/8+col/8];
//				UpperLeftArea=UpperLeftArea+!(CamByte&0x80)+!(CamByte&0x40)+!(CamByte&0x20)+!(CamByte&0x10)
//								 +!(CamByte&0x08)+!(CamByte&0x04)+!(CamByte&0x02)+!(CamByte&0x01);
//			}
//			for(Uint col=(CamWidth/2);col<CamWidth;col++){
//				CamByte=CameraBuffer[row*CamWidth/8+col/8];
//				UpperRightArea=UpperRightArea+!(CamByte&0x80)+!(CamByte&0x40)+!(CamByte&0x20)+!(CamByte&0x10)
//								 +!(CamByte&0x08)+!(CamByte&0x04)+!(CamByte&0x02)+!(CamByte&0x01);
//			}
//		}
//		RoadAreaDiff=UpperLeftArea-UpperRightArea;
//		if(RoadAreaDiff>20||RoadAreaDiff<-20){
//			SpeedPID(motorA,motorB,encoderA,encoderB,MotorSlowDownPower,MotorSlowDownPower);
//		}
//	}
//}
//
////Using area different to contol the car, turning servo and two motor speed with area different, working fine---------------
////FutabaS3010 *ServoPt;
////AlternateMotor *MotorAPt;
////AlternateMotor *MotorBPt;
//void CarControl(FutabaS3010 *servo,AlternateMotor *motorA,AlternateMotor *motorB,DirEncoder *encoderA,DirEncoder *encoderB){
//	Uint LeftArea=0;
//	Uint RightArea=0;
//	Uint degree=700;
//	int RoadAreaDiff=0;
//	int MotorDiff=0;
//	bool IsStraightLine=false;
//	for(Uint row=CamHeight-40;row<CamHeight-20;row++){
//		for(Uint col=0;col<(CamWidth/2);col++){
//			if(!Cam2DArray[row][col]){
//				LeftArea++;
//			}
//		}
//		for(Uint col=(CamWidth/2);col<CamWidth;col++){
//			if(!Cam2DArray[row][col]){
//				RightArea++;
//			}
//		}
//	}
//	RoadAreaDiff=LeftArea-RightArea;
//	degree=degree+RoadAreaDiff*ServoConstant;
//	if(degree>900){
//		degree=900;
//	}else if(degree<430){
//		degree=430;
//	}
//	servo->SetDegree(degree);
//	MotorDiff=RoadAreaDiff*MotorConstant;
//	if(RoadAreaDiff>45||RoadAreaDiff<-45){
//		SpeedPID(motorA,motorB,encoderA,encoderB,MotorPower+MotorDiff,MotorPower-MotorDiff);
//	}else{
//		SpeedPID(motorA,motorB,encoderA,encoderB,MotorStSpeed,MotorStSpeed);
//		IsStraightLine=true;
//	}
//	if(IsStraightLine==true){
//		Uint UpperLeftArea=0;
//		Uint UpperRightArea=0;
//		for(Uint row=CamHeight-60;row<CamHeight-40;row++){
//			for(Uint col=0;col<(CamWidth/2);col++){
//				if(!Cam2DArray[row][col]){
//					UpperLeftArea++;
//				}
//			}
//			for(Uint col=(CamWidth/2);col<CamWidth;col++){
//				if(!Cam2DArray[row][col]){
//					UpperRightArea++;
//				}
//			}
//		}
//		RoadAreaDiff=UpperLeftArea-UpperRightArea;
//		if(RoadAreaDiff>20||RoadAreaDiff<-20){
//			SpeedPID(motorA,motorB,encoderA,encoderB,MotorSlowDownPower,MotorSlowDownPower);
//		}
//	}
//}

//bool BtListener(const Byte *data, const size_t size);
AlternateMotor *motorAPt, *motorBPt;
FutabaS3010 *servoPt;
LcdConsole *console;
JyMcuBt106 *BtPt;

//Main Program--------------------------------------------------------------------------------------------------------------
int smart_car(Joystick *FiveWaySwitch,St7735r *LCD,LcdConsole *Console,Ov7725 *Cam,FutabaS3010 *Servo,AlternateMotor *MotorA,AlternateMotor *MotorB,DirEncoder *EncoderA,DirEncoder *EncoderB)
{
	System::Init();

//	JyMcuBt106::Config ConfigBT;
//	ConfigBT.id=0;
//	ConfigBT.baud_rate=libbase::k60::Uart::Config::BaudRate::k115200;
//	ConfigBT.rx_isr=&BtListener;
//	ConfigBT.tx_dma_channel=0;
//	JyMcuBt106 BT(ConfigBT);
//	BtPt=&BT;

//	MotorA->SetClockwise(true);
	MotorA->SetClockwise(false);
	motorAPt=MotorA;
	//SetMotorPower(&MotorA,&EncoderA,260);
	MotorA->SetPower(250);

//	MotorB->SetClockwise(false);
	MotorB->SetClockwise(true);
	motorBPt=MotorB;
	//SetMotorPower(&MotorB,&EncoderB,260);
	MotorB->SetPower(250);

	servoPt=Servo;
	Servo->SetDegree(800);//Servo 0 degree turned

	console=Console;
	Timer::TimerInt t=0;

	Led::Config configLed1;
	configLed1.id=1;
	Led led1(configLed1);

	Cam->Start();
	while (true){
		while(t!=System::Time()){
			t = System::Time();
			if(t % 5 == 0){
//				Camera2DArrayPrintTest(LCD,Cam);
				const Byte* camPtr;
				const Byte tempInt = 49;
				const Byte* temp = &tempInt;
				camPtr = Cam->LockBuffer();
//				if(FiveWaySwitch->GetState()==Joystick::State::kDown){
//					Servo->SetDegree(Servo->GetDegree()+10);
//				}
				CameraPrint(LCD,Cam);
				moveAlgo(camPtr,LCD,Servo);
//				EdgeFinder(camPtr,LCD,Servo);
				EncoderA->Update();
				EncoderB->Update();
				if(EncoderA->GetCount()>10||EncoderB->GetCount()>10){
					MotorA->SetPower(150);
					MotorB->SetPower(150);
				}else{
					MotorA->SetPower(150);
					MotorB->SetPower(150);
				}
//				Camera2DArrayPrint(LCD);
//				switch(featureIdentify(camPtr)){
//					case 0:
//						LCD->SetRegion(Lcd::Rect(0,0,100,100));
//						LCD->FillColor(Lcd::kBlue);
//						break;
//					case 1:
//						LCD->SetRegion(Lcd::Rect(0,0,100,100));
//						LCD->FillColor(Lcd::kGreen);
//						break;
//					case 2:
//						LCD->SetRegion(Lcd::Rect(0,0,100,100));
//						LCD->FillColor(Lcd::kRed);
//						break;
//				}

				//for bluetooth
//				BT.SendBuffer(temp,1);
//				BT.SendBuffer(camPtr, Cam->GetBufferSize());



				Cam->UnlockBuffer();
				led1.Switch();
			}
		}
	}

//	while(1){
//		Cam->Start();
//		if(Cam->IsAvailable()){
//			Camera2DConverterWithCarControl(Cam.LockBuffer(),&Servo,&MotorA,&MotorB,&EncoderA,&EncoderB);
//			Camera2DConverter(Cam->LockBuffer());
//			CameraFilter();
//			PathWidthFinder(FiveWaySwitch,Console);
//			PathFinder();
//			CenterLine();
//			CarControl(Servo,MotorA,MotorB,EncoderA,EncoderB);
//			print the original image
//			CameraPrint(LCD,Cam);
//			print the 2D Array on the LCD
//			Camera2DArrayPrint(LCD);
//			Cam->UnlockBuffer();
//		}
//	}
}


//bool BtListener(const Byte *data, const size_t size){
//	switch(data[0]){
//	case 0:
//		servoPt->SetDegree(700);
//		motorAPt->SetPower(0);
//		motorBPt->SetPower(0);
//		break;
//	case 1:
//		servoPt->SetDegree(430);
//		motorAPt->SetClockwise(false);
//		motorBPt->SetClockwise(true);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 2:
//		servoPt->SetDegree(565);
//		motorAPt->SetClockwise(false);
//		motorBPt->SetClockwise(true);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 3:
//		servoPt->SetDegree(700);
//		motorAPt->SetClockwise(false);
//		motorBPt->SetClockwise(true);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 4:
//		servoPt->SetDegree(800);
//		motorAPt->SetClockwise(false);
//		motorBPt->SetClockwise(true);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 5:
//		servoPt->SetDegree(900);
//		motorAPt->SetClockwise(false);
//		motorBPt->SetClockwise(true);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 6:
//		servoPt->SetDegree(800);
//		motorAPt->SetClockwise(true);
//		motorBPt->SetClockwise(false);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 7:
//		servoPt->SetDegree(700);
//		motorAPt->SetClockwise(true);
//		motorBPt->SetClockwise(false);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case 8:
//		servoPt->SetDegree(565);
//		motorAPt->SetClockwise(true);
//		motorBPt->SetClockwise(false);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	case '1':
//		servoPt->SetDegree(565);
//		motorAPt->SetClockwise(true);
//		motorBPt->SetClockwise(false);
//		motorAPt->SetPower(300);
//		motorBPt->SetPower(300);
//		break;
//	}
//	return true;
//}
