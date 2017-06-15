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
#define  BLACK					1
#define	WHITE					0

int distance[60] ;

bool GetBit(const Byte* b, uint8_t x, uint8_t y){
	return ((b[y*10 + x/8]>>(7 - x%8))&1);
}
using namespace libsc;
using namespace libsc::k60;
//JyMcuBt106*  bluetooth;
//////
//St7735r* tft;
//LcdTypewriter* typer;
char buffer[100];

bool carbegin[2] = {};
bool carinterval = false;
int carindex = 0;

bool RoundaboutTag = false;
bool FindCor = false;

float slope_p = 0;//30

float pos_p = 7;//3

int8_t Width[20]={79,78,77,76,74,72,69,66,64,61,58,55,52,49,47,43,40,37,33,30};

//边界点的状态
//enum EdgeState{
//	Unknown = 0,
//	Straight,		//原始图像为向前直线
//	Left,		//
//	Right,		//
//	BigLeft,
//	BigRight,
//	LeftCorner,
//	RightCorner,
//	LeftSide,
//	RightSide,
//	End
//};


int count = 0;


#define OUTSIDECRITERION	2
#define INSIDECRITERION		3
#define CORNERCRITERION		3
#define OFFSET				20

//赛道状态
enum TrackState{
	Crossroad,
	Crossroad2,
	Roundabout,
	Go,
	TurnLeft,
	TurnRight
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


/*	搜索方案

*/

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
	}
	if(carbegin[0] && !(leftblack[2] && rightblack[2] ))
		carinterval = true;
	if(leftblack[2] && rightblack[2] && carinterval)
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

//	if(count == 0 || count == 59){
//			distance[count] = RightEdge[count].x - LeftEdge[count].x;
//			sprintf(buffer,"%d\n",distance[count]);
//			bluetooth->SendStr(buffer);
//	}
//	count++;


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
		else if(LeftEdge[i].x == 0)
			MidPoint[i].x = RightEdge[i].x - Width[i]/2;
		else
			MidPoint[i].x = LeftEdge[i].x + Width[i]/2;
		MidPoint[i].y = LeftEdge[i].y;
	}

	if(MaxOrder < 3)
		return Go;

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
//				bluetooth->SendStr("   1   ");

			}
			else if(!LeftCorner1Found && LeftEdge[Order].slope < 4 && LeftEdge[Order].slope >= 0 ) {
				LeftCorner[1] = & LeftEdge[Order+1];
				LeftCornerOrder[1] = Order+1;
				LeftCorner1Found = true;
			}
		}

//		else if(!LeftCorner0Found && LeftEdge[Order].x == 0 && LeftEdge[Order-1].x >= 2){
//			LeftCorner0Found = true;
//			LeftCorner[0] = & LeftEdge[Order-1];
//			LeftCornerOrder[0] = Order-1;
//		}

			//判断右边
		if(RightEdge[Order].slope >= RightEdge[Order-1].slope + CORNERCRITERION){
			if(!RightCorner0Found && RightEdge[Order].slope >= 1){
				RightCorner[0] = & RightEdge[Order-1];
				RightCornerOrder[0] = Order-1;
				RightCorner0Found = true;
			}
			else if(!RightCorner1Found && RightEdge[Order].slope > -4 && RightEdge[Order].slope <=0) {
				RightCorner[1] = & RightEdge[Order+1];
				RightCornerOrder[1] = Order+1;
				RightCorner1Found = true;
			}
		}
