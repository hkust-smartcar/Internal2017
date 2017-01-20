/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../../inc/alternate_algorithms/area_method.h"

#include <libsc/system.h>
#include <string>

#include "../../inc/main.h"

using libsc::Lcd;
using libsc::St7735r;
using libsc::Timer;
using libsc::k60::Ov7725;

void areaMethod(Ov7725 *camera, St7735r *lcd) {
  const Byte *pBuffer = camera->LockBuffer();
  const Uint kBufferSize = camera->GetBufferSize();
  Byte bufferArr[kBufferSize];
  for (Uint i = 0; i < kBufferSize; ++i) {
    bufferArr[i] = pBuffer[i];
  }

  // unlock the buffer now that we have the data
  camera->UnlockBuffer();

  uint16_t leftCount = 0;
  uint16_t rightCount = 0;

  // 1d to 1d array
  bool image1d[kBufferSize * 8];
  for (Uint i = 0; i < kBufferSize; ++i) {
    std::string s = std::bitset<8>(static_cast<Uint>(bufferArr[i])).to_string();
    for (uint8_t j = 0; j < 8; ++j) {
      image1d[i * 8 + j] = static_cast<Uint>(s.at(j) - '0') == 1;
    }
  }

  for (uint16_t i = 0; i < camera->GetBufferSize() * 8; ++i) {
    if (!image1d[i]) {
      (i / 64 % 2) == 0 ? ++leftCount : ++rightCount;
    }
  }

  lcd->SetRegion(Lcd::Rect(0, 0, 128, 160));
  lcd->FillBits(Lcd::kBlack, Lcd::kWhite, bufferArr, camera->GetBufferSize() * 8);

  if (rightCount > leftCount) {
    lcd->SetRegion(Lcd::Rect(64, 0, 64, 20));
    lcd->FillColor(Lcd::kGreen);
  } else if (leftCount > rightCount) {
    lcd->SetRegion(Lcd::Rect(0, 0, 64, 20));
    lcd->FillColor(Lcd::kRed);
  }
}