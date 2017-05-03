GTextArea textArea;
List<String> constantList;

void taSetUp() {

  String[] constant = loadStrings("constant.txt");
  textArea = new GTextArea(this, 650, 80, 400, 500, G4P.SCROLLBARS_VERTICAL_ONLY);
  textArea.setText(constant);
  
}

void editConstant() {

  String[] tempConstantArr = textArea.getTextAsArray();
  constantList.clear();
  
  for (int i=0; i<tempConstantArr.length; i++) {
    constantList.add(tempConstantArr[i].split("=")[1].trim());
  }
  
  writer = createWriter("constant.txt");

  for (int i=0; i<tempConstantArr.length; i++) {
    writer.println(tempConstantArr[i]);
  }

  writer.flush();
  writer.close();
}