//
//		if(!LeftCorner0Found && RightCorner0Found){
//				int i ;
//				if(RightCornerOrder[0] >= 1) i = -1;
//				else i = 0;
//				for( ; i < 2 ; i ++){
//					if(LeftEdge[RightCornerOrder[0] + i + 1].x == 0 && LeftEdge[RightCornerOrder[0]+i].x>=2){
//						LeftCorner0Found = true;
//						LeftCorner[0] = & LeftEdge[RightCornerOrder[0] + i ];
//						LeftCornerOrder[0] = RightCornerOrder[0]+i;
//						bluetooth->SendStr("   2   ");
//						break;
//					}
//				}
//		}
//		else if(LeftCorner0Found && !RightCorner0Found){
//				int i ;
//				if(LeftCornerOrder[0] >= 1) i = -1;
//				else i = 0;
//				for( ; i < 2 ; i ++){
//					if(RightEdge[LeftCornerOrder[0] + i + 1].x == 79 && RightEdge[LeftCornerOrder[0]+i].x<=77){
//						RightCorner0Found = true;
//						RightCorner[0] = & RightEdge[LeftCornerOrder[0] + i ];
//						RightCornerOrder[0] = LeftCornerOrder[0]+i;
//						break;
//					}
//				}
//		}
//
//		else if(Order <= 3 && !LeftCorner0Found && !RightCorner0Found){
//			int i ;
//			if(Order >= 2 ) i = -1;
//			else if(Order == 1) i = 0;
//			else i = 1 ;
//			for( ; i < 2 ; i ++){
//				if(LeftEdge[Order].x == 0 && LeftEdge[Order - 1].x>=2 && RightEdge[Order+i].x == 79 && RightEdge[Order+i - 1].x <= 77){
//					LeftCorner0Found = true;
//					RightCorner0Found = true;
//					LeftCorner[0] = & LeftEdge[Order-1];
//					LeftCornerOrder[0] = Order-1;
//					bluetooth->SendStr("   3   ");
//					RightCorner[0] = & RightEdge[Order+i-1];
//					RightCornerOrder[0] = Order+i-1;
//					break;
//				}
//			}
//		}


//		else if(!RightCorner1)
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
	LeftCornerNum = LeftCornerOrder[1];
	RightCornerNum = RightCornerOrder[1];
//
//	if(LeftCorner[0])
//		bluetooth->SendStr("L  0  FOUND   ");
//	if(LeftCorner[1])
//		bluetooth->SendStr("L  1  FOUND   ");
//	if(RightCorner[0])
//		bluetooth->SendStr("R  0  FOUND   ");
//	if(RightCorner[1])
//		bluetooth->SendStr("R  1  FOUND   \n");

	if(LeftCorner[1] && RightCorner[1] && LeftCorner[1]->x < RightCorner[1]->x - 5 && LeftCorner[1]->y - RightCorner[1]->y < 9 &&  LeftCorner[1]->y - RightCorner[1]->y > -9){ // 用上面两个边界点判断十字并补线
//		bluetooth->SendStr("\t\tnonono");
		int8_t Base_x;
		int8_t Base_y;
		bool FindBlack = false;
		bool FindWhite = false;
		int8_t i=1;

		if(LeftCorner[0] && RightCorner[0] && LeftCorner[0]->x < LeftCorner[1]->x + 5 && RightCorner[0]->x + 5 > RightCorner[1]->x){
//			b->SendStr("all found\n");
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
//				sprintf(buffer,"\n%d\n",RightCorner[0]->y);
//				bluetooth->SendStr(buffer);
//				bluetooth->SendStr("crossroad1\n");
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("Crossroad");
				return Crossroad;
			}
//			else{
//				b->SendStr("roundabout\n");
//				return Roundabout;
//			}
		}
		else if(LeftCorner[0] && !RightCorner[0] && LeftCorner[0]->x < LeftCorner[1]->x+5){
			Base_x = (LeftCorner[0]->x + 79)/2;
			Base_y = LeftCorner[0]->y;
//			b->SendStr("left found, right not found\n");
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
//				int right = (RightCorner[0]->y - RightCorner[1]->y == 0)?1:(RightCorner[0]->y - RightCorner[1]->y);
				LeftCorner[0]->slope  = static_cast<float>((LeftCorner[1]->x - LeftCorner[0]->x)) / left * 3;
//				sprintf(buffer,"%f",slope);
//				b->SendStr(buffer);
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
//				bluetooth->SendStr("crossroad2\n");
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("Crossroad");
				return Crossroad;
			}
