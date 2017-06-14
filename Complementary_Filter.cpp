/*
 * Complementary_Filter.cpp
 *
 *  Created on: 2017¦~4¤ë15¤é
 *      Author: Ray
 */

#include <Complementary_Filter.h>
#include <libsc/mpu6050.h>
#include <array>
#include <cmath>
#include <libsc/system.h>
#include <stdlib.h>
#define PI 3.14159265


Complementary_Filter::Complementary_Filter() {
	
	roll=0.0f;
	pitch=0.0f;
	yaw=0.0f;

}


void Complementary_Filter::gyro_get_angle(float gx, float gy, float gz, float dt){
  float w[4]={0.0,gx*deg_to_radian,gy*deg_to_radian,gz*deg_to_radian};
  float dqdt[4]={0,0,0,0};
  
  dqdt[0]=(q[0]*w[0]-q[1]*w[1]-q[2]*w[2]-q[3]*w[3])/2.0;
  dqdt[1]=(q[0]*w[1]+q[1]*w[0]+q[2]*w[3]-q[3]*w[2])/2.0;
  dqdt[2]=(q[0]*w[2]-q[1]*w[3]+q[2]*w[0]+q[3]*w[1])/2.0;
  dqdt[3]=(q[0]*w[3]+q[1]*w[2]-q[2]*w[1]+q[3]*w[0])/2.0;
  
  q[0]+=dqdt[0]*dt;
  q[1]+=dqdt[1]*dt;
  q[2]+=dqdt[2]*dt;
  q[3]+=dqdt[3]*dt;
   
  float norm = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  q[0] /= norm;
  q[1] /= norm;
  q[2] /= norm;
  q[3] /= norm;
  
//   z-y-x  --> yaw-pitch-roll
  yaw = atan2(2.0f*q[1]*q[2] -2.0f*q[0]*q[3], 2.0f*q[0]*q[0] + 2.0f*q[1]*q[1] - 1.0f)  ;
  pitch = -asin(2.0f * (q[1]*q[3] + q[0]*q[2]));
  roll = atan2(2.0f*q[2]*q[3] - 2.0f*q[0]*q[3], 2.0f*q[0]*q[0] + 2.0f*q[3]*q[3] - 1.0f);
  
//
//  roll = atan2(-2.0f*q[1]*q[3] +2.0f*q[0]*q[2], q[3]*q[3] - q[2]*q[2] - q[1]*q[1] +q[0]*q[0]) ;
//  pitch = asin(2.0f * (q[2]*q[3] + q[0]*q[1]));
//  yaw = atan2(-2.0f*q[1]*q[2] + 2.0f*q[0]*q[3], q[2]*q[2] - q[3]*q[3] - q[1]*q[1] + q[0]*q[0]);


}

void Complementary_Filter::acc_get_angle(float ax, float ay, float az){
    // Normalise accelerometer measurement
    float recipNorm=0.0;
    recipNorm = sqrt(ax * ax + ay * ay + az * az);
    ax /= recipNorm;
    ay /= recipNorm;
    az /= recipNorm;

    if(ay>0.99 && ay<-0.99){
      roll=90.0;
    }else{
      roll=asin(ay);
    }
    
    if(az<0.001 && az>0.001){
      pitch=90.0;
    }else{
     pitch=atan2(ax,az);
    }
    
}
void Complementary_Filter::mag_get_angle(float mx, float my, float mz){
  float norm=0.0;
  //normalized magnetic sensor reading
  norm=sqrt(mx*mx+my*my+mz*mz);
  mx/=norm;
  my/=norm;
  mz/=norm;

  yaw=-atan2(mx,my);
  //to do: finding the pitch 
  
}
void Complementary_Filter::complimentary_filter(float gx, float gy, float gz, float ax, float ay, float az,float mx, float my, float mz, float dt){
  float weight=0.95;
  // Normalise accelerometer measurement
  float acc_norm=0.0;
  float mag_norm=0.0;
  
  float _yaw=0.0;
  float _roll=0.0;
  float _pitch=0.0;
  
  acc_norm = sqrt(ax * ax + ay * ay + az * az);
  ax /= acc_norm;
  ay /= acc_norm;
  az /= acc_norm;
  
  //normalized magnetic sensor reading
  mag_norm=sqrt(mx*mx+my*my+mz*mz);
  mx/=mag_norm;
  my/=mag_norm;
  mz/=mag_norm;
  
  _yaw=-atan2(my,mx);
  _pitch=atan2(ax,az);
  _roll=asin(ay);
  
  yaw=_yaw*(1-weight)+(yaw-gz*dt*deg_to_radian)*weight;
  pitch=_pitch*(1-weight)+(pitch-gy*dt*deg_to_radian)*weight;
  roll=_roll*(1-weight)+(roll+gx*dt*deg_to_radian)*weight;
 
}

