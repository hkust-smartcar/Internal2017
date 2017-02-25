GTextField txf[];
GTabManager tt;
int tuneButtonX = 900, tuneButtonY = 500;
int textFieldX = 200, textFieldY = 400;

void txfSetUp() {
  
  txf = new GTextField[conSize];
  
  txf[0] = new GTextField(this, textFieldX, textFieldY, 200, 20);
  txf[0].tag = "txf1";
  txf[0].setPromptText("Constant1");

  txf[1] = new GTextField(this, textFieldX, textFieldY+50, 200, 20);
  txf[1].tag = "txf2";
  txf[1].setPromptText("Constant2");
  
  txf[2] = new GTextField(this, textFieldX, textFieldY+100, 200, 20);
  txf[2].tag = "txf3";
  txf[2].setPromptText("Constant3");
  
}

void readConstant() {
  
  int counter = 0;
  String line;
  
  reader = createReader("constant.txt");
  
  do {
    
    try {
      line = reader.readLine();
    } catch (IOException e) {
      e.printStackTrace();
      line = null;
    }
    
    if (line != null) {
      constantArr[counter] = line;
      counter++;
    }
    
  } while (line != null);
  
}

void editConstant() {
  
  for (int i=0; i<conSize; i++) {
    if (txf[i].getText().length() != 0) {
      constantArr[i] = txf[i].getText();
    }
  }
  
  writer = createWriter("constant.txt");
  
  for (int i=0; i<conSize; i++) {
    writer.println(constantArr[i]);
  }
  
  writer.flush();
  writer.close();
  
}

void displayConstant() {
  
  fill(255);
  textSize(16);
  text(constantArr[0], textFieldX+200, textFieldY+20);
  text(constantArr[1], textFieldX+200, textFieldY+70);
  text(constantArr[2], textFieldX+200, textFieldY+120);
  
  fill(#003602);
  noStroke();
  ellipse(tuneButtonX, tuneButtonY, buttonDiameter, buttonDiameter);
  
}

void txfRemove() {
  
  for (int i=0; i<conSize; i++) {
    txf[i].markForDisposal();
  }
  
}