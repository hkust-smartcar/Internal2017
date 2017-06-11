/*
 * debug_console.h
 *
 *  Created on: 2017Äê5ÔÂ11ÈÕ
 *      Author: WONG
 */

#ifndef INC_DEBUG_CONSOLE_H_
#define INC_DEBUG_CONSOLE_H_

#include <libsc/joystick.h>
#include <libsc/lcd_typewriter.h>
#include <libsc/st7735r.h>
#include <vector>

using namespace libsc;
using namespace libbase::k60;

namespace DebugConsole{

typedef void(*Fptr)();

class Item{
	public:

	//Item(char* n):text(n){init();}
	Item(char* text=NULL,int* value=NULL,bool readOnly=false):text(text),value(value),readOnly(readOnly),listener(NULL){}
	//Item(){init();}

	//display text methods
	char* getText(){return text;}
	Item* setText(char* t){text=t;return this;}

	//listener methods
	Fptr getListenerptr(){return listener;}
	Fptr getListener(){return *listener;}
	Item* setListener(Fptr fptr){listener=fptr;return this;}

	//value methods
	Item* setValuePtr(int* v){value=v;return this;}
	Item* setValue(int v){if(value==NULL)return this;*value=v;return this;}
	int* getValuePtr(){return value;}
	int getValue(){if(value==NULL)return 0;return *value;}

	bool isReadOnly(){return readOnly;}
	Item* setReadOnly(bool isReadOnly){readOnly=isReadOnly;return this;}

	private:
		char* text;
		int* value=NULL;
		bool readOnly;
		Fptr listener;
		void init(){
		}
};

class DebugConsole{

	public:

		/*
		 * constructor of debug console
		 * joystick, lcd, writer pointer and the display length, which limit the display size of debug console
		 */
		DebugConsole(Joystick* joystick,St7735r* lcd, LcdTypewriter* writer, int displayLength):
			focus(0),topIndex(0),joystick(joystick),lcd(lcd),writer(writer),displayLength(displayLength),threshold(1000){
		}

		/*
		 * pause and start debugging
		 */
		void EnterDebug(){
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
		void Listen(){
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
		void PushItem(Item item){
			items.push_back(item);
			//items[length++]=item;
		}

		void InsertItem(Item item, int index=0){
			items.insert(items.begin()+index,item);
		}

		/*
		 * print item start from topIndex, total amount displayLength
		 */
		void ListItems(){
			clear();
			for(int i = topIndex; i < (items.size() < topIndex+displayLength ? items.size() : topIndex+displayLength);i++){

				printItem(i,i==focus);
			}
			//showFocus(0);
		}

		/*
		 * print item start from topIndex, total amount displayLength
		 */
		void ListItemValues(){
			clear();
			for(int i = topIndex; i < (items.size() < topIndex+displayLength ? items.size() : topIndex+displayLength);i++){

				printItemValue(i,i==focus);
			}
			//showFocus(0);
		}

		/*
		 * print a single item's text and value
		 */
		void printItem(int index, bool isInverted=false){
			printxy(0,index-topIndex,items[index].getText(),isInverted);
			printItemValue(index,isInverted);
		}

		/*
		 * print a single item's value
		 */
		void printItemValue(int index, bool isInverted=false){
			if(items[index].getValuePtr()!=NULL){
				char buff[20];
				sprintf(buff,"%d      ",items[index].getValue());
				printxy(7,index-topIndex,buff,isInverted);
			}
		}

		//parameters setters
		DebugConsole* SetDisplayLength(int length){
			displayLength=length;
			return this;
		}
		DebugConsole* SetLongClickThershold(int thershold){
			this->threshold = thershold;
			return this;
		}
		DebugConsole* SetLongClickCd(int cd){
			this->cd = cd;
			return this;
		}
		DebugConsole* SetOffset(int offset){
			this->offset = offset;
			return this;
		}

		//parameters getters
		int GetDisplayLength(){
			return displayLength;
		}
		int GetLongClickThershold(){
			return threshold;
		}
		int GetLongClickCd(){
			return cd;
		}
		int GetOffset(){
			return offset;
		}
	private:


		int focus;
		int topIndex;
		std::vector<Item> items;
		Joystick* joystick;
		St7735r* lcd;
		LcdTypewriter* writer;
		Joystick::State jState = Joystick::State::kIdle;
		int tFlag=0;
		int threshold;		//long click thershold
		int displayLength;	//limit the region to display
		int cd=0;	//time needed to trigger next long click listener
		int offset=0; //distance away from top of lcd



		void printxy(int x, int y, char* c, int inverted = false){
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

		void clear(){
			lcd->SetRegion(Lcd::Rect(0,offset,128,displayLength*15));
			lcd->FillColor(0x0000);
		}

		int listenerDo(Joystick::State key){
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
						item.setValue(item.getValue()-1);
						printItemValue(focus,true);
					}
					//ListItemValues();
					break;
				case Joystick::State::kRight:
					if(item.getValuePtr()!=NULL && !item.isReadOnly()){
						item.setValue(item.getValue()+1);
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
};
}


#endif /* INC_DEBUG_CONSOLE_H_ */
