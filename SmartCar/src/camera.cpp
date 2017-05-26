/*
 * camera.cpp
 *
 *  Created on: 2017Äê4ÔÂ24ÈÕ
 *      Author: Lee Chun Hei
 */

#include "../inc/camera.h"

namespace camera{

bool roundaboutDoubleCheck(int x,int y){
	bool findRoundaboutUpperBound=false;
	bool findRoundaboutLeftBound=false;
	int roundaboutMidPt=y;
	for(int row;row>20;row--){
		if(!camPointCheck(x,row)){
			roundaboutMidPt=(y+row)/2;
			findRoundaboutUpperBound=true;
		}
	}
	if(!findRoundaboutUpperBound){
		return false;
	}

	for(int xCoor=x;xCoor>0;xCoor--){
		if(!camPointCheck(xCoor,roundaboutMidPt)){
			findRoundaboutLeftBound=true;
		}
	}
	if(!findRoundaboutLeftBound){
		return false;
	}

	for(int xCoor=x;xCoor<camWidth;xCoor++){
		if(!camPointCheck(xCoor,roundaboutMidPt)){
			return true;
		}
	}
	return false;
}

feature prevFeature=straightRoad;

feature pathFinding(bool roundaboutTurnLeft){
	midPtFound=0;
	midPt[camHeight-4]=camWidth/2;
	bool findLeftCorner=false;
	bool findRightCorner=false;
	for(int y=camHeight-5;y>20;y--){
		if(!findLeftCorner){
			for(int x=midPt[y+1];x>0;x--){
				leftEdge[y]=x;
				if(camPointCheck(x,y)){
					break;
				}
			}
			findLeftCorner=cornerDetection(leftEdge[y],y);
		}else{
			leftEdge[y]=leftEdge[y+1];
		}

		if(!findRightCorner){
			for(int x=midPt[y+1];x<camWidth;x++){
				rightEdge[y]=x;
				if(camPointCheck(x,y)){
					break;
				}
			}
		}else{
			rightEdge[y]=rightEdge[y+1];
		}

		midPt[y]=(leftEdge[y]+rightEdge[y])/2;
		midPtFound++;

		if(findLeftCorner&&findRightCorner){
			return crossRoad;
		}

		if(leftEdge[y]==1&&rightEdge[y]!=camWidth-1&&prevFeature!=roundabout&&prevFeature!=inRoundabout){
			midPt[y-1]=0;
			midPtFound++;
			return turnLeft;
		}else if(leftEdge[y]!=1&&rightEdge[y]==camWidth-1&&prevFeature!=roundabout&&prevFeature!=inRoundabout){
			midPt[y-1]=camWidth-1;
			midPtFound++;
			return turnRight;
		}

		if(camPointCheck(midPt[y],y)){
			//check if it is in the entry of roundabout
			isRoundabout=roundaboutDoubleCheck(midPt[y],y);

			if(isRoundabout&&roundaboutTurnLeft){
				for(int x=midPt[y];x>0;x--){
					rightEdge[0]=x+1;
					if(!camPointCheck(x,y)){
						break;
					}
				}
				for(int x=rightEdge[0];x>0;x--){
					leftEdge[0]=x;
					if(camPointCheck(x,y)){
						break;
					}
				}
				midPt[0]=(rightEdge[0]+leftEdge[0])/2;
				if(prevFeature==roundabout){
					prevFeature=roundabout;
					return roundabout;
				}else if(prevFeature==inRoundabout){
					isRoundabout=false;
					prevFeature=exitRoundabout;
					return exitRoundabout;
				}
			}else if(isRoundabout&&!roundaboutTurnLeft){
				for(int x=midPt[y];x<camWidth;x++){
					leftEdge[0]=x-1;
					if(!camPointCheck(x,y)){
						break;
					}
				}
				for(int x=leftEdge[0];x<camWidth;x++){
					rightEdge[0]=x;
					if(camPointCheck(x,y)){
						break;
					}
				}
				midPt[0]=(rightEdge[0]+leftEdge[0])/2;
				if(prevFeature==roundabout){
					prevFeature=roundabout;
					return roundabout;
				}else{
					isRoundabout=false;
					prevFeature=exitRoundabout;
					return exitRoundabout;
				}
			}

			break;
		}
	}

	if(prevFeature==roundabout||prevFeature==inRoundabout){
		prevFeature=inRoundabout;
		return inRoundabout;
	}

	return notDetermine;
}

feature featureExtraction(){
	pathError=0;
	midPt[camHeight-4]=camWidth/2;
	midPtFound=0;
	bool leftCornerFound=false;
	bool rightCornerFound=false;
	for(int y=camHeight-5;y>15;y--){

		if(camPointCheck(midPt[y+1],y)){
			if(cornerDetection(midPt[y+1],y)){
				bool likelyRoundabout=false;
				for(int x=midPt[y+1];x<camWidth;x++){
					if(!camPointCheck(x,y)){
						likelyRoundabout=true;
						break;
					}
				}
				if(likelyRoundabout){
					for(int x=midPt[y+1];x>0;x--){
						if(!camPointCheck(x,y)){
							rightEdge[y]=x;
							break;
						}
					}
					for(int x=rightEdge[y];x>0;x--){
						leftEdge[y]=x;
						if(camPointCheck(x,y)){
							break;
						}
					}
					pathError=(leftEdge[y]+rightEdge[y])/2;
					midPtFound=1;
					return roundabout;
				}else{
					continue;
				}
			}
		}

		if(!leftCornerFound){
			for(int x=midPt[y+1];x>0;x--){
				leftEdge[y]=x;
				if(camPointCheck(x,y)){
					break;
				}
			}
			if(cornerDetection(leftEdge[y],y)){
//				lcd->SetRegion(Lcd::Rect(leftEdge[y]+4,y+40,5,5));
//				lcd->FillColor(Lcd::kGreen);
				leftCornerFound=true;
			}
//			if(std::abs(leftEdge[y]-leftEdge[y+1])>5&&y<camWidth-6){
//				lcd->SetRegion(Lcd::Rect(leftEdge[y+1],y,5,5));
//				lcd->FillColor(Lcd::kRed);
//				leftEdge[y]=leftEdge[y+1];
//				leftCornerFound=true;
//			}
		}else{
			leftEdge[y]=leftEdge[y+1];
		}

		if(!rightCornerFound){
			for(int x=midPt[y+1];x<camWidth;x++){
				rightEdge[y]=x;
				if(camPointCheck(x,y)){
					break;
				}
			}
			if(cornerDetection(rightEdge[y],y)){
//				lcd->SetRegion(Lcd::Rect(rightEdge[y]+4,y+40,5,5));
//				lcd->FillColor(Lcd::kGreen);
				rightCornerFound=true;
			}
//			if(std::abs(rightEdge[y]-rightEdge[y+1])>5&&y<camWidth-6){
//				lcd->SetRegion(Lcd::Rect(rightEdge[y+1],y,5,5));
//				lcd->FillColor(Lcd::kRed);
//				rightEdge[y]=rightEdge[y+1];
//				rightCornerFound=true;
//			}
		}else{
			rightEdge[y]=rightEdge[y+1];
		}

		if(leftCornerFound&&rightCornerFound){
			return notDetermine;
		}

		if(leftEdge[y]==1&&rightEdge[y]!=camWidth-1){
			midPt[y]=rightEdge[y]-y/2;
			midPtFound++;
			pathError+=midPt[y];
			if(midPt[y]<1){
				return notDetermine;
			}
		}else if(leftEdge[y]!=1&&rightEdge[y]==camWidth-1){
			midPt[y]=leftEdge[y]+y/2;
			midPtFound++;
			pathError+=midPt[y];
			if(midPt[y]>camWidth-1){
				return notDetermine;
			}
		}else{
			midPt[y]=(leftEdge[y]+rightEdge[y])/2;
			midPtFound++;
			pathError+=midPt[y];
		}
	}


	return notDetermine;
}

bool cornerDetection(int col,int row){
	if(col>3&&col<camWidth-3&&row>3&&row<camHeight-3){
		int sum=0;
		for(int y=row-3;y<row+4;y++){
			for(int x=col-3;x<col+4;x++){
				sum+=camPointCheck(x,y);
			}
		}
		return (sum>8&&sum<15);
	}else{
		return false;
	}
}

void cameraPrint(int xCoor,int yCoor){
//	lcd->SetRegion(Lcd::Rect(xCoor,yCoor,camWidth,camHeight));
//	lcd->FillBits(0x001F,0xFFFF,cam->LockBuffer(),cam->GetBufferSize()*8);
	for(Uint y=0;y<camHeight;y++){
		for(Uint x=0;x<camWidth;x++){
			lcd->SetRegion(Lcd::Rect(x+xCoor,y+yCoor,1,1));
			if(camPointCheck(x,y)==0){
				lcd->FillColor(0xFFFF);
			}else{
				lcd->FillColor(0x001F);
			}
		}
	}
}

int camPointCheck(int x,int y){
	int col=x;
	int row=y;
//	int col=transformMatrix[x][y][0];
//	int row=transformMatrix[x][y][1];
	if((camBuffer[row*camWidth/8+col/8] >> (7-(col%8))) & 1){
		return 1;
	}else{
		return 0;
	}
//	bool output;
//	Byte camByte=camBuffer[col/8+row*rawCamWidth/8];
//	switch (col%8){
//	case 0:
//		output=camByte&0x80;
//		break;
//	case 1:
//		output=camByte&0x40;
//		break;
//	case 2:
//		output=camByte&0x20;
//		break;
//	case 3:
//		output=camByte&0x10;
//		break;
//	case 4:
//		output=camByte&0x08;
//		break;
//	case 5:
//		output=camByte&0x04;
//		break;
//	case 6:
//		output=camByte&0x02;
//		break;
//	case 7:
//		output=camByte&0x01;
//		break;
//	}
//	return output;
}

}


