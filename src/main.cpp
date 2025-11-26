#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledL(U8G2_R3, /* clock=*/ SCK, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledR(U8G2_R1, /* clock=*/ SCK, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

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
  oledL.clearBuffer();
  if (eyeL.isBlinking) drawClosedEye(oledL, eyeL);
  else                 drawOpenEye(oledL, eyeL);
  oledL.sendBuffer();

  // 右目
  oledR.clearBuffer();
  if (eyeR.isBlinking) drawClosedEye(oledR, eyeR);
  else                 drawOpenEye(oledR, eyeR);
  oledR.sendBuffer();
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

  oledL.setI2CAddress(0x3C << 1); // 左目
  oledR.setI2CAddress(0x3D << 1); // 右目

  oledL.begin();
  oledR.begin();

  // oledL.clearBuffer();
  // oledL.setFont(u8g2_font_ncenB08_tr);
  // oledL.drawStr(0, 15, "LEFT");
  // oledL.sendBuffer();

  // oledR.clearBuffer();
  // oledR.setFont(u8g2_font_ncenB08_tr);
  // oledR.drawStr(0, 15, "RIGHT");
  // oledR.sendBuffer();
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

