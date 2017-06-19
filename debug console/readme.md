# Debug Console V3.1
for fun debugging

To get started, add the header file [debug_console.h](https://github.com/hkust-smartcar/Internal2017/blob/dipsy/debug%20console/V3/debug_console.h) and the source file [debug_console.cpp](https://github.com/hkust-smartcar/Internal2017/blob/dipsy/debug%20console/V3/debug_console.cpp) in `inc` and `src` folder respectively.

# Overview
Use joystick to navigate and adjust the value of destinated variables

In console version, use WASD spacebar instead

Use up and down to choose between rows, left right to adjust value, select to choose

You can also change to use your own function by setting listener function (v1)

# What is new in V3.1
- nothing but better interface

# What is new in V3
- support flash, which allow the value to stay in the program when restart the smartcar. The data still have some chances to lose when restart, but very low.

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
Item(char* text=NULL, float* value=NULL, bool flashable, bool readOnly=false);
```
##### add the item to console
```c++
void PushItem(Item item);//append the item at highest index before exit
void InsertItem(Item item, int index=0);//insert item to that index, default 0
```
##### example
```C++
//insert an item which can adjust the value of x
console.InsertItem(Item("adjust x = ",&x));

//append an item which can only read the value of x but cannot store in the chip
console.InsertItem(Item("display x = ",&x,false,true));
```

### Enter the interface of DebugConsole
```C++
DebugConsole console;
console.EnterDebug("Exit text");
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
	item.SetListener(display);
	console.InsertItem(item);
	console.EnterDebug();
}
```

### Shortcut for setting
```C++
console.PushItem(*(item.setText("hi")->setValue(&x)->setInterval(0.1)));
```

### Nested debug console
```C++
void nextPage();
DebugConsole* page2;
int main(){
    DebugConsole console1(...);
    DebugConsole console2(...);
    page2=&console2;
    DebugConsole::Item item("next page");
    item.SetListener(nextPage);
    console1.PushItem(item);
    console1.EnterDebug("Exit");
}
void nextPage(){page2->EnterDebug("back to page 1");}
```

## How to store permanant value (flash)
```C++
DebugConsole console(pJoystick,pLcd,pWriter,2); //joystick, lcd and typewriter pointer, diplay 2 items at the same time
Item item();
item.SetValuePtr(&z)->SetText("z")->SetFlashable(true);	//item set value pointer (must be float), printed text, and flashable(important, or will not store)
console.PushItem(item);
item.SetValuePtr(&q)->SetText("q")->SetFlashable(true);
console.PushItem(item);
console.SetFlash(pFlash);	//flash pointer
console.Load();			//load old value from mcu
console.ListItems();		//print console items to the lcd
while(1){
	console.Listen();	//console response to key press of joystick
	if(System::Time()%50==0){
		led->Switch();
		console.Save();	//store all value of all items in console which have marked flashable
	}
}
```
if use EnterDebug function, it will auto load and auto save
