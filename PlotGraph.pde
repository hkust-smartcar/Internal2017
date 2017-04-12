double[][] value;
double[] newValue;
color[] graphColor;

int graphLength = 1000;
int graphWidth = 400, graphHeight = 250;
int graphOneX = 100, graphOneY = 320;
int graphTwoX = 600, graphTwoY = 320;

void stringToDouble(String inputString) {
  
  String[] valStr = inputString.split(",");
  
  for (int i = 0; i < valSize; i++) {
    valStr[i+1] = valStr[i+1].replaceAll("[^0-9.-]+", "");
    newValue[i] = Double.parseDouble(valStr[i+1]);
  }
  
}

void getData() {
  
  for (int i = 0; i < valSize; i++) {
    for (int j = 1; j < graphLength; j++) {
      value[i][j-1] = value[i][j];
    }
  }
  
  for (int i = 0; i < valSize; i++) {
    if (value[i][graphLength-1] > range) {
      value[i][graphLength-1] = range;
    } else if (value[i][graphLength-1] < 0-range) {
      value[i][graphLength-1] = 0-range;
    } else {
      value[i][graphLength-1] = map((float)newValue[i], 0-range, range, -10000, 10000);
    }
  }
  
}

void plotGraph() {

  if (overviewMode == 0 || overviewMode == 1) {
    fill(#404040);
    stroke(0);
    rect(graphOneX, graphOneY, graphWidth, graphHeight);
    rect(graphTwoX, graphTwoY, graphWidth, graphHeight);
    stroke(255);
    strokeWeight(0);
    line(graphOneX+1, graphOneY+graphHeight/2, graphOneX+graphWidth-1, graphOneY+graphHeight/2);
    line(graphTwoX+1, graphTwoY+graphHeight/2, graphTwoX+graphWidth-1, graphTwoY+graphHeight/2);
  } else {
    fill(#404040);
    stroke(0);
    rect(graphOneX, graphOneY, graphWidth, graphHeight);
    stroke(255);
    strokeWeight(0);
    line(graphOneX+1, graphOneY+graphHeight/2, graphOneX+graphWidth-1, graphOneY+graphHeight/2);
  }
  
  strokeWeight(2);

  for (int i = 0; i < valSize; i++) {
    stroke(graphColor[i]);
    for(int j=1; j<graphWidth; j++) {
      point(graphOneX+j, graphOneY+graphHeight/2-(int)(value[i][graphLength-graphWidth+j]/10000*graphHeight/2));
    }
  }
}