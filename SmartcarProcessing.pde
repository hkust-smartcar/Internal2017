import processing.serial.*; //<>//
import java.util.*;
import controlP5.*;
import g4p_controls.*;

//Change constant
int consSize = 3;  //Number of constant to change
//Plot graph
int valSize = 3;  //Number of variable to plot
int range = 50;  //Upper limit of the graph
color colorArray[] = {#031DFF, #03FF04, #FF0303};  //Add color here for more variable
//BT port
int portNum = 0;  //Choose the correct port here

/*
//Example of receiving constants in smartcar.
string str;  //global
bool tuning = false;  //global
vector<double> constVector;  //global

bool bluetoothListener(const Byte *data, const size_t size) {

  if (data[0] == 't') {
    tuning = 1;
    inputStr = "";
  } else if (tuning && data[0] != 't') {

    if (data[0] != '\n') {
      inputStr += (char)data[0];
    } else {
      tuning = 0;
      constVector.clear();

      char * pch;
      pch = strtok(&inputStr[0], " ,");
      while (pch != NULL){
        int constant;
        stringstream(pch) >> constant;
        constVector.push_back(constant);
        pch = strtok (NULL, " ,");
      }

      if (constVector[0] > 300)
        constVector[0] = 300;
      leftMotorPtr->SetPower(constVector[0]);
      rightMotorPtr->SetPower(constVector[0]);
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
int overviewMode = -1;
String inputString = "";

BufferedReader reader;
PrintWriter writer;

void displayText(String text, int x, int y, int w, int size) {

  fill(#04001F);
  noStroke();
  rect(x-size, y-size, w, size*1.5);
  fill(255);
  textSize(size);
  text(text, x, y);
}

void keyPressed() {

  if (key == 'g') {
    //if (leftOver) {
    //  save("leftData.txt");
    //} else if (middleOver) {
    //  save("middleData.txt");
    //} else if  (rightOver) {
    //  save("rightData.txt");
    //}
  } else if (key == 'w') {
    myPort.write('w');
  } else if (key == 's') {
    myPort.write('s');
  } else if (key == 'i') {
    myPort.write('i');
  }
  //else if (key == 'a') {
  //  myPort.write('a');
  //} else if (key == 'd') {
  //  myPort.write('d');
  //}
}

void keyReleased() {

  if (key == 'w') {
    myPort.write('W');
  } else if (key == 's') {
    myPort.write('S');
  }
  //else if (key == 'a') {
  //  myPort.write('A');
  //} else if (key == 'd') {
  //  myPort.write('D');
  //}
}

void setup() {

  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[portNum], 115200);
  myPort.clear();
  startTime = millis();
  frameRate(300);
  size(1200, 650);
  background(#04001F);
  cp5 = new ControlP5(this);
  
  btName = new String[btSize];
  tfName = new String[textFieldSize];
  constantArr = new String[textFieldSize];
  
  buttonSetUp();
  tfSetUp();

  for (int y=0; y<camHeight; y++) {
    for (int x=0; x<camWidth; x++) {
      pixelArray[y][x] = false;
    }
  }
  
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
}

void draw() {

  if (myPort.available() > 0) {

    int inputInt = myPort.read();
    //println(inputInt);

    //encoder
    if (inputInt == 85) {

      while (true) {
        print(',');
        if (myPort.available() > 0) {
          inputString = myPort.readStringUntil('\n');
          break;
        }
      }
      if (inputString != null) {
        inputString = inputString.trim();
        displayText(inputString, 600, 600, 500, 32);
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
        turningResult();
      }
    }

    ////feedback
    //if (inputInt == 171) {

    //  String inputString = "";

    //  while (true) {
    //    print(',');
    //    if (myPort.available() > 0) {
    //      inputString = myPort.readStringUntil('\n');
    //      break;
    //    }
    //  }
    //  if (inputString != null) {
    //    println(inputString);
    //  }
    //  delay(1);

    //}
  }

  //println(frameRate);
}