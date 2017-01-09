#pragma once

// include the libraries for now
// libbase - k60-specific
#include <libbase/k60/adc.h>
#include <libbase/k60/dac.h>
#include <libbase/k60/dma.h>
#include <libbase/k60/gpio.h>
#include <libbase/k60/ftm_pwm.h>
#include <libbase/k60/soft_i2c_master.h>
#include <libbase/k60/spi_master.h>
#include <libbase/k60/uart.h>

// libsc
#include <libsc/alternate_motor.h>
#include <libsc/button.h>
#include <libsc/dir_motor.h>
#include <libsc/futaba_s3010.h>
#include <libsc/lcd_console.h>
#include <libsc/lcd_typewriter.h>
#include <libsc/led.h>
#include <libsc/simple_buzzer.h>
#include <libsc/st7735r.h>
#include <libsc/tower_pro_mg995.h>
#include <libsc/trs_d05.h>

// libsc - k60-specific
#include <libsc/k60/ftdi_ft232r.h>
#include <libsc/k60/jy_mcu_bt_106.h>

// libutil
// line below causes compilation error
// #include <libutil/incremental_pid_controller.h>
#include <libutil/kalman_filter.h>
#include <libutil/positional_pid_controller.h>
#include <libutil/remote_var_manager.h>
#include <libutil/string.h>
