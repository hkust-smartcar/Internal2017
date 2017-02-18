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

class DebugConsole{
	public:
		class Item{
			public:
			Item(char* n):text(n){}
			Item(char* n,int* v):text(n),value(v){}
			Item(){}
			char* text;
			void(*onSelectListener)();
			void(*onLeftClickListener)();
			void(*onRightClickListener)();
			int* value=NULL;
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
					if (key=='p'||key==' '){key=getch();}	//P = pause
					Item item=items[focus];
					switch(key){
						case'w':focus=(focus+1)%length;showFocus();break;
						case's':focus=(focus-1+length)%length;showFocus();break;
						case' ':
							if(item.onSelectListener!=NULL)
								item.onSelectListener();
							else if(item.text=="exit")
								flag=0;
							printxy(0,10,"enter");
							break;
						case'a':
							if(item.onLeftClickListener!=NULL)
								item.onLeftClickListener();
							else if(item.value!=NULL){
								(*item.value)--;
								char buff[20];
								sprintf(buff,"%s = %d      ",item.text,*item.value);
								printxy(1,focus,buff);
							}
							
							break;
						case'd':
							if(item.onLeftClickListener!=NULL)
								item.onLeftClickListener();
							else if(item.value!=NULL){
								(*item.value)++;
								char buff[20];
								sprintf(buff,"%s = %d      ",item.text,*item.value);
								printxy(1,focus,buff);
							}
							
							break;
					}
				}
				
			}
		}
		
		void pushItem(Item item){
			items[length++]=item;
		}
		
		void insertItem(Item item, int index=0){
			index = (index<0?0:(index>length?length:index));
			for(int i=length++;i>=index;i--)
				items[i]=items[i-1];
			items[index]=item;
		}
		
		void listItems(){
			for(int i=0;i<length;i++){
				if(items[i].value!=NULL){
					char buff[20];
					sprintf(buff,"%s = %d      ",items[i].text,*items[i].value);
					printxy(1,i,buff);
				}else
					printxy(1,i,items[i].text);
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

int main(){
	DebugConsole console;
	
	int x=0;
	
	console.insertItem(DebugConsole::Item("hi",&x));
	
	console.enterDebug();
	
	cout<<"debug end";
}
