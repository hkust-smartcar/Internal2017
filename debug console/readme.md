# Debug Console V2
for fun debugging

To get started, include the header file [MCU_debug_console_V2.h](https://github.com/hkust-smartcar/Internal2017/blob/dipsy/debug%20console/MCU_debug_console_V2.h) as `debug_console.h` in `inc` folder.

# Overview
Use joystick to navigate and adjust the value of destinated variables

In console version, use WASD spacebar instead

Use up and down to choose between rows, left right to adjust value, select to choose

You can also change to use your own function by setting listener function (v1)

# What is new in V2
- Changed item array to vector, allowing infinite items in a single console
- Changed the cursor from > to inverted color to save space
- Removed listeners except on select listener, which will become long click listener automatically
- Added `Listen()`, which enable the use of debug console without pausing the program
- Added `displayLength`, which let the debug console would not occupy the whole LCD
- Added `interval` for setting value increment/decrement interval
- Value changed to `float`
- Added `offset` which change the y location of the whole console in the lcd

# How to use
### Init a debug console
`displayLength` is the number of items to show in a single page
```c++
DebugConsole console(Joystick*,St7735r*,LcdTypewriter*,int displayLength);
```
### Add an Item in Console
debug console is displaying multiple items
##### default constructor of an item
```c++
/*readOnly is for Item which can change value*/
Item(char* text=NULL,float* value=NULL,bool readOnly=false);
```
##### add the item to console
```c++
void pushItem(Item item);//append the item at highest index before exit
void insertItem(Item item, int index=0);//insert item to that index, default 0
```
##### example
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

### Using the debug console while running the program
The led will be shining while the debug console is working
```C++
console.ListItems();
while(true){
    if(System::Time()%250){
        Led0.Switch();
    }
    console.Listen();
}
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
    DebugConsole::Item item("goto page2");
	item.setListener(display);
	console.insertItem(item);
	console.enterDebug();
}
```
BTW, an item can have multiple listener

### Shortcut for setting
```C++
console.push(*(item.setText("hi")->setValue(&x)->setInterval(0.1)));
```

### Nested debug console
```C++
void nextPage();
DebugConsole* page2;
int main(){
    DebugConsole console1;
    DebugConsole console2;
    page2=&console2;
    DebugConsole::Item item("next page");
    item.setListener(nextPage);
    console1.PushItem(item);
    console1.EnterDebug();
}
void nextPage(){page2->EnterDebug();}
```
