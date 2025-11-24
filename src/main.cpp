#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C oled1(U8G2_R1, /* clock=*/ D2, /* data=*/ D3, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C oled2(U8G2_R1, /* clock=*/ D4, /* data=*/ D5, /* reset=*/ U8X8_PIN_NONE);

// 目の状態を表すための構造体
struct EyeState {
  int cx, cy;          // 目の中心
  int pupilX;          // 黒目の相対位置 X
  int pupilY;          // 黒目の相対位置 Y
  bool isBlinking;     // 瞬き中フラグ
  unsigned long nextBlink;
  unsigned long nextMove;
};

EyeState eyeL = {32, 64, 0, 0, false, 0, 0};
EyeState eyeR = {32, 64, 0, 0, false, 0, 0};

// 開いた目（黒目あり）
void drawOpenEye(U8G2 &oled, EyeState &eye) {
  oled.drawCircle(eye.cx, eye.cy, 20);             // 白目
  oled.drawDisc(eye.cx + eye.pupilX, eye.cy + eye.pupilY, 6); // 黒目
}

// 閉じた目（横線）
void drawClosedEye(U8G2 &oled, EyeState &eye) {
  oled.drawLine(eye.cx - 20, eye.cy, eye.cx + 20, eye.cy);
}

void updateEye(EyeState &eye) {
  unsigned long now = millis();

  // ---- ランダム視線移動（左右キョロキョロ） ----
  if (now > eye.nextMove && !eye.isBlinking) {
    eye.pupilX = random(-8, 9);   // 左右に動く
    eye.pupilY = random(-3, 4);   // 上下は少なめ
    eye.nextMove = now + random(300, 800);  // 動く間隔も個別
  }

  // ---- まばたき ----
  if (now > eye.nextBlink) {
    eye.isBlinking = true;
    eye.nextBlink = now + random(2000, 6000); // 次の瞬きはランダム
  }
}

void drawEyes() {
  // 左目
  oled1.clearBuffer();
  if (eyeL.isBlinking) drawClosedEye(oled1, eyeL);
  else                 drawOpenEye(oled1, eyeL);
  oled1.sendBuffer();

  // 右目
  oled2.clearBuffer();
  if (eyeR.isBlinking) drawClosedEye(oled2, eyeR);
  else                 drawOpenEye(oled2, eyeR);
  oled2.sendBuffer();
}

void handleBlink(EyeState &eye) {
  if (eye.isBlinking) {
    static unsigned long blinkStart = millis();
    if (millis() - blinkStart > 150) {  // 150ms くらいで開く
      eye.isBlinking = false;
      blinkStart = millis();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello, World!");

  // pinMode(LED_BUILTIN, OUTPUT);

  oled1.begin();
  oled2.begin();
}

void loop() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(500);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(500);

  // 動きの更新
  updateEye(eyeL);
  updateEye(eyeR);

  handleBlink(eyeL);
  handleBlink(eyeR);

  // 描画
  drawEyes();

  delay(30);
}

