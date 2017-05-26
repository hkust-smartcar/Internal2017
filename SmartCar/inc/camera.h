/*
 * camera.h
 *
 *  Created on: 2017Äê4ÔÂ24ÈÕ
 *      Author: Lee Chun Hei
 */

#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#include "../inc/global.h"

namespace camera{

enum feature{
	roundabout,
	inRoundabout,
	exitRoundabout,
	crossRoad,
	turnLeft,
	turnRight,
	straightRoad,
	straightRoadSlowDown,
	notDetermine
};

bool roundaboutDoubleCheck(int x,int y);
feature pathFinding(bool roundaboutTurnLeft);
feature featureExtraction();
bool cornerDetection(int col,int row);
void cameraPrint(int xCoor,int yCoor);
int camPointCheck(int x,int y);

}

#endif /* INC_CAMERA_H_ */
