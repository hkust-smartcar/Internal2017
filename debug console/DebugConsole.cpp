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
			
			//Item(char* n):text(n){init();}
			Item(char* text=NULL,int* value=NULL,bool readOnly=false):text(text),value(value),readOnly(readOnly){init();}
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
				Fptr listeners[12];
				int* value=NULL;
				char* text;
				bool readOnly;
				bool upLongExclusive;
				void init(){
					for(int i=0;i<8;i++)
						listeners[i]=NULL;
				}
		};
	public:
		
		
		DebugConsole():length(0),focus(0),topIndex(0){
			Item item("exit");
			pushItem(item);
		}
		
		void enterDebug(){
			listItems();
			int flag=1,time_next;
			char key_img;
			while(flag){
				if(kbhit()){
					char key=getch();
					Item item=items[focus];
					if (key!=key_img){
						key_img=key;
						flag = listen(key,DOWN);
						time_next=GET_TIME+threshold;
					}
					else if(GET_TIME>time_next){
						flag = listen(key,LONG);
					}
					
				}
				else{
					flag = listen(key_img,UP);
					key_img='\o';
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
		
		void listItems(int start=0){
			clear();
			for(int i=start;i<(length<start+10?length:start+10);i++){
				printItem(i);
			}
			showFocus(0);
		}
		
		void printItem(int index){
			if(items[index].getValuePtr()!=NULL){
					char buff[20];
					sprintf(buff,"%s%d      ",items[index].getText(),items[index].getValue());
					printxy(1,index-topIndex,buff);
				}else
					printxy(1,index-topIndex,items[index].getText());
		}
		
	private:
		
		int length;
		int focus;
		int topIndex;
		int threshold;
		Item items[1000];
		
		
		
		void printxy(int x, int y, char* c){
			gotoxy(x,y);
			printf("%s",c);
			showFocus(0);
		}
		
		void showFocus(bool flag=1){
			if(flag) printf(" ");
			gotoxy(0,focus-topIndex);
			printf(">\b");
		}
		
		void clear(){system("CLS");}
		
		bool listen(char key,int state){
			Item item=items[focus];
			switch(key){
				case's':
					if(state!=UP)
						focus=(focus+1)%length;
					break;
				case'w':
					if(state!=UP)
						focus=(focus-1+length)%length;
					break;
				case' ':
					if(item.getListener(SELECT+state*4)!=NULL){
						item.getListener(SELECT+state*4)();
						//listItems(topIndex);
					}
					else if(item.getText()=="exit")
						return 0;
					break;
				case'a':
					if(item.getListener(LEFT+state*4)!=NULL){
						item.getListener(LEFT+state*4)();
						//listItems(topIndex);
					}
					else if(item.getValuePtr()!=NULL&&state!=UP&&!item.isReadOnly()){
						item.setValue(item.getValue()-1);
						//printItem(focus);
					}
					
					break;
				case'd':
					if(item.getListener(RIGHT+state*4)!=NULL){
						item.getListener(RIGHT+state*4)();
						//listItems(topIndex);
					}
					else if(item.getValuePtr()!=NULL&&state!=UP&&!item.isReadOnly()){
						item.setValue(item.getValue()+1);
						//printItem(focus);
					}
					
					break;
				default:
					return 1;
			}
			//showFocus();
			listItems(topIndex);
			return 1;
		}
};

void display(){
	system("CLS");
	printf("type any key return to debug console");
	getch();
	return;
}

void gt();
DebugConsole page2;
int main(){
	DebugConsole console;
	
	int x=0;
	
	console.insertItem(DebugConsole::Item("display x",&x,true));
	console.insertItem(DebugConsole::Item("nope"));
	console.insertItem(DebugConsole::Item("xddd"));
	console.insertItem(DebugConsole::Item("adjust x",&x));
	DebugConsole::Item item("trapper");
	item.setListener(UP_SELECT,&display);
	console.insertItem(item);
	item.setText("another trapper");
	console.pushItem(item);
	
	
	DebugConsole::Item gtp2("goto page2");
	
	gtp2.setListener(DOWN_SELECT,&gt);
	console.insertItem(gtp2);
	
	console.enterDebug();
	
	cout<<"debug end";
}
void gt(){
		page2.enterDebug();
}