//			else{
//				b->SendStr("roundabout\n");
//				return Roundabout;
//			}
		}
		else if(!LeftCorner[0] && RightCorner[0] && RightCorner[0]->x +5> RightCorner[1]->x){
			Base_x = RightCorner[0]->x / 3;
			Base_y =  RightCorner[0]->y;
//			b->SendStr("RIght found, left not found");
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
//				sprintf(buffer,"%f",slope);
//				b->SendStr(buffer);
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
//								sprintf(buffer,"\n%d\n",RightCorner[0]->y);
//								bluetooth->SendStr(buffer);
//				bluetooth->SendStr("crossroad3\n");
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("Crossroad");
				return Crossroad;
			}
//			else{
//				b->SendStr("roundabout\n");
//				return Roundabout;
//			}
		}
		else if(!LeftCorner[0] && !RightCorner[0]){
			Base_x =  79/2;
			Base_y = 59;
//			b->SendStr("all not found");
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
//				sprintf(buffer,"%f",slope);
//				b->SendStr(buffer);
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
//				bluetooth->SendStr("crossroad4\n");
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("Crossroad");
				return Crossroad;
			}
//			else{
//				b->SendStr("roundabout\n");
//				return Roundabout;
//			}
		}
	}
	if(LeftCorner[0] && RightCorner[0] && LeftCorner[0]->y > 40 && RightCorner[0]->y > 40){
		if(LeftCorner[1] && !RightCorner[1]){
			int8_t Base_x = RightCorner[0]->x+1;
			int8_t Base_y = RightCorner[0]->y;
			bool FindWhite1 = false;
			bool FindWhite2 = false;
			bool FindBlack1 = false;
			bool FindBlack2 = false;
			int8_t i = 1;
			while(!FindWhite2 && i < Base_y ){
				if(GetBit(Bin,Base_x,Base_y -i) == BLACK && !FindBlack1 && !FindBlack2 && ! FindWhite1 && !FindWhite2){
					FindBlack1 = true;
				}
				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && !FindWhite1 && ! FindWhite2 && !FindBlack2){
					FindWhite1 = true;
				}
				else if(GetBit(Bin,Base_x,Base_y -i) == BLACK && FindBlack1 && FindWhite1 && ! FindWhite2 && !FindBlack2)
					FindBlack2 = true;
				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && FindWhite1 && ! FindWhite2 && FindBlack2)
					FindWhite2 = true;
				i++;
			}
			if(FindWhite2){
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("Crossroad2");
				return Crossroad2;
			}

		}
		else if(!LeftCorner[1] && RightCorner[1]){
			int8_t Base_x = LeftCorner[0]->x-1;
			int8_t Base_y = LeftCorner[0]->y;
			bool FindWhite1 = false;
			bool FindWhite2 = false;
			bool FindBlack1 = false;
			bool FindBlack2 = false;
			int8_t i = 1;
			while(!FindWhite2 && i < Base_y ){
				if(GetBit(Bin,Base_x,Base_y -i) == BLACK && !FindBlack1 && !FindBlack2 && ! FindWhite1 && !FindWhite2){
					FindBlack1 = true;
				}
				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && !FindWhite1 && ! FindWhite2 && !FindBlack2){
					FindWhite1 = true;
				}
				else if(GetBit(Bin,Base_x,Base_y -i) == BLACK && FindBlack1 && FindWhite1 && ! FindWhite2 && !FindBlack2)
					FindBlack2 = true;
				else if(GetBit(Bin,Base_x,Base_y -i) == WHITE && FindBlack1 && FindWhite1 && ! FindWhite2 && FindBlack2)
					FindWhite2 = true;
				i++;
			}
			if(FindWhite2){
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("crossroad2");
				return Crossroad2;
			}

		}
		else {
			int8_t Base_x = (LeftCorner[0]->x + RightCorner[0]->x)/2;
			int8_t Base_y = (LeftCorner[0]->y + RightCorner[0]->y)/2;
			bool FindBlack = false;
//			bool FindWhite = false;
			int8_t i=1;
			while( i < Base_y - 3){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
				}
//				if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
//					FindWhite = true;
//				}
				i++;
			}
			if(FindBlack){
//				tft->SetRegion(Lcd::Rect(1,101,80,20));
//				typer->WriteString("roudabout");
//				bluetooth->SendStr("Roundabout1\n");
				return Roundabout;
			}
		}
	}

