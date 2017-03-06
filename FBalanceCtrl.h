/*
 * FBalanceCtrl.h
 *
 *  Created on: Feb 23, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_FBALANCECTRL_H_
#define SRC_FBALANCECTRL_H_

#include "Filter.h"
#include "PID.h"

class BalanceCtrl
{
private:
	Filter* m_filter;

	PID* m_left_pid;
	PID* m_right_pid;

	float m_calculate_time_interval;
	float m_ctrl_time_interval;
	float m_ctrl_period;

	struct BalanceSpeedCo{
		float m_k1;
		float m_k2;
//		float m_k;
	}BSC;

	std::array<float,3>m_angle;
	std::array<float,3>m_angular_speed;

	float m_left_calculate_setpoint_new=0;
	float m_left_calculate_setpoint_old=0;

	float m_right_calculate_setpoint_new=0;
	float m_right_calculate_setpoint_old=0;



	float m_acceleration=0;

public:
	void SetBSC(float k1, float k2){
		BSC.m_k1=k1;
		BSC.m_k2=k2;
	}

	void Config(Filter* filter, PID* left_pid, PID* right_pid,float calculate_time_interval,float ctrl_time_interval){
		m_filter = filter;
		m_left_pid = left_pid;
		m_right_pid = right_pid;
		m_calculate_time_interval=calculate_time_interval;
		m_ctrl_time_interval = ctrl_time_interval;
		m_ctrl_period = m_calculate_time_interval / m_ctrl_time_interval;
	}

	bool UpdateFilter(){
		m_angle = m_filter->GetFilterAng();
		m_angular_speed=m_filter->GetAngSpeed();
		return true;
	}

	bool UpdateSetpoint(){
		m_acceleration = BSC.m_k1 * m_angle[2] + BSC.m_k2 * m_angular_speed[2];
		m_left_calculate_setpoint_old = m_left_calculate_setpoint_new;
		m_right_calculate_setpoint_old = m_right_calculate_setpoint_new;
		m_left_calculate_setpoint_new = m_acceleration * m_calculate_time_interval/1000;
		m_right_calculate_setpoint_new = m_acceleration * m_calculate_time_interval/1000;
		if(m_left_calculate_setpoint_new > 1000) {  m_left_calculate_setpoint_new=1000;	  m_right_calculate_setpoint_new=1000;}
		if(m_left_calculate_setpoint_new < -1000) {  m_left_calculate_setpoint_new=-1000; m_right_calculate_setpoint_new=-1000;}
		return true;
	}

	bool Update(){
		UpdateFilter();
		UpdateSetpoint();
		return  true;
	}

	static uint8_t count ;

	float GetCtrlPoint(){
		count++;
		float speed_out = m_left_calculate_setpoint_old + count*(m_left_calculate_setpoint_new - m_left_calculate_setpoint_old)/m_ctrl_period;
		count = (count == m_ctrl_period)?0:count;
		return speed_out;
	}

	float GetLeftSetPoint(){
		return m_left_calculate_setpoint_new;
	}

	float GetRightSetPoint(){
		return m_right_calculate_setpoint_new;
	}


	float GetAcceleration(){
		return m_acceleration;
	}


};



#endif /* SRC_FBALANCECTRL_H_ */
