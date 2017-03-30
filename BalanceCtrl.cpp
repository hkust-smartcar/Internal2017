/*
 * BalanceCtrl.cpp
 *
 *  Created on: Mar 13, 2017
 *      Author: lzhangbj
 */

#include "Header/BalanceCtrl.h"

uint8_t BalanceCtrl::count = 0;


BalanceCtrl::BalanceCtrl(uint8_t calculate_time_interval,uint8_t ctrl_time_interval,float kp, float kd):
							m_CalTime(calculate_time_interval), m_CtrlTime(ctrl_time_interval),
							m_Kp(kp), m_Kd(kd)
{
	m_CtrlPeriod = m_CalTime / m_CtrlTime;
	m_Angle = {};
	m_AngSpeed = {};
	m_CurCalOutput = {};
	m_PreCalOutput = {};
	m_CtrlOutput = {};
}

void BalanceCtrl::SetK(float k1, float k2){
	m_Kp = k1;
	m_Kd = k2;
}
void BalanceCtrl::InputData(float angle, float angleSpeed){
	m_Angle = angle;
	m_AngSpeed = angleSpeed;
}

void BalanceCtrl::OutputCalculate(){
	float acceleration;
	acceleration = m_Kp * m_Angle - m_Kd * m_AngSpeed;

	m_PreCalOutput = m_CurCalOutput;
	m_CurCalOutput = acceleration * m_CalTime / 1000;

	if(m_CurCalOutput > 800) m_CurCalOutput = 800;
	if(m_CurCalOutput < -800) m_CurCalOutput = -800;
}

float BalanceCtrl::AngleControl(){
	count++;
	m_CtrlOutput = m_PreCalOutput +  (static_cast<float>(count) / m_CtrlPeriod * (m_CurCalOutput - m_PreCalOutput) );
	if(count == m_CtrlPeriod) count = 0;
	return m_CurCalOutput;
}

