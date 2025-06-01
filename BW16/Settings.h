#undef max
#undef min
#include "Display.h"
//#include ""

void bw_16_set_up(){
  Serial.begin(115200);
  setupButtons();
  //Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 F"));
    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.drawBitmap(0,0,LOGO,128,64,SSD1306_WHITE);
  display.display();
  delay(500);
  main_names = {"Attack", "Scan", "Select","Settings"};
  at_names = {"Deauth", "All Deauth","Becaon","Authentication","Association","Evil Twin","Sour Apple"};
  Serial.println("done");

  // if (scanNetworks("Booting") != 0) {
  //   while(true) delay(1000);
  // }
  //SendToEsp("BW16");
  draw_menu(main_names, 0);
}