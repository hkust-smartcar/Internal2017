//int sb1x = 250, sb1y = 400, sb1w = 500;
//int sb2x = 250, sb2y = 500, sb2w = 500;

//class HScrollbar {
//  int swidth, sheight;    // width and height of bar
//  float xpos, ypos;       // x and y position of bar
//  float spos, newspos;    // x position of slider
//  float sposMin, sposMax; // max and min values of slider
//  int loose;              // how loose/heavy
//  boolean over;           // is the mouse over the slider?
//  boolean locked;
//  float ratio;

//  HScrollbar (float xp, float yp, int sw, int sh, int l) {
//    swidth = sw;
//    sheight = sh;
//    int widthtoheight = sw - sh;
//    ratio = (float)sw / (float)widthtoheight;
//    xpos = xp;
//    ypos = yp-sheight/2;
//    spos = xpos + swidth/2 - sheight/2;
//    newspos = spos;
//    sposMin = xpos;
//    sposMax = xpos + swidth - sheight;
//    loose = l;
//  }

//  void update() {
//    if (overEvent()) {
//      over = true;
//    } else {
//      over = false;
//    }
//    if (mousePressed && over) {
//      locked = true;
//    }
//    if (!mousePressed) {
//      locked = false;
//    }
//    if (locked) {
//      newspos = constrain(mouseX-sheight/2, sposMin, sposMax);
//    }
//    if (abs(newspos - spos) > 1) {
//      spos = spos + (newspos-spos)/loose;
//    }
//  }

//  float constrain(float val, float minv, float maxv) {
//    return min(max(val, minv), maxv);
//  }

//  boolean overEvent() {
//    if (mouseX > xpos && mouseX < xpos+swidth &&
//       mouseY > ypos && mouseY < ypos+sheight) {
//      return true;
//    } else {
//      return false;
//    }
//  }

//  void display() {
//    noStroke();
//    fill(204);
//    rect(xpos, ypos, swidth, sheight);
//    if (over || locked) {
//      fill(0, 0, 0);
//    } else {
//      fill(102, 102, 102);
//    }
//    rect(spos, ypos, sheight, sheight);
//  }

//  float getPos() {
//    // Convert spos to be values between
//    // 0 and the total width of the scrollbar
//    return (spos-xpos) * ratio;
//  }
//}

//void updateSB() {
//    hs1.update();
//    hs1.display();
//    //hs2.update();
//    //hs2.display();
//    outputVal = (int)(hs1.getPos() / 500 * 1000);
//    fill(255);
//    textSize(24);
//    text(Integer.toString(outputVal), sb1x+sb1w+10, sb1y+8);
//    //text((int)(1000*hs2.getPos()/sb2w), sb2x+sb2w+10, sb2y+5);
//}