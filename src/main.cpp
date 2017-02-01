/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../inc/main.h"

#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <algorithm/centerline_method.h>
#include <util/unit_tests.h>

namespace libbase {
namespace k60 {
Mcg::Config Mcg::GetMcgConfig() {
  Mcg::Config config;
  config.external_oscillator_khz = 50000;
  config.core_clock_khz = 150000;
  return config;
}

}  // namespace k60
}  // namespace libbase

using libsc::System;

int main() {
  System::Init();

  CenterLineMethod();

  while (true) {
  }

  return 0;
}
