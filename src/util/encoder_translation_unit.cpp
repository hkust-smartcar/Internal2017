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
  if (motor->GetPower() > kMotorUpperBound) {
    motor->SetPower(kMotorUpperBound);
  } else if (motor->GetPower() < kMotorLowerBound) {
    motor->SetPower(kMotorLowerBound);
  }

  // sets the correction target speed to the new speed, if
  // commit_target_flag is true.
  if (commit_target_flag) {
    curr_speed = target_speed;
    commit_target_flag = false;
  }

  int32_t encoder_val = GetEncoderVal();

  // checks if the motor direction differs from our target
  if (!HasSameSign(encoder_val, static_cast<int32_t>(curr_speed))) {
    motor->SetClockwise(!motor->IsClockwise());
  }

  // get the speed difference and add power linearly.
  // bigger difference = higher power difference
  int16_t speed_diff = static_cast<int16_t>(std::abs(encoder_val) - std::abs(curr_speed));
  motor->AddPower(-speed_diff / kMotorDFactor);

  // bounds checking
  if (motor->GetPower() > kMotorUpperHardLimit) {
    motor->SetPower(kMotorUpperHardLimit);
  } else if (motor->GetPower() < kMotorLowerHardLimit) {
    motor->SetPower(kMotorLowerHardLimit);
  }

  // reset the encoder values and the time taken
  ResetEncoder();
  time_encoder_start = libsc::System::Time();
}

void EncoderTranslationUnit::CommitTargetSpeed() {
  commit_target_flag = true;
  DoCorrection();
}

int32_t EncoderTranslationUnit::GetEncoderVal() {
  ResetEncoder();
  return encoder->GetCount() * 1000 / GetTimeElapsed();
}

void EncoderTranslationUnit::SetTargetSpeed(const int16_t speed, bool commit_now) {
  target_speed = speed;
  if (commit_now) {
    CommitTargetSpeed();
  }
}

void EncoderTranslationUnit::AddToTargetSpeed(const int16_t d_speed, bool commit_now) {
  target_speed += d_speed;
  if (commit_now) {
    CommitTargetSpeed();
  }
}
}  // namespace util
