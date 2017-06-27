/*
 * debug_console.cpp
 *
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: Dipsy Wong (dipsywong98)
 *
 * Implementation for DebugConsole (v4) class.
 *
 */

#include "debug_console.h"

#include <cstdio>

#include "libsc/joystick.h"
#include "libsc/lcd_typewriter.h"
#include "libsc/st7735r.h"
#include "libsc/system.h"
#include "libbase/k60/flash.h"

using libsc::Joystick;
using libsc::Lcd;
using libsc::LcdTypewriter;
using libsc::St7735r;
using libsc::System;

DebugConsole::DebugConsole(Joystick* joystick, St7735r* lcd, LcdTypewriter* writer, int displayLength)
    : joystick(joystick), lcd(lcd), writer(writer), displayLength(displayLength) {}

void DebugConsole::EnterDebug(char* leave_msg) {
  flag = true;
  Item item;
  item.text=leave_msg;
  items.insert(items.begin()+0, item);

  int index = items.size();
  Load();
  ListItems();
  while (flag) {
    Listen();
//    if(System::Time()%100==0)
    	Save();
  }
  items.erase(items.begin());
  Clear();
}

void DebugConsole::Listen() {
  if (jState != joystick->GetState()) {
    jState = joystick->GetState();
    tFlag = System::Time() + threshold;
    ListenerDo(jState);
  } else if (System::Time() > tFlag) {
    tFlag = System::Time() + cd;
    ListenerDo(jState);
  }
}

void DebugConsole::PushItem(char* text, int* valuePtr, float interval){
	Item item;
	item.text = text;
	item.type = VarType::kInt;
	item.vIndex = int_values.size();
	item.interval=interval;
	int_values.push_back(valuePtr);
	items.push_back(item);
	flash_sum+=sizeof(*valuePtr);
}
void DebugConsole::PushItem(char* text, float* valuePtr, float interval){
	Item item;
	item.text = text;
	item.type = VarType::kFloat;
	item.vIndex = float_values.size();
	item.interval=interval;
	float_values.push_back(valuePtr);
	items.push_back(item);
	flash_sum+=sizeof(*valuePtr);
}
void DebugConsole::PushItem(char* text, bool* valuePtr){
	Item item;
	item.text = text;
	item.type = VarType::kBool;
	item.vIndex = bool_values.size();
	bool_values.push_back(valuePtr);
	items.push_back(item);
	flash_sum+=sizeof(*valuePtr);
}

void DebugConsole::ListItems() {
  Clear();
  for (int i = topIndex; i < (items.size() < topIndex + displayLength ? items.size() : topIndex + displayLength); i++) {
    PrintItem(i, i == focus);
  }
}

void DebugConsole::ListItemValues() {
  Clear();
  for (int i = topIndex; i < (items.size() < topIndex + displayLength ? items.size() : topIndex + displayLength); i++) {
    PrintItemValue(i, i == focus);
  }
}

void DebugConsole::ChangeItemValue(int index, bool IsIncrement){
	Item item = items[index];
	float c = IsIncrement?1.0:-1.0;
	switch(item.type){
	    case VarType::kNan:
	    	return;break;
	    case VarType::kInt:
	    	*int_values[item.vIndex]+= int(c*item.interval);
	    	break;
	    case VarType::kFloat:
	    	*float_values[item.vIndex]+=float(c*item.interval);
	    	break;
	    case VarType::kBool:
	    	*bool_values[item.vIndex]=!*bool_values[item.vIndex];
	    	break;
	    default:
	    	return;break;
	    }
}

void DebugConsole::PrintItem(int index, bool isInverted) {
  Printxy(0, index - topIndex, items[index].text, isInverted);
  PrintItemValue(index, isInverted);
}

void DebugConsole::PrintItemValue(int index, bool isInverted) {

    char buff[20];
    Item item = items[index];
    switch(item.type){
    case VarType::kNan:
    	return;break;
    case VarType::kInt:
    	sprintf(buff, "%d", *int_values[item.vIndex]);
    	break;
    case VarType::kFloat:
    	sprintf(buff, "%.3lf", *float_values[item.vIndex]);
    	break;
    case VarType::kBool:
    	sprintf(buff, "%s", *bool_values[item.vIndex] ? "true" : "false");
    	break;
    default:
    	return;break;
    }
    Printxy(9, index - topIndex, buff, isInverted);
    return;
}

