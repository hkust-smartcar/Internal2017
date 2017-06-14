//=============================================================================================
// MadgwickAHRS.h
//=============================================================================================
//
// Implementation of Madgwick's IMU and AHRS algorithms.
// See: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
//
// From the x-io website "Open-source resources available on this website are
// provided under the GNU General Public Licence unless an alternative licence
// is provided in source."
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=============================================================================================
#ifndef MadgwickAHRS_h
#define MadgwickAHRS_h
#include <math.h>

//--------------------------------------------------------------------------------------------
// Variable declaration
class Madgwick{
private:
    static float invSqrt(float x);
    const float deg_to_radian = 0.0174533f;
    const float radian_to_deg = 57.29578f;
    
    float beta;				// algorithm gain
    
    float q[4] = {1.0, 0.0, 0.0, 0.0};

    float roll;
    float pitch;
    float yaw;




//-------------------------------------------------------------------------------------------
// Function declarations
public:
    Madgwick(void);
    void update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float delta_t);
    void updateIMU(float gx, float gy, float gz, float ax, float ay, float az, float delta_t);
    

    
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
