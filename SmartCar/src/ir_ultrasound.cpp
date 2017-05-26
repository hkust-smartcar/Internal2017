/*
 * ir_ultrasound.cpp
 *
 *  Created on: May 26, 2017
 *      Author: LeeChunHei
 */

#include <inc/ir_ultrasound.h>

IRUltrasoundSensor::IRUltrasoundSensor(libbase::k60::Pin::Name pin){
	gpiConfig.pin=pin;
	gpiConfig.interrupt=Pin::Config::Interrupt::kBoth;
	gpiConfig.isr=&listener;
	gpiConfig.config.set(Pin::Config::kPassiveFilter);
	m_pin=Gpi(gpiConfig);
}

void IRUltrasoundSensor::listener(Gpi *gpi){
	if(gpi->Get()){
		impulseStartTime=libsc::System::Time();
	}else{
		int temp=(libsc::System::Time()-impulseStartTime)*340/1000;
		if(temp<5500){
			distance=temp;
		}
	}
}
