import processing.serial.*;
import java.util.*;

Serial myPort;

int globalWidth = 80;
int globalHeight = 60;
int globalSpeed = 0;
int displaySpeed;

int pixelSide = 5;
Boolean[][] pixelArray = new Boolean[globalHeight][globalWidth];
int imageX = 10, imageY = 10;

int background_color = 17;
color red = #ff0000;
color black = #000000;

int arrayPosX, arrayPosY;

int leftX = 600, leftY = 600;
int middleX = 700, middleY = 600;
int rightX = 800, rightY = 600;
int diameter = 50;

Boolean stop = false;
String dir = " ";
char c = 'x';
char keyPress = ' ';

int Lencoder_count = 0, Rencoder_count = 0;

void setup() {
  
  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.buffer(1);
  size(900, 650);
  arrayPosX = 0;
  arrayPosY = 0;
  background(background_color);
  
  for (int y=0; y<globalHeight; y++) {
    for (int x=0; x<globalWidth; x++) {
      pixelArray[y][x] = false;
    }
  }
  
  fill(0);
  ellipse(leftX, leftY, diameter, diameter);
  ellipse(middleX, middleY, diameter, diameter);
  ellipse(rightX, rightY, diameter, diameter);
  

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

void outputImage() {
  
  for (int y=0; y<globalHeight; y++) {
    for (int x=0; x<globalWidth; x++) {
      if (pixelArray[y][x]) {
        fill(black);
      } else {
        fill(255);
      }
      noStroke();
      rect(imageX + pixelSide*x, imageY + pixelSide*y, pixelSide, pixelSide);
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

void draw() {
  
  fill(200);
  stroke(255);
  rect(460, 40, 370, 400, 10);
  
  textSize(32);
  fill(0);
  text("current speed: ", 500, 90);
  if(stop) {
    fill(red);
    text("stopped!", 580, 130);
  }
  fill(0);
  text(globalSpeed, 750, 90);
  
  textSize(32);
  fill(0);
  switch(keyPress){
    case 'w':
      dir = "forward";
      break;
    case 'a':
      dir = "left";
      break;
    case 's':
      dir = "backward";
      break;
    case 'd':
      dir = "right";
      break;
    case '.':
      dir = ">";
      break;
    case ',':
      dir = "<";
      break;
    case ' ':
      dir = " ";
      break;
  }
  text(dir, 580, 200);
  
  text("Lencoder_count: "+String.valueOf(Lencoder_count), 500, 300);
  text("Rencoder_count: "+String.valueOf(Rencoder_count), 500, 350);
  
  if (myPort.available() > 0) {
    int inputInt = myPort.read();
    
    if (inputInt == 170) {
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
      outputImage();
      delay(1);
      
    }
    else if(inputInt == 172){
      if(myPort.available() > 0){
        Lencoder_count = myPort.read();
      }
    }
    else if(inputInt == 171){
      if(myPort.available() > 0) {
        //c = myPort.readChar();
        textSize(32);
        fill(0);
        text(myPort.readChar(), 580, 250);
      }
    }
    else if(inputInt == 169){
      if(myPort.available() > 0){
        globalSpeed = myPort.read();
      }
    }
    else if(inputInt == 168){
      if(myPort.available() > 0){
        Rencoder_count = myPort.read();
      }
    }
    
  }
  
}
