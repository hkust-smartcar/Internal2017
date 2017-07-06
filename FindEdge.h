///*
// * FindEdge.h
// *
// *  Created on: Apr 19, 2017
// *      Author: lzhangbj
// */

#ifndef SRC_CAMERAHEADER_FINDEDGE_H_
#define SRC_CAMERAHEADER_FINDEDGE_H_

#include <libsc/k60/uart_device.h>
#include <libbase/k60/uart.h>
#include <libsc/k60/jy_mcu_bt_106.h>
#include <libsc/lcd_typewriter.h>
#include "CameraHeader/img.h"

#define  BLACK					1
#define	WHITE					0

int distance[60] ;

bool GetBit(const Byte* b, uint8_t x, uint8_t y){
	return ((b[y*10 + x/8]>>(7 - x%8))&1);
}

using namespace libsc;
using namespace libsc::k60;
JyMcuBt106*  BluetoothPtr;
JyMcuBt106*  bluetooth;

St7735r* TftPtr;
LcdTypewriter* TyperPtr;

char buffer[100];

bool carbegin[2] = {false,false};
bool carinterval = false;
int carindex = 0;

int RoundaboutCount = 1;
bool RoundaboutTag = false;
bool RoundaboutPreTag = false;
//bool FindInCor = false;
//bool InIncor = false;
//bool OutInCor = false;
//bool FindOutCor = false;

int32_t OutRoundEncoderValue = 0;
int32_t InRoundEncoderValue  = 0;

bool InRoundStart = false;
bool InRoundEnd = false;
bool OutRoundStart = false;
bool OutRoundEnd = true;

float slope_p = 1;//30//0.2
float pos_p = 0.4;//3



int8_t Width[20]={79,78,77,76,74,72,69,66,64,64,60,60,62,63,64,65,66,67,68,70};
int time = 0;
float Diff = 0;
float posDiff = 0;
float slopeDiff = 0;

#define OUTSIDECRITERION	2
#define INSIDECRITERION		3
#define CORNERCRITERION		3
#define OFFSET				20

bool use_bt = 0;
bool use_TFT = 0;


//赛道状态
enum TrackState{
	Crossroad,
	Crossroad2,
	Crossroad3,
	Roundabout,
	Go,
	Straight,
	TurnLeft,
	TurnRight,
	LeftObstacle,
	RightObstacle
};



//点集    version   2
struct Point{
	float x ;
	float y ;
	float slope;
	bool operator == (const Point& point);
	Point():x(0),y(0),slope(0){}
	Point(int32_t x, int32_t y, float slope = 0 ): x(x),y(y),slope(slope){}
};


bool Point::operator == (const Point& point){
	if(this->x == point.x && this->y == point.y)
		return true;
	return false;
}


//Find Edge  version 2
void  FindEdge(const Byte*  Bin,  Point* LeftEdge,  Point* RightEdge,  uint8_t& LeftEdgeNumber,  uint8_t& RightEdgeNumber){
//从中间开始搜索起点
	bool LeftStart = false;
	bool RightStart = false;

	bool leftblack[3] = {};
	bool rightblack[3] = {};

	/*
	 * 从底线搜索第一个边界点
	 * 从右往左搜索   黑——白 为第一个左边界点
	 * 从左往右搜索	白——黑 为第一个右边界点
	 * */

	for(int i = 6; i <= 78 ; i ++){
		if(GetBit(Bin, 79-i, 59) == WHITE && GetBit(Bin, 78-i,59) == BLACK && !LeftStart){
			LeftEdge[0] = Point(78-i,59);
			LeftStart = true;
			leftblack[0] = true;
		}
		if(GetBit(Bin,i, 59) == WHITE && GetBit(Bin, i+1,59) == BLACK && !RightStart){
			RightEdge[0] = Point(i+1,59);
			RightStart = true;
			rightblack[0]=true;
		}
		if(leftblack[0] && GetBit(Bin,79 - i,59) == BLACK && GetBit(Bin,78-i,59) == WHITE ){
			leftblack[1] = true;
		}
		if(leftblack[1] && GetBit(Bin,79 - i,59) == WHITE && GetBit(Bin,78-i,59) == BLACK && i <= 39){
			leftblack[2] = true;
		}
		if(rightblack[0] && GetBit(Bin,i,59) == BLACK && GetBit(Bin,i+1,59) == WHITE ){
			rightblack[1] = true;
		}
		if(rightblack[1] && GetBit(Bin,i,59) == WHITE && GetBit(Bin,i+1,59) == BLACK &&  i <= 39){
			rightblack[2] = true;
		}
	}

	if(leftblack[2] && rightblack[2] && !carbegin[0]){
		carbegin[0] = true;
		time = System::Time();
	}
	if(carbegin[0] && !(leftblack[2] && rightblack[2] ))
		carinterval = true;
	if(leftblack[2] && rightblack[2] && carinterval && System::Time() - time > 5000)
		carbegin[1] = true;
	if(leftblack[2] && rightblack[2]){
		LeftStart = RightStart = false;
		for(int i = 0 ; i < 79 ; i ++){
			if(GetBit(Bin, i, 59) == BLACK && GetBit(Bin, i+1,59) == WHITE && !LeftStart){
				LeftEdge[0] = Point(i,59);
				LeftStart = true;
			}
			if(GetBit(Bin,79-i, 59) == BLACK && GetBit(Bin, 78 - i,59) == WHITE && !RightStart){
				RightEdge[0] = Point(79-i,59);
				RightStart = true;
			}
		}
	}
	//若未搜到，定义左右极点为边界
	if(!LeftStart)
		LeftEdge[0] = Point(0,59,0);
	if(!RightStart)
		RightEdge[0] = Point(79,59,0);

	//从起点开始向上每三行搜寻一次边界
	uint8_t Order ;

	for( Order = 0 ; Order < 19 ; Order++){//标定19个边界			0——56行
		//寻找左边界坐标

		//若底线边界点为右极点， 退出搜索左边界
		if(LeftEdge[Order].x + 1 >79){
			LeftEdgeNumber  = Order;
			break;
		}

		else {
			int i = 1;
			if(GetBit(Bin, LeftEdge[Order].x + 1, LeftEdge[Order].y - 3) == BLACK){
				while(LeftEdge[Order].x + 1 + i <= 79 &&(GetBit(Bin, LeftEdge[Order].x + 1 + i, LeftEdge[Order].y - 3) == BLACK)){
					i++;
				}
				i--;
				LeftEdge[Order+1] = Point( LeftEdge[Order].x + 1 + i, LeftEdge[Order].y-3);
				LeftEdge[Order].slope  = 1+i;
			}
			else{
				while(LeftEdge[Order].x + 1 - i  >= 0 &&(GetBit(Bin, LeftEdge[Order].x + 1 - i, LeftEdge[Order].y - 3) == WHITE)){
					i++;
				}
				LeftEdge[Order+1] = Point( LeftEdge[Order].x + 1 - i, LeftEdge[Order].y-3);
				LeftEdge[Order].slope  = 1-i;
			}
		}
	}

	if(Order == 19)
		LeftEdgeNumber  = 20;

	for( Order = 0 ; Order < 19 ; Order++){	//标定19个边界
	// 寻找右边界坐标
		if(RightEdge[Order].x - 1 <0){
			RightEdgeNumber = Order;
			break;
		}
		else {
				int i = 1;
				if(GetBit(Bin, RightEdge[Order].x -1, RightEdge[Order].y - 3) == BLACK){
					while(RightEdge[Order].x - 1 - i >= 0  &&  (GetBit(Bin, RightEdge[Order].x -1- i, RightEdge[Order].y - 3) == BLACK)){
						i++;
					}
					i--;
					RightEdge[Order+1] = Point(RightEdge[Order].x-1- i, RightEdge[Order].y-3);
					RightEdge[Order].slope = -1-i;
				}
				else{
					while(RightEdge[Order].x - 1 + i <= 79  &&  (GetBit(Bin, RightEdge[Order].x - 1 + i, RightEdge[Order].y - 3) == WHITE)){
						i++;
					}
					RightEdge[Order+1] = Point(RightEdge[Order].x-1 + i, RightEdge[Order].y-3);
					RightEdge[Order].slope = -1+i;
				}
		}
	}
	if(Order== 19)
		RightEdgeNumber = 20;


	return;
}

