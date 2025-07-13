struct Button {
  uint8_t pin;
  unsigned long lastDebounceTime = 0;
  int state = HIGH;
  int lastState = HIGH;
  Button(uint8_t p)
    : pin(p), lastDebounceTime(0), state(HIGH), lastState(HIGH) {}
};

struct RotaryEncoder {
  uint8_t clkPin;
  uint8_t dtPin;
  uint8_t btnPin;

  int lastClkState;
  unsigned long lastRotaryTime = 0;

  unsigned long lastDebounceTime = 0;
  int btnState = HIGH;
  int lastBtnState = HIGH;

  unsigned long lastClickTime = 0;
  int clickCount = 0;
  bool waitingForSecondClick = false;

  RotaryEncoder(uint8_t clk, uint8_t dt, uint8_t btn)
    : clkPin(clk), dtPin(dt), btnPin(btn) {}
};

// 핀 정의
#define BTN_DOWN PA27
#define BTN_UP PA12
#define BTN_OK PA13
#define BTN_BACK PB3

#define ROTARY_CLK PB3
#define ROTARY_DT PA12
#define ROTARY_BTN PA27

// 객체
Button btnDown(BTN_DOWN);
Button btnUp(BTN_UP);
Button btnOk(BTN_OK);
Button btnBack(BTN_BACK);
RotaryEncoder rotary(ROTARY_CLK, ROTARY_DT, ROTARY_BTN);

// 전역 방향 상태
int lastRotaryDirection = 0;
unsigned long lastRotaryDetectedTime = 0;

void setupButtons() {
  pinMode(rotary.clkPin, INPUT_PULLUP);
  pinMode(rotary.dtPin, INPUT_PULLUP);
  pinMode(rotary.btnPin, INPUT_PULLUP);
  rotary.lastClkState = digitalRead(rotary.clkPin);
}

// 로터리 상태 갱신 (매 루프마다 호출할 것)
void pollRotary() {
  int clkState = digitalRead(rotary.clkPin);
  if (clkState != rotary.lastClkState) { // 상태 변화 시에만 실행
    if (millis() - rotary.lastRotaryTime > 20) { // 감도 조정: 20ms로 설정
      int dtState = digitalRead(rotary.dtPin);
      if (clkState == LOW) { // LOW일 때만 방향 감지
        lastRotaryDirection = dtState == HIGH ? 1 : -1;
        lastRotaryDetectedTime = millis();
        rotary.lastRotaryTime = millis();

        if (lastRotaryDirection == 1)
          Serial.println("Rotary -> CW (Scroll UP)");
        else
          Serial.println("Rotary -> CCW (Scroll DOWN)");
      }
    }
  }
  rotary.lastClkState = clkState;
}
bool ButtonPress(Button& btn, unsigned long debounceDelay = 50) {
    // 로터리 버튼 상태 업데이트
    int reading = digitalRead(rotary.btnPin);
    if (reading != rotary.lastBtnState) {
        rotary.lastDebounceTime = millis();
    }
    if ((millis() - rotary.lastDebounceTime) > debounceDelay) {
        if (reading != rotary.btnState) {
            rotary.btnState = reading;
            if (rotary.btnState == LOW) { // 버튼이 눌렸을 때
                unsigned long currentTime = millis();
                if (!rotary.waitingForSecondClick || (currentTime - rotary.lastClickTime) > 450) {
                    rotary.waitingForSecondClick = true;
                    rotary.lastClickTime = currentTime;
                    rotary.clickCount = 1; // 첫 번째 클릭
                } else {
                    rotary.clickCount = 2; // 두 번째 클릭 (더블 클릭)
                    rotary.waitingForSecondClick = false;
                }
            }
        }
    }
    rotary.lastBtnState = reading;

    // 특정 버튼 처리
    if (&btn == &btnUp && lastRotaryDirection == -1) {
        lastRotaryDirection = 0;
        return true;
    }
    if (&btn == &btnDown && lastRotaryDirection == 1) {
        lastRotaryDirection = 0;
        return true;
    }
    if (&btn == &btnOk) {
        if (rotary.waitingForSecondClick && (millis() - rotary.lastClickTime) > 300) {
            rotary.waitingForSecondClick = false;
            rotary.clickCount = 0;
            return true; // 단일 클릭 감지
        }
        return false;
    }
    if (&btn == &btnBack) {
        if (rotary.clickCount >= 2) {
            rotary.clickCount = 0;
            rotary.waitingForSecondClick = false;
            return true; // 더블 클릭 감지
        }
        return false;
    }
    return false;
}