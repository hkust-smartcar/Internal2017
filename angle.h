/*
 * angle.h
 *
 *  Created on: Mar 13, 2017
 *      Author: lzhangbj
 */

#ifndef SRC_ANGLE_H_
#define SRC_ANGLE_H_

#include <array>
#include <cmath>

#include <libsc/mpu6050.h>

using namespace libsc;


class Angle
{
private:
	Mpu6050* m_MPU;
	std::array<float, 3>m_AngleOffset;
	float m_TimeInterval=0;

	// Tsing Hua filter
	float m_Tg;

	// Complementary filter
	float m_AccelK;
	float m_GyroK;


	// sample period = 20
	std::array<std::array<float, 3>,20> m_HistoryAccelAngles;
	std::array<float, 3>m_CurAccelAngle;

	std::array<std::array<float, 3>,20> m_HistoryGyroAngSpeed;
	std::array<float, 3>m_CurGyroAngSpeed;
	std::array<float, 3>m_GyroAngle;
	float m_GyroOffset;

	std::array<float, 3>m_Angle;


public:
	enum struct AngCalMethod
	{
		DirectAsin,
		DirectAtan,
		ThreeAxisAsin
	};

	enum struct FilterMethod
	{
		TsinghuaFilter,
		ComplementaryFilter,
		KalmanFilter
	};



	Angle(Mpu6050* mpu, float Tg, float Comple1, float Comple2, float time_interval);


	void SetOffSet(std::array<float,3> a);

	 void StoreHistory(AngCalMethod);

	 void MediumFilter();

	 void AngleCalculate(FilterMethod);

	 void CalibrateGyro(){
		 m_GyroAngle = {0};
	 }

	 void GyroOffset(float a){
		 m_GyroOffset = a;
	 }

	 std::array<float, 3> GetAccelAng(){
		 return m_CurAccelAngle;
	 }


	 std::array<float, 3> GetAngle();

	 std::array<float, 3> GetAngSpeed();
};



#endif /* SRC_ANGLE_H_ */