TrackState ModifyEdge(const Byte* Bin , Point* LeftEdge,  Point* RightEdge, Point*  MidPoint, const uint8_t& LeftNumber, const uint8_t& RightNumber, uint8_t& MidPointNumber, uint8_t & LeftCornerNum, uint8_t& RightCornerNum, Point* LeftCorner[], Point* RightCorner[]){
	LeftCorner[0] = LeftCorner[1] = NULL;
	RightCorner[0] = RightCorner[1] = NULL;
	int8_t LeftCornerOrder[2] = {0};
	int8_t RightCornerOrder[2] = {0};


	int8_t MaxOrder = (LeftNumber > RightNumber)?((RightNumber  >20)?20:RightNumber):((LeftNumber > 20)?20:LeftNumber);
	MidPointNumber = MaxOrder;
	for(int i = 0 ; i < MidPointNumber ; i ++){
		if(LeftEdge[i].x!= 0 && RightEdge[i].x!=79)
		MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
		else if(LeftEdge[i].x == 0 && RightEdge[i].x == 79)
			MidPoint[i].x = 39.5;
		else if(LeftEdge[i].x == 0){
			if(i > 5)
				MidPoint[i].x = RightEdge[i].x - Width[i]/2;
			else
				MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
		}
		else
			if(i > 5)
				MidPoint[i].x = LeftEdge[i].x + Width[i]/2;
			else
				MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
		MidPoint[i].y = LeftEdge[i].y;
	}


	if(MaxOrder < 3){
		if(use_TFT){
			TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
			TyperPtr->WriteString("Track\tgo");
		}
		if(use_bt)
			BluetoothPtr->SendStr("go\t");
		return Go;
	}

	int8_t Order = 1;
	bool LeftCorner0Found = false;
	bool LeftCorner1Found = false;
	bool RightCorner0Found = false;
	bool RightCorner1Found = false;

	while(Order + 1 < MaxOrder){
		//判断是否是正常十字
			//判断左边

		if(LeftEdge[Order].slope <= LeftEdge[Order-1].slope - CORNERCRITERION){//<
			if(!LeftCorner0Found && LeftEdge[Order].slope <= - 1){//-2
				LeftCorner[0] = & LeftEdge[Order-1];
				LeftCornerOrder[0] = Order-1;
				LeftCorner0Found = true;
			}
			else if(!LeftCorner1Found && LeftEdge[Order].slope <= 5 && LeftEdge[Order].slope >= 0 ) {
				LeftCorner[1] = & LeftEdge[Order+1];
				LeftCornerOrder[1] = Order+1;
				LeftCorner1Found = true;
			}
		}

			//判断右边
		if(RightEdge[Order].slope >= RightEdge[Order-1].slope + CORNERCRITERION){
			if(!RightCorner0Found && RightEdge[Order].slope >= 1 && RightEdge[Order-1].slope<=0){
				RightCorner[0] = & RightEdge[Order-1];
				RightCornerOrder[0] = Order-1;
				RightCorner0Found = true;
			}
			else if(!RightCorner1Found && RightEdge[Order].slope >= -5 && RightEdge[Order].slope <=0) {
				RightCorner[1] = & RightEdge[Order+1];
				RightCornerOrder[1] = Order+1;
				RightCorner1Found = true;
			}
		}
		Order++;
	}


	if(LeftCorner[0]->y < 20){
		LeftCorner[0] = NULL;
		LeftCornerOrder[0] = 0;
	}
	if(RightCorner[0]->y < 20){
		RightCorner[0] = NULL;
		RightCornerOrder[0] = 0;
	}
	if(LeftCorner[0] && LeftCorner[1]){
		if(LeftCorner[0]->y < LeftCorner[1]->y)
			LeftCorner[1] = 0;
	}
	if(RightCorner[0] && RightCorner[1]){
		if(RightCorner[0]->y < RightCorner[1]->y)
			RightCorner[1] = 0;
	}
	LeftCornerNum = LeftCornerOrder[1];
	RightCornerNum = RightCornerOrder[1];




	if(LeftCorner[1] && RightCorner[1] && LeftCorner[1]->x < RightCorner[1]->x - 5 && LeftCorner[1]->y - RightCorner[1]->y <15 &&  LeftCorner[1]->y - RightCorner[1]->y > -15){ // 用上面两个边界点判断十字并补线
		int8_t Base_x;
		int8_t Base_y;
		bool FindBlack = false;
		bool FindWhite = false;
		int8_t i=1;

		if(  !RoundaboutTag &&LeftCorner[0] && RightCorner[0] && LeftCorner[0]->x < LeftCorner[1]->x + 5 && RightCorner[0]->x + 5 > RightCorner[1]->x){
			Base_x = (LeftCorner[0]->x + RightCorner[0]->x)/2;
			Base_y = (LeftCorner[0]->y > RightCorner[0]->y)?LeftCorner[0]->y:RightCorner[0]->y;

			while(!FindWhite && i < Base_y-(LeftCorner[1]->y+RightCorner[1]->y)/2.0){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
				}
				else if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
					FindWhite = true;
				}
				i++;
			}

			if(!FindWhite){
				int left = (LeftCorner[0]->y - LeftCorner[1]->y == 0)?1:(LeftCorner[0]->y - LeftCorner[1]->y);
				int right = (RightCorner[0]->y - RightCorner[1]->y == 0)?1:(RightCorner[0]->y - RightCorner[1]->y);
				LeftCorner[0]->slope  = static_cast<float>((LeftCorner[1]->x - LeftCorner[0]->x)) / left * 3;
				RightCorner[0]->slope = static_cast<float>((RightCorner[1]->x - RightCorner[0]->x)) / right * 3;
				for(int8_t  i = LeftCornerOrder[0]+1 ; i<LeftCornerOrder[1] ; i++){
					LeftEdge[i].x = LeftCorner[0]->x + LeftCorner[0]->slope * (i - LeftCornerOrder[0]);
					LeftEdge[i].slope = LeftCorner[0]->slope;
				}
				for(int8_t i = RightCornerOrder[0]+1; i < RightCornerOrder[1] ; i++){
					RightEdge[i].x = RightCorner[0]->x + RightCorner[0]->slope *(i - RightCornerOrder[0]);
					RightEdge[i].slope = RightCorner[0]->slope;
				}
				for(int i = 0 ; i < MaxOrder ; i ++){
					MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
					MidPoint[i].y = LeftEdge[i].y;
				}
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("crossroad");
				}
				if(use_bt)
					BluetoothPtr->SendStr("crossroad\t");
				return Crossroad;
			}
		}

		else if( !RoundaboutTag &&LeftCorner[0] && !RightCorner[0] && LeftCorner[0]->x < LeftCorner[1]->x+5){
			Base_x = (LeftCorner[0]->x + 79)/2;
			Base_y = LeftCorner[0]->y;
			while(!FindWhite && i < Base_y -(LeftCorner[1]->y + RightCorner[1]->y) / 2.0){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
				}
				if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
					FindWhite = true;
				}
				i++;
			}

			if(!FindWhite){
				int left = (LeftCorner[0]->y - LeftCorner[1]->y == 0)?1:(LeftCorner[0]->y - LeftCorner[1]->y);
				LeftCorner[0]->slope  = static_cast<float>((LeftCorner[1]->x - LeftCorner[0]->x)) / left * 3;
				for(int8_t  i = LeftCornerOrder[0]+1 ; i<LeftCornerOrder[1] ; i++){
					LeftEdge[i].x = LeftCorner[0]->x + LeftCorner[0]->slope*(i - LeftCornerOrder[0]);
					LeftEdge[i].slope = LeftCorner[0]->slope;
				}
				int right = ( 59 - RightCorner[1]->y == 0)?1:( 59 - RightCorner[1]->y);
				float slope = static_cast<float>(RightCorner[1]->x -79)/right*3;
				for(int8_t i = RightCornerOrder[1]-1 ; i >=0 ; i-- ){
					RightEdge[i].x =  RightCorner[1]->x - slope*(RightCornerOrder[1] - i);
					RightEdge[i].slope = slope;
				}
				for(int i = 0 ; i < MaxOrder ; i ++){
					MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
					MidPoint[i].y = LeftEdge[i].y;
				}
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("crossroad");
				}
				if(use_bt)
					BluetoothPtr->SendStr("crossroad\t");
				return Crossroad;
			}
		}
		else if( !RoundaboutTag &&!LeftCorner[0] && RightCorner[0] && RightCorner[0]->x +5> RightCorner[1]->x){
			Base_x = RightCorner[0]->x / 3;
			Base_y =  RightCorner[0]->y;
			while(!FindWhite && i < Base_y -(LeftCorner[1]->y+RightCorner[1]->y)/2.0){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
				}
				if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
					FindWhite = true;
				}
				i++;
			}
			if(!FindWhite){
				int right = (RightCorner[0]->y - RightCorner[1]->y == 0)?1:(RightCorner[0]->y - RightCorner[1]->y);
				RightCorner[0]->slope = static_cast<float>(RightCorner[1]->x - RightCorner[0]->x) / right * 3;
				for(int8_t  i = RightCornerOrder[0]+1 ; i<RightCornerOrder[1] ; i++){
					RightEdge[i].x = RightCorner[0]->x + RightCorner[0]->slope*(i - RightCornerOrder[0]);
					RightEdge[i].slope = RightCorner[0]->slope;
				}
				int left = (59 - LeftCorner[1]->y == 0)?1:(59 - LeftCorner[1]->y);
				float slope = static_cast<float>(LeftCorner[1]->x)/ left * 3;
				for(int8_t  i = LeftCornerOrder[1] - 1 ; i >=0 ; i--){
					LeftEdge[i].x = LeftCorner[1]->x - slope*(LeftCornerOrder[1] - i) ;
					LeftEdge[i].slope = slope;
				}
				for(int i = 0 ; i < MaxOrder ; i ++){
					MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
					MidPoint[i].y = LeftEdge[i].y;
				}
//				BluetoothPtr->SendStr("crossroad3\n");
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("crossroad");
				}
				if(use_bt)
					BluetoothPtr->SendStr("crossroad\t");
				return Crossroad;
			}
		}
		else if( !RoundaboutTag &&!LeftCorner[0] && !RightCorner[0]){
			Base_x =  79/2;
			Base_y = 59;
			while(!FindWhite && i < Base_y - (LeftCorner[1]->y+RightCorner[1]->y)/2.0){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
				}
				if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
					FindWhite = true;
				}
				i++;
			}
			if(!FindWhite){
				float slope;
				int right = (59 - RightCorner[1]->y == 0)?1:(59 - RightCorner[1]->y);
				slope = static_cast<float>( RightCorner[1]->x-79)/ right * 3;
				for(int8_t  i = RightCornerOrder[1]-1 ; i>=0 ; i--){
					RightEdge[i].x =RightCorner[1]->x - slope*(RightCornerOrder[1] - i);
					RightEdge[i].slope = slope;
				}
				int left = (59 - LeftCorner[1]->y == 0)?1:(59 - LeftCorner[1]->y);
				slope = static_cast<float>(LeftCorner[1]->x )/left*3;
				for(int8_t  i = LeftCornerOrder[1] - 1 ; i >= 0 ; i--){
					LeftEdge[i].x =LeftCorner[1]->x - slope*(LeftCornerOrder[1] - i);
					LeftEdge[i].slope = slope;
				}
				for(int i = 0 ; i < MaxOrder ; i ++){
					MidPoint[i].x = (LeftEdge[i].x + RightEdge[i].x)/2;
					MidPoint[i].y = LeftEdge[i].y;
				}
				if(use_TFT){
				TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
				TyperPtr->WriteString("crossroad");
				}
				if(use_bt)
					BluetoothPtr->SendStr("crossroad\t");
				return Crossroad;
			}
		}
	}
	if(LeftCorner[0] && RightCorner[0] && LeftCorner[0]->y > 5 && RightCorner[0]->y > 5){
		if( !RoundaboutTag &&LeftCorner[1] && !RightCorner[1]){
			int8_t Base_x = (RightCorner[0]->x + LeftCorner[1]->x)/2;
			int8_t Base_y = RightCorner[0]->y;
//			bool FindWhite1 = false;
//			bool FindWhite2 = false;
//			bool FindBlack1 = false;
//			bool FindBlack2 = false;
			bool FindBlack = false;
			int8_t i = 1;
//			while(!FindWhite2 && i < Base_y ){
//				if(GetBit(Bin,Base_x,Base_y -i) == BLACK && !FindBlack1 && !FindBlack2 && ! FindWhite1 && !FindWhite2){
//					FindBlack1 = true;
//				}
//				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && !FindWhite1 && ! FindWhite2 && !FindBlack2){
//					FindWhite1 = true;
//				}
//				else if(GetBit(Bin,Base_x,Base_y -i) == BLACK && FindBlack1 && FindWhite1 && ! FindWhite2 && !FindBlack2)
//					FindBlack2 = true;
//				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && FindWhite1 && ! FindWhite2 && FindBlack2)
//					FindWhite2 = true;
//				i++;
			for(int i = 0 ; i < Base_y - LeftCorner[1]->y ; i++){
				if(GetBit(Bin,Base_x,Base_y - i) == BLACK){
					FindBlack =  true;
				}
			}
			if(!FindBlack){
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("crossroad2");
				}
				if(use_bt)
					BluetoothPtr->SendStr("crossroad2\t");
				return Crossroad2;
			}

		}

		else if( !RoundaboutTag &&!LeftCorner[1] && RightCorner[1]){
			int8_t Base_x = (LeftCorner[0]->x + RightCorner[0]->x)/2;
			int8_t Base_y = LeftCorner[0]->y;
//			bool FindWhite1 = false;
//			bool FindWhite2 = false;
//			bool FindBlack1 = false;
//			bool FindBlack2 = false;
			bool FindBlack = false;
			int8_t i = 1;
//			while(!FindWhite2 && i < Base_y ){
//				if(GetBit(Bin,Base_x,Base_y -i) == BLACK && !FindBlack1 && !FindBlack2 && ! FindWhite1 && !FindWhite2){
//					FindBlack1 = true;
//				}
//				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && !FindWhite1 && ! FindWhite2 && !FindBlack2){
//					FindWhite1 = true;
//				}
//				else if(GetBit(Bin,Base_x,Base_y -i) == BLACK && FindBlack1 && FindWhite1 && ! FindWhite2 && !FindBlack2)
//					FindBlack2 = true;
//				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && FindWhite1 && ! FindWhite2 && FindBlack2)
//					FindWhite2 = true;
//				i++;
//			}
			for(int i = 0 ; i < Base_y - RightCorner[1]->y ; i ++){
				if(GetBit(Bin, Base_x,Base_y-i) == BLACK){
					FindBlack = true;
				}
			}
			if(!FindBlack){
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("crossroad2");
				}
				if(use_bt)
					BluetoothPtr->SendStr("crossroad2\t");
				return Crossroad2;
			}
		}
		else if(System::Time()>3000&&GetBit(Bin,0,LeftCorner[0]->y-6) == BLACK && GetBit(Bin,79,RightCorner[0]->y-6) == BLACK )  {//!LeftCorner[1] && !RightCorner[1] && !RoundaboutTag &&if( System::Time()10000)
			int8_t Base_x = (LeftCorner[0]->x + RightCorner[0]->x)/2;
			int8_t Base_y = (LeftCorner[0]->y + RightCorner[0]->y)/2;
			bool FindBlack = false;
			bool FindLeftWhite = false;
			bool FindRightWhite = false;
//			bool FindWhite = false;
			int8_t i=1;
			while( i < Base_y-5){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
					break;
				}
//				if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
//					FindWhite = true;
//				}
				i++;
			}
			if(FindBlack && i < 35)
			for(int t = 1 ;Base_x - t >=0 || Base_x +t <=79 ; t ++ ){
				if(Base_x - t >=0 && GetBit(Bin,Base_x - t,Base_y - i)==WHITE){
					FindLeftWhite = true;
				}
				if(Base_x + t <= 79 && GetBit(Bin,Base_x+t,Base_y-i) == WHITE)
					FindRightWhite = true;
			}
			if(FindLeftWhite && FindRightWhite ){
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("roudabout");
				}
				if(use_bt)
					BluetoothPtr->SendStr("roundabout\t");
				return Roundabout;
			}
		}
	}

		int LeftStraightCount = 0;
		int RightStraightCount = 0;
		int MidStraightCount = 0;
		for(int i = 0 ; i < MidPointNumber-4 ; i ++){
			if(MidPoint[i].x <= MidPoint[i+4].x+1 && MidPoint[i].x >= MidPoint[i+4].x-1)
				MidStraightCount ++;
			if(MidPoint[i].x + 4 >= MidPoint[i+4].x && MidPoint[i].x <MidPoint[i+4].x)
				RightStraightCount++;
			if(MidPoint[i].x - 4 <= MidPoint[i+4].x && MidPoint[i].x > MidPoint[i+4].x)
				LeftStraightCount++;

		}
		if(System::Time() > 3000 &&( LeftStraightCount > MidPointNumber-9|| RightStraightCount > MidPointNumber - 9 || MidStraightCount > MidPointNumber-9)){
			Point* LeftCor = LeftCorner[1]?LeftCorner[1]:(LeftCorner[0]?LeftCorner[0]:NULL);
			int LeftCorNo = LeftCor?(59- LeftCor->y)/3:0;
			Point* RightCor = RightCorner[1]?RightCorner[1]:(RightCorner[0]?RightCorner[0]:NULL);
			int RightCorNo = RightCor?(59- RightCor->y)/3:0;
			bool in = false;
			bool out = false;
			int num = LeftNumber > RightNumber?RightNumber:LeftNumber;
			for(int i = 1 ; i  < num ; i ++){
				if(RightEdge[i].x - LeftEdge[i].x + 5 < RightEdge[i-1].x - LeftEdge[i - 1].x){
					in = true;
				}
				if(in && RightEdge[i].x - LeftEdge[i].x + 3 > RightEdge[i-1].x - LeftEdge[i - 1].x)
					out = true;
			}
			if(LeftCor && LeftCor->y<30 && LeftCor->y > 3 && (!RightCor) && out && GetBit(Bin,LeftCor->x,LeftCor->y-3) == WHITE){
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("LObstacle");
				}
				if(use_bt)
					BluetoothPtr->SendStr("LObstacle\t");
				return LeftObstacle;
			}
			else if(RightCor && RightCor->y< 30 && RightCor->y > 3 && !LeftCor && out && GetBit(Bin,RightCor->x,RightCor->y-3) == WHITE){
				if(use_TFT){
					TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
					TyperPtr->WriteString("RObstacle");
				}
				if(use_bt)
					BluetoothPtr->SendStr("RObstacle\t");
				return RightObstacle;
			}
			if(use_TFT){
				TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
				TyperPtr->WriteString("Straight");
			}
			if(use_bt)
				BluetoothPtr->SendStr("St\t");
			return Straight;
		}

	if(!RoundaboutTag &&LeftEdge[0].x == LeftEdge[3].x  && LeftEdge[3].x == LeftEdge[6].x && LeftEdge[6].x == 0
			&& RightEdge[0].x == RightEdge[3].x && RightEdge[3].x == RightEdge[6].x && RightEdge[6].x == 79){
		if(use_TFT){
			TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
			TyperPtr->WriteString("Crossroad3");
		}
		if(use_bt)
			BluetoothPtr->SendStr("Crossroad3\t");
		return Crossroad3;
	}





	if(use_TFT){
		TftPtr->SetRegion(Lcd::Rect(2,61,80,12));
		TyperPtr->WriteString("go");
	}
	if(use_bt)
		BluetoothPtr->SendStr("go\t");
	return Go;
}

