String btName[];
int btSize = 6;

int buttonDiameter = 50;
int leftX = 100, leftY = 600;
int middleX = 200, middleY = 600;
int rightX = 300, rightY = 600;
int modeButtonX = 1100, modeButtonY = 30;
boolean leftOver = false, middleOver = false, rightOver = false;
boolean modeButtonOver = false;
boolean tuneButtonOver = false;

void buttonSetUp() {

  btName[0] = "mode";
  btName[1] = "left";
  btName[2] = "middle";
  btName[3] = "right";
  btName[4] = "refresh";
  btName[5] = "tune";

  cp5.addButton(btName[0])
    .setValue(0)
    .setPosition(1100, 30)
    .setSize(50, 20)
    ;

  cp5.addButton(btName[1])
    .setValue(0)
    .setPosition(100, 600)
    .setSize(50, 20)
    ;

  cp5.addButton(btName[2])
    .setValue(0)
    .setPosition(200, 600)
    .setSize(50, 20)
    ;

  cp5.addButton(btName[3])
    .setValue(0)
    .setPosition(300, 600)
    .setSize(50, 20)
    ;

  cp5.addButton(btName[4])
    .setValue(0)
    .setPosition(1100, 50)
    .setSize(50, 20)
    ;
    
  cp5.addButton(btName[5])
    .setValue(0)
    .setPosition(1000, 600)
    .setSize(50, 20)
    ;
    
  cp5.get(Button.class, btName[5]).hide();
}

void mode() {

  background(#04001F);
  overviewMode = ++overviewMode % 3;

  if (overviewMode == 0) {

    graphWidth = 400;
    graphHeight = 250;
    graphOneX = 100;
    graphOneY = 300;
    graphTwoX = 600;
    graphTwoY = 300;

    fill(#898989);
    noStroke();
    rect(imageX, imageY, camWidth*pixelSide, camHeight*pixelSide);
    rect(boundaryX, boundaryY, camWidth*pixelSide, camHeight*pixelSide);
    rect(regionX, regionY, camWidth*pixelSide, camHeight*pixelSide);

    cp5.get(Button.class, btName[1]).show();
    cp5.get(Button.class, btName[2]).show();
    cp5.get(Button.class, btName[3]).show();
  } else if (overviewMode == 1) {
    
    graphOneY = 50;
    graphTwoY = 50;

    readConstant();
    displayConstant();
    tfShow();

    cp5.get(Button.class, btName[5]).show();
    cp5.get(Button.class, btName[1]).hide();
    cp5.get(Button.class, btName[2]).hide();
    cp5.get(Button.class, btName[3]).hide();
  } else if (overviewMode == 2) {
    
    graphWidth = 400;
    graphHeight = 500;
    graphOneY = 50;
    graphTwoY = 50;

    cp5.get(Button.class, btName[5]).hide();
    tfHide();
  }
}

void refresh() {
  myPort.clear();
}

void tune() {
  if (overviewMode == 1) {
    editConstant();
    displayConstant();
    String sendStr = "";
    sendStr += constantArr[0];
    for (int i=1; i<consSize; i++) {
      sendStr += ",";
      sendStr += constantArr[i];
    }
    myPort.write('t' + sendStr + '\n');
  }
}