void Complementary_Filter::complimentary_filter_2(float gx, float gy, float gz, float ax, float ay, float az,float mx, float my, float mz, float dt){
  float w[4]={0.0,-gx*deg_to_radian,-gy*deg_to_radian,-gz*deg_to_radian};
  float dqdt[4]={0,0,0,0};
  
  float weight=0.90;

  dqdt[0]=(q[0]*w[0]-q[1]*w[1]-q[2]*w[2]-q[3]*w[3])/2.0;
  dqdt[1]=(q[0]*w[1]+q[1]*w[0]+q[2]*w[3]-q[3]*w[2])/2.0;
  dqdt[2]=(q[0]*w[2]-q[1]*w[3]+q[2]*w[0]+q[3]*w[1])/2.0;
  dqdt[3]=(q[0]*w[3]+q[1]*w[2]-q[2]*w[1]+q[3]*w[0])/2.0;
  
  
  // Normalise accelerometer measurement
  float acc_norm=0.0;
  acc_norm = sqrt(ax * ax + ay * ay + az * az);
  ax /= acc_norm;
  ay /= acc_norm;
  az /= acc_norm;
  
  //normalized magnetic sensor reading
  float mag_norm=0.0;
  mag_norm=sqrt(mx*mx+my*my+mz*mz);
  mx/=mag_norm;
  my/=mag_norm;
  mz/=mag_norm;

  float _roll=0.0;
  float _pitch=0.0;
  float _yaw=0.0;
  
  _roll=asin(ay);
  _pitch=atan2(ax,az);
  _yaw=-atan2(my,mx);

   float _q[4]={0.0, 0.0, 0.0, 0.0};

   _q[0]=cos(_roll/2)*cos(_pitch/2)*cos(_yaw/2)-sin(_roll/2)*sin(_pitch/2)*sin(_yaw/2);
   _q[1]=cos(_roll/2)*sin(_pitch/2)*cos(_yaw/2)-sin(_roll/2)*cos(_pitch/2)*sin(_yaw/2);
   _q[2]=cos(_roll/2)*sin(_pitch/2)*sin(_yaw/2)+sin(_roll/2)*cos(_pitch/2)*cos(_yaw/2);
   _q[3]=cos(_roll/2)*cos(_pitch/2)*sin(_yaw/2)+sin(_roll/2)*sin(_pitch/2)*cos(_yaw/2);
   
  q[0]=_q[0]*(1-weight)+(q[0]-dqdt[0]*dt)*weight;
  q[1]=_q[1]*(1-weight)+(q[1]-dqdt[1]*dt)*weight;
  q[2]=_q[2]*(1-weight)+(q[2]-dqdt[2]*dt)*weight;
  q[3]=_q[3]*(1-weight)+(q[3]-dqdt[3]*dt)*weight;


  float norm = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  q[0] /= norm;
  q[1] /= norm;
  q[2] /= norm;
  q[3] /= norm;
  
  //   z-y-x  --> yaw-pitch-roll
  roll = atan2(-2.0f*q[1]*q[3] +2.0f*q[0]*q[2], q[3]*q[3] - q[2]*q[2] - q[1]*q[1] +q[0]*q[0]) ;
  pitch = asin(2.0f * (q[2]*q[3] + q[0]*q[1]));
  yaw = atan2(-2.0f*q[1]*q[2] + 2.0f*q[0]*q[3], q[2]*q[2] - q[3]*q[3] - q[1]*q[1] + q[0]*q[0]);

  
}


