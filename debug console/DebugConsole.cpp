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


//=================================
#define ON_IDLE 	0
#define ON_CLICK 	1
#define ON_LEFT 	2
#define ON_RIGHT 	3
#define LONG_IDLE 	4
#define LONG_CLICK 	5
#define LONG_LEFT 	6
#define LONG_RIGHT 	7

class DebugConsole{
	public:
		typedef void(*Fptr)();
		class Item{
			public:
			
			//Item(char* n):text(n){init();}
			Item(char* text=NULL,int* value=NULL,bool readOnly=true):text(text),value(value),readOnly(readOnly){init();}
			//Item(){init();}
			/*Item(const Item &item){
				this->setText(item.getText());
				value=item.getValuePtr();
				for(int i=0;i<8;i++)
					listeners[i] = item.getListener(i);
			}*/
			
			//display text methods
			char* getText(){return text;}
			void setText(char* t){text=t;}
			
			//listener methods
			Fptr getListener(int type){return listeners[type];}
			Fptr getListeners(){return *listeners;}
			void setListener(int type, Fptr fptr){listeners[type]=fptr;}
			
			//value methods
			void setValuePtr(int* v){value=v;}
			void setValue(int v){*value=v;}
			int* getValuePtr(){return value;}
			int getValue(){return *value;}
			
			bool isReadOnly(){return readOnly;}
			void setReadOnly(bool isReadOnly){readOnly=isReadOnly;}
			
			private:
				Fptr listeners[8];
				int* value=NULL;
				char* text;
				bool readOnly;
				void init(){
					for(int i=0;i<8;i++)
						listeners[i]=NULL;
				}
		};
	public:
		
		
		DebugConsole():length(0),focus(0){
			Item item("exit");
			pushItem(item);
		}
		
		void enterDebug(){
			listItems();
			int flag=1;
			while(flag){
				if(kbhit()){
					char key=getch();
					Item item=items[focus];
					switch(key){
						case's':focus=(focus+1)%length;showFocus();break;
						case'w':focus=(focus-1+length)%length;showFocus();break;
						case' ':
							if(item.getListener(ON_CLICK)!=NULL)
								item.getListener(ON_CLICK)();
							else if(item.getText()=="exit")
								flag=0;
							printxy(0,10,"enter");
							break;
						case'a':
							if(item.getListener(ON_LEFT)!=NULL)
								item.getListener(ON_LEFT)();
							else if(item.getValuePtr()!=NULL){
								item.setValue(item.getValue()-1);
								char buff[20];
								sprintf(buff,"%s = %d      ",item.getText(),item.getValue());
								printxy(1,focus,buff);
							}
							
							break;
						case'd':
							if(item.getListener(ON_RIGHT)!=NULL)
								item.getListener(ON_RIGHT)();
							else if(item.getValuePtr()!=NULL){
								item.setValue(item.getValue()+1);
								char buff[20];
								sprintf(buff,"%s = %d      ",item.getText(),item.getValue());
								printxy(1,focus,buff);
							}
							
							break;
					}
				}
				
			}
		}
		
		void pushItem(Item item){
			insertItem(item,length-1);
			//items[length++]=item;
		}
		
		void insertItem(Item item, int index=0){
			index = (index<0?0:(index>length?length:index));
			for(int i=length++;i>=index;i--)
				items[i]=items[i-1];
			items[index]=item;
		}
		
		void listItems(){
			for(int i=0;i<length;i++){
				if(items[i].getValuePtr()!=NULL){
					char buff[20];
					sprintf(buff,"%s = %d      ",items[i].getText(),items[i].getValue());
					printxy(1,i,buff);
				}else
					printxy(1,i,items[i].getText());
			}
			showFocus(0);
		}
		
	private:
		
		int length;
		int focus;
		Item items[1000];
		
		
		
		void printxy(int x, int y, char* c){
			gotoxy(x,y);
			printf("%s",c);
			showFocus(0);
		}
		
		void showFocus(bool flag=1){
			if(flag) printf(" ");
			gotoxy(0,focus);
			printf(">\b");
		}
};


void gt();
DebugConsole page2;
int main(){
	DebugConsole console;
	
	int x=0;
	
	console.insertItem(DebugConsole::Item("hi",&x));
	
	
	DebugConsole::Item gtp2("goto page2");
	
	gtp2.setListener(ON_CLICK,&gt);
	console.insertItem(gtp2);
	
	console.enterDebug();
	
	cout<<"debug end";
}
void gt(){
		page2.enterDebug();
}
