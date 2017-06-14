/*
 * Complementary_Filter.h
 *
 *  Created on: 2017¦~4¤ë15¤é
 *      Author: Ray
 */

#ifndef COMPLEMENTARY_FILTER_H_
#define COMPLEMENTARY_FILTER_H_
#include <array>
#include <libsc/mpu6050.h>
#include <array>
#include <cmath>
#include <stdlib.h>

#include <libsc/system.h>


class Complementary_Filter{
	
private:
    const float deg_to_radian = 0.0174533f;
    const float radian_to_deg = 57.29578f;
    
	float q[4]={1.0,0.0,0.0,0.0};
    float roll;
    float pitch;
    float yaw;



//-------------------------------------------------------------------------------------------
// Function declarations
public:
    Complementary_Filter(void);

    //for testing purpose
    void gyro_get_angle(float gx, float gy, float gz, float dt);
    void acc_get_angle(float ax, float ay, float az);
    void mag_get_angle(float mx, float my, float mz);
    
    void complimentary_filter(float gx, float gy, float gz, float ax, float ay, float az,  float mx, float my, float mz, float dt);
    void complimentary_filter_2(float gx, float gy, float gz, float ax, float ay, float az,  float mx, float my, float mz, float dt);
    
    float getRoll() {
        return roll * radian_to_deg ;
    }
    float getPitch() {
        return pitch * radian_to_deg;
    }
    float getYaw() {
        return yaw * radian_to_deg;
    }
    float getRollRadians() {
        return roll;
    }
    float getPitchRadians() {
        return pitch;
    }
    float getYawRadians() {
        return yaw;
    }
};
#endif
