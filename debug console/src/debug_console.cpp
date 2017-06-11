/*
 * debug_console.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: dipsy
 */

#include "debug_console.h"

#include <cstdio>

#include "libsc/joystick.h"
#include "libsc/lcd_typewriter.h"
#include "libsc/st7735r.h"
#include "libsc/system.h"
#include "libutil/misc.h"

using namespace libsc;
using namespace libbase::k60;
using namespace DebugConsole;

/*
 * constructor of debug console
 * joystick, lcd, writer pointer and the display length, which limit the display size of debug console
 */
DebugConsole::DebugConsole(Joystick* joystick,St7735r* lcd, LcdTypewriter* writer, int displayLength):
	focus(0),topIndex(0),joystick(joystick),lcd(lcd),writer(writer),displayLength(displayLength),threshold(1000){
}

void DebugConsole::EnterDebug(){
	int flag=1;
	Item item(">>exit<<",&flag);
	PushItem(item);
	int index=items.size();
	ListItems();
	while(flag){
		Listen();
	}
	items.erase(items.end() - (index - items.size()));
	clear();
}

/*
 * just listen and do, no pause
 */
void DebugConsole::Listen(){
	if(jState!=joystick->GetState()){
		jState = joystick->GetState();
		tFlag = System::Time()+threshold;
		listenerDo(jState);
	}
	else if(System::Time()>tFlag){
		tFlag=System::Time()+cd;
		listenerDo(jState);
	}
}


/*
 * adding items to debug console
 */
void DebugConsole::PushItem(Item item){
	items.push_back(item);
	//items[length++]=item;
}

void DebugConsole::InsertItem(Item item, int index=0){
	items.insert(items.begin()+index,item);
}

/*
 * print item start from topIndex, total amount displayLength
 */
void DebugConsole::ListItems(){
	clear();
	for(int i = topIndex; i < (items.size() < topIndex+displayLength ? items.size() : topIndex+displayLength);i++){

		printItem(i,i==focus);
	}
	//showFocus(0);
}

/*
 * print item start from topIndex, total amount displayLength
 */
void DebugConsole::ListItemValues(){
	clear();
	for(int i = topIndex; i < (items.size() < topIndex+displayLength ? items.size() : topIndex+displayLength);i++){

		printItemValue(i,i==focus);
	}
	//showFocus(0);
}

/*
 * print a single item's text and value
 */
void DebugConsole::printItem(int index, bool isInverted=false){
	printxy(0,index-topIndex,items[index].getText(),isInverted);
	printItemValue(index,isInverted);
}

/*
 * print a single item's value
 */
void DebugConsole::printItemValue(int index, bool isInverted=false){
	if(items[index].getValuePtr()!=NULL){
		char buff[20];
		sprintf(buff,"%d      ",items[index].getValue());
		printxy(7,index-topIndex,buff,isInverted);
	}
}

//parameters setters
DebugConsole* DebugConsole::SetDisplayLength(int length){
	displayLength=length;
	return this;
}
DebugConsole* DebugConsole::SetLongClickThershold(int thershold){
	this->threshold = thershold;
	return this;
}
DebugConsole* DebugConsole::SetLongClickCd(int cd){
	this->cd = cd;
	return this;
}
DebugConsole* DebugConsole::SetOffset(int offset){
	this->offset = offset;
	return this;
}

//parameters getters
int DebugConsole::GetDisplayLength(){
	return displayLength;
}
int DebugConsole::GetLongClickThershold(){
	return threshold;
}
int DebugConsole::GetLongClickCd(){
	return cd;
}
int DebugConsole::GetOffset(){
	return offset;
}

void DebugConsole::printxy(int x, int y, char* c, int inverted = false){
	if(inverted){
		writer->SetTextColor(0x0000);
		writer->SetBgColor(0xFFFF);
	}
	lcd->SetRegion(Lcd::Rect(5+x*10,offset+y*15,128-5-x*10,15));
	writer->WriteString(c);
	if(inverted){
		writer->SetTextColor(0xFFFF);
		writer->SetBgColor(0x0000);
	}
	//showFocus(0);
}

/*void showFocus(bool flag=1){
	if(flag) writer->WriteString(" ");
	lcd->SetRegion(Lcd::Rect(0,(focus-topIndex)*15,10,15));
	writer->WriteString(">");
}*/

void DebugConsole::clear(){
	lcd->SetRegion(Lcd::Rect(0,offset,128,displayLength*15));
	lcd->FillColor(0x0000);
}

int DebugConsole::listenerDo(Joystick::State key){
	Item item=items[focus];
	switch(key){
		case Joystick::State::kDown:
			printItem(focus);	//remove highLighting
			focus++;				//move down focus
			if(focus>=items.size()){	//focus below last item then jump back to first
				topIndex = 0;
				focus = 0;
				ListItems();
			}
			else if(focus>=topIndex+displayLength){	//next page
				topIndex+=displayLength;
				ListItems();
			}
			else{
				printItem(focus,true);	//update highlighted item
			}
			break;
		case Joystick::State::kUp:
			printItem(focus);
			focus--;
			if(focus<0){
				focus=items.size()-1;
				topIndex = items.size()-displayLength;
				topIndex = (topIndex>0?topIndex:0);
				ListItems();
			}
			else if(focus<topIndex){
				topIndex-=displayLength;
				topIndex = (topIndex>0?topIndex:0);
				ListItems();
			}
			else{
				printItem(focus,true);
			}
			break;
		case Joystick::State::kSelect:
			if(item.getListener()!=NULL){
				item.getListener()();
			}
			else if(item.getText()==">>exit debug<<")
				return 0;
			break;
		case Joystick::State::kLeft:
			if(item.getValuePtr()!=NULL && !item.isReadOnly()){
				item.setValue(item.getValue()-item.getInterval());
				printItemValue(focus,true);
			}
			//ListItemValues();
			break;
		case Joystick::State::kRight:
			if(item.getValuePtr()!=NULL && !item.isReadOnly()){
				item.setValue(item.getValue()+item.getInterval());
				printItemValue(focus,true);
			}
			//ListItemValues();
			break;
		default:
			return 1;
	}
	//showFocus(1);
	//listItems(topIndex);
	return 1;
}