//		b->SendStr("go\n");
//	tft->SetRegion(Lcd::Rect(1,101,80,20));
//	typer->WriteString("go");
		return Go;
}

float FindPath(const Point* LeftEdge, const Point* RightEdge, Point* MidPoint,const uint8_t MidPointNum,const TrackState trackstate,Point* LeftCorner[], Point* RightCorner[],uint8_t & LeftEdgeNum,uint8_t & RightEdgeNum){
	Point* LeftCor = (LeftCorner[1])?LeftCorner[1]:(LeftCorner[0]?LeftCorner[0]:NULL);
	if(LeftCor){
		if(LeftCor->y <= 30)
			LeftCor = NULL;
	}
	int t = (LeftEdgeNum > RightEdgeNum)?((RightEdgeNum > 10)?10:RightEdgeNum):((LeftEdgeNum > 10)?10:LeftEdgeNum);
	if(t == 0) t = 1;
	const int PosRow = t;
	const int row = t;
//	int row = PosRow;
	t = LeftEdgeNum > RightEdgeNum ?((RightEdgeNum > 5)?5:RightEdgeNum):((LeftEdgeNum > 5)?5:LeftEdgeNum);
	if(t == 0) t = 1;
	const int SlopeRow = t;
	int8_t Order = 0;
	float Diff=0;

	FindCor = false;

	if(RoundaboutTag){
		for(int i = 2 ; i < MidPointNum-2 ; i ++){
			MidPoint[i].x = (MidPoint[i-2].x + MidPoint[i-1].x + MidPoint[i].x + MidPoint[i+1].x + MidPoint[i+2].x)/5;
		}
	}

	if(trackstate == Roundabout&&!RoundaboutTag){
		RoundaboutTag = true;
//		tft->SetRegion(Lcd::Rect(1,81,80,20));
//		typer->WriteString("go into roud");
//		const Byte speedByte = 85;
//		bluetooth.SendBuffer(&speedByte, 1);
//		const Byte speedByte = 85;
//		bluetooth->SendBuffer(&speedByte, 1);
//		bluetooth->SendStr("=\t\tgo into roundabout\n");
	}

	if(RoundaboutTag && trackstate!=Roundabout){
		Order = 0;
		int count = 0;
		while(Order<row){
			if(LeftEdge[Order].x == 0)
				break;
			Order++;
		}
		if(LeftEdge[4].x - LeftEdge[0].x <=2  && LeftEdge[4].x - LeftEdge[0].x >= -1 &&LeftEdge[row-1].x - LeftEdge[0].x <=2  && LeftEdge[row-1].x - LeftEdge[0].x >= -1 &&Order == row){
			RoundaboutTag = false;
			FindCor = false;
//			tft->SetRegion(Lcd::Rect(1,81,80,20));
//			typer->WriteString("go out roud");
//			const Byte speedByte = 85;
//			bluetooth->SendBuffer(&speedByte, 1);
//			bluetooth->SendStr("=\t\tgo out of roundabout\n");
			Order= 0 ;
		}
	}

	if(RoundaboutTag && !FindCor){
		if(LeftCor && trackstate!= Roundabout)
			FindCor = true;
	}

	if(trackstate == Crossroad2){
		Diff = 0;
	}
	else if(!RoundaboutTag ){
		Order = 1;
		float slope = 0;
		float pos= 0;
		while(Order < PosRow){
			float k = 0;
			if(Order < 3)
				k = 0.5;
			else if(Order < 6)
				k=0.4;
			else
				k=0.3;

//			if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
//				pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39.5);
//			else if(LeftEdge[Order].x  != 0 ){
////				if(Order < 3)
////					pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39.5);
////				else
//					pos+= k*(LeftEdge[Order].x + OFFSET -39.5);
//			}
//			else if(RightEdge[Order].x != 79){
////				if(Order < 3)
////					pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39.5);
////				else
//					pos+= k*(RightEdge[Order].x - OFFSET - 39.5);
//			}
			pos+=k*(MidPoint[Order].x - 39.5);

			if(Order < SlopeRow){
				if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
					slope += ((LeftEdge[Order].slope - LeftEdge[Order - 1].slope + RightEdge[Order].slope - RightEdge[Order - 1].slope)/2);
				else if(LeftEdge[Order].x == 0 && RightEdge[Order].x != 79 )
					slope += RightEdge[Order].slope - RightEdge[Order - 1].slope;
				else if(LeftEdge[Order].x != 0 && RightEdge[Order].x == 79 )
					slope += LeftEdge[Order].slope - LeftEdge[Order - 1].slope;
			}
			Order++;
		}

		slope/=SlopeRow;
		pos/=PosRow;
		Diff = slope_p * slope + pos_p * pos;
//		tft->SetRegion(Lcd::Rect(1,81,80,20));
//		typer->WriteString("out of roud");
//		sprintf(buffer,"\n%d\n",Diff);
//		bluetooth->SendStr(buffer);
//		const Byte speedByte = 85;
//		bluetooth->SendBuffer(&speedByte, 1);
//		bluetooth->SendStr("=\t\tout of roundabout\n");
		}




	else if(trackstate == Roundabout){
//		FindCor = true;
		Order = 1;
		float pos = 0,slope = 0;
		while( Order < PosRow ){
			float k=0;
			if(Order < 3)
				k = 0.5;
			else if(Order < 6)
				k=0.4;
			else
				k=0.3;

//			pos+=k*(MidPoint[Order].x - 39.5);
			pos += k*(LeftEdge[Order].x - 39.5 +30 ) ;

			if(Order < SlopeRow)
				slope += LeftEdge[Order].slope - LeftEdge[Order - 1].slope;
			Order++;
		}
		pos/=PosRow;
		slope/=SlopeRow;
		Diff = pos_p * pos + slope_p * slope;
//		bluetooth->SendStr("thisthisthis");
//		tft->SetRegion(Lcd::Rect(1,81,80,20));
//		typer->WriteString("in roud1");
//		const Byte speedByte = 85;
//		bluetooth->SendBuffer(&speedByte, 1);
//		bluetooth->SendStr("=\t\tin roudaboout1\n");
	}

	else if( LeftCor!=NULL){
		float pos = LeftCor->x - 39.5 + 20;
		if(Order < 3)
			pos*=0.5;
		else if(Order < 6)
			pos*=0.4;
		else
			pos*=0.3;
		Diff = pos_p * pos;
//		tft->SetRegion(Lcd::Rect(1,81,80,20));
//		typer->WriteString("in roud3");
//		sprintf(buffer,"\n%d\n", LeftCor->y);
//		bluetooth->SendStr(buffer);
//		const Byte speedByte = 85;
//		bluetooth->SendBuffer(&speedByte, 1);
//		bluetooth->SendStr("=\t\tin roudaboout3\n");
	}

	else {
		Order = 1;
		float slope = 0;
		float pos= 0;
		while(Order < PosRow){
			float k = 0;
			if(Order < 3)
				k = 0.5;
			else if(Order < 6)
				k=0.4;
			else
				k=0.3;

//			if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
//				pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39);
//			else if(LeftEdge[Order].x  != 0 )
			if(LeftEdge[Order].x!=0)
				pos+= k*(LeftEdge[Order].x + 30 - 39.5);
			else
				pos-=5;
//			else if(RightEdge[Order].x != 0)
//				pos+=(RightEdge[Order].x - OFFSET - 39);
//			else {
//				pos-= k*100;
//			}
//			if(Order < 5)
//				pos*=0.7;
//			else
//				pos*=0.3;

			if(Order < SlopeRow){
				if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
					slope += ((LeftEdge[Order].slope - LeftEdge[Order - 1].slope + RightEdge[Order].slope - RightEdge[Order - 1].slope)/2); ///SLOPE CHANGE
				else if(LeftEdge[Order].x == 0 && RightEdge[Order].x != 79 )
					slope += RightEdge[Order].slope - RightEdge[Order-1].slope;
				else if(LeftEdge[Order].x != 0 && RightEdge[Order].x == 79 )
					slope += LeftEdge[Order].slope - LeftEdge[Order - 1].slope;
			}
			Order++;
		}

		slope/=SlopeRow;
		pos/=PosRow;
		Diff = slope_p * slope + pos_p * pos;
//		tft->SetRegion(Lcd::Rect(1,81,80,20));
//		typer->WriteString("in roud2");
//		const Byte speedByte = 85;
//		bluetooth->SendBuffer(&speedByte, 1);
//		bluetooth->SendStr("=\t\tin roudaboout2\n");
	}



//	return (Diff+40)/80;
//	Diff-=200;
	return Diff;
}


