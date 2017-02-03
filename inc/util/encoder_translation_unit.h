/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#pragma once

#include <libsc/alternate_motor.h>
#include <libsc/dir_encoder.h>
#include <libsc/futaba_s3010.h>
#include <libsc/system.h>
#include <memory>

namespace util {
/**
 * Framework for a one-motor speed setting and correction system using encoders.
 */
class EncoderTranslationUnit {
 public:
  /**
   * Constructor accepting an already-created encoder object.
   *
   * @param e Pointer to an encoder object
   * @param m Pointer to an AlternateMotor object
   * @param s Pointer to a FutubaS3010 object
   */
  EncoderTranslationUnit(libsc::DirEncoder *e, libsc::AlternateMotor *m, libsc::FutabaS3010 *s)
      : curr_speed(0), target_speed(0), commit_target_flag(false), motor(m), servo(s), encoder(e) {}
  /**
   * Constructor which creates an encoder object.
   *
   * @param id ID of the encoder.
   * @param m Pointer to an AlternateMotor object
   * @param s Pointer to a FutubaS3010 object
   */
  EncoderTranslationUnit(const uint8_t *id, libsc::AlternateMotor *m, libsc::FutabaS3010 *s)
      : curr_speed(0), target_speed(0), commit_target_flag(false), motor(m), servo(s) {
    libsc::Encoder::Config e_config;
    e_config.id = *id;
    encoder.reset(new libsc::DirEncoder(e_config));
  }

  ~EncoderTranslationUnit() {
    encoder.reset(nullptr);
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
  void SetCommitFlag(bool b) { commit_target_flag = b; }
  /**
   * Sets the target speed.
   *
   * @param speed Speed in units per second; value will be directly compared
   * with encoder values.
   * @param commit_now Whether to commit the target speed immediately. This will
   * also reset the encoder values.
   */
  void SetTargetSpeed(const int16_t speed, bool commit_now = true);
  /**
   * Adds to the target speed.
   *
   * @param d_speed Speed difference in units per second; value will be directly
   * compared with encoder values.
   * @param commit_now Whether to commit the target speed immediately. This will
   * also reset the encoder values.
   */
  void AddToTargetSpeed(const int16_t d_speed, bool commit_now = true);

  // Getters
  /**
   * @return The time elapsed between now and last time the encoder values
   * were reset.
   */
  libsc::Timer::TimerInt GetTimeElapsed() const { return libsc::System::Time() - time_encoder_start; }
  /**
   * @return Current target speed.
   */
  int16_t GetTargetSpeed() const { return target_speed; }

 private:
  /**
   * Commits the target speed.
   */
  void CommitTargetSpeed();
  /**
   * Resets the encoder and updates the time taken.
   */
  void ResetEncoder() {
    encoder->Update();
    time_encoder = GetTimeElapsed();
  }

  // Getters
  /**
   * @return Speed difference between target and current.
   */
  int16_t GetSpeedDiff() const { return target_speed - curr_speed; }
  /**
   * @return Encoder value in units per second
   */
  int32_t GetEncoderVal();

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
  int16_t curr_speed;
  /**
   * User-defined target speed
   */
  int16_t target_speed;
  /**
   * Whether to commit the user-defined target speed on next call to
   * @c DoCorrection()
   */
  bool commit_target_flag;

  // Timekeepers
  /**
   * When the current encoder cycle started.
   */
  libsc::Timer::TimerInt time_encoder_start;
  /**
   * How long the last encoder cycle lasted.
   */
  libsc::Timer::TimerInt time_encoder;

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
        kMotorLowerBound = 10,
    /**
     * Upper bound of motor power which should not be used for extended periods
     * of time. [0,1000]
     */
        kMotorUpperBound = 500,
    /**
     * Lower bound of motor power which should never be exceeded.
     * [0,kMotorLowerBound]
     */
        kMotorLowerHardLimit = 10,
    /**
     * Upper bound of motor power which should never be exceeded.
     * [kMotorUpperBound,1000]
     */
        kMotorUpperHardLimit = 500,
  };

  libsc::AlternateMotor *motor;
  libsc::FutabaS3010 *servo;
  std::unique_ptr<libsc::DirEncoder> encoder;
};
}  // namespace util
