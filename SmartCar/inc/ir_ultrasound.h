/*
 * ir_ultrasound.h
 *
 *  Created on: May 26, 2017
 *      Author: LeeChunHei
 */

#ifndef INC_IR_ULTRASOUND_H_
#define INC_IR_ULTRASOUND_H_

#include <libbase/k60/gpio.h>

using namespace libbase::k60;

class IRUltrasoundSensor{
public:
	IRUltrasoundSensor(Pin::Name pin);
	void listener(Gpi *gpi);
	const int getDistance() const{ return distance; }

private:
	Gpi m_pin;
	Gpi::Config gpiConfig;
	libsc::Timer::TimerInt impulseStartTime=0;
	uint distance=0;
};



#endif /* INC_IR_ULTRASOUND_H_ */
