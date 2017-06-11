/*
 * debug_console.h
 *
 *  Created on: 2017爛5堎11
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

//template <class Number>
class Item{
	public:

	//Item(char* n):text(n){init();}
	//Item(char* text=NULL,float* value=NULL,bool readOnly=false):text(text),value(value),readOnly(readOnly),listener(NULL){}
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
	Item* setInterval(int v){interval=v;return this;}
	int getInterval(){return interval;}

	bool isReadOnly(){return readOnly;}
	Item* setReadOnly(bool isReadOnly){readOnly=isReadOnly;return this;}

	private:
		char* text;
		//float* value=NULL;
		int*value=NULL;
		bool readOnly;
		Fptr listener;
		int interval=1;
		void init(){
		}
};

class DebugConsole{

	public:

		/*
		 * constructor of debug console
		 * joystick, lcd, writer pointer and the display length, which limit the display size of debug console
		 */
		DebugConsole(Joystick* joystick,St7735r* lcd, LcdTypewriter* writer, int displayLength);

		/*
		 * pause and start debugging
		 */
		void EnterDebug();

		/*
		 * just listen and do, no pause
		 */
		void Listen();

		/*
		 * adding items to debug console
		 */
		void PushItem(Item item);
		void InsertItem(Item item, int index=0);

		/*
		 * print item start from topIndex, total amount displayLength
		 */
		void ListItems();

		/*
		 * print item start from topIndex, total amount displayLength
		 */
		void ListItemValues();

		/*
		 * print a single item's text and value
		 */
		void printItem(int index, bool isInverted=false);

		/*
		 * print a single item's value
		 */
		void printItemValue(int index, bool isInverted=false);

		//parameters setters
		DebugConsole* SetDisplayLength(int length);
		DebugConsole* SetLongClickThershold(int thershold);
		DebugConsole* SetLongClickCd(int cd);
		DebugConsole* SetOffset(int offset);

		//parameters getters
		int GetDisplayLength();
		int GetLongClickThershold();
		int GetLongClickCd();
		int GetOffset();
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

		void printxy(int x, int y, char* c, int inverted = false);
		void clear()
		int listenerDo(Joystick::State key);
};
}


#endif /* INC_DEBUG_CONSOLE_H_ */
