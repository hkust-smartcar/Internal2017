/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "algorithm/area_method.h"

#include <array>
#include <cstring>
#include <memory>
#include <string>

#include "libsc/st7735r.h"
#include "libsc/system.h"
#include "libsc/k60/ov7725.h"

#include "util/util.h"

using libsc::Lcd;
using libsc::St7735r;
using libsc::Timer;
using libsc::k60::Ov7725;
using std::array;
using std::unique_ptr;
using util::CopyByteArray;
using util::ByteTo1DBitArray;

void AreaMethod() {
  // initialize camera
  Ov7725::Config camera_config;
  camera_config.id = 0;
  camera_config.w = 80;
  camera_config.h = 60;
  unique_ptr<Ov7725> camera(new Ov7725(camera_config));
  constexpr Uint kBufferSize = 80 * 60 / 8;
  if (kBufferSize != camera->GetBufferSize()) {
    return;
  }
  camera->Start();
  while (!camera->IsAvailable()) {}

  // initialize LCD
  St7735r::Config lcd_config;
  lcd_config.fps = 10;
  unique_ptr<St7735r> lcd(new St7735r(lcd_config));
  lcd->Clear();

  while (true) {
    const Byte *p_buffer = camera->LockBuffer();
    array<Byte, kBufferSize> buffer_arr;
    CopyByteArray(*p_buffer, &buffer_arr);

    // unlock the buffer now that we have the data
    camera->UnlockBuffer();

    // 1d to 1d array
    array<bool, kBufferSize * 8> image_1d;
    ByteTo1DBitArray(buffer_arr, &image_1d);

    uint16_t leftCount = 0;
    uint16_t rightCount = 0;

    for (uint16_t i = 0; i < camera->GetBufferSize() * 8; ++i) {
      if (!image_1d[i]) {
        (i / 64 % 2) == 0 ? ++leftCount : ++rightCount;
      }
    }

    lcd->SetRegion(Lcd::Rect(0, 0, 128, 160));
    lcd->FillBits(Lcd::kBlack, Lcd::kWhite, buffer_arr.data(), camera->GetBufferSize() * 8);

    if (rightCount > leftCount) {
      lcd->SetRegion(Lcd::Rect(64, 0, 64, 20));
      lcd->FillColor(Lcd::kGreen);
    } else if (leftCount > rightCount) {
      lcd->SetRegion(Lcd::Rect(0, 0, 64, 20));
      lcd->FillColor(Lcd::kRed);
    }
  }
}
