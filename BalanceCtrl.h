/*
 * BalanceCtrl.h
 *
 *  Created on: Mar 13, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_BALANCECTRL_H_
#define SRC_BALANCECTRL_H_

#include <array>

#include "angle.h"
#include "PID.h"

class BalanceCtrl
{
private:
	uint8_t m_CalTime;
	uint8_t m_CtrlTime;
	uint8_t m_CtrlPeriod;

	float m_Kp;
	float m_Kd;

	// for this class , only need the ANGLE and ANGULAR_Velocity of Z axis;
	float m_Angle;
	float m_AngSpeed;

	float m_CurCalOutput;
	float m_PreCalOutput;

	float m_CtrlOutput;

public:
	static uint8_t count;

	BalanceCtrl(uint8_t calculate_time_interval,uint8_t ctrl_time_interval, float kp, float kd);
	void SetK(float,float);

	void InputData(float angle, float angleSpeed);

	void OutputCalculate();

	float AngleControl();

	float GetOutput(){
		return m_CtrlOutput;
	}

};



#endif /* SRC_BALANCECTRL_H_ */
