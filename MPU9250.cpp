/*
 * mpu6050.cpp
 *
 * Author: Harrison Ng
 * Copyright (c) 2014-2015 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include <cassert>
#include <cmath>
#include <cstdint>

#include <array>
#include <vector>

#include "libbase/log.h"
#include "libbase/helper.h"
#include LIBBASE_H(i2c_master)
#include LIBBASE_H(soft_i2c_master)

#include "libsc/config.h"
#include "libsc/device_h/mpu6050.h"
#include "libsc/mpu6050.h"
#include "libsc/system.h"
#include "libutil/misc.h"
#include "MPU9250.h"

#define ABS(v) ((v < 0)? -v : v)

using namespace LIBBASE_NS;
using namespace std;

namespace libsc
{

#ifdef LIBSC_USE_MPU6050

namespace
{

Mpu6050::I2cMaster::Config GetI2cConfig()
{
	Mpu6050::I2cMaster::Config config;
	config.scl_pin = LIBSC_MPU6050_SCL;
	config.sda_pin = LIBSC_MPU6050_SDA;
	config.baud_rate_khz = 400;
	config.scl_low_timeout = 1000;
#if !LIBSC_USE_SOFT_MPU6050
	config.min_scl_start_hold_time_ns = 600;
	config.min_scl_stop_hold_time_ns = 600;
#endif
	return config;
}

}

MPU9250::MPU9250(const Config &config)
		:
		  m_gyro_range(config.gyro_range),
		  m_accel_range(config.accel_range),
		  m_magnetic_range(config.magnetic_range),
		  m_mode(config.Mmode)
{
	if(!config.i2c_master_ptr){
		m_i2c = new I2cMaster(GetI2cConfig());
	}else{
		m_i2c = config.i2c_master_ptr;
	}

	getAres();
	getGres();
	getMres();

	assert(Verify());
	assert(Verify_mag());

	initMPU9250();
	initAK8963(magAdjustment);

}

bool MPU9250::Verify()
{
	Byte who_am_i;
	if (!m_i2c->GetByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250, &who_am_i))
	{
		return false;
	}
	else
	{
		return (who_am_i == 0x71);
	}
}

bool MPU9250::Verify_mag()
{
	Byte who_am_i;
	if (!m_i2c->GetByte(AK8963_ADDRESS, WHO_AM_I_AK8963, &who_am_i))
	{
		return false;
	}
	else
	{
		return (who_am_i == 0x48);
	}
}

void MPU9250::initMPU9250()
{

	// wake up device
	// Clear sleep mode bit (6), enable all sensors
	assert(m_i2c->SendByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00));
	System::DelayUs(1); // Wait for all registers to reset

	// Get stable time source
	// Auto select clock source to be PLL gyroscope reference if ready else
	assert(m_i2c->SendByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x01));
	System::DelayUs(1);

	// Configure Gyro and Thermometer
	// Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz,
	// respectively;
	// minimum delay time for this setting is 5.9 ms, which means sensor fusion
	// update rates cannot be higher than 1 / 0.0059 = 170 Hz
	// DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
	// With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!),
	// 8 kHz, or 1 kHz

	// 0x03 -> gyro low pass filter BW=41Hz, Delay=5.9ms ; temperature low pass filter BW=42Hz, Delay4.8ms
	// 0x00 -> gyro low pass filter BW=250Hz, Delay=0.97ms ; temperature low pass filter BW=4000Hz, Delay0.04ms
	assert(m_i2c->SendByte(MPU9250_ADDRESS, CONFIG, 0x03));

	// Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
	// Use a 200 Hz rate; a rate consistent with the filter update rate
	// determined inset in CONFIG above.
	assert(m_i2c->SendByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x04));

	// Set gyroscope full scale range
	// Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are
	// left-shifted into positions 4:3

	uint8_t c = 0x00;
	c = c |  0x00; // enable DLPF -> Fchoice_b=0x00 or disable -> Fchoice_b=0x02
	c = c | static_cast<int>(m_gyro_range) << 3; // Set full scale range for the gyro

	// Write new GYRO_CONFIG value to register
	assert(m_i2c->SendByte(MPU9250_ADDRESS, GYRO_CONFIG, c ));

	// Set accelerometer full-scale range configuration

	c = 0x00;
	c = c | static_cast<int>(m_accel_range) << 3; // Set full scale range for the accelerometer
	// Write new ACCEL_CONFIG register value
	assert(m_i2c->SendByte(MPU9250_ADDRESS, ACCEL_CONFIG, c));

	// Set accelerometer sample rate configuration
	// It is possible to get a 4 kHz sample rate from the accelerometer by
	// choosing 1 for accel_fchoice_b bit [3]; in this case the bandwidth is
	// 1.13 kHz
	c = 0x00;
	c = c | 0x00; // enable DLPF -> accel_fchoice_b=0x00 or disable -> Fchoice_b=0x04
	c = c | 0x03; // 0x03 -> gyro low pass filter BW=41Hz, Delay=11.8ms ;
	assert(m_i2c->SendByte(MPU9250_ADDRESS, ACCEL_CONFIG2, c));
	// The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
	// but all these rates are further reduced by a factor of 5 to 200 Hz because
	// of the SMPLRT_DIV setting
}

void MPU9250::initAK8963(float *destination)
{
	// First extract the factory calibration for each magnetometer axis

	assert(m_i2c->SendByte(AK8963_ADDRESS, AK8963_CNTL, 0x00)); // Power down magnetometer
	System::DelayUs(1);
	assert(m_i2c->SendByte(AK8963_ADDRESS, AK8963_CNTL, 0x0F)); // Enter Fuse ROM access mode
	System::DelayUs(1);

	// Read the x-, y-, and z-axis calibration values

	const vector<Byte> &rawData=m_i2c->GetBytes(AK8963_ADDRESS, AK8963_ASAX, 3);

	// Return x-axis sensitivity adjustment values, etc.
	destination[0] =  (float)(rawData[0] - 128)/256. + 1.;
	destination[1] =  (float)(rawData[1] - 128)/256. + 1.;
	destination[2] =  (float)(rawData[2] - 128)/256. + 1.;
	assert(m_i2c->SendByte(AK8963_ADDRESS, AK8963_CNTL, 0x00)); // Power down magnetometer
	System::DelayUs(1);

	// Configure the magnetometer for continuous read and highest resolution.
	// Set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL
	// register, and enable continuous mode data acquisition Mmode (bits [3:0]),
	// 0010 for 8 Hz and 0110 for 100 Hz sample rates.

	// Set magnetometer data resolution and sample ODR
	assert(m_i2c->SendByte(AK8963_ADDRESS, AK8963_CNTL, static_cast<int>(m_magnetic_range) << 4 | static_cast<int>(m_mode)));
	System::DelayUs(1);
}

void MPU9250::gyro_calibration()
{
	int num_of_samples = 256;
	int count = 0;
	float temp[3]={0.0, 0.0, 0.0};
	float temp_total[3]={0.0, 0.0, 0.0};

	while(count < num_of_samples){
		System::DelayUs(50);
		readGyroData(temp);
		temp_total[0]+=temp[0];
		temp_total[1]+=temp[1];
		temp_total[2]+=temp[2];
		count++;
	}
	gyroCalibration[0]=temp_total[0]/count;
	gyroCalibration[1]=temp_total[1]/count;
	gyroCalibration[2]=temp_total[2]/count;
}

void MPU9250::getMres()
{
	switch (m_magnetic_range)
	{
	default:
		LOG_EL("MPU9250 Gyro in illegal state");
	case Config::Bit::MFS_14BITS:
		mRes = 4912.0f / 8190.0f; // Proper scale to return mircroTesla
		break;
	case Config::Bit::MFS_16BITS:
		mRes = 4912.0f / 32760.0f; // Proper scale to return mircroTesla
		break;
	}
}

void MPU9250::getAres()
{
	switch (m_accel_range)
	{
	default:
		LOG_EL("MPU9250 Accel in illegal state");
	case Config::Range::kSmall:
		aRes = 2.0f / 32768.0f;

	case Config::Range::kMid:
		aRes = 4.0f / 32768.0f;

	case Config::Range::kLarge:
		aRes = 8.0f / 32768.0f;

	case Config::Range::kExtreme:
		aRes = 16.0f / 32768.0f;
	}
}

void MPU9250::getGres()
{
	switch (m_gyro_range)
	{
	default:
		LOG_EL("MPU9250 Accel in illegal state");
	case Config::Range::kSmall:
      gRes = 250.0f / 32768.0f;
      break;
    case Config::Range::kMid:
      gRes = 500.0f / 32768.0f;
      break;
    case Config::Range::kLarge:
      gRes = 1000.0f / 32768.0f;
      break;
    case Config::Range::kExtreme:
      gRes = 2000.0f / 32768.0f;
      break;
	}
}

void MPU9250::readAccData(float * destination)
{

	// x/y/z accel register data stored here
	// Read the six raw data registers into data array
	const vector<Byte> &rawData=m_i2c->GetBytes(MPU9250_ADDRESS, ACCEL_XOUT_H, 6);

	// Turn the MSB and LSB into a signed 16-bit value
	destination[0] = (float)((int16_t)((rawData[0] << 8) | rawData[1])) * aRes;
    destination[1] = (float)((int16_t)((rawData[2] << 8) | rawData[3])) * aRes;
    destination[2] = (float)((int16_t)((rawData[4] << 8) | rawData[5])) * aRes;


}


void MPU9250::readGyroData(float * destination)
{

	const vector<Byte> &rawData=m_i2c->GetBytes(MPU9250_ADDRESS, GYRO_XOUT_H, 6);  // x/y/z gyro register data stored here

	// Turn the MSB and LSB into a signed 16-bit value
	destination[0] = (float)((int16_t)((rawData[0] << 8) | rawData[1])) * gRes - gyroCalibration[0];
	destination[1] = (float)((int16_t)((rawData[2] << 8) | rawData[3])) * gRes - gyroCalibration[1];
	destination[2] = (float)((int16_t)((rawData[4] << 8) | rawData[5])) * gRes - gyroCalibration[2];

}

void MPU9250::readMagData(float * destination)
{
	Byte is_set;
	// x/y/z gyro register data, ST2 register stored here, must read ST2 at end
	// of data acquisition
	const vector<Byte> &rawData=m_i2c->GetBytes(AK8963_ADDRESS, AK8963_XOUT_L, 7);


	m_i2c->GetByte(AK8963_ADDRESS, AK8963_ST1,&is_set);
	// Wait for magnetometer data ready bit to be set
	if (is_set & 0x01){

	// Read the six raw data and ST2 registers sequentially into data array

		uint8_t c = rawData[6]; // End data read by reading ST2 register
		// Check if magnetic sensor overflow set, if not then report data
		if (!(c & 0x08)){
			  // Turn the MSB and LSB into a signed 16-bit value
			  destination[0] = (float)((int16_t)((rawData[1] << 8) | rawData[0])) * mRes*magAdjustment[0];
			  destination[1] = (float)((int16_t)((rawData[3] << 8) | rawData[2])) * mRes*magAdjustment[1];
			  destination[2] = (float)((int16_t)((rawData[5] << 8) | rawData[4])) * mRes*magAdjustment[2];

			}
	}

}

float MPU9250::readTempData()
{

	const vector<Byte> &rawData=m_i2c->GetBytes(MPU9250_ADDRESS, TEMP_OUT_H, 2); // temperature raw data
	int16_t temp_raw;
	// Read the two raw data registers sequentially into data array

	// Turn the MSB and LSB into a 16-bit value
	temp_raw=(int16_t)((rawData[0] << 8) | rawData[1]);

	return ((float)temp_raw)/333.87+21.0;

}
// ******************** for calibration purpose ************************
// remove it if you don't need it or you can calibrate yourself
void MPU9250::acc_calibration(float * output){
  readAccData(acc_raw);
  moving_average_acc_x[3]=moving_average_acc_x[2];
  moving_average_acc_x[2]=moving_average_acc_x[1];
  moving_average_acc_x[1]=moving_average_acc_x[0];
  moving_average_acc_x[0]=acc_raw[0];

  moving_average_acc_y[3]=moving_average_acc_y[2];
  moving_average_acc_y[2]=moving_average_acc_y[1];
  moving_average_acc_y[1]=moving_average_acc_y[0];
  moving_average_acc_y[0]=acc_raw[1];

  moving_average_acc_z[3]=moving_average_acc_z[2];
  moving_average_acc_z[2]=moving_average_acc_z[1];
  moving_average_acc_z[1]=moving_average_acc_z[0];
  moving_average_acc_z[0]=acc_raw[2];

  output[0]=(moving_average_acc_x[3]+moving_average_acc_x[2]+moving_average_acc_x[1]+moving_average_acc_x[0])/4;
  output[1]=(moving_average_acc_y[3]+moving_average_acc_y[2]+moving_average_acc_y[1]+moving_average_acc_y[0])/4;
  output[2]=(moving_average_acc_z[3]+moving_average_acc_z[2]+moving_average_acc_z[1]+moving_average_acc_z[0])/4;
}

void MPU9250::mag_calibration(float * output){
  readMagData(mag_raw);
  moving_average_mag_x[3]=moving_average_mag_x[2];
  moving_average_mag_x[2]=moving_average_mag_x[1];
  moving_average_mag_x[1]=moving_average_mag_x[0];
  moving_average_mag_x[0]=mag_raw[0];

  moving_average_mag_y[3]=moving_average_mag_y[2];
  moving_average_mag_y[2]=moving_average_mag_y[1];
  moving_average_mag_y[1]=moving_average_mag_y[0];
  moving_average_mag_y[0]=mag_raw[1];

  moving_average_mag_z[3]=moving_average_mag_z[2];
  moving_average_mag_z[2]=moving_average_mag_z[1];
  moving_average_mag_z[1]=moving_average_mag_z[0];
  moving_average_mag_z[0]=mag_raw[2];

  output[0]=(moving_average_mag_x[3]+moving_average_mag_x[2]+moving_average_mag_x[1]+moving_average_mag_x[0])/4;
  output[1]=(moving_average_mag_y[3]+moving_average_mag_y[2]+moving_average_mag_y[1]+moving_average_mag_y[0])/4;
  output[2]=(moving_average_mag_z[3]+moving_average_mag_z[2]+moving_average_mag_z[1]+moving_average_mag_z[0])/4;
}

void MPU9250::gyro_calibration(float * output){
  readGyroData(gyro_raw);
  moving_average_gyro_x[3]=moving_average_gyro_x[2];
  moving_average_gyro_x[2]=moving_average_gyro_x[1];
  moving_average_gyro_x[1]=moving_average_gyro_x[0];
  moving_average_gyro_x[0]=gyro_raw[0];

  moving_average_gyro_y[3]=moving_average_gyro_y[2];
  moving_average_gyro_y[2]=moving_average_gyro_y[1];
  moving_average_gyro_y[1]=moving_average_gyro_y[0];
  moving_average_gyro_y[0]=gyro_raw[1];

  moving_average_gyro_z[3]=moving_average_gyro_z[2];
  moving_average_gyro_z[2]=moving_average_gyro_z[1];
  moving_average_gyro_z[1]=moving_average_gyro_z[0];
  moving_average_gyro_z[0]=gyro_raw[2];

  output[0]=(moving_average_gyro_x[3]+moving_average_gyro_x[2]+moving_average_gyro_x[1]+moving_average_gyro_x[0])/4;
  output[1]=(moving_average_gyro_y[3]+moving_average_gyro_y[2]+moving_average_gyro_y[1]+moving_average_gyro_y[0])/4;
  output[2]=(moving_average_gyro_z[3]+moving_average_gyro_z[2]+moving_average_gyro_z[1]+moving_average_gyro_z[0])/4;
}

void MPU9250::gyro_auto_calibration(){
  int count=0;

  float temp[3]={0.0,0.0,0.0};
  float temp_x=0.0;
  float temp_y=0.0;
  float temp_z=0.0;

  while(count<256){
    readGyroData(temp);
    temp_x+=temp[0];
    temp_y+=temp[1];
    temp_z+=temp[2];
    count++;
  }
  temp_x/=count;
  temp_y/=count;
  temp_z/=count;
  x_off_set_gyro=temp_x;
  y_off_set_gyro=temp_y;
  z_off_set_gyro=temp_z;
}

// **********************************************************************
#else
MPU9250::MPU9250(const Config&)
		: m_i2c(nullptr),
		  m_gyro_range(Config::Range::kSmall),
		  m_accel_range(Config::Range::kSmall),
		  m_magnetic_range(Config::Bit::MFS_14BITS),
		  m_mode(Config::M_MODE::M_8HZ),
		  aRes(0.0),
		  gRes(0.0),
		  mRes(0.0)
{
	LOG_DL("Configured not to use MPU9250");
}

#endif /* LIBSC_USE_MPU9250 */

}
