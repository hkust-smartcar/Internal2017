import processing.serial.*; //<>// //<>//
import java.util.*;
import controlP5.*;
import g4p_controls.*;

//Frame rate
int frate = 160; // Increase frame rate for real time graph plotting, depending on the rate of data
//BT port
Boolean btEnable = true;
int portNum = 1;  //Choose the correct port here
int baudRate = 115200; //Baud rate of the bluetooth
//Plot graph
Boolean graphEnable = false;
float range1 = 20000;  //Upper limit of the graph
color[] colorArray = {#031DFF, #03FF04, #FF0303, #FF0303};  //Add color here for more variable
//Image
String imageFileName = "imageData128x480.txt";  //match with camWith and camHeight
int camWidth = 128, camHeight = 480;
int pixelSide = 1;

/*
//Example of receiving constants in smartcar.
 string str;  //global
 bool tuning = false;  //global
 vector<double> constVector;  //global
 
 bool bluetoothListener(const Byte *data, const size_t size) {
 
 if (data[0] == 't') {
 tune = 1;
 inputStr = "";
 }
 if (tune) {
 int i = 0;
 while (i<size) {
 if (data[i] != 't' && data[i] != '\n') {
 inputStr += (char)data[i];
 } else if (data[i] == '\n') {
 tune = 0;
 }
 i++;
 }
 if (!tune) {
 constVector.clear();
 
 char * pch;
 pch = strtok(&inputStr[0], " ,");
 while (pch != NULL){
 double constant;
 stringstream(pch) >> constant;
 constVector.push_back(constant);
 pch = strtok (NULL, " ,");
 }
 
 //global variable
 powAngP = constVector[0];
 powAngI = constVector[1];
 powAngD = constVector[2];
 }
 }
 
 }
 
 */

/*
//Example of sending 2 variables from smartcar.
 char speedChar[15] = {};
 sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, curSpeed, targetSpeed, curAng, targetAng); //first one must be 1.0, seperated by ',', end by '\n'
 string speedStr = speedChar;
 const Byte speedByte = 85;
 bluetooth1.SendBuffer(&speedByte, 1);
 bluetooth1.SendStr(speedStr);
*/
 
/*
//Sending image
  const Byte imageByte = 170;
  bluetooth1.SendBuffer(&imageByte, 1);
  bluetooth1.SendBuffer(camInput, Cam1.GetBufferSize());
*/

float constant1 = 0.03, constant2 = 0.03, constant3 = 0.07;
//Camera algorithm
void trytry() {

  if (selectedIndex >= 0) {

    background(#D3D3D3);

    Boolean imageTemp[][] = imageData.get(selectedIndex);
    int zeroX = 280, zeroY = 200;

    //output the image
    noStroke();
    for (int y=0; y<camHeight; y++) {
      for (int x=0; x<camWidth; x++) {
        if (imageTemp[y][x]) {
          fill(0);
        } else {
          fill(255);
        }
        rect(zeroX + pixelSide*x, zeroY + pixelSide*y, pixelSide, pixelSide);
      }
    }

    int curX = camWidth/2, curY = camHeight-1;
    if (mouseX>zeroX && mouseX<zeroX+pixelSide*camWidth
      && mouseY>zeroY && mouseY<zeroY+pixelSide*camHeight) {
      curX = (mouseX-zeroX)/pixelSide;
      curY = (mouseY-zeroY)/pixelSide;
      fill(100);
      rect(zeroX + pixelSide*curX, zeroY + pixelSide*curY, pixelSide, pixelSide);
    }

    int centreX = 920, centreY = 300;
    float upLeftSum = 0, upRightSum = 0;
    float leftSum = 0, rightSum = 0, upSum = 0;

    strokeWeight(3);
    stroke(100);
    upSum = getSum(imageTemp, curX, curY, 0, -1, 60);
    line(centreX, centreY, centreX, centreY-upSum);
    upLeftSum = getSum(imageTemp, curX, curY, -1, -1, 30);
    line(centreX, centreY, centreX-upLeftSum*cos(45), centreY-upLeftSum*cos(45));
    upRightSum = getSum(imageTemp, curX, curY, 1, -1, 30);
    line(centreX, centreY, centreX+upRightSum*cos(45), centreY-upRightSum*cos(45));
    leftSum = getSum(imageTemp, curX, curY, -1, 0, 30);
    line(centreX, centreY, centreX-leftSum, centreY);
    rightSum = getSum(imageTemp, curX, curY, 1, 0, 30);
    line(centreX, centreY, centreX+rightSum, centreY);
    
    noStroke();
    float tempX = curX, tempY = curY;
    while (true) {
      fill(100);
      rect(zeroX + pixelSide*(int)tempX, zeroY + pixelSide*(int)tempY, pixelSide, pixelSide);
      upSum = getSum(imageTemp, (int)tempX, (int)tempY, 0, -1, 30);
      upLeftSum = getSum(imageTemp, (int)tempX, (int)tempY, -1, -1, 30);
      upRightSum = getSum(imageTemp, (int)tempX, (int)tempY, 1, -1, 30);
      leftSum = getSum(imageTemp, (int)tempX, (int)tempY, -1, 0, 30);
      rightSum = getSum(imageTemp, (int)tempX, (int)tempY, 1, 0, 30);
      
      tempX += (constant1*(upLeftSum-upRightSum) * (constant3*upSum) + constant2*(leftSum-rightSum));
      //tempX += (constant1*(upLeftSum-upRightSum) + constant2*(leftSum-rightSum));
      tempY--;
      if (tempX<0 || tempX>camWidth || tempY<0 || upSum>80) {
        break;
      }
      
    }
  }
}

ControlP5 cp5;
Serial myPort;

int startTime = 0, currentTime = 0;
int viewMode = 0;
String inputString = "";
Boolean pressed = false;

BufferedReader reader;
PrintWriter writer;

float getDistance(float x1, float y1, float x2, float y2) {
  return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}

int getSum(Boolean[][] imageTemp, int x, int y, int xInc, int yInc, int size) {
  float sum = 0;
  int xTemp = x+xInc;
  int yTemp = y+yInc;
  int counter = 0;
  while (xTemp>=0 && xTemp<camWidth
    && yTemp>=0 && yTemp<camHeight
    && counter<size) {
    if (imageTemp[yTemp][xTemp]) {
      sum += (float)50/getDistance(x, y, xTemp, yTemp);
    }
    counter++;
    xTemp += xInc;
    yTemp += yInc;
  }
  return (int)sum;
}

void displayText(String text, int x, int y, int w, int size) {

  fill(#D3D3D3);
  noStroke();
  rect(x-size, y-size, w, size*1.5);
  fill(0);
  textSize(size);
  text(text, x, y);
}

void keyPressed() {

  if (key == ' ' && btEnable) {
    myPort.write('P');
  }
  if (!pressed && btEnable) {
    if (keyCode == UP) {
      myPort.write('w');
      pressed = true;
    } else if (keyCode == DOWN) {
      myPort.write('s');
      pressed = true;
    } else if (keyCode == LEFT) {
      myPort.write('a');
      pressed = true;
    } else if (keyCode == RIGHT) {
      myPort.write('d');
      pressed = true;
    }
  }
}

void keyReleased() {

  if (keyCode == UP) {
    myPort.write('W');
    pressed = false;
  } else if (keyCode == DOWN) {
    myPort.write('S');
    pressed = false;
  } else if (keyCode == LEFT) {
    myPort.write('A');
    pressed = false;
  } else if (keyCode == RIGHT) {
    myPort.write('D');
    pressed = false;
  }
}

void setup() {

  printArray(Serial.list());
  if (btEnable) {
    myPort = new Serial(this, Serial.list()[portNum], baudRate);
    myPort.clear();
  }
  startTime = millis();
  frameRate(frate);
  size(1200, 660);
  background(#D3D3D3);
  cp5 = new ControlP5(this);

  btName = new String[btSize];
  constantList = new ArrayList();
  imageData = new ArrayList();
  imageName = new ArrayList();

  buttonSetUp();
  taSetUp();
  listSetUp();
  sliderSetUp();

  for (int y=0; y<camHeight; y++) {
    for (int x=0; x<camWidth; x++) {
      pixelArray[y][x] = false;
    }
  }

  fill(#898989);
  noStroke();
  rect(imageX, imageY, camWidth*pixelSide, camHeight*pixelSide);
  fill(#404040);
  stroke(0);
  rect(graphOneX, graphOneY, graphWidth, graphHeight);
  stroke(255);
  strokeWeight(0);
  line(graphOneX+1, graphOneY+graphHeight/2, graphOneX+graphWidth-1, graphOneY+graphHeight/2);
}

void draw() {

  if (viewMode == 1) {
    trytry();
  }

  if (viewMode == 0 && btEnable && myPort.available() > 0) {

    int inputInt = myPort.read();
    println(inputInt);

    //graph
    if (inputInt == 85 && viewMode == 0) {

      while (true) {
        print(',');
        inputString = myPort.readStringUntil('\n');
        if (inputString != null) {
          break;
        }
      }
      if (inputString != null) {
        inputString = inputString.trim();
        displayText(inputString, 150, 620, 1000, 32);
      }
      if (graphEnable) {
        stringToDouble(inputString);
        getData();
        plotGraph();
      }
    }

    //image
    if (inputInt == 170) {

      int i = 0, camByte = 0;
      arrayPosX = 0;
      arrayPosY = 0;

      while (i < camWidth*camHeight/8) {
        print('.');
        if (myPort.available() > 0) {
          camByte = myPort.read();
          getImage(camByte);
          i++;
        }
      }
      if (viewMode == 0) {
        outputImage();
      }
    }

    //feedback
    if (inputInt == 171) {

      String inputString = "";

      while (true) {
        print(',');
        if (myPort.available() > 0) {
          inputString = myPort.readStringUntil('\n');
          break;
        }
      }
      if (inputString != null) {
        println(inputString);
      }
      delay(1);
    }
  }
}