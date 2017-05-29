/*
 * ir_ultrasound.cpp
 *
 *  Created on: May 26, 2017
 *      Author: LeeChunHei
 */

#include "../inc/ir_ultrasound.h"

uint32_t IRUltrasoundSensor::impulseStartTime=0;
int IRUltrasoundSensor::distance=0;

void IRUltrasoundSensor::listener(Gpi *gpi){
	if(gpi->Get()){
		impulseStartTime=libsc::System::Time();
	}else{
		int temp=(libsc::System::Time()-impulseStartTime)*340;
		if(temp<5500000){
			distance=temp;
		}
	}
}

IRUltrasoundSensor::IRUltrasoundSensor(libbase::k60::Pin::Name pin){
	gpiConfig.pin=pin;
	gpiConfig.interrupt=Pin::Config::Interrupt::kBoth;
	gpiConfig.isr=listener;
	gpiConfig.config.set(Pin::Config::kPassiveFilter);
	m_pin=Gpi(gpiConfig);
}
