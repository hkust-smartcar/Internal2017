String tfName[];
String constantArr[];
int textFieldSize = 12;
int tuneButtonX = 900, tuneButtonY = 500;
int textFieldX = 100, textFieldY = 400;

void tfSetUp() {
  
  for (int i=0; i<textFieldSize; i++) {
    tfName[i] = "Constant" + Integer.toString(i+1);
  
    cp5.addTextfield(tfName[i])
       .setPosition(textFieldX + (i/3)*250, textFieldY + (i%3)*50)
       .setWidth(150)
       .setAutoClear(false)
       ;
  }
  
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
  
  for (int i=0; i<textFieldSize; i++) {
    if (cp5.get(Textfield.class,tfName[i]).getText().length() != 0) {
      constantArr[i] = cp5.get(Textfield.class,tfName[i]).getText();
    }
  }
  
  writer = createWriter("constant.txt");
  
  for (int i=0; i<textFieldSize; i++) {
    writer.println(constantArr[i]);
  }
  
  writer.flush();
  writer.close();
  
}

void displayConstant() {
  
  for (int i=0; i<consSize; i++) {
    displayText(constantArr[i], textFieldX+(i/3)*250+160, textFieldY+50*(i%3)+10, 100, 24);
  }
  
}

void tfShow() {
  
  for (int i=0; i<textFieldSize; i++) {
    cp5.get(Textfield.class,tfName[i]).show();
  }
  
}

void tfHide() {
  
  for (int i=0; i<textFieldSize; i++) {
    cp5.get(Textfield.class,tfName[i]).hide();
  }
  
}