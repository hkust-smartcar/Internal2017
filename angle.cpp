/*
 * angle.cpp
 *
 *  Created on: Mar 13, 2017
 *      Author: lzhangbj
 */



#include "Header/angle.h"

Angle::Angle(Mpu6050* mpu, float Tg,float Comple1, float Comple2, float time_interval){
		m_MPU = mpu;
		m_Tg = Tg;
		m_AccelK = Comple1;
		m_GyroK = Comple2;
		m_AngleOffset = {0};
		m_TimeInterval = time_interval;
		std::array<std::array<float, 3>,20> m_HistoryAccelAngles{};
		std::array<float, 3>m_CurAccelAngle{0};
		std::array<std::array<float, 3>,20> m_HistoryGyroAngSpeed{};
		std::array<float, 3>m_CurGyroAngSpeed{0};
		std::array<float, 3>m_Angle{0};
		std::array<float, 3>m_GyroAngle{0};
		m_GyroOffset = 0;
	}



void Angle::StoreHistory(AngCalMethod method){
	//first, convert raw data into readable ones

		// for accelerator
	if( method == AngCalMethod::DirectAsin){
			float factor = 4096 ;
			std::array<int32_t, 3>raw_accel_angle = m_MPU->GetAccel();
			for(int i = 0 ; i < 3 ; i++){
				float t = raw_accel_angle[i] / factor;
				if( t > 1 )  t = 1;
				else if ( t  < -1 )	t = -1;
				m_CurAccelAngle[i] = asin(t) / M_PI * 180;
			}
		}
		else if(method == AngCalMethod::DirectAtan){
			std::array<int32_t, 3>raw_accel_angle = m_MPU->GetAccel();
			for(int i = 0 ; i < 3 ; i++){
				m_CurAccelAngle[i] = atan( raw_accel_angle[2] /static_cast<float>(raw_accel_angle[0])) /(float) M_PI * 180;
			}
		}
		else if(method == AngCalMethod::ThreeAxisAsin){
			std::array<int32_t, 3>raw_accel_angle = m_MPU->GetAccel();
			for(int i = 0 ; i < 3 ; i++){
				float t = raw_accel_angle[i]/sqrt( pow(raw_accel_angle[(i-1)%3],2)
													+ pow(raw_accel_angle[i],2)
														+ pow(raw_accel_angle[(i+1)%3],2));
				if(t>1) t=1;
				if(t<-1) t=-1;

				m_CurAccelAngle[i] =asin(t)/M_PI*180;
			}
		}

		// for gyro
	float gyroFactor = 5248.0;
	for(int i = 0 ; i < 3 ; i ++){
		m_CurGyroAngSpeed[i] = static_cast<float>(m_MPU->GetOmega()[(i-1)%3])/gyroFactor;
	}
		//second, store them into history values

	for(int i = 0 ; i < 19 ; i++){
		m_HistoryGyroAngSpeed[i] = m_HistoryGyroAngSpeed[i+1];
		m_HistoryAccelAngles[i] = m_HistoryAccelAngles[i+1];
	}
	m_HistoryAccelAngles[19] = m_CurAccelAngle;
	m_HistoryGyroAngSpeed[19] = m_CurGyroAngSpeed;
}


void Angle::MediumFilter(){
	std::array<float,3>angle_sum {{0}};
	std::array<float,3>anglespeed_sum{{0}};
	uint8_t sample_period = 20;
	for(int j = 0 ; j < 3 ; j ++ ){
		for(int i = 0 ; i < sample_period ; i ++){
			angle_sum[j] += m_HistoryAccelAngles[i][j];
			anglespeed_sum[j] += m_HistoryGyroAngSpeed[i][j];
		}
	}
	for(int i = 0 ; i < 3 ; i++ ){
		m_CurAccelAngle[i] = angle_sum[i] / sample_period;
		m_CurGyroAngSpeed[i] = anglespeed_sum[i] / sample_period;
	}
}


void Angle::AngleCalculate(FilterMethod method){
	if(method == FilterMethod::TsinghuaFilter)
		for(int i = 0 ; i < 3 ; i++){
			m_Angle[i] += (	(m_CurAccelAngle[i] - m_Angle[i] )/m_Tg
							//use accelerator to calibrate gyroscope
							+ m_CurGyroAngSpeed[i]
											)*m_TimeInterval/1000;  //integral

		}

	else if(method == FilterMethod::ComplementaryFilter){
		for(int i = 0 ; i < 3 ; i ++){
			m_GyroAngle[i] += ( m_CurGyroAngSpeed[i]*m_TimeInterval/1000);
			m_Angle[i] = m_AccelK * m_CurAccelAngle[i] + m_GyroK * (m_GyroAngle[i]-m_GyroOffset);
		}
	}
	else if(method == FilterMethod::KalmanFilter){
	}

}
std::array<float, 3> Angle::GetAngle(){
	std::array<float,  3> angle;
	for(int i  = 0 ; i < 3 ; i++ )
		angle[i] = m_Angle[i] - m_AngleOffset[i];
	return angle;
}

void Angle::SetOffSet(std::array<float,3> a){
	m_AngleOffset = a;
}

std::array<float, 3> Angle::GetAngSpeed(){
	return m_CurGyroAngSpeed;
}


