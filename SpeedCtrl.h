/*
 * SpeedCtrl.h
 *
 *  Created on: Mar 13 , 2017
 *      Author: lzhangbj
 */

#ifndef SRC_SPEEDCTRL_H_
#define SRC_SPEEDCTRL_H_

#include <cstdint>

class SpeedCtrl
{
private:

	uint8_t m_CalTime;
	uint8_t m_CtrlTime;
	uint8_t m_CtrlPeriod;

	float m_Speed;

	float m_Kp;
	float m_Kd;
	float m_Integral;

	float m_delta_speed;

	float m_PreCalOutput;
	float m_CurCalOutput;

	float m_Output;



public:

	SpeedCtrl(float p, float d, float calculate_interval,float ctrl_time_interval);

	void SetSpeed(float speed);

	void SetK(float kp, float kd){
		m_Kp = kp;
		m_Kd = kd;
		m_Integral = 0;
	}

	void InputCount(int32_t left_count, int32_t right_count);

	void SpeedControl();

	float SpeedControlOutput();

	static uint8_t count;

};



#endif /* SRC_SPEEDCTRL_H_ */
