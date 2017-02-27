/*
 * main.cpp
 *
 * Author: Leslie
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/camera.h"
#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

#include <functional>

//Looper Header File----------------------------------------------------------------------------------------------
//#include <libutil/looper.h>

//led Header File-----------------------------------------------------------------------------------------------------------
//#include <libsc/led.h>

//LCD Header File-----------------------------------------------------------------------------------------------------------
//#include<libsc/st7735r.h>
//#include<libsc/lcd_console.h>

//Camera Header File--------------------------------------------------------------------------------------------------------
//#include<libsc/k60/ov7725.h>

//namespace-----------------------------------------------------------------------------------------------------------------
using namespace libsc;
using namespace libsc::k60;
using namespace libbase::k60;
using namespace libutil;
using namespace std;

//Global variable-----------------------------------------------------------------------------------------------------------
const Uint CamHeight=120;
const Uint CamWidth=160;
const Uint MotorPower=25;
const Uint MotorStSpeed=25;
const Uint MotorSlowDownPower=25;
#define ServoConstant 1.2;
#define MotorConstant 0.001;

//Change Camera Buffer from 1D array to 2D array----------------------------------------------------------------------------
//row is row number
//col is column number
//CamByte is the Byte CameraBuffer
bool Cam2DArray[CamHeight][CamWidth];
void Camera2DConverter(const Byte* CameraBuffer){
	Byte CamByte;
	for(Uint row=0;row<CamHeight;row++){
		for(Uint col=0;col<CamWidth;col+=8){
			CamByte=CameraBuffer[row*CamWidth/8+col/8];
			Cam2DArray[row][col]=CamByte&0x80;
			Cam2DArray[row][col+1]=CamByte&0x40;
			Cam2DArray[row][col+2]=CamByte&0x20;
			Cam2DArray[row][col+3]=CamByte&0x10;
			Cam2DArray[row][col+4]=CamByte&0x08;
			Cam2DArray[row][col+5]=CamByte&0x04;
			Cam2DArray[row][col+6]=CamByte&0x02;
			Cam2DArray[row][col+7]=CamByte&0x01;
		}
	}
}

//Image filter (Median Filter)----------------------------------------------------------------------------------------------
//row is row number
//col is column number
void CameraFilter(){
	for(Uint row=1;row<(CamHeight-1);row++){
		for(Uint col=1;col<(CamWidth-1);col++){
			Uint temp=0;
			temp=Cam2DArray[row-1][col-1]+Cam2DArray[row-1][col]+Cam2DArray[row-1][col+1]
			     +Cam2DArray[row][col-1]+Cam2DArray[row][col]+Cam2DArray[row][col+1]
				 +Cam2DArray[row+1][col-1]+Cam2DArray[row+1][col]+Cam2DArray[row+1][col+1];
			if(temp>4){
				Cam2DArray[row][col]=1;
			}else{
				Cam2DArray[row][col]=0;
			}
		}
	}
}

void camera()
{
////LCD Configuration---------------------------------------------------------------------------------------------------------
//	St7735r::Config ConfigLCD;
//	ConfigLCD.is_revert=true;
//	ConfigLCD.is_bgr=false;
//	ConfigLCD.fps=60;
//	St7735r LCD(ConfigLCD);
//
//	LcdConsole::Config ConfigConsole;
//	ConfigConsole.lcd=&LCD;
//	ConfigConsole.region=Lcd::Rect(0,0,128,80);
//	LcdConsole Console(ConfigConsole);
//
////Camera Configuration------------------------------------------------------------------------------------------------------
//	Ov7725::Config ConfigCam;
//	ConfigCam.id=0;
//	ConfigCam.h=CamHeight;
//	ConfigCam.w=CamWidth;
//	Ov7725 Cam(ConfigCam);
//
//	while(1){
//	}
}

//Center Line finding function------------------------------------------------------------------------------------
void CenterLine(){
	int MidPt=CamWidth/2;
	int LeftEdge=0;
	int RightEdge=0;
	int RowClear=0;
	bool PrevRowClear=false;
	for(int y=CamHeight-2;y>-1;y--){
		bool LeftClear=false;
		bool RightClear=false;
		if(Cam2DArray[y][MidPt]==1){
			break;
		}
		for(int x=MidPt;x>-1;x--){
			if(Cam2DArray[y][x]==1){
				LeftEdge=x+1;
				break;
			}
			if(x==0){
				LeftEdge=0;
				LeftClear=true;
				break;
			}
		}
		for(int x=MidPt;x<CamWidth;x++){
			if(Cam2DArray[y][x]==1){
				RightEdge=x-1;
				break;
			}
			if(x==CamWidth-1){
				RightEdge=CamWidth-1;
				RightClear=true;
				break;
			}
		}
		MidPt=(RightEdge+LeftEdge)/2;
		Cam2DArray[y][MidPt]=2;
		if(LeftClear!=true||RightClear!=true){
			PrevRowClear=false;
		}else if(LeftClear==true&&RightClear==true&&PrevRowClear==true){
			PrevRowClear=true;
			RowClear++;
		}else if(LeftClear==true&&RightClear==true){
			PrevRowClear=true;
		}
		if(RowClear>1&&LeftClear==true&&RightClear==true){
			Cam2DArray[y][MidPt]=3;
		}
	}
}

void PathWidthFinder(Joystick *FiveWaySwitch,LcdConsole *console){
	int error[60];
	int MidPt=CamWidth/2;
	int LeftEdge=0;
	int RightEdge=0;
	for(int y=CamHeight-1;y>=0;y--){
		for(int x=MidPt;x>-1;x--){
			if(Cam2DArray[y][x]==1){
				LeftEdge=x+1;
				break;
			}
			if(x==0){
				LeftEdge=0;
				break;
			}
		}
		for(int x=MidPt;x<CamWidth;x++){
			if(Cam2DArray[y][x]==1){
				RightEdge=x-1;
				break;
			}
			if(x==CamWidth-1){
				RightEdge=CamWidth-1;
				break;
			}
		}
		MidPt=(RightEdge+LeftEdge)/2;
		Cam2DArray[y][MidPt]=2;
		error[y]=(CamWidth/2)-LeftEdge;
		Cam2DArray[y][CamWidth/2]=3;
	}
	if(FiveWaySwitch->GetState()==Joystick::State::kSelect){
		for(int i=0;i<60;i++){
			char buff[20];
			sprintf(buff,"%d",error[i]);
			console->SetCursorRow(0);
			console->WriteString(buff);
			System::DelayS(1);
		}
	}
}

void PathFinder(){
	int error[60]={2,2,2,3,4,6,6,7,7,8,9,10,10,11,12,13,14,15,16,16,17,18,18,19,20,21,21,21,22,22,23,24,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,40,40,40,40,40};
	int MidPt=CamWidth/2;
	int LeftEdge=0;
	int RightEdge=0;
	int RowClear=0;
	bool PrevRowClear=false;
	for(int y=CamHeight-3;y>-1;y--){
		bool LeftClear=false;
		bool RightClear=false;
		if(Cam2DArray[y][MidPt]==1){
			break;
		}
		for(int x=MidPt;x>-1;x--){
			if(Cam2DArray[y][x]==1){
				LeftEdge=x+1;
				break;
			}
			if(x==0){
				LeftEdge=0;
				LeftClear=true;
				break;
			}
		}
		for(int x=MidPt;x<CamWidth;x++){
			if(Cam2DArray[y][x]==1){
				RightEdge=x-1;
				break;
			}
			if(x==CamWidth-1){
				RightEdge=CamWidth-1;
				RightClear=true;
				break;
			}
		}
		MidPt=(RightEdge+LeftEdge)/2;
		if(LeftClear!=RightClear){
			if(LeftClear==true){
				MidPt=RightEdge-error[y];
			}else{
				MidPt=LeftEdge+error[y];
			}
			if(MidPt<0||MidPt>CamWidth){
				continue;
			}
		}
		Cam2DArray[y][MidPt]=2;
		if(LeftClear!=true||RightClear!=true){
			PrevRowClear=false;
		}else if(LeftClear==true&&RightClear==true&&PrevRowClear==true){
			PrevRowClear=true;
			RowClear++;
		}else if(LeftClear==true&&RightClear==true){
			PrevRowClear=true;
		}
		if(RowClear>1&&LeftClear==true&&RightClear==true){
			Cam2DArray[y][MidPt]=3;
		}
	}
}

void EdgeFinder(){
	for(int y=CamHeight-1;y>-1;y--){
		for(int x=1;x<CamWidth;x++){
			if(Cam2DArray[y][x]!=Cam2DArray[y][x-1]&&Cam2DArray[y][x-1]!=2&&Cam2DArray[y][x]!=2){
				Cam2DArray[y][x]=2;
			}
		}
	}
	for(int x=0;x<CamWidth;x++){
		for(int y=CamHeight-1;y>-1;y--){
			if(Cam2DArray[y][x]!=Cam2DArray[y+1][x]&&Cam2DArray[y+1][x]!=2&&Cam2DArray[y][x]!=2){
				Cam2DArray[y][x]=2;
			}
		}
	}
}

void EdgeSlope(){
	int LeftEdge[10][2];
	int RightEdge[10][2];
	for(int y=CamHeight-2;y>-1;y--){
		int EdgeCount=0;
		for(int x=0;x<CamWidth;x++){
			if(Cam2DArray[y][x]==2&&y!=0&&x!=0&&y!=CamHeight-1&&x!=CamWidth-1){
				EdgeCount++;
				if(EdgeCount==1){
					LeftEdge[0][0]=x;
					LeftEdge[0][1]=y;
				}else{
					RightEdge[0][0]=x;
					RightEdge[0][1]=y;
				}
			}
			if(EdgeCount==2){
				break;
			}
		}
		if(EdgeCount==2){
			break;
		}
	}
	vector <float> LeftEdgeSlope;
	while(1){
		int direction=8; /* 1 2 3
							4 5 6
		 	 	 	 	 	7 8 9 */
		int EdgeCount=1;
		while(EdgeCount<9){
			int LeftEdgeX=LeftEdge[EdgeCount-1][0];
			int LeftEdgeY=LeftEdge[EdgeCount-1][1];
			if(Cam2DArray[LeftEdgeY-1][LeftEdgeX]==2&&direction!=2){
				LeftEdge[EdgeCount][0]=LeftEdgeX;
				LeftEdge[EdgeCount][1]=LeftEdgeY-1;
				direction=8;
			}else if(Cam2DArray[LeftEdgeY][LeftEdgeX-1]==2&&direction!=4){
				LeftEdge[EdgeCount][0]=LeftEdgeX-1;
				LeftEdge[EdgeCount][1]=LeftEdgeY;
				direction=6;
			}else if(Cam2DArray[LeftEdgeY][LeftEdgeX+1]==2&&direction!=6){
				LeftEdge[EdgeCount][0]=LeftEdgeX+1;
				LeftEdge[EdgeCount][1]=LeftEdgeY;
				direction=4;
			}else if(Cam2DArray[LeftEdgeY+1][LeftEdgeX]==2&&direction!=8){
				LeftEdge[EdgeCount][0]=LeftEdgeX;
				LeftEdge[EdgeCount][1]=LeftEdgeY+1;
				direction=2;
			}else if(Cam2DArray[LeftEdgeY-1][LeftEdgeX-1]==2&&direction!=1){
				LeftEdge[EdgeCount][0]=LeftEdgeX-1;
				LeftEdge[EdgeCount][1]=LeftEdgeY-1;
				direction=9;
			}else if(Cam2DArray[LeftEdgeY-1][LeftEdgeX+1]==2&&direction!=3){
				LeftEdge[EdgeCount][0]=LeftEdgeX+1;
				LeftEdge[EdgeCount][1]=LeftEdgeY-1;
				direction=7;
			}else if(Cam2DArray[LeftEdgeY+1][LeftEdgeX-1]==2&&direction!=7){
				LeftEdge[EdgeCount][0]=LeftEdgeX-1;
				LeftEdge[EdgeCount][1]=LeftEdgeY+1;
				direction=3;
			}else if(Cam2DArray[LeftEdgeY+1][LeftEdgeX+1]==2&&direction!=9){
				LeftEdge[EdgeCount][0]=LeftEdgeX+1;
				LeftEdge[EdgeCount][1]=LeftEdgeY+1;
				direction=1;
			}
			if(LeftEdge[EdgeCount][0]==0||LeftEdge[EdgeCount][0]==CamWidth-1||LeftEdge[EdgeCount][1]==0||LeftEdge[EdgeCount][1]==CamHeight-1){
				break;
			}
			EdgeCount++;
		}
		LeftEdgeSlope.push_back((LeftEdge[EdgeCount][1]-LeftEdge[0][1])/(LeftEdge[EdgeCount][0]-LeftEdge[0][0]));
		if(LeftEdge[EdgeCount][0]==0||LeftEdge[EdgeCount][0]==CamWidth-1||LeftEdge[EdgeCount][1]==0||LeftEdge[EdgeCount][1]==CamHeight-1){
			break;
		}
	}

	vector <float> RightEdgeSlope;
	while(1){
		int direction=8; /* 1 2 3
							4 5 6
		 	 	 	 	 	7 8 9 */
		int EdgeCount=1;
		while(EdgeCount<9){
			int RightEdgeX=RightEdge[EdgeCount-1][0];
			int RightEdgeY=RightEdge[EdgeCount-1][1];
			if(Cam2DArray[RightEdgeY-1][RightEdgeX]==2&&direction!=2){
				RightEdge[EdgeCount][0]=RightEdgeX;
				RightEdge[EdgeCount][1]=RightEdgeY-1;
				direction=8;
			}else if(Cam2DArray[RightEdgeY][RightEdgeX-1]==2&&direction!=4){
				RightEdge[EdgeCount][0]=RightEdgeX-1;
				RightEdge[EdgeCount][1]=RightEdgeY;
				direction=6;
			}else if(Cam2DArray[RightEdgeY][RightEdgeX+1]==2&&direction!=6){
				RightEdge[EdgeCount][0]=RightEdgeX+1;
				RightEdge[EdgeCount][1]=RightEdgeY;
				direction=4;
			}else if(Cam2DArray[RightEdgeY+1][RightEdgeX]==2&&direction!=8){
				RightEdge[EdgeCount][0]=RightEdgeX;
				RightEdge[EdgeCount][1]=RightEdgeY+1;
				direction=2;
			}else if(Cam2DArray[RightEdgeY-1][RightEdgeX-1]==2&&direction!=1){
				RightEdge[EdgeCount][0]=RightEdgeX-1;
				RightEdge[EdgeCount][1]=RightEdgeY-1;
				direction=9;
			}else if(Cam2DArray[RightEdgeY-1][RightEdgeX+1]==2&&direction!=3){
				RightEdge[EdgeCount][0]=RightEdgeX+1;
				RightEdge[EdgeCount][1]=RightEdgeY-1;
				direction=7;
			}else if(Cam2DArray[RightEdgeY+1][RightEdgeX-1]==2&&direction!=7){
				RightEdge[EdgeCount][0]=RightEdgeX-1;
				RightEdge[EdgeCount][1]=RightEdgeY+1;
				direction=3;
			}else if(Cam2DArray[RightEdgeY+1][RightEdgeX+1]==2&&direction!=9){
				RightEdge[EdgeCount][0]=RightEdgeX+1;
				RightEdge[EdgeCount][1]=RightEdgeY+1;
				direction=1;
			}
			if(RightEdge[EdgeCount][0]==0||RightEdge[EdgeCount][0]==CamWidth-1||RightEdge[EdgeCount][1]==0||RightEdge[EdgeCount][1]==CamHeight-1){
				break;
			}
			EdgeCount++;
		}
		RightEdgeSlope.push_back((RightEdge[EdgeCount][1]-RightEdge[0][1])/(RightEdge[EdgeCount][0]-RightEdge[0][0]));
		if(RightEdge[EdgeCount][0]==0||RightEdge[EdgeCount][0]==CamWidth-1||RightEdge[EdgeCount][1]==0||RightEdge[EdgeCount][1]==CamHeight-1){
			break;
		}
	}

	float LeftEdgeError=0;

}

