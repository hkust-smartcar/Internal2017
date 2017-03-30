/*
 * SpeedCtrl.cpp
 *
 *  Created on: Mar 13 , 2017
 *      Author: lzhangbj
 */

#include "Header/SpeedCtrl.h"

uint8_t SpeedCtrl::count = 0;

SpeedCtrl::SpeedCtrl(float p, float d,float calculate_interval,float ctrl_time_interval):
							m_Kp(p), m_Kd(d), m_CalTime(calculate_interval),m_CtrlTime(ctrl_time_interval)
{
	m_Speed = 0;
	m_Kp = p;
	m_Kd = d;
	m_CtrlPeriod =m_CalTime/ m_CtrlTime;
	m_Integral = 0;
	m_delta_speed = 0;
	m_PreCalOutput = 0;
	m_CurCalOutput = 0;
	m_Output = 0;
}


void SpeedCtrl::SetSpeed(float speed){
	m_Speed = speed;
}

void SpeedCtrl::InputCount(int32_t left_count, int32_t right_count){
	m_delta_speed = m_Speed - (left_count + right_count)/2;
}

void SpeedCtrl::SpeedControl(){
	m_Integral += m_Kp * m_delta_speed * m_CalTime / 1000;
	m_PreCalOutput = m_CurCalOutput;
	m_CurCalOutput = m_Integral + m_Kd * m_delta_speed ;
	if(m_CurCalOutput > 1000) m_CurCalOutput = 1000;
	if(m_CurCalOutput < -1000) m_CurCalOutput = -1000;
}

float SpeedCtrl::SpeedControlOutput(){
	count++;
	float speed_out = m_PreCalOutput + count * (m_CurCalOutput -m_PreCalOutput)/ m_CtrlPeriod ;
	count = ( count==m_CtrlPeriod / m_CtrlTime)?0:count;
	return speed_out;
}
