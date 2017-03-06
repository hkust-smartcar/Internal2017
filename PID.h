/*
 * PID.h
 *
 *  Created on: Feb 9, 2017
 *      Author: lzhangbj
 */

#include <stdint.h>
#include <cmath>

#ifndef SRC_PID_H_
#define SRC_PID_H_

class PID
{


private:

	//coefficient
	float m_kp;
	float m_ki=0;
	float m_kd=0;


	bool m_for_dir;			//forward directiomn of motor
	bool m_dir;				// used to set the motor dir

	float m_input_count;
	uint32_t m_input_power;

	int8_t m_wheel_sign;

	//output control
	uint32_t m_output_power=0;
	uint32_t m_output_power_max;
	uint32_t m_output_power_min;

	//set point
	float m_set_point=0;

	//error
	std::array<float, 2>m_error{ {0} };


	//if you do "I", set these values
	long double m_error_sum=0;
	double m_error_sum_max=0;
	double m_error_sum_min=0;

public:
	bool ConfigCo(float kp, float ki, float kd,int8_t wheelsign){
		m_kp=kp;
		m_ki=ki;
		m_kd=kd;
		m_wheel_sign = wheelsign;
		return true;
	}

	// about the dir
	void SetMotorDir(bool flag){
		m_for_dir = flag;
	}



	bool ConfigIMaxMin(float error_sum_max, float error_sum_min){
			m_error_sum_max=error_sum_max;
			m_error_sum_min=error_sum_min;
		return true;
	}

	bool ConfigOutMaxMin(uint32_t output_max, uint32_t output_min){
			m_output_power_max=output_max;
			m_output_power_min=output_min;
		return true;
	}

	bool SetPoint(float set_point){
		m_set_point=set_point;
		return true;
	}

	bool InputValue(float input_count,uint32_t input_power ){
			m_input_count=input_count;
			m_input_power=input_power;
		return true;
	}

	inline bool Update();

	//used for motor
	uint32_t GetOutPut(){
		return m_output_power;
	}

	bool GetMotorDir(){
			return m_dir;
	}

};


bool PID::Update()
{
	if(m_set_point > 0)
		m_error[1] = m_set_point - m_wheel_sign * m_input_count;
	else if(m_set_point < 0)
		m_error[1] = -m_set_point + m_wheel_sign  * m_input_count;

	m_error_sum += m_error[1];

	if(m_error_sum > m_error_sum_max)       m_error_sum = m_error_sum_max;
	else if(m_error_sum < m_error_sum_min) 	m_error_sum = m_error_sum_min;

	m_output_power = m_input_power + m_kp*m_error[1] + m_ki*m_error_sum + m_kd*(m_error[1] - m_error[0]);


	if(m_output_power > m_output_power_max) m_output_power = m_output_power_max;
	else if(m_output_power < m_output_power_min)  m_output_power = m_output_power_min;

	m_error[0] = m_error[1];

	if(m_set_point > 0){
		m_dir = m_for_dir;
	}

	else if(m_set_point < 0){
		m_dir = !m_for_dir;

	}

	return true;
}


/*
 *   outside loop
 *   -> ConfigCo
 *   ->SetMotorDir
 *   -> ConfigIMaxMin
 *   ->ConfigOutMaxMin
 *   ->SetPower
 *
 *   inside loop
 *   ->InputValue
 *   ->Update()
 *
 */



#endif /* SRC_PID_H_ */
