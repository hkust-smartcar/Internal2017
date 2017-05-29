/*
 * ir_ultrasound.h
 *
 *  Created on: May 26, 2017
 *      Author: LeeChunHei
 */

#ifndef INC_IR_ULTRASOUND_H_
#define INC_IR_ULTRASOUND_H_

#include <libbase/k60/gpio.h>
#include "../inc/global.h"

using namespace libbase::k60;

class IRUltrasoundSensor{
public:
	IRUltrasoundSensor(Pin::Name pin);
	Gpi::OnGpiEventListener test;
	static void listener(Gpi *gpi);
	const int getDistance() const{ return distance; }

private:
	Gpi m_pin;
	Gpi::Config gpiConfig;
	static uint32_t impulseStartTime;
	static int distance;
};



#endif /* INC_IR_ULTRASOUND_H_ */