#endif /* SRC_CAMERAHEADER_FINDEDGE_H_ */


//														main.cpp
//
//#include <cassert>
//#include <vector>
//#include <stdio.h>
//#include <cstring>
//#include <cstdint>
////#include <stdint.h>
//#include <inttypes.h>
//#include <sstream>
//#include <cmath>
//#include <array>
//#include <libsc/system.h>
//#include <libbase/k60/mcg.h>
//#include <libutil/string.h>
//#include <libutil/misc.h>
//#include <libsc/tower_pro_mg995.h>
//#include <libbase/k60/pit.h>
//#include<fstream>
//
//#include <libsc/led.h>
////#include <libsc/k60/uart_device.h>
////#include <libbase/k60/uart.h>
//#include <libsc/k60/ov7725.h>
//
//#include <libsc/alternate_motor.h>
//
//#include <libsc/dir_encoder.h>
//
//#include <libsc/button.h>
//
//#include <libsc/joystick.h>
//
////#include <libsc/k60/jy_mcu_bt_106.h>
//
//
//
////#include <libsc/mpu6050.h>
//#include "CameraHeader/FindEdge.h"
//#include "libsc/futaba_s3010.h"
//
//namespace libbase
//{
//    namespace k60
//    {
//
//        Mcg::Config Mcg::GetMcgConfig()
//        {
//            Mcg::Config config;
//            config.external_oscillator_khz = 50000;  //50000
//            config.core_clock_khz = 150000;			//150000
//            return config;
//        }
//
//    }
//}
//
//
//using namespace libsc;
//using namespace libbase::k60;
//using namespace libsc::k60;
//using namespace std;
//
//bool q[4800];
//bool CameraBin[60][80];
//Point LeftEdge[150];
//uint8_t LeftEdgeNum;
//uint8_t RightEdgeNum;
//
//uint8_t LeftCornerOrder;
//uint8_t RightCornerOrder;
//Point RightEdge[150];
//
//Point* LeftCorner[2] = {NULL};
//Point* RightCorner[2] = {NULL};
//Point ModLeftEdge[150];
//Point ModRightEdge[150];
////TrackState T;
//
//
//
//void PrintEdges(const Point* LeftEdge, const Point* RightEdge, const uint8_t LeftEdgeNum,const uint8_t RightEdgeNum,Point* LeftCorner[], Point* RightCorner[]){
//	for(int i = 0 ; i < RightEdgeNum ; i ++){
//		tft->SetRegion(Lcd::Rect(RightEdge[i].x+1,RightEdge[i].y+1,1,1));
//		tft->FillColor(Lcd::kBlack);
//	}
//	for(int i = 0 ; i < LeftEdgeNum; i ++){
//		tft->SetRegion(Lcd::Rect(LeftEdge[i].x+1,LeftEdge[i].y+1,1,1));
//		tft->FillColor(Lcd::kRed);
//	}
//	for(int i = 0 ; i < 2 ; i ++){
//		if(LeftCorner[i]){
//			tft->SetRegion(Lcd::Rect(LeftCorner[i]->x,LeftCorner[i]->y, 2 ,2));
//			tft->FillColor(Lcd::kBlue);
//		}
//		if(RightCorner[i]){
//			tft->SetRegion(Lcd::Rect(RightCorner[i]->x,RightCorner[i]->y, 2 ,2));
//			tft->FillColor(Lcd::kBlack);
//		}
//	}
//
//}
//
//
//
//
//
//
//
//
//int main(){
// 	System::Init();
//
//
//
//
//	Ov7725::Config camera_config;
//	camera_config.id = 0;
//	camera_config.w = 80;
//	camera_config.h = 60;
//	camera_config.fps = Ov7725Configurator::Config::Fps::kHigh;
//	Ov7725 CAMERA(camera_config);
//
//	JyMcuBt106::Config bluetooth_config;
//	bluetooth_config.id=0;
//	bluetooth_config.baud_rate=libbase::k60::Uart::Config::BaudRate::k115200;
//	JyMcuBt106 BT(bluetooth_config);
//	bluetooth=&BT;
//
////	FutabaS3010::Config servo_config;
////	servo_config.id = 0;
////	FutabaS3010 SERVO(servo_config);
////	SERVO.SetDegree(730);  //730  1000
//
//	Led::Config led_config;
//	led_config.id=0;
//	led_config.is_active_low=false;
//	Led led1(led_config);
////
////	DirEncoder::Config Lconfig;
////	Lconfig.id = 0;
////	DirEncoder LENCODER(Lconfig);
////	DirEncoder::Config Rconfig;
////	Rconfig.id = 1;
////	DirEncoder RENCODER(Rconfig);
//
//
//	led_config.id=1;
//	led_config.is_active_low=false;
//	Led led2(led_config);
//
//	St7735r::Config tft_config;
//	tft_config.is_revert = true;
//	St7735r	TFT(tft_config);
//
//	TFT.SetRegion(Lcd::Rect(1,1,80,60));
//	tft = &TFT;
//
//
//	LcdTypewriter::Config writer_config;
//	writer_config.lcd = &TFT;
//	LcdTypewriter TYPER(writer_config);
//	typer = &TYPER;
//
////	AlternateMotor::Config Lmotor_config;
////	Lmotor_config.id = 0;
////	AlternateMotor::Config Rmotor_config;
////	Rmotor_config.id = 1;
////	AlternateMotor LMOTOR(Lmotor_config);
////	AlternateMotor	RMOTOR(Rmotor_config);
////	LMOTOR.SetClockwise(true);
////	RMOTOR.SetClockwise(false);
////	LMOTOR.SetPower(250);
////	RMOTOR.SetPower(250);
//
//
//
//
//	uint32_t CurTime = System::Time(), PreTime = System::Time();
//	while(System::Time() - PreTime < 1500);
//
//	CAMERA.Start();
//	PreTime = System::Time();
//	int count=0;
//	float predif = 0;
//	bool T = true;
//	float dif=0;
//	float kp = 0.1;
//	char buffer[100];
//	while(T){
//		CurTime = System::Time();
//
//		if(CurTime - PreTime >= 100){
//			PreTime = CurTime;
//
//			const Byte* image= CAMERA.LockBuffer();
//			CAMERA.UnlockBuffer();
//
////			led1.Switch();
//			led2.Switch();
//
//			TFT.SetRegion(Lcd::Rect(1,1,80,60));
//			TFT.FillBits(Lcd::kYellow,Lcd::kWhite,image,4800);
////			TFT.FillColor(Lcd::kBlue);
//			FindEdge(image,LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum);
////			ModifyEdge(image,LeftEdge, RightEdge,LeftEdgeNum, RightEdgeNum,LeftCornerOrder,RightCornerOrder,LeftCorner, RightCorner);
//			dif = FindPath(LeftEdge,RightEdge,ModifyEdge(image,LeftEdge, RightEdge,LeftEdgeNum, RightEdgeNum,LeftCornerOrder,RightCornerOrder,LeftCorner,RightCorner),LeftCorner,RightCorner,LeftEdgeNum, RightEdgeNum);
//			int16_t servo_power;
//			PrintEdges(LeftEdge,RightEdge,LeftEdgeNum,RightEdgeNum,LeftCorner,RightCorner);
//			servo_power = 700 - dif;
//
////			if(servo_power > 1000)
////				SERVO.SetDegree(1000);
////			else if(servo_power > 900)
////				SERVO.SetDegree(900);
////			else if(servo_power > 800)
////				SERVO.SetDegree(800);
////			else if(servo_power > 750)
////				SERVO.SetDegree(750);
////			else if(servo_power > 650)
////				SERVO.SetDegree(700);
////			else if(servo_power < 400)
////				SERVO.SetDegree(400);
////			else if(servo_power < 500)
////				SERVO.SetDegree(500);
////			else if(servo_power < 600)
////				SERVO.SetDegree(600);
////			else
////				SERVO.SetDegree(650);
//////
////			if(servo_power > 800){
////				LMOTOR.SetPower(200);
////				RMOTOR.SetPower(250);
////			}
////			else if(servo_power<600){
////				LMOTOR.SetPower(250);
////				RMOTOR.SetPower(200);
////			}
////			else{
////				LMOTOR.SetPower(250);
////				RMOTOR.SetPower(250);
////			}
//
//
////			else
////				SERVO.SetDegree(400);
////			else
////				SERVO.SetDegree(servo_power);
////			SERVO.SetDegree(400);//1050         530   700      1030    290
//			sprintf(buffer,"%f",dif);
//			TFT.SetRegion(Lcd::Rect(50,100,50,20));
//			TYPER.WriteString(buffer);
////			bluetooth->SendStr(buffer);
//
////			for(int i = 47 ; i < 60; i+=2 ){
////				tft->SetRegion(Lcd::Rect(39+(59-i)*dif/12,i,2,2));
////				tft->FillColor(Lcd::kCyan);
////			}
//
//			//  MOTOR PROTECTION
////			LENCODER.Update();
////			RENCODER.Update();
////			int L  = LENCODER.GetCount();
////			int R = RENCODER.GetCount();
////			if(L< 5 && L> -5 &&R< 5 && R> -5  ){
////				LMOTOR.SetPower(0);
////				RMOTOR.SetPower(0);
////				T = false;
////			}
//
//
//			//SHOW TRACK STATE
////			tft->SetRegion(Lcd::Rect(1,90,20,10));
////			switch(T){
////			case  Normal:
////				TYPER.WriteString("Normal");
////				break;
////			case  Crossroad:
////				TYPER.WriteString("Crossroad");
////				break;
////			case Roundabout:
////				TYPER.WriteString("Roundabout");
////				break;
////			}
//
//			}
//		}
//	CAMERA.Stop();
//
//
//return 0;
//
//}
//
//
//

