void TransformEdge(Point LeftEdge[], Point RightEdge[], Point* MidPoint, Point* ModifiedMidPoint, uint8_t LeftEdgeNum, uint8_t RightEdgeNum,uint8_t MidPointNum, uint8_t& ModifiedMidPointNum){
	for(int i = 0 ; i < LeftEdgeNum ; i ++){
		if(LeftEdge[i].x<0) LeftEdge[i].x = 0;

		else if(LeftEdge[i].x > 79)  LeftEdge[i].x = 79;

		int t =LeftEdge[i].x;
		LeftEdge[i].x =transformMatrix[t][59-(int)(LeftEdge[i].y)][0];
		LeftEdge[i].y = 59-transformMatrix[t][59-(int)(LeftEdge[i].y)][1];
	}
	for(int i = 0 ; i < RightEdgeNum ; i ++){
		if(RightEdge[i].x<0) RightEdge[i].x = 0;
		else if(RightEdge[i].x > 79)  RightEdge[i].x = 79;

		int t = RightEdge[i].x;
		RightEdge[i].x = transformMatrix[t][59-(int)RightEdge[i].y][0];
		RightEdge[i].y = 59-transformMatrix[t][59-(int)RightEdge[i].y][1];
	}
	for(int i = 0 ; i < MidPointNum ; i ++){
		if(MidPoint[i].x < 0) MidPoint[i].x = 0;
		else if(MidPoint[i].x > 79)  MidPoint[i].x = 79;
		int t =MidPoint[i].x;
		MidPoint[i].x = transformMatrix[(int)MidPoint[i].x][59-(int)MidPoint[i].y][0];
		MidPoint[i].y =59- transformMatrix[(int)MidPoint[i].x][59-(int)MidPoint[i].y][1];
	}
	for(int i = 2 ; i < MidPointNum-2 ; i ++){
		MidPoint[i].x = (MidPoint[i-2].x + MidPoint[i-1].x + MidPoint[i].x + MidPoint[i+1].x + MidPoint[i+2].x)/5;
	}
	ModifiedMidPointNum  =0;
	int i = 0 ;
	while( MidPoint[i].y > 30 && i < MidPointNum){
		if(i == 0 ){
			ModifiedMidPoint[ModifiedMidPointNum] = MidPoint[i];
			ModifiedMidPointNum++;
		}
		else{
//			if(MidPoint[i].y == ModifiedMidPoint[ModifiedMidPointNum-1].y )
//				ModifiedMidPoint[ModifiedMidPointNum-1].x = (ModifiedMidPoint[ModifiedMidPointNum-1].x + MidPoint[i].x)/2;
			 if(MidPoint[i].y < ModifiedMidPoint[ModifiedMidPointNum-1].y-1){
				ModifiedMidPoint[ModifiedMidPointNum] = MidPoint[i];
				ModifiedMidPointNum ++;
			}
		}
		i++;
	}



	return;
}