void EdgeTurnDetermine(){
	int LeftEdge[2];
	int RightEdge[2];
	for(int y=CamHeight-2;y>-1;y--){
		int EdgeCount=0;
		for(int x=0;x<CamWidth;x++){
			if(Cam2DArray[y][x]==2&&y!=0&&x!=0&&y!=CamHeight-1&&x!=CamWidth-1){
				EdgeCount++;
				if(EdgeCount==1){
					LeftEdge[0]=x;
					LeftEdge[1]=y;
				}else{
					RightEdge[0]=x;
					RightEdge[1]=y;
				}
			}
			if(EdgeCount==2){
				break;
			}
		}
		if(EdgeCount==2){
			break;
		}
	}
	while(1){
		int direction=8; /* 1 2 3
							4 5 6
		 	 	 	 	 	7 8 9 */
		enum turn{negative,neutral,positive};
		turn X=positive;
		turn Y=negative;
		turn LeftEdgeTurn=neutral;
		turn RightEdgeTurn=neutral;
		bool checkY=false;
		int EdgeCount=1;
		while(1){
			int LeftEdgeX=LeftEdge[0];
			int LeftEdgeY=LeftEdge[1];
			turn PrevX=X;
			turn PrevY=Y;
			if(Cam2DArray[LeftEdgeY-1][LeftEdgeX]==2&&direction!=2){
				LeftEdge[0]=LeftEdgeX;
				LeftEdge[1]=LeftEdgeY-1;
				Y=negative;
				direction=8;
			}else if(Cam2DArray[LeftEdgeY][LeftEdgeX-1]==2&&direction!=4){
				LeftEdge[0]=LeftEdgeX-1;
				LeftEdge[1]=LeftEdgeY;
				X=negative;
				Y=neutral;
				direction=6;
			}else if(Cam2DArray[LeftEdgeY][LeftEdgeX+1]==2&&direction!=6){
				LeftEdge[0]=LeftEdgeX+1;
				LeftEdge[1]=LeftEdgeY;
				X=positive;
				Y=neutral;
				direction=4;
			}else if(Cam2DArray[LeftEdgeY+1][LeftEdgeX]==2&&direction!=8){
				LeftEdge[0]=LeftEdgeX;
				LeftEdge[1]=LeftEdgeY+1;
				Y=positive;
				direction=2;
			}else if(Cam2DArray[LeftEdgeY-1][LeftEdgeX-1]==2&&direction!=1){
				LeftEdge[0]=LeftEdgeX-1;
				LeftEdge[1]=LeftEdgeY-1;
				X=negative;
				Y=negative;
				direction=9;
			}else if(Cam2DArray[LeftEdgeY-1][LeftEdgeX+1]==2&&direction!=3){
				LeftEdge[0]=LeftEdgeX+1;
				LeftEdge[1]=LeftEdgeY-1;
				X=positive;
				Y=negative;
				direction=7;
			}else if(Cam2DArray[LeftEdgeY+1][LeftEdgeX-1]==2&&direction!=7){
				LeftEdge[0]=LeftEdgeX-1;
				LeftEdge[1]=LeftEdgeY+1;
				X=negative;
				Y=positive;
				direction=3;
			}else if(Cam2DArray[LeftEdgeY+1][LeftEdgeX+1]==2&&direction!=9){
				LeftEdge[0]=LeftEdgeX+1;
				LeftEdge[1]=LeftEdgeY+1;
				X=positive;
				Y=positive;
				direction=1;
			}
			if(X!=PrevX){
				if(checkY==false){
					checkY=true;
				}else{
					checkY=false;
				}
			}
			if(checkY==true){
				if(Y!=PrevY){
					LeftEdgeTurn=X;
					checkY=false;
					break;
				}
			}
			if(LeftEdge[0]==0||LeftEdge[0]==CamWidth-1||LeftEdge[1]==0||LeftEdge[1]==CamHeight-1){
				break;
			}
			EdgeCount++;
		}
	}
}

void CameraPrint(St7735r *lcd,Ov7725 *Cam){
	lcd->SetRegion(Lcd::Rect(1,1,80,60));
	lcd->FillBits(0x001F,0xFFFF,Cam->LockBuffer(),Cam->GetBufferSize()*8);
}

void Camera2DArrayPrint(St7735r *lcd){
	for(Uint y=0;y<CamHeight;y++){
		for(Uint x=0;x<CamWidth;x++){
			lcd->SetRegion(Lcd::Rect(x,y,1,1));
			if(Cam2DArray[y][x]==0){
				lcd->FillColor(0xFFFF);
			}else if(Cam2DArray[y][x]==1){
				lcd->FillColor(0x001F);
			}else if(Cam2DArray[y][x]==2){
				lcd->FillColor(0xF800);
			}else if(Cam2DArray[y][x]==3){
				lcd->FillColor(0xFFE0);
			}
		}
	}
}
