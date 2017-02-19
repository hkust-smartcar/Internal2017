# Debug Console
for fun debugging
console version is done in commit [39c70f0 ](https://github.com/hkust-smartcar/Internal2017/commit/09093f3420c14adf03c32004ad762ef9dd9e06ca)

MCU version is still on development, anyone can feel free to play with console version first

# How to Use
use joystick to navigate and adjust the value of destinated variables
in console version, use WASD spacebar instead
use up and down to choose between rows, left right to adjust value, select to choose
you can also change to use your own function by setting listener function


### Init a debug console
```c++
DebugConsole console;
```
### Add an Item in Console
debug console is displaying multiple items
###### default constructor of an item
```c++
/*readOnly is for Item which can change value*/
Item(char* text=NULL,int* value=NULL,bool readOnly=false);
```
###### add the item to console
```c++
void pushItem(Item item);//append the item at highest index before exit
void insertItem(Item item, int index=0);//insert item to that index, default 0
```
###### example
```C++
//insert an item which can adjust the value of x
console.insertItem(DebugConsole::Item("adjust x = ",&x));

//append an item which can only read the value of x
console.insertItem(DebugConsole::Item("display x = ",&x,true));
```

### Enter the interface of DebugConsole
```C++
DebugConsole console;
console.enterDebug();
```

### Add a listener to item
```C++
void display(){
	system("CLS");
	printf("type any key return to debug console");
	getch();
	return;
}

int main(){
    DebugConsole console;
    DebugConsole::Item item("trapper");
	item.setListener(UP_SELECT,display);
	console.insertItem(item);
	console.enterDebug();
}
```
BTW, an item can have multiple listener

### Nested debug console
```C++
void nextPage();
DebugConsole* page2;
int main(){
    DebugConsole console1;
    DebugConsole console2;
    page2=&console2;
    DebugConsole::Item item("next page");
    item.setListener(DOWN_SELECT,nextPage);
    console1.pushItem(item);
    console1.enterDebug();
}
void nextPage(){page2->enterDebug();}
```
