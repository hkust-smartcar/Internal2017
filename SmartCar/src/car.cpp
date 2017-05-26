/*
 * car1.cpp
 *
 *  Created on: 2017Äê4ÔÂ28ÈÕ
 *      Author: Lee Chun Hei
 */

#include "../inc/car.h"

int servoP=20;
int servoSlowP=23;
int Kd=2000;
int turningServoP=43;
int straightRoadSpeed=340;
int slowDownSpeed=320;
int turningSpeed=280;
int brakeSpeed=220;
int frontPathError=0;
int prevPathError=0;
bool isStraightRoad=false;
int slowDownCount=0;

void car1Main(){
	carInit(car2);

	camera::feature featureNow;

	cam->Start();

	FutabaS3010::Config ConfigServo;
	ConfigServo.id=0;
	std::unique_ptr<FutabaS3010> servo(new FutabaS3010(ConfigServo));

//	while(true){
//		if(bluetooth->getBufferSpeed()==100){
//			break;
//		}
//	}

	std::unique_ptr<util::MpcDual> mpc(new util::MpcDual(motor1,motor2,encoder1,encoder2));
//	util::MpcDual mpc(motor2,motor1,encoder2,encoder1);

	CarManager::Config ConfigMgr;
	ConfigMgr.servo=std::move(servo);
	ConfigMgr.car=CarManager::Car::kNew;
	ConfigMgr.epc=std::move(mpc);
	CarManager::Init(std::move(ConfigMgr));


	int speed=straightRoadSpeed;
	int Kp=servoP;
	Timer::TimerInt t=0;
	Timer::TimerInt lastTime=0;
	Timer::TimerInt timeDiff=0;
	while (true){
		while(t!=System::Time()){
			t = System::Time();
			if(t % 10 == 0){
				camBuffer=cam->LockBuffer();
				timeDiff=t-lastTime;
//				camera::cameraPrint(4, 40);
				featureNow=camera::featureExtraction();
				switch(featureNow){
				case camera::feature::roundabout:
					speed=turningSpeed;
					pathError=pathError-40;
					Kp=turningServoP;
					break;
				case camera::feature::notDetermine:
					pathError=pathError/midPtFound-40;
					if(std::abs(pathError)<4){
						speed=straightRoadSpeed;
						Kp=servoP;
						frontPathError=midPt[camHeight-4-midPtFound];
						frontPathError=frontPathError-40;
						if(std::abs(frontPathError)>4){
							speed=slowDownSpeed;
							Kp=servoP;
						}else{
							isStraightRoad=true;
							slowDownCount=0;
							speed=straightRoadSpeed;
							Kp=servoSlowP;
						}
					}else{
						if(isStraightRoad){
							speed=brakeSpeed;
							slowDownCount++;
							if(slowDownCount>10){
								isStraightRoad=false;
							}
						}else{
							speed=turningSpeed;
						}
						speed=turningSpeed;
						Kp=turningServoP;
					}
					break;
				default:
					pathError=pathError/midPtFound-40;
					break;
				}

				int degree=midAngle-pathError*Kp-(pathError-prevPathError)/Kd;
				if(degree>leftAngle){
					degree=leftAngle;
				}else if(degree<rightAngle){
					degree=rightAngle;
				}

				CarManager::SetTargetAngle(degree);

				motor1->SetPower(speed);
				motor2->SetPower(speed);
//				CarManager::UpdateParameters();
//				CarManager::SetTargetSpeed(CarManager::MotorSide::kBoth, speed);

				prevPathError=pathError;

				cam->UnlockBuffer();
				lastTime=System::Time();
			}
		}
	}
}

void car2Main(){
//	carInit(car2);
//
//	bool roundaboutTurnLeft=false;
//	int average=0;
//	camera::feature featureNow;
//
//	cam->Start();
//
//	servo->SetDegree(midAngle);
//
//	while(true){
//		if(joystick->GetState()==Joystick::State::kSelect){
//			bluetooth->sendSpeed(100);
//			System::DelayMs(3000);
//			break;
//		}
//	}
//
//	Timer::TimerInt t=0;
//	while (true){
//		while(t!=System::Time()){
//			t = System::Time();
//			if(t % 10 == 0){
//				camBuffer=cam->LockBuffer();
//
//				featureNow=camera::pathFinding(roundaboutTurnLeft);
//
//				for(int y=camHeight-5;y>camHeight-5-midPtFound;y--){
//					average+=midPt[y];
//				}
//				average=average/midPtFound;
//
////				switch(featureNow){
////				case camera::feature::crossRoad:
////					degree=midAngle-(average-40)*servoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					motor1->SetPower(straightRoadSpeed);
////					motor2->SetPower(straightRoadSpeed);
////					break;
////
////				case camera::feature::roundabout:
////					degree=midAngle-(midPt[0]-40)*turningServoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					if(degree>midAngle){
////						motor1->SetPower(turningSpeedLow);
////						motor2->SetPower(turningSpeedHigh);
////					}else{
////						motor1->SetPower(turningSpeedHigh);
////						motor2->SetPower(turningSpeedLow);
////					}
////					break;
////
////				case camera::feature::inRoundabout:
////					motor1->SetPower(0);
////					motor2->SetPower(0);
////					while(true){
////						if(bluetooth->getBufferSpeed()==200){
////							System::DelayMs(200);
////							break;
////						}
////					}
////					degree=midAngle-(average-40)*turningServoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					if(degree>midAngle){
////						motor1->SetPower(turningSpeedLow);
////						motor2->SetPower(turningSpeedHigh);
////					}else{
////						motor1->SetPower(turningSpeedHigh);
////						motor2->SetPower(turningSpeedLow);
////					}
////					break;
////
////				case camera::feature::exitRoundabout:
////					degree=midAngle-(midPt[0]-40)*turningServoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					if(degree>midAngle){
////						motor1->SetPower(turningSpeedLow);
////						motor2->SetPower(turningSpeedHigh);
////					}else{
////						motor1->SetPower(turningSpeedHigh);
////						motor2->SetPower(turningSpeedLow);
////					}
////					bluetooth->sendSpeed(200);
////					break;
////
////				case camera::feature::notDetermine:
////					degree=midAngle-(average-40)*servoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					if(degree>midAngle){
////						motor1->SetPower(turningSpeedLow+30);
////						motor2->SetPower(turningSpeedHigh+30);
////					}else{
////						motor1->SetPower(turningSpeedHigh+30);
////						motor2->SetPower(turningSpeedLow+30);
////					}
////					break;
////
////				case camera::feature::turnLeft:
////					degree=midAngle-(average-40)*turningServoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					motor1->SetPower(turningSpeedHigh);
////					motor2->SetPower(turningSpeedLow);
////					break;
////
////				case camera::feature::turnRight:
////					degree=midAngle-(average-40)*turningServoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					motor1->SetPower(turningSpeedLow+30);
////					motor2->SetPower(turningSpeedHigh+30);
////					break;
////
////				default:
////					degree=midAngle-(average-40)*servoP;
////					if(degree>leftAngle){
////						degree=leftAngle;
////					}else if(degree<rightAngle){
////						degree=rightAngle;
////					}
////					servo->SetDegree(degree);
////					motor1->SetPower(turningSpeedLow);
////					motor2->SetPower(turningSpeedLow);
////					break;
////				}
//
//				camera::cameraPrint(4, 40);
////				lcd->SetRegion(Lcd::Rect(4,40,80,60));
////				lcd->FillBits(0x001F,0xFFFF,cam->LockBuffer(),Cam.GetBufferSize()*8);
//				cam->UnlockBuffer();
//			}
//		}
//	}
}


