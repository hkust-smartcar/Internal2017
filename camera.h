/*
 * FindEdge.h
 *
 *  Created on: Apr 19, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_CAMERAHEADER_FINDEDGE_H_
#define SRC_CAMERAHEADER_FINDEDGE_H_

//#include <libsc/k60/uart_device.h>
//#include <libbase/k60/uart.h>
#include <libsc/k60/jy_mcu_bt_106.h>
//#include <libsc/lcd_typewriter.h>
#define  BLACK					1
#define	WHITE					0



bool GetBit(const Byte* b, uint8_t x, uint8_t y){
	return ((b[y*10 + x/8]>>(7 - x%8))&1);
}
using namespace libsc;
using namespace libsc::k60;
//JyMcuBt106*  bluetooth;

//St7735r* tft;
//LcdTypewriter* typer;
char buffer[100];

bool RoundaboutTag = false;
bool FindCor = false;

float slope_p = 30;//100
float pos_p = 100;//100

int8_t Width[8]={82,79,76,73,70,67,64,61};

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

#define OUTSIDECRITERION	2
#define INSIDECRITERION		3
#define CORNERCRITERION		3
#define OFFSET				40

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
	else
		return false;
}


/*	搜索方案
*/

//Find Edge  version 2
void  FindEdge(const Byte*  Bin,  Point* LeftEdge,  Point* RightEdge,  uint8_t& LeftEdgeNumber,  uint8_t& RightEdgeNumber){
//从中间开始搜索起点
	bool LeftStart = false;
	bool RightStart = false;
	/*
	 * 从底线搜索第一个边界点
	 * 从右往左搜索   黑——白 为第一个左边界点
	 * 从左往右搜索	白——黑 为第一个右边界点
	 */
	for(int i = 6; i <= 78 ; i ++){
		if(GetBit(Bin, 79-i, 59) == WHITE && GetBit(Bin, 78-i,59) == BLACK && !LeftStart){
			LeftEdge[0] = Point(78-i,59);
			LeftStart = true;
		}
		if(GetBit(Bin,i, 59) == WHITE && GetBit(Bin, i+1,59) == BLACK && !RightStart){
			RightEdge[0] = Point(i+1,59);
			RightStart = true;
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
				if(GetBit(Bin, RightEdge[Order].x -1- i, RightEdge[Order].y - 3) == BLACK){
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

TrackState ModifyEdge(const Byte* Bin , Point* LeftEdge,  Point* RightEdge, const uint8_t& LeftNumber, const uint8_t& RightNumber, uint8_t & LeftCornerNum, uint8_t& RightCornerNum, Point* LeftCorner[], Point* RightCorner[]){
	LeftCorner[0] = LeftCorner[1] = NULL;
	RightCorner[0] = RightCorner[1] = NULL;
	int8_t LeftCornerOrder[2] = {0};
	int8_t RightCornerOrder[2] = {0};
	int8_t MaxOrder = (LeftNumber > RightNumber)?((RightNumber  >20)?20:RightNumber):((LeftNumber > 20)?20:LeftNumber);

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

		if(LeftEdge[Order].slope < LeftEdge[Order-1].slope - CORNERCRITERION){
			if(!LeftCorner0Found && LeftEdge[Order].slope <= - 2){
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
		if(RightEdge[Order].slope > RightEdge[Order-1].slope + CORNERCRITERION){
			if(!RightCorner0Found && RightEdge[Order].slope >= 2){
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
	if(LeftCorner[0]->y < 30){
		LeftCorner[0] = NULL;
		LeftCornerOrder[0] = 0;
	}
	if(RightCorner[0]->y < 30){
		RightCorner[0] = NULL;
		RightCornerOrder[0] = 0;
	}
	LeftCornerNum = LeftCornerOrder[1];
	RightCornerNum = RightCornerOrder[1];

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
				LeftCorner[0]->slope  = static_cast<float>((LeftCorner[1]->x - LeftCorner[0]->x)) / (LeftCorner[0]->y - LeftCorner[1]->y) * 3;
				RightCorner[0]->slope = static_cast<float>((RightCorner[1]->x - RightCorner[0]->x)) / (RightCorner[0]->y - RightCorner[1]->y) * 3;
				for(int8_t  i = LeftCornerOrder[0]+1 ; i<LeftCornerOrder[1] ; i++){
					LeftEdge[i].x = LeftCorner[0]->x + LeftCorner[0]->slope * (i - LeftCornerOrder[0]);
					LeftEdge[i].slope = LeftCorner[0]->slope;
				}
				for(int8_t i = RightCornerOrder[0]+1; i < RightCornerOrder[1] ; i++){
					RightEdge[i].x = RightCorner[0]->x + RightCorner[0]->slope *(i - RightCornerOrder[0]);
					RightEdge[i].slope = RightCorner[0]->slope;
				}
//				sprintf(buffer,"\n%d\n",RightCorner[0]->y);
//				bluetooth->SendStr(buffer);
//				bluetooth->SendStr("crossroad1\n");
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
				LeftCorner[0]->slope  = static_cast<float>((LeftCorner[1]->x - LeftCorner[0]->x)) / (LeftCorner[0]->y - LeftCorner[1]->y) * 3;
//				sprintf(buffer,"%f",slope);
//				b->SendStr(buffer);
				for(int8_t  i = LeftCornerOrder[0]+1 ; i<LeftCornerOrder[1] ; i++){
					LeftEdge[i].x = LeftCorner[0]->x + LeftCorner[0]->slope*(i - LeftCornerOrder[0]);
					LeftEdge[i].slope = LeftCorner[0]->slope;
				}
				float slope = static_cast<float>(RightCorner[1]->x -79)/( 59 - RightCorner[1]->y)*3;
				for(int8_t i = RightCornerOrder[1]-1 ; i >=0 ; i-- ){
					RightEdge[i].x =  RightCorner[1]->x - slope*(RightCornerOrder[1] - i);
					RightEdge[i].slope = slope;
				}
//				bluetooth->SendStr("crossroad2\n");
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
				RightCorner[0]->slope = static_cast<float>(RightCorner[1]->x - RightCorner[0]->x) / (RightCorner[0]->y - RightCorner[1]->y) * 3;
//				sprintf(buffer,"%f",slope);
//				b->SendStr(buffer);
				for(int8_t  i = RightCornerOrder[0]+1 ; i<RightCornerOrder[1] ; i++){
					RightEdge[i].x = RightCorner[0]->x + RightCorner[0]->slope*(i - RightCornerOrder[0]);
					RightEdge[i].slope = RightCorner[0]->slope;
				}
				float slope = static_cast<float>(LeftCorner[1]->x)/(59 - LeftCorner[1]->y) * 3;
				for(int8_t  i = LeftCornerOrder[1] - 1 ; i >=0 ; i--){
					LeftEdge[i].x = LeftCorner[1]->x - slope*(LeftCornerOrder[1] - i) ;
					LeftEdge[i].slope = slope;
				}
//								sprintf(buffer,"\n%d\n",RightCorner[0]->y);
//								bluetooth->SendStr(buffer);
//				bluetooth->SendStr("crossroad3\n");
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
				slope = static_cast<float>( RightCorner[1]->x-79)/(59 - RightCorner[1]->y)*3;
//				sprintf(buffer,"%f",slope);
//				b->SendStr(buffer);
				for(int8_t  i = RightCornerOrder[1]-1 ; i>=0 ; i--){
					RightEdge[i].x =RightCorner[1]->x - slope*(RightCornerOrder[1] - i);
					RightEdge[i].slope = slope;
				}
				slope = static_cast<float>(LeftCorner[1]->x )/(59 - LeftCorner[1]->y)*3;
				for(int8_t  i = LeftCornerOrder[1] - 1 ; i >= 0 ; i--){
					LeftEdge[i].x =LeftCorner[1]->x - slope*(LeftCornerOrder[1] - i);
					LeftEdge[i].slope = slope;
				}
//				bluetooth->SendStr("crossroad4\n");
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
				return Crossroad2;
			}

		}
		else {
			int8_t Base_x = (LeftCorner[0]->x + RightCorner[0]->x)/2;
			int8_t Base_y = (LeftCorner[0]->y + RightCorner[0]->y)/2;
			bool FindBlack = false;
			bool FindWhite = false;
			int8_t i=1;
			while(!FindWhite && i < Base_y - 10){
				if(GetBit(Bin,Base_x, Base_y - i) == BLACK){
					FindBlack = true;
				}
				if(FindBlack  &&  GetBit(Bin, Base_x, Base_y - i) == WHITE){
					FindWhite = true;
				}
				i++;
			}
			if(FindWhite){
//				bluetooth->SendStr("Roundabout1\n");
				return Roundabout;
			}
		}
	}

//		b->SendStr("go\n");
		return Go;
}

float FindPath(const Point* LeftEdge, const Point* RightEdge, const TrackState trackstate,Point* LeftCorner[], Point* RightCorner[],uint8_t & LeftEdgeNum,uint8_t & RightEdgeNum){
	Point* LeftCor = (LeftCorner[1])?LeftCorner[1]:NULL;
	if(LeftCor){
		if(LeftCor->y <= 30)
			LeftCor = NULL;
	}
	int row = (LeftEdgeNum > RightEdgeNum)?((RightEdgeNum > 10)?10:RightEdgeNum):((LeftEdgeNum > 10)?10:LeftEdgeNum);
	if(row == 0)   row = 1;
	int8_t Order = 0;
	float Diff=0;
	float Path[8]={0};

	FindCor = false;

	if(trackstate == Roundabout&&!RoundaboutTag){
		RoundaboutTag = true;
//		tft->SetRegion(Lcd::Rect(1,81,100,10));
//		typer->WriteString("go into roud");
//		bluetooth->SendStr("\t\tgo into roundabout\n");
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
//			tft->SetRegion(Lcd::Rect(1,81,80,10));
//			typer->WriteString("go out roud");
//			bluetooth->SendStr("\t\tgo out of roundabout\n");
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
		while(Order < row){
			float k = 0;
			if(Order < 3)
				k = 0.25;
			else if(Order < 6)
				k=0.5;
			else
				k=0.25;

			if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
				pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39.5);
			else if(LeftEdge[Order].x  != 0 ){
				if(Order < 3)
					pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39.5);
				else
					pos+= k*(LeftEdge[Order].x + OFFSET -39.5);
			}
			else if(RightEdge[Order].x != 0){
				if(Order < 3)
					pos+= k*((LeftEdge[Order].x + RightEdge[Order].x)/2-39.5);
				else
					pos+= k*(RightEdge[Order].x - OFFSET - 39.5);
			}

			if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
				slope += ((LeftEdge[Order].slope-1 + RightEdge[Order].slope+1)/2);
			else if(LeftEdge[Order].x == 0 && RightEdge[Order].x != 79 )
				slope += RightEdge[Order].slope+1;
			else if(LeftEdge[Order].x != 0 && RightEdge[Order].x == 79 )
				slope += LeftEdge[Order].slope-1;

			Order++;
		}

		slope/=row;
		pos/=row;
		Diff = slope_p * slope + pos_p * pos;
//		sprintf(buffer,"\n%d\n",Diff);
//		bluetooth->SendStr(buffer);
//		bluetooth->SendStr("\t\tout of roundabout\n");
		}
//		tft->SetRegion(Lcd::Rect(1,81,100,20));
//		typer->WriteString("out of roud");



	else if(trackstate == Roundabout){
//		FindCor = true;
		Order = 0;
		float pos = 0,slope = 0;
		while( Order < row ){
			float k=0;
			if(Order < 3)
				k = 0.25;
			else if(Order < 6)
				k=0.5;
			else
				k=0.25;

			pos += k*(LeftEdge[Order].x - 39.5 +30 ) ;

			slope += LeftEdge[Order].slope-1 ;
			Order++;
		}
		pos/=row;
		slope/=row;
		Diff = pos_p * pos + slope_p * slope;
//		bluetooth->SendStr("thisthisthis");
//		tft->SetRegion(Lcd::Rect(1,81,100,20));
//		typer->WriteString("in roud");
//		bluetooth->SendStr("\t\tin roudaboout1\n");
	}

	else if( LeftCor!=NULL){
		float pos = LeftCor->x - 39 + OFFSET - 20;
		if(Order < 3)
			pos*=0.25;
		else if(Order < 6)
			pos*=0.5;
		else
			pos*=0.25;
		Diff = pos_p * pos;
//		sprintf(buffer,"\n%d\n", LeftCor->y);
//		bluetooth->SendStr(buffer);
//		bluetooth->SendStr("\t\tin roudaboout3\n");
	}

	else {
		Order = 1;
		float slope = 0;
		float pos= 0;
		while(Order < row){
			float k = 0;
			if(Order < 3)
				k = 0.25;
			else if(Order < 6)
				k=0.5;
			else
				k=0.25;

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

			if(LeftEdge[Order].x != 0 && RightEdge[Order].x != 79)
				slope += ((LeftEdge[Order].slope-1 + RightEdge[Order].slope+1)/2);
			else if(LeftEdge[Order].x == 0 && RightEdge[Order].x != 79 )
				slope += RightEdge[Order].slope+1;
			else if(LeftEdge[Order].x != 0 && RightEdge[Order].x == 79 )
				slope += LeftEdge[Order].slope-1;

			Order++;
		}
		slope/=row;
		pos/=row;
		Diff = slope_p * slope + pos_p * pos;
//		bluetooth->SendStr("\t\tin roudaboout2\n");
	}


//	return (Diff+40)/80;
	return Diff;
}


#endif /* SRC_CAMERAHEADER_FINDEDGE_H_ */
