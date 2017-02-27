String tfName[];
String constantArr[];
int conSize = 3;
int tuneButtonX = 900, tuneButtonY = 500;
int textFieldX = 200, textFieldY = 400;

void tfSetUp() {
  
  tfName[0] = "Constant1";
  tfName[1] = "Constant2";
  tfName[2] = "Constant3";
  
  cp5.addTextfield(tfName[0])
     .setPosition(textFieldX, textFieldY)
     .setAutoClear(false)
     ;
     
  cp5.addTextfield(tfName[1])
     .setPosition(textFieldX, textFieldY+50)
     .setAutoClear(false)
     ;
     
   cp5.addTextfield(tfName[2])
     .setPosition(textFieldX, textFieldY+100)
     .setAutoClear(false)
     ;
  
  tfHide();
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
    if (cp5.get(Textfield.class,tfName[i]).getText().length() != 0) {
      constantArr[i] = cp5.get(Textfield.class,tfName[i]).getText();
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
  
  displayText(constantArr[0], textFieldX+250, textFieldY+20, 100, 24);
  displayText(constantArr[1], textFieldX+250, textFieldY+70, 100, 24);
  displayText(constantArr[2], textFieldX+250, textFieldY+120, 100, 24);
  
}

void tfShow() {
  
  for (int i=0; i<conSize; i++) {
    cp5.get(Textfield.class,tfName[i]).show();
  }
  
}

void tfHide() {
  
  for (int i=0; i<conSize; i++) {
    cp5.get(Textfield.class,tfName[i]).hide();
  }
  
}