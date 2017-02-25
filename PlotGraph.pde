double[] leftSpeedValue, rightSpeedValue;
double leftSpeed = 0, rightSpeed = 0;

int graphLength = 800;
int graphWidth = 400, graphHeight = 250;
int graphOneX = 100, graphOneY = 300;
int graphTwoX = 600, graphTwoY = 300;

void stringToDouble(String inputString) {
  
  int strLength = inputString.length();
  int index1 = inputString.indexOf(',');
  
  String leftStr = inputString.substring(0, index1);
  String rightStr = inputString.substring(index1+1, strLength-1);
  
  leftStr = leftStr.replaceAll("[^0-9.-]+","");
  rightStr = rightStr.replaceAll("[^0-9.-]+","");
  
  leftSpeed = Double.parseDouble(leftStr);
  rightSpeed = Double.parseDouble(rightStr);
  
}

void getData() {
  
  int range = 50000;
  
  for(int i = 1; i < graphLength; i++) {
    leftSpeedValue[i-1] = leftSpeedValue[i];
    rightSpeedValue[i-1] = rightSpeedValue[i];
  }
  
  leftSpeed = (sin(frameCount*0.1)*10)*4000;
  rightSpeed = (sin(frameCount*0.1)*10)*4000;
  leftSpeedValue[graphLength-1] = map((float)leftSpeed, 0-range, range, -graphHeight/2, graphHeight/2);
  rightSpeedValue[graphLength-1] = map((float)rightSpeed, 0-range, range, -graphHeight/2, graphHeight/2);
  
}

void plotGraph() {

  stroke(255);
  strokeWeight(0);
  line(graphOneX+1, graphOneY+graphHeight/2, graphOneX+graphWidth-1, graphOneY+graphHeight/2);
  line(graphTwoX+1, graphTwoY+graphHeight/2, graphTwoX+graphWidth-1, graphTwoY+graphHeight/2);
  
  stroke(60, 3, 255);
  strokeWeight(3);

  for(int i=1; i<graphWidth; i++) {
    point(graphOneX+i, graphOneY+graphHeight/2-(int)leftSpeedValue[graphLength-graphWidth+i]);
    point(graphTwoX+i, graphTwoY+graphHeight/2-(int)rightSpeedValue[graphLength-graphWidth+i]);
  }
  
}