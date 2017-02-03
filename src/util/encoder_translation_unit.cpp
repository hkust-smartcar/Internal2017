/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "util/encoder_translation_unit.h"

using libsc::AlternateMotor;
using libsc::FutabaS3010;
using libsc::System;
using libsc::Timer;

namespace util {
void EncoderTranslationUnit::DoCorrection() {
  // cleanup from previous cycle if it is out of range
  // [kMotorLowerBound,kMotorUpperBound]
  if (motor_->GetPower() > kMotorUpperBound) {
    motor_->SetPower(kMotorUpperBound);
  } else if (motor_->GetPower() < kMotorLowerBound) {
    motor_->SetPower(kMotorLowerBound);
  }

  // sets the correction target speed to the new speed, if
  // commit_target_flag_ is true.
  if (commit_target_flag_) {
    curr_speed_ = target_speed_;
    commit_target_flag_ = false;
  }

  int32_t encoder_val = GetEncoderVal();

  // checks if the motor direction differs from our target
  if (!HasSameSign(encoder_val, static_cast<int32_t>(curr_speed_))) {
    motor_->SetClockwise(!motor_->IsClockwise());
  }

  // get the speed difference and add power linearly.
  // bigger difference = higher power difference
  int16_t speed_diff = static_cast<int16_t>(std::abs(encoder_val) - std::abs(curr_speed_));
  motor_->AddPower(-speed_diff / kMotorDFactor);

  // bounds checking
  if (motor_->GetPower() > kMotorUpperHardLimit) {
    motor_->SetPower(kMotorUpperHardLimit);
  } else if (motor_->GetPower() < kMotorLowerHardLimit) {
    motor_->SetPower(kMotorLowerHardLimit);
  }

  // reset the encoder values and the time taken
  ResetEncoder();
  time_encoder_start_ = libsc::System::Time();
}

void EncoderTranslationUnit::CommitTargetSpeed() {
  commit_target_flag_ = true;
  DoCorrection();
}

int32_t EncoderTranslationUnit::GetEncoderVal() {
  ResetEncoder();
  return encoder_->GetCount() * 1000 / GetTimeElapsed();
}

void EncoderTranslationUnit::SetTargetSpeed(const int16_t speed, bool commit_now) {
  target_speed_ = speed;
  if (commit_now) {
    CommitTargetSpeed();
  }
}

void EncoderTranslationUnit::AddToTargetSpeed(const int16_t d_speed, bool commit_now) {
  target_speed_ += d_speed;
  if (commit_now) {
    CommitTargetSpeed();
  }
}
}  // namespace util
