import processing.serial.*; //<>//
import java.util.*;
import controlP5.*;
import g4p_controls.*;

//Change constant
int consSize = 16;  //Number of constant to change
//Plot graph
int valSize = 2;  //Number of variable to plot
int range = 100;  //Upper limit of the graph
color[] colorArray = {#031DFF, #03FF04, #FF0303};  //Add color here for more variable
//BT port
Boolean btEnable = false;
int portNum = 0;  //Choose the correct port here
int baudRate = 115200; //Baud rate of the bluetooth
//Frame rate
int frate = 150; // Increase frame rate for real time graph plotting, depending on the rate of data

//Camera algorithm
void trytry() {

  if (millis() > 1000 && selectedIndex >= 0) {

    Boolean imageTemp[][] = imageData.get(selectedIndex);
    int zeroX = 460, zeroY = 320;

    for (int y=0; y<camHeight; y++) {
      for (int x=0; x<camWidth; x++) {
        if (imageTemp[y][x]) {
          fill(255);
        } else {
          fill(0);
        }
        noStroke();
        rect(zeroX + pixelSide*x, zeroY + pixelSide*y, pixelSide, pixelSide);
      }
    }
  }
}

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
 sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, leftSpeed, rightSpeed); //first one must be positive, seperated by commas, end by '\n'
 string speedStr = speedChar;
 const Byte speedByte = 85;
 bluetooth1.SendBuffer(&speedByte, 1);
 bluetooth1.SendStr(speedStr);
 */

ControlP5 cp5;
Serial myPort;

int startTime = 0, currentTime = 0;
int overviewMode = 0;
String inputString = "";

BufferedReader reader;
PrintWriter writer;

void displayText(String text, int x, int y, int w, int size) {

  fill(#D3D3D3);
  noStroke();
  rect(x-size, y-size, w, size*1.5);
  fill(0);
  textSize(size);
  text(text, x, y);
}

void keyPressed() {

  if (overviewMode == 0) {
    if (key == 'w') {
      myPort.write('w');
    } else if (key == 's') {
      myPort.write('s');
    } else if (key == 'i') {
      myPort.write('i');
    } else if (key == 'a') {
      myPort.write('a');
    } else if (key == 'd') {
      myPort.write('d');
    }
  }
}

void keyReleased() {

  if (overviewMode == 0) {
    if (key == 'w') {
      myPort.write('W');
    } else if (key == 's') {
      myPort.write('S');
    } else if (key == 'a') {
      myPort.write('A');
    } else if (key == 'd') {
      myPort.write('D');
    }
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
  tfName = new String[textFieldSize];
  constantArr = new String[textFieldSize];
  imageData = new ArrayList();
  imageName = new ArrayList();

  buttonSetUp();
  tfSetUp();
  listSetUp();

  for (int y=0; y<camHeight; y++) {
    for (int x=0; x<camWidth; x++) {
      pixelArray[y][x] = false;
    }
  }

  fill(#898989);
  noStroke();
  rect(imageX, imageY, camWidth*pixelSide, camHeight*pixelSide);
  rect(boundaryX, boundaryY, camWidth*pixelSide, camHeight*pixelSide);
  rect(regionX, regionY, camWidth*pixelSide, camHeight*pixelSide);

  value = new double[valSize][graphLength];
  newValue = new double[valSize];
  graphColor = new color[valSize];

  for (int i = 0; i < valSize; i++) {
    newValue[i] = 0;
    for (int j = 0; j < graphLength; j++) {
      value[i][j] = 0;
    }
  }
  for (int i = 0; i < valSize; i++) {
    graphColor[i] = colorArray[i];
  }
  plotGraph();
}

void draw() {

  
  if (btEnable && myPort.available() > 0) {

    int inputInt = myPort.read();

    //encoder
    if (inputInt == 85 && overviewMode <= 1) {

      while (true) {
        print(',');
        if (myPort.available() > 0) {
          inputString = myPort.readStringUntil('\n');
          break;
        }
      }
      if (inputString != null) {
        inputString = inputString.trim();
        //displayText(inputString, 600, 600, 500, 32);
        stringToDouble(inputString);
      }
      getData();
      plotGraph();
    }

    //image
    if (inputInt == 170) {

      int i = 0;
      arrayPosX = 0;
      arrayPosY = 0;

      while (i < camWidth*camHeight/8) {
        print('.');
        if (myPort.available() > 0) {
          inputInt = myPort.read();
          getImage(inputInt);
          i++;
        }
      }
      if (overviewMode == 0) {
        outputImage();
        getBoundary();
        outputBoundary();
        getRegion();
        outputRegion();
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