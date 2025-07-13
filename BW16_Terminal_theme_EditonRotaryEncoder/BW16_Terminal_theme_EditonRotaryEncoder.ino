//#include "Settings.h"
#include "display.h"
void setup(){
  setupButtons();
  web_setup();
  display_setup();
  draw_menu(Main,1,"root");
}
void loop(){
  display_update();
}