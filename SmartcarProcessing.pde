import processing.serial.*; //<>//
import java.util.*;
import controlP5.*;
import g4p_controls.*;

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
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.clear();
  startTime = millis();
  frameRate(300);
  size(1200, 650);
  background(#04001F);
  cp5 = new ControlP5(this);
  
  btName = new String[btSize];
  tfName = new String[conSize];
  constantArr = new String[conSize];
  
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
  
  graphColor[0] = #031DFF;
  graphColor[1] = #03FF04;
  graphColor[2] = #FF0303;
}

void draw() {

  if (myPort.available() > 0) {

    int inputInt = myPort.read();

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
        displayText(inputString, 600, 600, 300, 32);
        stringToDouble(inputString);
      }
      getData();
      plotGraph();
      delay(1);
    }

    ////image
    //if (inputInt == 170) {

    //  int i = 0;
    //  arrayPosX = 0;
    //  arrayPosY = 0;

    //  while (i < camWidth*camHeight/8) {
    //    print('.');
    //    if (myPort.available() > 0) {
    //      inputInt = myPort.read();
    //      getImage(inputInt);
    //      i++;
    //    }
    //  }
    //  if (overviewMode == 0) {
    //    outputImage();
    //    getBoundary();
    //    outputBoundary();
    //    getRegion();
    //    outputRegion();
    //    turningResult();
    //  }
    //  delay(1);
    //}

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

  println(frameRate);
}