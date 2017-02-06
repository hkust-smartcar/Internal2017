/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

#include <libsc/alternate_motor.h>
#include <libsc/dir_encoder.h>
#include <libsc/system.h>
#include <memory>

namespace util {
/**
 * Framework for a one-motor speed setting and correction system using encoders.
 */
class EncoderPowerUnit {
 public:
  /**
   * Constructor accepting an already-created encoder object.
   *
   * @param e Pointer to an encoder object
   * @param m Pointer to an AlternateMotor object
   */
  explicit EncoderPowerUnit(libsc::DirEncoder *e, libsc::AlternateMotor *m)
      : motor_(m), encoder_(e) {
    motor_->SetPower(0);
    UpdateEncoder();
  }
  /**
   * Constructor which creates an encoder object.
   *
   * @note When creating this object, cast @c id as a @c uint8_t to prevent
   * ambiguous constructor definition.
   *
   * @param id ID of the encoder.
   * @param m Pointer to an AlternateMotor object
   */
  EncoderPowerUnit(const uint8_t id, libsc::AlternateMotor *m)
      : motor_(m) {
    libsc::Encoder::Config e_config;
    e_config.id = id;
    encoder_.reset(new libsc::DirEncoder(e_config));
    motor_->SetPower(0);
    UpdateEncoder();
  }

  ~EncoderPowerUnit() {
    encoder_.reset(nullptr);
    motor_.reset(nullptr);
  }

  /**
   * Does motor power correction using encoder, and resets the encoder count.
   * Also commits the user-given target speed if @c commit_target_flag is true.
   */
  void DoCorrection();

  // Setters
  /**
   * Sets @c commit_target_flag.
   *
   * @param b If true, next call to @c DoCorrection() will use the target speed
   * for correction. Otherwise the current speed will be used.
   */
  void SetCommitFlag(bool flag) { commit_target_flag_ = flag; }
  /**
   * Sets the target speed.
   *
   * @param speed Speed in units per second; value will be directly compared
   * with encoder values.
   * @param commit_now Whether to commit the target speed immediately. This will
   * also reset the encoder values.
   */
  void SetTargetSpeed(const int16_t speed, bool commit_now = true) {
    target_speed_ = speed;
    if (commit_now) {
      CommitTargetSpeed();
    }
  }
  /**
   * Adds to the target speed.
   *
   * @param d_speed Speed difference in units per second; value will be directly
   * compared with encoder values.
   * @param commit_now Whether to commit the target speed immediately. This will
   * also reset the encoder values.
   */
  void AddToTargetSpeed(const int16_t d_speed, bool commit_now = true) {
    target_speed_ += d_speed;
    if (commit_now) {
      CommitTargetSpeed();
    }
  }

  // Getters
  /**
   * @return The time elapsed between now and last time the encoder values
   * were reset.
   */
  inline libsc::Timer::TimerInt GetTimeElapsed() const { return libsc::System::Time() - time_encoder_start_; }
  /**
   * @return Current target speed.
   */
  inline int16_t GetTargetSpeed() const { return target_speed_; }

 protected:
  /**
   * Commits the target speed.
   */
  void CommitTargetSpeed() {
    commit_target_flag_ = true;
    DoCorrection();
  }
  /**
   * Updates the encoder value and resets the encoder
   */
  void UpdateEncoder() {
    last_encoder_duration_ = GetTimeElapsed();
    encoder_->Update();
    last_encoder_val_ = encoder_->GetCount() * 1000 / static_cast<int32_t>(last_encoder_duration_);
    time_encoder_start_ = libsc::System::Time();
  }

  /**
   * Compares if two variables have the same sign.
   *
   * @tparam T Any numeric type
   * @param val1 First value
   * @param val2 Second value
   * @return True if both variables have the same sign
   */
  template<typename T>
  bool HasSameSign(T val1, T val2) const { return (val1 >= 0 && val2 >= 0) || (val1 < 0 && val2 < 0); }

  // Speed-related variables
  /**
   * Current reference target speed
   */
  int16_t curr_speed_ = 0;
  /**
   * User-defined target speed
   */
  int16_t target_speed_ = 0;
  /**
   * The value of the encoder in units per second
   */
  int32_t last_encoder_val_ = 0;

  // Timekeepers
  /**
   * When the current encoder cycle started.
   */
  libsc::Timer::TimerInt time_encoder_start_ = 0;
  /**
   * How long the last encoder cycle lasted.
   */
  libsc::Timer::TimerInt last_encoder_duration_ = 0;

  std::unique_ptr<libsc::AlternateMotor> motor_;
  std::unique_ptr<libsc::DirEncoder> encoder_;

 private:
  /**
   * Whether to commit the user-defined target speed on next call to
   * @c DoCorrection()
   */
  bool commit_target_flag_ = false;
  /**
   * Constants for encoder to motor value conversions
   */
  enum MotorConstants {
    /**
     * Conversion factor from encoder difference to motor power difference.
     *
     * @example If set to 50, for every encoder value difference of 50, the
     * motor power will increase/decrease by 1.
     */
        kMotorDFactor = 50,
    /**
     * Lower bound of motor power which should not be used for extended periods
     * of time. [0,1000]
     */
        kMotorLowerBound = 75,
    /**
     * Upper bound of motor power which should not be used for extended periods
     * of time. [0,1000]
     */
        kMotorUpperBound = 500,
    /**
     * Lower bound of motor power which should never be exceeded.
     * [0,kMotorLowerBound]
     */
        kMotorLowerHardLimit = 75,
    /**
     * Upper bound of motor power which should never be exceeded.
     * [kMotorUpperBound,1000]
     */
        kMotorUpperHardLimit = 500,
  };
};
}  // namespace util