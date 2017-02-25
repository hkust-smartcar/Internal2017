import processing.serial.*;
import java.util.*;
import g4p_controls.*;

Serial myPort;

BufferedReader reader;
PrintWriter writer;

int startTime = 0, currentTime = 0;
HScrollbar hs1, hs2;

int overviewMode = 0;

int buttonDiameter = 50;
int leftX = 100, leftY = 600;
int middleX = 200, middleY = 600;
int rightX = 300, rightY = 600;
int modeButtonX = 1100, modeButtonY = 60;
boolean leftOver = false;
boolean middleOver = false;
boolean rightOver = false;
boolean modeButtonOver = false;
boolean tuneButtonOver = false;

int outputVal = 0;
String constantArr[];
int conSize = 3;

boolean overCircle(int x, int y, int buttonDiameter) {
  float disX = x - mouseX;
  float disY = y - mouseY;
  if (sqrt(sq(disX) + sq(disY)) < buttonDiameter/2 ) {
    return true;
  } else {
    return false;
  }
}

void checkOver() {
  
  leftOver = middleOver = rightOver = modeButtonOver = tuneButtonOver = false;
  
  if (overCircle(modeButtonX, modeButtonY, buttonDiameter)) {
    modeButtonOver = true;
  }
  
  if (overviewMode == 0) {
    
    leftOver = overCircle(leftX, leftY, buttonDiameter);
    middleOver = overCircle(middleX, middleY, buttonDiameter);
    rightOver = overCircle(rightX, rightY, buttonDiameter);
    
  } else if (overviewMode == 1) {
    
    tuneButtonOver = overCircle(tuneButtonX, tuneButtonY, buttonDiameter);
  }
  
}

void updateScreen() {
  
  if (overviewMode == 0) {
    
    fill(#898989);
    noStroke();
    rect(imageX, imageY, camWidth*pixelSide, camHeight*pixelSide);
    rect(boundaryX, boundaryY, camWidth*pixelSide, camHeight*pixelSide);
    rect(regionX, regionY, camWidth*pixelSide, camHeight*pixelSide);
    fill(0);
    noStroke();
    ellipse(leftX, leftY, buttonDiameter, buttonDiameter);
    ellipse(middleX, middleY, buttonDiameter, buttonDiameter);
    ellipse(rightX, rightY, buttonDiameter, buttonDiameter);
    fill(#404040);
    stroke(0);
    rect(graphOneX, graphOneY, graphWidth, graphHeight);
    rect(graphTwoX, graphTwoY, graphWidth, graphHeight);
    
  } else if (overviewMode == 1) {
    
    fill(#404040);
    stroke(0);
    rect(graphOneX, graphOneY, graphWidth, graphHeight);
    rect(graphTwoX, graphTwoY, graphWidth, graphHeight);
    
  } else if (overviewMode == 2) {
      
  }
    
  fill(#6F0000);
  noStroke();
  ellipse(modeButtonX, modeButtonY, buttonDiameter, buttonDiameter);
  
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

void mousePressed() {
  
  if (modeButtonOver) {
    
    background(#04001F);
    overviewMode = ++overviewMode % 3;
    
    if (overviewMode == 0) {
      
      graphWidth = 400;
      graphOneX = 100;
      graphOneY = 300;
      graphTwoX = 600;
      graphTwoY = 300;
      
    } else if (overviewMode == 1) {
      
      graphWidth = 400;
      graphOneY = 50;
      graphTwoY = 50;
      readConstant();
      txfSetUp();
      
    } else if (overviewMode == 2) {
      
      txfRemove();
      
    }
    
  } else if (tuneButtonOver && overviewMode == 1) {
    
    editConstant();
    myPort.write('t' + constantArr[0] + '\n');
    
  }
  
}

void setup() {
  
  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[1], 115200);
  startTime = millis();
  size(1150, 650);
  
  for (int y=0; y<camHeight; y++) {
    for (int x=0; x<camWidth; x++) {
      pixelArray[y][x] = false;
    }
  }
  
  leftSpeedValue = new double[graphLength];
  rightSpeedValue = new double[graphLength];
  for(int i = 0; i < graphWidth; i++) {
    leftSpeedValue[i] = 0;
    rightSpeedValue[i] = 0;
  }
  
  constantArr = new String[conSize];
  //hs1 = new HScrollbar(sb1x, sb1y, sb1w, 16, 1);
  //hs2 = new HScrollbar(sb2x, sb2y, sb2w, 16, 1);

}

void draw() {
  
  if (myPort.available() > 0) {
    
    int inputInt = myPort.read();
    
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
      
      delay(1);
      
    }
    
    //encoder
    else if (inputInt == 85) {
      
      String inputString = "";
      
      while (true) {
        print(',');
        if (myPort.available() > 0) {
          inputString = myPort.readStringUntil('\n');
          break;
        }
      }
      if (inputString != null) {
        stringToDouble(inputString);
        //fill(255);
        //textSize(32);
        //text(inputString, 610, 600);
      }
      delay(1);
      
    }
    
    //feedback
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
    
  background(#04001F);
  updateScreen();
  getData();
    
  if (overviewMode == 0) {
    outputImage();
    getBoundary();
    outputBoundary();
    getRegion();
    outputRegion();
    turningResult();
    plotGraph();
  } else if (overviewMode == 1) {
    plotGraph();
    displayConstant();
  }
  
  checkOver();
  
}