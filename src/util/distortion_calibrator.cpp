/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "util/distortion_calibrator.h"

#include <memory>

#include "libsc/joystick.h"
#include "libsc/lcd_console.h"
#include "libsc/led.h"
#include "libsc/st7735r.h"
#include "libsc/system.h"
#include "libsc/k60/ov7725.h"

#include "util/util.h"

namespace {
struct {
  Uint track_count;    // Number of pixels of the track
  Uint left_bound;    // Left boundary of the track
  Uint right_bound;   // Right boundary of the track
  Uint center_point;  // Index of the center point
} RowInfo[80];
}  // namespace

using libsc::Joystick;
using libsc::Lcd;
using libsc::LcdConsole;
using libsc::Led;
using libsc::St7735r;
using libsc::System;
using libsc::Timer;
using libsc::k60::Ov7725;
using std::array;
using std::unique_ptr;
using std::vector;

namespace util {
std::array<float, 2> distortion_slope{};

bool DistortionCalibrator() {
  // initialize LEDs
  Led::Config led_config;
  led_config.is_active_low = true;
  led_config.id = 0;
  Led led1(led_config);  // main loop heartbeat
  led_config.id = 1;
  Led led2(led_config);  // buffer processing led
  led_config.id = 2;
  Led led3(led_config);  // unused
  led_config.id = 3;
  Led led4(led_config);  // initialization led
  led1.SetEnable(false);
  led2.SetEnable(false);
  led3.SetEnable(false);
  led4.SetEnable(true);

  // initialize camera
  Ov7725::Config camera_config;
  camera_config.id = 0;
  camera_config.w = 64;
  camera_config.h = 80;
  unique_ptr<Ov7725> camera(new Ov7725(camera_config));
  camera->Start();
  while (!camera->IsAvailable()) {}

  // initialize LCD
  St7735r::Config lcd_config;
  lcd_config.fps = 10;
  lcd_config.is_revert = true;
  unique_ptr<St7735r> lcd(new St7735r(lcd_config));
  lcd->Clear();

  LcdConsole::Config console_config;
  console_config.lcd = lcd.get();
  console_config.region = Lcd::Rect(0, 80, 128, 80);
  LcdConsole console(console_config);

  // initialize joystick
  Joystick::Config joystick_config;
  joystick_config.id = 0;
  joystick_config.is_active_low = true;
  Joystick joystick(joystick_config);

  Timer::TimerInt time_img = System::Time();  // current execution time
  float stretch_left_slope = 0.0;
  float stretch_right_slope = 0.0;

  led4.SetEnable(false);

  int mode_state = 1;

  while (true) {
    if (time_img != System::Time()) {
      time_img = System::Time();

      // heartbeat
      led1.SetEnable(time_img % 1000 >= 500);

      // declare variables first
      const Byte *pbuffer = nullptr;
      Byte image1d[640]{};
      array<array<bool, 64>, 80> image2d{};

      switch (mode_state) {
        case 1: {  // calibration mode
          // jump to preview mode after this calculation
          if (joystick.GetState() == Joystick::State::kDown) {
            mode_state = 2;
          }

          // force 500ms execution cycle
          if (time_img % 500 != 0) continue;

          Timer::TimerInt cycle_start_time = System::Time();

          // copy the buffer for processing
          pbuffer = camera->LockBuffer();
          util::CopyByteArray(*pbuffer, image1d, 640);
          camera->UnlockBuffer();

          led2.SetEnable(true);

          // 1d to 2d array
          util::ByteTo2DBitArray(*image1d, &image2d);

          // apply median filter
          array<array<bool, 64>, 80> image2d_median;
          util::MedianFilter(image2d, &image2d_median);

          // fill in row information
          for (int rowIndex = 80; rowIndex-- > 0;) {
            if (rowIndex == 79) {
              RowInfo[rowIndex].right_bound = static_cast<Uint>(
                  util::FindElement(image2d_median[rowIndex], 32, 63, true, true));
              RowInfo[rowIndex].left_bound = static_cast<Uint>(
                  util::FindElement(image2d_median[rowIndex], 32, 0, true, true));
            } else {
              RowInfo[rowIndex].right_bound = static_cast<Uint>(
                  util::FindElement(image2d_median[rowIndex], RowInfo[rowIndex + 1].center_point, 63, true, true));
              RowInfo[rowIndex].left_bound = static_cast<Uint>(
                  util::FindElement(image2d_median[rowIndex], RowInfo[rowIndex + 1].center_point, 0, true, true));
            }
            // calculate the dependencies
            RowInfo[rowIndex].center_point = (RowInfo[rowIndex].left_bound + RowInfo[rowIndex].right_bound) / 2;
            RowInfo[rowIndex].track_count = RowInfo[rowIndex].right_bound - RowInfo[rowIndex].left_bound;
          }

          // y-values to take to calculate regression
          array<unsigned int, 9> sample_y{79, 74, 69, 64, 59, 54, 49, 44, 39};
          vector<int> x_left{};
          vector<int> x_right{};
          vector<int> y{};

          // fill in the sample points
          for (unsigned int i : sample_y) {
            x_left.emplace_back(RowInfo[i].left_bound);
            x_right.emplace_back(RowInfo[i].right_bound);
            y.emplace_back(i);
          }

          // calculate the linear regression slope
          float left_slope = 1 / util::CalcLinearRegressionSlope(x_left.data(), y.data(), y.size());
          float right_slope = 1 / util::CalcLinearRegressionSlope(x_right.data(), y.data(), y.size());

          // calculate the top left/right corners of the track when slope is
          // extrapolated to the bottom corners
          float stretch_track_top_left =
              (32 - (RowInfo[79].left_bound - (left_slope * 79))) / (32 - RowInfo[79].left_bound) * 32;
          float stretch_track_top_right =
              ((RowInfo[79].right_bound - (right_slope * 79)) - 32) / (RowInfo[79].right_bound - 32) * 32;

          // calculate the newly extrapolated slopes
          stretch_left_slope = (stretch_track_top_left - 32.0) / 80.0;
          stretch_right_slope = (32.0 - stretch_track_top_right) / 80;

          led2.SetEnable(false);

          Timer::TimerInt cycle_end_time = System::Time();

          auto cycle_time = cycle_end_time - cycle_start_time;  // currently takes 58ms

          // render the image and absolute center line
          lcd->SetRegion(Lcd::Rect(0, 0, 64, 80));
          lcd->FillBits(Lcd::kBlue, Lcd::kWhite, image1d, 640 * 8);
          lcd->SetRegion(Lcd::Rect(32, 0, 1, 80));
          lcd->FillColor(Lcd::kBlack);

          // render different parts
          for (int i = 80; i-- > 0;) {
            // render left/right bound
            lcd->SetRegion(Lcd::Rect(RowInfo[i].left_bound, i, 1, 1));
            lcd->FillColor(Lcd::kBlack);
            lcd->SetRegion(Lcd::Rect(RowInfo[i].right_bound, i, 1, 1));
            lcd->FillColor(Lcd::kBlack);

            // render center line
            lcd->SetRegion(Lcd::Rect(RowInfo[i].center_point, i, 1, 1));
            lcd->FillColor(Lcd::kRed);

            // render unadjusted distortion line
            unsigned int left = static_cast<unsigned>(RowInfo[79].left_bound - left_slope * (79 - i));
            lcd->SetRegion(Lcd::Rect(left > 63 ? 63 : left, i, 1, 1));
            lcd->FillColor(Lcd::kRed);
            unsigned int right = static_cast<unsigned>(RowInfo[79].right_bound - right_slope * (79 - i));
            lcd->SetRegion(Lcd::Rect(right > 63 ? 63 : right, i, 1, 1));
            lcd->FillColor(Lcd::kRed);

            // render adjusted distortion line
            left = static_cast<unsigned>(0 - stretch_left_slope * (79 - i));
            lcd->SetRegion(Lcd::Rect(left > 63 ? 63 : left, i, 1, 1));
            lcd->FillColor(Lcd::kYellow);
            right = static_cast<unsigned>(63 - stretch_right_slope * (79 - i));
            lcd->SetRegion(Lcd::Rect(right > 63 ? 63 : right, i, 1, 1));
            lcd->FillColor(Lcd::kYellow);
          }

          // display slope values and cycle times
          console.Clear(false);
          std::string s = "L: " + std::to_string(left_slope) + "\nR: " + std::to_string(right_slope)
              + "\nCL: "
              + std::to_string(stretch_left_slope)
              + "\nCR: "
              + std::to_string(stretch_right_slope)
              + "\nCycle: " + std::to_string(cycle_time) + " ms";
          console.WriteString(s.c_str());

          distortion_slope.at(0) = stretch_left_slope;
          distortion_slope.at(1) = stretch_right_slope;

          // skip preview mode if we are not signaled to break
          break;
        }
        case 2:
          // sleep 2s while we get things set up
          mode_state = 0;
          lcd->Clear();
          console.SetCursorRow(0);
          console.WriteString("Please wait...");
          System::DelayMs(2000);
          lcd->Clear();
        case 0: {  // preview mode
          // handle joystick actions
          switch (joystick.GetState()) {
            case Joystick::State::kDown:  // return to calibration mode
              mode_state = 1;
              lcd->Clear();
              break;
            case Joystick::State::kLeft:  // discard changes
              lcd->Clear();
              console.WriteString("Please wait...");
              System::DelayMs(2000);
              lcd->Clear();
              return false;
            case Joystick::State::kRight:  // save changes
              lcd->Clear();
              console.WriteString("Please wait...");
              System::DelayMs(2000);
              lcd->Clear();
              return true;
            default:
              // not handled
              break;
          }

          // force 500ms execution cycle
          if (time_img % 500 != 0) continue;

          // copy the buffer for processing
          pbuffer = camera->LockBuffer();
          util::CopyByteArray(*pbuffer, image1d, 640);
          camera->UnlockBuffer();

          led2.SetEnable(true);

          // 1d to 2d array
          util::ByteTo2DBitArray(*image1d, &image2d);

          // apply median filter
          util::MedianFilter(&image2d);

          // write the perspective-distorted image to a new array
          ApplyDistortionFilter(&image2d);

          led2.SetEnable(false);

          // render the image and absolute center line
          lcd->SetRegion(Lcd::Rect(0, 0, 64, 80));
          lcd->FillBits(Lcd::kBlue, Lcd::kWhite, image1d, 640 * 8);

          // render the undistorted image and absolute center line
          lcd->SetRegion(Lcd::Rect(63, 0, 64, 80));
          for (int i = 0; i < 80; ++i) {
            for (int j = 0; j < 64; ++j) {
              lcd->SetRegion(Lcd::Rect(j + 64, i, 1, 1));
              lcd->FillColor(image2d.at(i).at(j) ? Lcd::kBlue : Lcd::kWhite);
            }
          }
          lcd->SetRegion(Lcd::Rect(32, 0, 1, 80));
          lcd->FillColor(Lcd::kBlack);
          lcd->SetRegion(Lcd::Rect(96, 0, 1, 80));
          lcd->FillColor(Lcd::kBlack);
        }
      }
    }
  }
}
}  // namespace util