float angDiff = 3;//3
float DiffMax = 5;
//

float bottomLine_k 	= 0.2;
float middleLine_k 	= 0.5;
float topLine_k 	= 0.8;

float r_bottomLine_k 	= 0.2;
float r_middleLine_k 	= 0.5;
float r_topLine_k		= 0.8;

float r_bottomLine_dis 		= 	30;

float r_middleLine_dis 		= 	30;
//float r_middleLine_white_dis 	= 	25;

float r_topLine_dis	 		=	20;
//float r_topLine_white_dis	 	= 	18;

float pos_Diff = 0;
float slope_Diff = 0;



int zhanglin = 0;

bool LeftObstacleBegin = false;
bool RightObstacleBegin = false;
bool ObstacleEnd = true;
int ObstacleEncoderValue = 0;

float FindPath(const Byte * Bin, const Point* LeftEdge, const Point* RightEdge, Point* MidPoint,const uint8_t MidPointNum,const TrackState trackstate,Point* LeftCorner[], Point* RightCorner[],uint8_t & LeftEdgeNum,uint8_t & RightEdgeNum){
	Point* LeftCor = (LeftCorner[0]?LeftCorner[0]:(LeftCorner[1]?LeftCorner[1]:NULL));
	Point* RightCor = (RightCorner[0]?RightCorner[0]:(RightCorner[1]?RightCorner[1]:NULL));
//	if(LeftCor->y <40 )
//		LeftCor =NULL;
	int t = (LeftEdgeNum > RightEdgeNum)?((RightEdgeNum > 15)?15:RightEdgeNum):((LeftEdgeNum > 15)?15:LeftEdgeNum);
	if(t == 0) t = 1;
	const int PosRow = t;
	const int row = t;
	const int SlopeRow = t;

	int8_t Order = 0;
	posDiff = 0;
	slopeDiff = 0;
//	RoundaboutCount = 3;



    if(trackstate == LeftObstacle && ObstacleEnd){
    	LeftObstacleBegin = true;
    	ObstacleEnd = false;
    }
    else if(trackstate == RightObstacle && ObstacleEnd){
    	RightObstacleBegin = true;
    	ObstacleEnd = false;
    }
    else if(ObstacleEncoderValue < -6000){
    	LeftObstacleBegin = false;
    	RightObstacleBegin = false;
    	ObstacleEnd = true;
    	ObstacleEncoderValue = 0;
    }


	if(trackstate == Roundabout && !RoundaboutTag){
		RoundaboutPreTag = true;
	}
	if(RoundaboutPreTag &&RoundaboutCount <= 2 &&  LeftCor && LeftCor->y > 30){
		InRoundStart = true;
		RoundaboutTag = true;
		RoundaboutPreTag = false;
		OutRoundEnd = false;
	}
	else if(RoundaboutPreTag &&RoundaboutCount > 2 &&  RightCor && RightCor->y > 30){
		InRoundStart = true;
		RoundaboutTag = true;
		RoundaboutPreTag = false;
		OutRoundEnd = false;
	}
	else if( InRoundEncoderValue < - 4500){
		InRoundEnd = true;
		InRoundStart = false;
		InRoundEncoderValue = 0;
	}
	if(RoundaboutCount <= 2 && InRoundEnd && LeftCor && LeftCor->y > 30  && GetBit(Bin,LeftCor->x,LeftCor->y-6) == WHITE ){
		OutRoundStart = true;
		InRoundEnd = false;
	}
	else if(RoundaboutCount > 2 && InRoundEnd && RightCor && RightCor->y > 30  && GetBit(Bin,RightCor->x,RightCor->y-6) == WHITE ){
		OutRoundStart = true;
		InRoundEnd = false;
	}
    if(OutRoundEncoderValue < -3000){
		OutRoundEncoderValue = 0;
		OutRoundEnd = true;
		OutRoundStart = false;
		RoundaboutTag = false;
		RoundaboutPreTag = false;
		RoundaboutCount++;
	}

     if(trackstate == Crossroad3){}

	//find the diff
    else if(trackstate == Crossroad2){
		Diff = 0.5*((LeftCorner[0]->x + RightCorner[0]->x)/2 - 39.5);
		if(use_TFT ){
			TftPtr->SetRegion(Lcd::Rect(2,73,80,12));
			TyperPtr->WriteString("out of roundabout\t");
		}
		if(use_bt)
			BluetoothPtr->SendStr("out of roundabout\t");
	}


	else if(!RoundaboutTag || (InRoundEnd && !OutRoundStart) ){//|| (trackstate == Roundabout &&LeftCor->y < 5 )  (trackstate == Roundabout &&LeftCor->y < 10 )(!LeftCor || ( LeftCor->y<30 && LeftCor->x > 30))  if(!RoundaboutTag  ||  (trackstate == Roundabout && LeftCor->y<20) ){//||(trackstate == Roundabout && LeftCor->y<20
			Order = 1;
			float slope = 0;
			float pos= 0;
			if(!LeftObstacleBegin && !RightObstacleBegin){
				while(Order < PosRow){
					float k = 1;
					if(Order < 4)
						k = bottomLine_k;
					else if(Order < 8)
						k= middleLine_k;
					else
						k=topLine_k;
					slope+=k*(MidPoint[Order].x - 39.5);
					Order++;
				}
				slope/=PosRow;
				Diff = 1.2*slope_p * slope + 1*pos_p * pos;
				posDiff = 1*pos_p * pos;
				slopeDiff = 1.2*slope_p * slope;

					if(use_TFT ){
						TftPtr->SetRegion(Lcd::Rect(2,73,80,12));
						TyperPtr->WriteString("out of roudabout");
					}
					if(use_bt)
						BluetoothPtr->SendStr("out of roundabout\t");
			}
			else if(LeftObstacleBegin){
				while(Order < PosRow){
					float k = 1;
					if(Order < 4)
						k = bottomLine_k;
					else if(Order < 8)
						k= middleLine_k;
					else
						k=topLine_k;
					MidPoint[Order].x = RightEdge[Order].x - 12;
					slope+=k*(MidPoint[Order].x - 39.5);
					Order++;
				}
				slope/=PosRow;
				Diff = 1.2*slope_p * slope + 1*pos_p * pos;
				posDiff = 1*pos_p * pos;
				slopeDiff = 1.2*slope_p * slope;

					if(use_TFT ){
						TftPtr->SetRegion(Lcd::Rect(2,73,80,12));
						TyperPtr->WriteString("in left obstacle");
					}
					if(use_bt)
						BluetoothPtr->SendStr("in left obstacle\t");
			}
			else{
				while(Order < PosRow){
					float k = 1;
					if(Order < 4)
						k = bottomLine_k;
					else if(Order < 8)
						k= middleLine_k;
					else
						k=topLine_k;
					MidPoint[Order].x = LeftEdge[Order].x + 12.8;
					slope+=k*(MidPoint[Order].x - 39.5);
					Order++;
				}
				slope/=PosRow;
				Diff = 1.2*slope_p * slope + 1*pos_p * pos;
				posDiff = 1*pos_p * pos;
				slopeDiff = 1.2*slope_p * slope;

					if(use_TFT ){
						TftPtr->SetRegion(Lcd::Rect(2,73,80,12));
						TyperPtr->WriteString("in Right obstacle");
					}
					if(use_bt)
						BluetoothPtr->SendStr("in Right obstacle\t");
			}

		}

	else {
		Order = 1;
		float pos = 0;
		float slope = 0;
		while(Order < PosRow){
			if(Order < 5){
				if(RoundaboutCount <= 2 )
					MidPoint[Order].x = LeftEdge[Order].x  + 30;
				else
					MidPoint[Order].x = RightEdge[Order].x  - 30;
			}
			else if(Order < 10){
				if(RoundaboutCount <= 2 ){
					if(LeftEdge[Order].x <= 1)
						MidPoint[Order].x = 10;
					else if(LeftEdge[Order].slope <= 1)
						MidPoint[Order].x = LeftEdge[Order].x  + 20;
					else
						MidPoint[Order].x = LeftEdge[Order].x +	25;
				}
				else {
					if(RightEdge[Order].x >=78 )
						MidPoint[Order].x = 70;
					else if(RightEdge[Order].slope >= 1)
						MidPoint[Order].x = RightEdge[Order].x  - 20;
					else
						MidPoint[Order].x = RightEdge[Order].x -	25;
				}
			}
			else{
				if(RoundaboutCount<=2){
					if(LeftEdge[Order].x <= 1)
						MidPoint[Order].x = 0;
					else if(LeftEdge[Order].slope<=1)
						MidPoint[Order].x = LeftEdge[Order].x  + 15;
					else
						MidPoint[Order].x = LeftEdge[Order].x  + 20;
				}
				else{
					if(RightEdge[Order].x >= 78)
						MidPoint[Order].x = 70;
					else if(RightEdge[Order].slope>=1)
						MidPoint[Order].x = RightEdge[Order].x  - 15;
					else
						MidPoint[Order].x = RightEdge[Order].x  - 20;
				}
			}
			Order++;
		}
		for(int i = 2 ; i < PosRow-3 ; i ++){
			MidPoint[i].x = (MidPoint[i-2].x + MidPoint[i-1].x + MidPoint[i].x + MidPoint[i+1].x + MidPoint[i+2].x)/5;
		}
		while(Order--){
			if(Order < 4){
				slope += r_bottomLine_k*(MidPoint[Order].x - 39.5);
//				pos+= ( MidPoint[Order].x - 39.5 );
			}
			else if(Order < 8)
				slope += r_middleLine_k*(MidPoint[Order].x - 39.5);
			else
				slope += r_topLine_k*(MidPoint[Order].x - 39.5);
		}

//		pos/=5;
		slope/=2;
		slope/=PosRow;
		Diff = 1.2*slope_p * slope +1.3* pos_p * pos;
		posDiff = 1.3*pos_p * pos;
		slopeDiff = 1.2*slope_p * slope;

		if(use_TFT){
			TftPtr->SetRegion(Lcd::Rect(2,73,80,12));
			TyperPtr->WriteString("in roudabout");
		}
		if(use_bt){
			BluetoothPtr->SendStr("in roudabout\t");
		}
	}


//		int count = 0 ;
//		for(int i = 0 ; i < 10 ; i ++){
//			if(MidPoint[i].x == 39.5)
//				count++;
//		}
//		if(count >= 5){
//			Diff = -10;
//		}
//		else{
//			if(LeftCor->y>40)
//			Order = 1;
//			float pos = 0,slope = 0;
//			while(Order<PosRow){
//				float k = 0;
//				if(Order < 3)
//					k = 0.2;
//				else if(Order < 6)
//					k=0.4;
//				else
//					k=1;
//				pos+=LeftEdge[Order].x + 10 -39.5;
//				Order++;
//			}
//			pos/=PosRow;
//
//			Diff = 2*pos_p * pos + slope_p * slope;
//			Diff = -7;
//		}
//		if(use_TFT){
//			TftPtr->SetRegion(Lcd::Rect(1,71,80,20));
//			TyperPtr->WriteString("in roud1");
//		}
//		BluetoothPtr->SendStr("in roudaboout1\n");
//	}
//
//	else if( LeftCor!=NULL && LeftCor->y > 30){
//		int count = 0 ;
//		for(int i = 0 ; i < 10 ; i ++){
//			if(MidPoint[i].x == 39.5)
//				count++;
//		}
//		if(count >= 5){
//			Diff = -10;
//		}
//		else {
//			float pos = (LeftCor->x - 39.5 + 23 );
//			Diff =0.5* pos_p * pos;
//		}
//		if(use_TFT){
//			TftPtr->SetRegion(Lcd::Rect(1,71,80,20));
//			TyperPtr->WriteString("in roud3");
//		}
//		BluetoothPtr->SendStr("in roudaboout3\n");
//	}
//
//	else  {
//		int count = 0 ;
//		for(int i = 0 ; i < 10 ; i ++){
//			if(MidPoint[i].x == 39.5)
//				count++;
//		}
//		if(count >= 5){
//			Diff = -10;
//		}
//		else{
//		Order = 1;
//		float slope = 0;
//		float pos= 0;
//		while(Order < PosRow){
//			float k = 0.3;
//			if(Order < 3)
//				k = 0.3;
//			else if(Order < 6)
//				k=0.8;
//			else
//				k=1.5;
//
//			if(LeftEdge[Order].x!=0 && LeftEdge[Order].slope>= 0){
//				if(RightEdge[Order].x==79)
//					MidPoint[Order].x = LeftEdge[Order].x + 10;
//				pos += 1.5*k*(MidPoint[Order].x- 39.5);
//			}
//			else if(LeftEdge[Order].x==0){
//				pos-=20;
//			}
//			else{
//				MidPoint[Order].x = LeftEdge[Order].x +5;
//				pos += 1.5*k*(MidPoint[Order].x- 39.5);
//			}
//
//			if(Order < SlopeRow){
//				if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
//					slope += ((LeftEdge[Order].slope - LeftEdge[Order - 1].slope + RightEdge[Order].slope - RightEdge[Order - 1].slope)/2); ///SLOPE CHANGE
//				else if(LeftEdge[Order].x == 0 && RightEdge[Order].x != 79 )
//					slope += RightEdge[Order].slope - RightEdge[Order-1].slope;
//				else if(LeftEdge[Order].x != 0 && RightEdge[Order].x == 79 )
//					slope += LeftEdge[Order].slope - LeftEdge[Order - 1].slope;
//			}
//			Order++;
//		}
//
//		slope/=SlopeRow;
//			pos/=PosRow;
//		Diff = slope_p * slope + pos_p * pos;
//		if(use_TFT){
//			TftPtr->SetRegion(Lcd::Rect(1,71,80,20));
//			TyperPtr->WriteString("in roud2");
//		}
//		BluetoothPtr->SendStr("in roundabout2\n");
//		}
//	}
//

	slope_Diff = slopeDiff;
	pos_Diff = posDiff;
	if(use_TFT){
		TftPtr->SetRegion(Lcd::Rect(2,85,120,50));
		sprintf(buffer,"posDiff\t%d\nslopeDiff\t%.2lf\nDiff\t%.2lf",posDiff,slopeDiff,Diff);
		TyperPtr->WriteString(buffer);
	}
	if(Diff > DiffMax)  Diff = DiffMax;
	else if(Diff < -DiffMax )   Diff = -DiffMax;
	if(use_bt ){
		sprintf(buffer,"%.2f\t",Diff);
		BluetoothPtr->SendStr(buffer);
	}
	return Diff;
}


#endif /* SRC_CAMERAHEADER_FINDEDGE_H_ */