void DebugConsole::Load(){

	if(flash == nullptr) return;
	int start=0;
	Byte* buff = new Byte[flash_sum];
	flash->Read(buff,flash_sum);
	for(int i=0;i<int_values.size();i++){
		int* v=int_values[i];
		int temp=0;
		memcpy((unsigned char*) &temp, buff+start, sizeof(*v));
		start+=sizeof(*v);
		if(temp==temp)*v=temp;
	}
	for(int i=0;i<float_values.size();i++){
		float* v=float_values[i];
		float temp=0;
		memcpy((unsigned char*) &temp, buff+start, sizeof(*v));
		start+=sizeof(*v);
		if(temp==temp)*v=temp;
	}
	for(int i=0;i<bool_values.size();i++){
		bool* v=bool_values[i];
		bool temp=0;
		memcpy((unsigned char*) &temp, buff+start, sizeof(*v));
		start+=sizeof(*v);
		if(temp==temp)*v=temp;
	}
	delete [] buff;
}

void DebugConsole::Save(){
	if(flash == nullptr) return;
	int start=0;
	Byte* buff = new Byte[flash_sum];
	for(int i=0;i<int_values.size();i++){
		int* v=int_values[i];
		memcpy(buff+start, (unsigned char*) v, sizeof(*v));
		start+=sizeof(*v);
	}
	for(int i=0;i<float_values.size();i++){
		float* v=float_values[i];
		memcpy(buff+start, (unsigned char*) v, sizeof(*v));
		start+=sizeof(*v);
	}
	for(int i=0;i<bool_values.size();i++){
		bool* v=bool_values[i];
		memcpy(buff+start, (unsigned char*) v, sizeof(*v));
		start+=sizeof(*v);
	}
	flash->Write(buff, flash_sum);
	System::DelayMs(100);
	delete [] buff;
}

DebugConsole* DebugConsole::SetDisplayLength(int length) {
  displayLength = length;
  return this;
}

DebugConsole* DebugConsole::SetLongClickThreshold(int threshold) {
  this->threshold = threshold;
  return this;
}

DebugConsole* DebugConsole::SetLongClickCd(int cd) {
  this->cd = cd;
  return this;
}

DebugConsole* DebugConsole::SetOffset(int offset) {
  this->offset = offset;
  return this;
}

DebugConsole* DebugConsole::SetAutoFlash(bool flag) {
  this->auto_flash = flag;
  return this;
}

void DebugConsole::Printxy(int x, int y, char* c, int inverted) {
  if (inverted) {
    writer->SetTextColor(0x0000);
    writer->SetBgColor(0xFFFF);
  }
  lcd->SetRegion(Lcd::Rect(5 + x * 10, offset + y * 15, 128 - 5 - x * 10, 15));
  writer->WriteString(c);
  if (inverted) {
    writer->SetTextColor(0xFFFF);
    writer->SetBgColor(0x0000);
  }
}

void DebugConsole::Clear() {
  lcd->SetRegion(Lcd::Rect(0, offset, 128, displayLength * 15));
  lcd->FillColor(0x0000);
}

void DebugConsole::ListenerDo(Joystick::State key) {
  Item item = items[focus];
  switch (key) {
    case Joystick::State::kDown:
      PrintItem(focus);    //remove highLighting
      focus++;                //move down focus
      if (focus >= items.size()) {    //focus below last item then jump back to first
        topIndex = 0;
        focus = 0;
        ListItems();
      } else if (focus >= topIndex + displayLength) {    //next page
        topIndex += displayLength;
        ListItems();
      } else {
        PrintItem(focus, true);    //update highlighted item
      }
      break;
    case Joystick::State::kUp:
      PrintItem(focus);
      focus--;
      if (focus < 0) {
        focus = items.size() - 1;
        topIndex = items.size() - displayLength;
        topIndex = (topIndex > 0 ? topIndex : 0);
        ListItems();
      } else if (focus < topIndex) {
        topIndex -= displayLength;
        topIndex = (topIndex > 0 ? topIndex : 0);
        ListItems();
      } else {
        PrintItem(focus, true);
      }
      break;
    case Joystick::State::kSelect:
      if (item.listener != nullptr) {
        item.listener();
      } else if (flag&&focus==0)//leave item click
        flag=false;
      break;
    case Joystick::State::kLeft:
      if (flag&&focus==0)//leave item click
        flag=false;
      if (item.type!=VarType::kNan) {
    	ChangeItemValue(focus,0);
        PrintItemValue(focus, true);
      }
      break;
    case Joystick::State::kRight:
      if (flag&&focus==0)//leave item click
        flag=false;
      if (item.type!=VarType::kNan) {
    	ChangeItemValue(focus,1);
        PrintItemValue(focus, true);
      }
      break;
    default:
      return;
  }
  return;
}
