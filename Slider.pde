void sliderSetUp() {
  
  cp5.addSlider("constant1")
     .setPosition(300,500)
     .setSize(200,15)
     .setRange(0,0.3)
     .setDecimalPrecision(4);
     ;
  cp5.addSlider("constant2")
     .setPosition(300,520)
     .setSize(200,15)
     .setRange(0,0.3)
     .setDecimalPrecision(4);
     ;
  cp5.addSlider("constant3")
     .setPosition(300,540)
     .setSize(200,15)
     .setRange(0,0.3)
     .setDecimalPrecision(4);
     ;
     

  cp5.get(Slider.class, "constant1").hide();
  cp5.get(Slider.class, "constant2").hide();
  cp5.get(Slider.class, "constant3").hide();
}