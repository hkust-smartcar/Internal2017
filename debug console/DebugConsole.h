#include <stdio.h>
#include <iostream>
#include <time.h>
#include <conio.h>

using namespace std;

#ifdef _WIN32

  #include <windows.h>

  void gotoxy( int x, int y ){
    COORD p = { x, y };
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), p );
    }

#else

  #include <unistd.h>
  #include <term.h>

  void gotoxy( int x, int y ){
    int err;
    if (!cur_term)
      if (setupterm( NULL, STDOUT_FILENO, &err ) == ERR)
        return;
    putp( tparm( tigetstr( "cup" ), y, x, 0, 0, 0, 0, 0, 0, 0 ) );
    }
    
#endif


//syntax sugars
//just down or start
#define START_IDLE 	0
#define DOWN_SELECT 1
#define DOWN_LEFT 	2
#define DOWN_RIGHT 	3
//just up or end5
#define END_IDLE 	4
#define UP_SELECT 	5
#define UP_LEFT 	6
#define UP_RIGHT 	7
//down for a while
#define LONG_IDLE 	8
#define LONG_SELECT 9
#define LONG_LEFT 	10
#define LONG_RIGHT 	11

#define IDLE 	0
#define SELECT 	1
#define LEFT 	2
#define RIGHT	3

#define START 	0
#define END 	1
#define DOWN 	0
#define UP		1
#define LONG 	2


#define GET_TIME 	clock()


//TODO 1 srink in LCD displayable

class DebugConsole{
	public:
		typedef void(*Fptr)();
		class Item{
			public:
			
			
			Item(char* text=NULL,int* value=NULL,bool readOnly=false);
			
			//display text methods
			char* getText();
			void setText(char* t);
			
			//listener methods
			Fptr getListener(int type);
			Fptr getListeners();
			void setListener(int type, Fptr fptr);
			
			//value methods
			void setValuePtr(int* v);
			void setValue(int v);
			int* getValuePtr();
			int getValue();
			
			bool isReadOnly();
			void setReadOnly(bool isReadOnly);
			
			private:
				Fptr listeners[12];
				int* value=NULL;
				char* text;
				bool readOnly;
				bool upLongExclusive;
				void init();
		};
	public:
		
		
		DebugConsole();
		
		void enterDebug();
		
		void pushItem(Item item);
		
		void insertItem(Item item, int index=0);
		
		void listItems(int start=0);
		
		void printItem(int index);
		
	private:
		
		int length;
		int focus;
		int topIndex;
		int threshold;
		Item items[1000];
		
		
		
		void printxy(int x, int y, char* c);
		
		void showFocus(bool flag=1);
		
		void clear();
		
		bool listen(char key,int state);
};
