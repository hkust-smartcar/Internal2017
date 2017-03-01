import controlP5.*;
import java.util.*;
import processing.serial.*;

ControlP5 cp5;
Serial myPort;

PFont f1;
int NUM = 100;
color darkgray = #333333;
int inputInt;

int globalWidth = 80;
int globalHeight = 60;
int globalSpeed = 0;
int displaySpeed;

int pixelSide = 5;
Boolean[][] pixelArray = new Boolean[globalHeight][globalWidth];

int background_color = 17;
color red = #ff0000;
color black = #000000;

int arrayPosX, arrayPosY;
Boolean stop = false;
String dir = " ", var_string = " ";
char data_received = ' ';
char keyPress = ' ';

String Lencoder_count = "0", Rencoder_count = "0";
String center_line_received_x = " ";
String data_string;
int total_var = 9;

//for finding center line
int white_num;
int left_end = 0, right_end = globalWidth, center_x;

void setup() {
  
  printArray(Serial.list());
  //myPort = new Serial(this, Serial.list()[1], 115200);
  //myPort.buffer(1);
  size(800, 400 ,P3D);
  f1 = createFont("Helvetica", 12);

  cp5 = new ControlP5( this );
  cp5.addButton("auto");
  cp5.addButton("manual");
  cp5.addButton("stop_").setLabel("stop");
  
  Controller auto_btn = cp5.getController("auto");
  auto_btn.setColorBackground(color(#00AA00));
  auto_btn.setColorForeground(color(#00EE00));
  auto_btn.setPosition(715, 20);
  
  Controller manual_btn = cp5.getController("manual");
  manual_btn.setPosition(715, 50);
  
  Controller stop_btn = cp5.getController("stop_");
  stop_btn.setPosition(715, 80);
  stop_btn.setColorBackground(color(#AA0000));
  stop_btn.setColorForeground(color(#EE0000));

  // create a custom SilderList with name menu, notice that function 
  // menu will be called when a menu item has been clicked.

  SilderList m = new SilderList( cp5, "menu", 250, 350 );

  m.setPosition(40, 20);
  // add some items to our SilderList
  m.addItem(makeItem("Kp", 0, 0, 5));
  m.addItem(makeItem("Ki", 0, 0, 5));
  m.addItem(makeItem("Kd", 0, 0, 5));
  m.addItem(makeItem("speed", 300, 0, 500));
  m.addItem(makeItem("Max servo deg", 45, 0, 90));
  m.addItem(makeItem("Min servo deg", -45, -90, 0));
  m.addItem(makeItem("Max speed", 450, 300, 600));
  m.addItem(makeItem("Min speed", 0, 0, 200));
  m.addItem(makeItem("updateTime(ms)", 100, 0, 1000));
  /*for (int i=0;i<NUM;i++) {
    m.addItem(makeItem("slider-"+i, 0, -PI, PI ));
  }*/
}

public void auto(){
  println("auto btn is pressed!");
  myPort.write(1000);
}
public void manual(){
  println("manual btn is pressed!");
  myPort.write(1001);
}
public void stop_(){
  println("stop btn is pressed!");
  myPort.write(1002);
}

// a convenience function to build a map that contains our key-value  
// pairs which we will then use to render each item of the SilderList.
//
Map<String, Object> makeItem(String theLabel, float theValue, float theMin, float theMax) {
  Map m = new HashMap<String, Object>();
  m.put("label", theLabel);
  m.put("sliderValue", theValue);
  m.put("sliderValueMin", theMin);
  m.put("sliderValueMax", theMax);
  return m;
}

void getImage(int data) {

  String binData = "";
  int strLen = 0;
  
  binData = Integer.toBinaryString(data);
  strLen = binData.length();
  //println(binData);
  
  for (int i=0; i<8; i++) {
    
    if (i < strLen) {
      
      if (binData.charAt(strLen-1-i) == '1') {
        pixelArray[arrayPosY][arrayPosX+7-i] = true;
      } else {
        pixelArray[arrayPosY][arrayPosX+7-i] = false;
      }
      
    } else {
      pixelArray[arrayPosY][arrayPosX+7-i] = false;
    }
    
  }
  
  arrayPosX += 8;
  
  if (arrayPosX >= globalWidth) {
    arrayPosX = 0;
    arrayPosY++;
  }
  if (arrayPosY >= globalHeight) {
    arrayPosY = 0;
  }

}

void getCenterLine(int left_end_x, int right_end_x, int current_y){
  
  right_end_x-=1;
  if(left_end_x-5>=0) left_end_x-=5;
  if(right_end_x+5<=globalWidth-1) right_end_x+=5;
  
  if(!pixelArray[current_y][0]) left_end = 0;
  else {
    for(int x=left_end_x;x<globalWidth/2;x++){
      if(!pixelArray[current_y][x]&&!pixelArray[current_y][x+1]&&!pixelArray[current_y][x+2]){
        left_end = x;
        break;
      }
    }
  }
  
  if(!pixelArray[current_y][globalWidth-1]) right_end = globalWidth-1;
  else {
    for(int x=right_end_x;x>globalWidth/2;x--){
      if(!pixelArray[current_y][x]&&!pixelArray[current_y][x-1]&&!pixelArray[current_y][x-2]){
          right_end = x;
          break;
        }
    }
  }
  center_x = (left_end+right_end)/2;
  myPort.write(center_x);
}

void outputImage() {
  
  for (int y=0; y<globalHeight; y++) {
    getCenterLine(left_end, right_end ,y);
    for (int x=0; x<globalWidth; x++) {
      if (pixelArray[y][x]) {
        fill(black);
      } else {
        if(x == center_x) fill(red);
        else fill(255);
      }
      noStroke();
      rect(pixelSide*x, pixelSide*y, pixelSide, pixelSide);
    }
  }
  
}
void keyPressed() {
  
  keyPress = key;
  background(255);
  background(background_color);
  if(keyPress == ' ') {
    myPort.write(' ');
    stop = true;
  }
  else {
    stop = false;
    
    if(keyPress == 'w') myPort.write('w');
    else if(keyPress == 'a') myPort.write('a');
    else if(keyPress == 's') myPort.write('s');
    else if(keyPress == 'd') myPort.write('d');
    else if(keyPress == ',') myPort.write(',');
    else if(keyPress == '.') myPort.write('.');
  }
}
void keyReleased(){
  if(keyPress != '.' && keyPress!= ','){
    myPort.write('r');
    stop = true;
  }
  
}


void menu(int i) {
  println("got some slider-list event from item with index "+i);
}

public void controlEvent(ControlEvent theEvent) {
  if (theEvent.isFrom("menu")) {
    int index = int(theEvent.getValue());
    Map m = ((SilderList)theEvent.getController()).getItem(index);
    println("got a slider event from item : "+m);
    data_string = "";
    for(int i=0;i<total_var;i++){
      data_string += ((SilderList)theEvent.getController()).getItem(i).get("sliderValue") + " ";
    }
    println(data_string);
    myPort.write(data_string);
  }
}

void getKeyPressed(){
  switch(keyPress){
    case 'w':
      dir = "w";
      break;
    case 'a':
      dir = "a";
      break;
    case 's':
      dir = "s";
      break;
    case 'd':
      dir = "d";
      break;
    case '.':
      dir = ">";
      break;
    case ',':
      dir = "<";
      break;
    case ' ':
      dir = "stop";
      break;
  }
}

void draw() {
  
  background( 220 );
  fill(255);
  noStroke();
  pushMatrix();
  translate(300, 20);
  rect(0, 0, 400, 300);
  popMatrix();
  
  pushMatrix();
  translate(300, 330);
  fill(0);
  text("key pressed: "+dir, 0, 0);
  text("received: "+Character.toString(data_received), 0, 15);
  text("center_line_x: "+center_line_received_x, 0, 30);
  pushMatrix();
  translate(60, 0);
  text("Lencoder_count: "+Lencoder_count, 80, 0);
  text("Rencoder_count: "+Rencoder_count, 80, 15);
  popMatrix();
  popMatrix();
 
  /*if(myPort.available() > 0){
    inputInt = myPort.read();
    if(inputInt == 170){
      int i = 0;
      arrayPosX = 0;
      arrayPosY = 0;
      
      while (i<globalWidth*globalHeight/8) {
        print(' ');
        if (myPort.available() > 0) {  
          inputInt = myPort.read();
          getImage(inputInt);
          i++;
        }
      }
      
      noStroke();
      pushMatrix();
      translate(300, 20);
      outputImage();
      popMatrix();
      delay(1);
     
    } else if(inputInt == 171){ //receive data
      if(myPort.available() > 0) {
        data_received = myPort.readChar();
      }
    } else if(inputInt == 172){
      if(myPort.available() > 0) {
        var_string = myPort.readString();
        String []split = var_string.split("\\s+");
        cp5.getController("Kp").setValue(Integer.valueOf(split[0]));
        cp5.getController("Ki").setValue(Integer.valueOf(split[1]));
        cp5.getController("Kd").setValue(Integer.valueOf(split[2]));
        cp5.getController("speed").setValue(Integer.valueOf(split[3]));
        cp5.getController("Max servo deg").setValue(Integer.valueOf(split[4]));
        cp5.getController("Min servo deg").setValue(Integer.valueOf(split[5]));
        cp5.getController("Max speed").setValue(Integer.valueOf(split[6]));
        cp5.getController("Min speed").setValue(Integer.valueOf(split[7]));
        cp5.getController("updateTime(ms)").setValue(Integer.valueOf(split[8]));
        Lencoder_count = split[9];
        Rencoder_count = split[10];
        center_line_received_x = split[11];
      }
    }
  }*/
}


// A custom Controller that implements a scrollable SilderList.  
// Here the controller uses a PGraphics element to render customizable 
// list items. The SilderList can be scrolled using the scroll-wheel,  
// touchpad, or mouse-drag. Slider are triggered by a press or drag.  
// clicking the scrollbar to the right makes the list scroll to the item  
// correspoinding to the click-location.  
 
class SilderList extends Controller<SilderList> {

  float pos, npos;
  int itemHeight = 60;
  int scrollerLength = 40;
  int sliderWidth = 150;
  int sliderHeight = 15;
  int sliderX = 10;
  int sliderY = 25;

  int dragMode = 0;
  int dragIndex = -1;

  List< Map<String, Object>> items = new ArrayList< Map<String, Object>>();
  PGraphics menu;
  boolean updateMenu;

  SilderList(ControlP5 c, String theName, int theWidth, int theHeight) {
    super( c, theName, 0, 0, theWidth, theHeight );
    c.register( this );
    menu = createGraphics(getWidth(), getHeight());

    setView(new ControllerView<SilderList>() {

      public void display(PGraphics pg, SilderList t ) {
        if (updateMenu) {
          updateMenu();
        }
        if (inside() ) { // draw scrollbar
          menu.beginDraw();
          int len = -(itemHeight * items.size()) + getHeight();
          int ty = int(map(pos, len, 0, getHeight() - scrollerLength - 2, 2 ) );
          menu.fill( 128 );
          menu.rect(getWidth()-6, ty, 4, scrollerLength );
          menu.endDraw();
        }
        pg.image(menu, 0, 0);
      }
    }
    );
    updateMenu();
  }

  // only update the image buffer when necessary - to save some resources
  void updateMenu() {
    int len = -(itemHeight * items.size()) + getHeight();
    npos = constrain(npos, len, 0);
    pos += (npos - pos) * 0.1;

    /// draw the SliderList
    menu.beginDraw();
    menu.noStroke();
    menu.background(240);
    menu.textFont(cp5.getFont().getFont());
    menu.textSize(12);
    menu.pushMatrix();
    menu.translate( 0, int(pos) );
    menu.pushMatrix();

    int i0 = PApplet.max( 0, int(map(-pos, 0, itemHeight * items.size(), 0, items.size())));
    int range = ceil((float(getHeight())/float(itemHeight))+1);
    int i1 = PApplet.min( items.size(), i0 + range );

    menu.translate(0, i0*itemHeight);

    for (int i=i0;i<i1;i++) {
      Map m = items.get(i);
      menu.noStroke();
      menu.fill(200);
      menu.rect(0, itemHeight-1, getWidth(), 1 );
      menu.fill(darkgray);
      // uncomment the following line to use a different font than the default controlP5 font
      //menu.textFont(f1); 
      String txt = String.format("%s   %.2f", m.get("label").toString().toUpperCase(), f(items.get(i).get("sliderValue")));
      menu.text(txt, 10, 20 );
      menu.fill(255);
      menu.rect(sliderX, sliderY, sliderWidth, sliderHeight);
      menu.fill(100, 230, 128);
      float min = f(items.get(i).get("sliderValueMin"));
      float max = f(items.get(i).get("sliderValueMax"));
      float val = f(items.get(i).get("sliderValue"));
      menu.rect(sliderX, sliderY, map(val, min, max, 0, sliderWidth), sliderHeight);
      menu.translate( 0, itemHeight );
    }
    menu.popMatrix();
    menu.popMatrix();
    menu.endDraw();
    updateMenu = abs(npos-pos)>0.01 ? true:false;
  }

  // when detecting a click, check if the click happend to the far right,  
  // if yes, scroll to that position, otherwise do whatever this item of 
  // the list is supposed to do.
  public void onClick() {
    if (getPointer().x()>getWidth()-10) {
      npos= -map(getPointer().y(), 0, getHeight(), 0, items.size()*itemHeight);
      updateMenu = true;
    }
  }


  public void onPress() {
    int x = getPointer().x();
    int y = (int)(getPointer().y()-pos)%itemHeight;
    boolean withinSlider = within(x, y, sliderX, sliderY, sliderWidth, sliderHeight); 
    dragMode =  withinSlider ? 2:1;
    if (dragMode==2) {
      dragIndex = getIndex();
      float min = f(items.get(dragIndex).get("sliderValueMin"));
      float max = f(items.get(dragIndex).get("sliderValueMax"));
      float val = constrain(map(getPointer().x()-sliderX, 0, sliderWidth, min, max), min, max);
      items.get(dragIndex).put("sliderValue", val);
      setValue(dragIndex);
    }
    updateMenu = true;
  }

  public void onDrag() {
    switch(dragMode) {
      case(1): // drag and scroll the list
      npos += getPointer().dy() * 2;
      updateMenu = true;
      break;
      case(2): // drag slider
      float min = f(items.get(dragIndex).get("sliderValueMin"));
      float max = f(items.get(dragIndex).get("sliderValueMax"));
      float val = constrain(map(getPointer().x()-sliderX, 0, sliderWidth, min, max), min, max);
      items.get(dragIndex).put("sliderValue", val);
      setValue(dragIndex);
      updateMenu = true;
      break;
    }
  } 

  public void onScroll(int n) {
    npos += ( n * 4 );
    updateMenu = true;
  }
  void addItem(Map<String, Object> m) {
    items.add(m);
    updateMenu = true;
  }

  Map<String, Object> getItem(int theIndex) {
    return items.get(theIndex);
  }

  private int getIndex() {
    int len = itemHeight * items.size();
    int index = int( map( getPointer().y() - pos, 0, len, 0, items.size() ) ) ;
    return index;
  }
}

public static float f( Object o ) {
  return ( o instanceof Number ) ? ( ( Number ) o ).floatValue( ) : Float.MIN_VALUE;
}

public static boolean within(int theX, int theY, int theX1, int theY1, int theW1, int theH1) {
  return (theX>theX1 && theX<theX1+theW1 && theY>theY1 && theY<theY1+theH1);
}
