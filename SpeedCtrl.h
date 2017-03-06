/*
 * SpeedCtrl.h
 *
 *  Created on: Feb 28, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_SPEEDCTRL_H_
#define SRC_SPEEDCTRL_H_

#include "FBalanceCtrl.h"

class SpeedCtrl
{
private:

	float m_calculate_time_interval;
	float m_ctrl_time_interval;
	uint8_t m_ctrl_period;
	float m_aim_speed;

	float m_p;
	float m_d;
	float m_speed_integral = 0;

	float m_delta_speed;

	float m_calculate_speed_old=0;
	float m_calculate_speed_new=0;



public:

	void Config( float AimSpeed, float p, float d,float calculate_interval,float ctrl_time_interval){
		m_calculate_time_interval = calculate_interval;
		m_ctrl_time_interval = ctrl_time_interval;
		m_aim_speed = AimSpeed;
		m_p = p;
		m_d = d;
		m_ctrl_period =m_calculate_time_interval / m_ctrl_time_interval;
	}

	void SetSpeed(float speed){
		m_aim_speed = speed;
	}

	void GetCount(int32_t left_count, int32_t right_count){
		m_delta_speed = m_aim_speed - (left_count + right_count)/2;
	}

	void Update(){
		m_speed_integral += m_p*m_delta_speed * m_calculate_time_interval/1000;
		m_calculate_speed_old = m_calculate_speed_new;
		m_calculate_speed_new = m_speed_integral + m_d * m_delta_speed ;
	}

	static uint8_t count;

	float GetSpeedOut(){
		count++;
		float speed_out =  count*(m_calculate_speed_new - m_calculate_speed_old)/ m_ctrl_period + m_calculate_speed_old;
		count = ( count==m_ctrl_period / m_ctrl_time_interval)?0:count;
		return speed_out;
	}

};



#endif /* SRC_SPEEDCTRL_H_ */
