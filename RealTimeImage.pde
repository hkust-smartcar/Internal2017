int arrayPosX, arrayPosY;

Boolean[][] pixelArray = new Boolean[camHeight][camWidth];
int imageX = 200, imageY = 40;

void getImage(int data) {

  String binData = "";
  int strLen = 0;

  binData = Integer.toBinaryString(data);
  strLen = binData.length();
  //println(binData);

  for (int i=0; i<8; i++) {

    if (i < strLen) {

      if (binData.charAt(strLen-1-i) == '1') {
        pixelArray[arrayPosY][arrayPosX+7-i] = true;
      } else {
        pixelArray[arrayPosY][arrayPosX+7-i] = false;
      }
    } else {
      pixelArray[arrayPosY][arrayPosX+7-i] = false;
    }
  }

  arrayPosX += 8;

  if (arrayPosX >= camWidth) {
    arrayPosX = 0;
    arrayPosY++;
  }
  if (arrayPosY >= camHeight) {
    arrayPosY = 0;
  }
}

void outputImage() {

  for (int y=0; y<camHeight; y++) {
    for (int x=0; x<camWidth; x++) {
      if (pixelArray[y][x]) {
        fill(0);
      } else {
        fill(255);
      }
      noStroke();
      rect(imageX + pixelSide*x, imageY + pixelSide*y, pixelSide, pixelSide);
    }
  }
}