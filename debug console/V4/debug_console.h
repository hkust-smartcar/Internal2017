/*
 * debug_console.h
 *
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: Dipsy Wong (dipsywong98)
 *
 * DebugConsole (v4) class
 * Console-based GUI for debugging purposes.
 *
 */

#ifndef CHASING17_DEBUG_CONSOLE_H_
#define CHASING17_DEBUG_CONSOLE_H_

#include <vector>

#include "libsc/joystick.h"
#include "libsc/lcd_typewriter.h"
#include "libsc/st7735r.h"
#include "libbase/k60/flash.h"

typedef void(* Fptr)();

typedef enum{
	  kNan,
	  kInt,
	  kFloat,
	  kBool
} VarType;

typedef struct Item{
	  char* text=nullptr;
	  VarType type=VarType::kNan;
	  uint8_t vIndex=0;
	  float interval=1;
	  Fptr listener=nullptr;
} Item;

class DebugConsole {
 public:
  /**
   * constructor of debug console
   * joystick, lcd, writer pointer and the display length, which limit the display size of debug console
   */
  DebugConsole(libsc::Joystick* joystick, libsc::St7735r* lcd, libsc::LcdTypewriter* writer, int displayLength = 10);

  /**
   * pause and start debugging
   */
  void EnterDebug(char* leave_msg);

  /**
   * just listen and do, no pause
   */
  void Listen();

  /**
   * adding items to debug console
   */
  void PushItem(char* text, int* valuePtr, float interval);
  void PushItem(char* text, float* valuePtr, float interval);
  void PushItem(char* text, bool* valuePtr);

  /**
   * print item start from topIndex, total amount displayLength
   */
  void ListItems();

  /**
   * print item start from topIndex, total amount displayLength
   */
  void ListItemValues();

  /**
   * Increment or decrement of value of item given the index
   */
  void ChangeItemValue(int index, bool IsIncrement);

  /**
   * print a single item's text and value
   */
  void PrintItem(int index, bool isInverted = false);

  /**
   * print a single item's value
   */
  void PrintItemValue(int index, bool isInverted = false);

  /**
   * Load all variable of items which were set flashable
   */
  void Load();

  /*
   * Save all variable of items which were set flashable
   */
  void Save();

  //parameters setters
  DebugConsole* SetDisplayLength(int length);
  DebugConsole* SetLongClickThreshold(int threshold);
  DebugConsole* SetLongClickCd(int cd);
  DebugConsole* SetOffset(int offset);
  DebugConsole* SetAutoFlash(bool flag);
  void SetFlash(libbase::k60::Flash* flash){this->flash=flash;}

  //parameters getters
  int GetDisplayLength() { return displayLength; }
  int GetLongClickThershold() { return threshold; }
  int GetLongClickCd() { return cd; }
  int GetOffset() { return offset; }

 private:


  int focus = 0;
  int topIndex = 0;
  std::vector<Item> items;
  std::vector<int*> int_values;
  std::vector<float*> float_values;
  std::vector<bool*> bool_values;
  libsc::Joystick* joystick;
  libsc::St7735r* lcd;
  libsc::LcdTypewriter* writer;
  libbase::k60::Flash* flash = nullptr;
  libsc::Joystick::State jState = libsc::Joystick::State::kIdle;
  int tFlag = 0;
  int threshold = 1000; //long click thershold
  int displayLength;    //limit the region to display
  int cd = 0;    //time needed to trigger next long click listener
  int offset = 0; //distance away from top of lcd
  bool auto_flash = true; //flash
  int flash_sum=0;
  bool flag=false;

  void Printxy(int x, int y, char* c, int inverted = false);

  void Clear();

  void ListenerDo(libsc::Joystick::State key);
};


#endif // CHASING17_DEBUG_CONSOLE_H_
