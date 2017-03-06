/*
 * filter.h
 *
 *  Created on: Feb 22, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_FILTER_H_
#define SRC_FILTER_H_

#include <libsc/mpu6050.h>

using namespace libsc;


class Filter
{
private:
	Mpu6050* m_mpu;
	float m_angle_offset;
	float m_time_interval=0;
	float m_Tg;



	// sample period = 20
	std::array<std::array<float, 3>,20>m_history_accel_angles = {};
	std::array<float, 3>m_pre_accel_angle {{0}};
	std::array<float, 3>m_cur_accel_angle;

	std::array<float, 3>m_cur_gyro_angular_speed;
	std::array<float, 3>m_gyro_angle{{0}};

	std::array<float, 3>m_filter_angle{{0}};


public:
	void Config(Mpu6050* mpu, float Tg, float time_interval,float offset_angle){
		m_mpu = mpu;
		m_Tg = Tg;
		m_angle_offset=offset_angle;
		m_time_interval = time_interval;
	}

	inline bool UpdateAccelAng(int);

	inline bool StoreHistory();

	inline bool FilterAccelAng();

	inline bool UpdateGyroAng();

	inline bool UpdateGyroAngSpeed();

	inline bool UpdateFilterAng();



	inline std::array<float,3>  GetFilterAng();
	inline std::array<float,3>  GetAngSpeed()		{  return m_cur_gyro_angular_speed;  }
	inline std::array<float, 3> GetAccelAngle()		{  return m_cur_accel_angle;         }
};

/*******************************************************************************/


bool Filter::UpdateAccelAng(int flag){
	if(flag == 1 ){
		float factor = 4096 ;
		std::array<int32_t, 3>raw_accel_angle = m_mpu->GetAccel();
		for(int i = 0 ; i < 3 ; i++){
			float t = raw_accel_angle[i] / factor;
			if( t > 1 )  t = 1;
			else if ( t  < -1 )	t = -1;
			m_cur_accel_angle[i] = asin(t) / M_PI * 180;
		}
	}
	else if(flag == 2){
		std::array<int32_t, 3>raw_accel_angle = m_mpu->GetAccel();
		for(int i = 0 ; i < 3 ; i++){
			m_cur_accel_angle[i] = atan( raw_accel_angle[2] /static_cast<float>(raw_accel_angle[0])) / M_PI * 180;
		}
	}
	else if(flag == 3){
		std::array<int32_t, 3>raw_accel_angle = m_mpu->GetAccel();
		for(int i = 0 ; i < 3 ; i++){
			float t = raw_accel_angle[i]/sqrt( pow(raw_accel_angle[(i-1)%3],2)
												+ pow(raw_accel_angle[i],2)
													+ pow(raw_accel_angle[(i+1)%3],2));
			m_cur_accel_angle[i] =asin(t)/M_PI*180;
		}
	}
	return true;
}


bool Filter::StoreHistory(){
	for(int i = 0 ; i < 20 ; i++){
		m_history_accel_angles[i] = m_history_accel_angles[i+1];
	}
	m_history_accel_angles[19] = m_cur_accel_angle;
	return true;
}

bool Filter::FilterAccelAng(){
	std::array<float,3>angle_sum {{0}};
	uint8_t sample_period = 20;
	for(int j = 0 ; j < 3 ; j ++ ){
		for(int i = 0 ; i < sample_period ; i ++){
			angle_sum[j] += m_history_accel_angles[i][j];
		}
	}
	for(int i = 0 ; i < 3 ; i++ ){
		m_cur_accel_angle[i] = angle_sum[i] / sample_period	;
	}
	return true;
}


bool Filter::UpdateGyroAngSpeed(){
	float factor =5248.0;										// also this
	std::array<int32_t, 3>raw_gyro_angular_speed = m_mpu->GetOmega();
	for(int i = 0 ; i < 3 ; i++){
		m_cur_gyro_angular_speed[i]=raw_gyro_angular_speed[(i-1)%3]/factor;
	}
	return true;
}

bool Filter::UpdateGyroAng(){
	for(int i = 0 ; i < 3 ; i++){
		m_gyro_angle[i] += (m_cur_gyro_angular_speed[i] * m_time_interval/1000);
	}
	return true;
}


bool Filter::UpdateFilterAng(){
	for(int i = 0 ; i < 3 ; i++){
		if(i != 2)
		m_filter_angle[i] += (
								(m_cur_accel_angle[i] - m_filter_angle[i])/m_Tg
								//use accelerator to calibrate gyroscope
								+ m_cur_gyro_angular_speed[i]
												)*m_time_interval/1000;  //integral
		else
		m_filter_angle[i] +=(
								(m_cur_accel_angle[i] - m_filter_angle[i] - m_angle_offset)/m_Tg
								//use accelerator to calibrate gyroscope
								+ m_cur_gyro_angular_speed[i]
												)*m_time_interval/1000;  //integral
	}
	return true;
}


std::array<float,3> Filter::GetFilterAng(){
	return m_filter_angle;
}


#endif /* SRC_FILTER_H_ */
