/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

#include <libsc/alternate_motor.h>
#include <libsc/dir_encoder.h>
#include <libsc/futaba_s3010.h>
#include <libsc/lcd_console.h>
#include <libsc/system.h>
#include <memory>

#include "encoder_proportional_controller.h"

namespace util {
/**
 * Test class for @c EncoderProportionalController. Provides access to private variables for
 * debugging purposes.
 *
 * To use methods in this class, declare an @c EncoderProportionalControllerDebug object and
 * use it like an @c EncoderProportionalController.
 */
class EncoderProportionalControllerDebug final : public EncoderProportionalController {
 public:
  /**
 * Constructor accepting an already-created encoder object.
 *
 * @param e Pointer to an encoder object
 * @param m Pointer to an AlternateMotor object
 */
  explicit EncoderProportionalControllerDebug(libsc::DirEncoder *e, libsc::AlternateMotor *m, libsc::FutabaS3010 *s)
      : EncoderProportionalController(e, m) {};
  /**
   * Constructor which creates an encoder object.
   *
   * @param id ID of the encoder.
   * @param m Pointer to an AlternateMotor object
   * @param s Pointer to a FutubaS3010 object
   */
  EncoderProportionalControllerDebug(const uint8_t id, libsc::AlternateMotor *m, libsc::FutabaS3010 *s)
      : EncoderProportionalController(static_cast<uint8_t>(id), m) {};

  /**
   * Outputs the encoder value (in units per second) and power of the managed motor.
   *
   * @param console Pointer to a console object
   */
  void OutputEncoderValues(libsc::LcdConsole *console) const {
    console->WriteString((std::to_string(last_encoder_val_) + " " + std::to_string(motor_->GetPower()) + "\n").c_str());
  }

  // Setters
  /**
   * Manually sets the power of the motor. [0,1000]
   *
   * @note Motor power will be overriden when next @c DoCorrection() is called
   *
   * @param pwr Power of the motor
   * @param is_clockwise True if the motor should be spinning clockwise
   */
  void SetMotorPower(uint16_t power, bool is_clockwise) {
    motor_->SetClockwise(is_clockwise);
    motor_->SetPower(power);
  }

  // Getters
  /**
   * @return The period of the last encoder execution.
   */
  inline libsc::Timer::TimerInt GetLastRunDuration() const { return last_encoder_duration_; }
  /**
   * @return The encoder value in units per second
   */
  inline int32_t GetEncoderVal() const { return last_encoder_val_; }
};
}  // namespace util
