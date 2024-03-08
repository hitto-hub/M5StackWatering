#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// wifi 設定
// ssidを設定
#define D_WIFI_SSID ""
// パスワードを設定
#define D_WIFI_PASS ""

//赤色LED
#define LED_PIN   10
// このLEDは、GPIO10の電位を下げることで発光するタイプ
#define LED_ON  LOW
#define LED_OFF HIGH
// センサー
#define INPUT_PIN 33
#define PUMP_PIN  32
// 最大待機時間
#define MAX_LATENCY 300
// 給水無効時間
#define WATER_INVALID_TIME 200
// センサーしきい値
#define SENSER_THRESHOLD 1800
// 給水継続時間
#define WATER_SUPPLY_DURATION 100
// 水分量
int rawADC = 0;
// URL
const char *host_val = "" ;
const char *host_notice = "" ;
const char *host_flag = "" ;
const char *host_flag_last = "" ;
// Json設定
StaticJsonDocument<255> json_request;
char buffer[255];
//setup
void setup() {
  Serial.begin(115200);
  Serial.println("demo!");
  //ライブラリ初期化
  M5.begin();

  M5.Lcd.setRotation(1);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("Connecting to %s ", D_WIFI_SSID);
  // Wi-Fi接続init
  WiFi.begin(D_WIFI_SSID, D_WIFI_PASS);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  M5.Lcd.println(" CONNECTED");

  configTime(9 * 3600, 0, "ntp.nict.jp"); // Set ntp time to local

  //M5StickC設定
  pinMode(36,INPUT_PULLUP);
  pinMode(LED_PIN,OUTPUT);
  pinMode(INPUT_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  M5.Lcd.setRotation(1); // (0:M5が下, 1:M5が右, 2:M5が上, 3:M5が左)

  M5.Lcd.setCursor(0, 6);
  M5.Lcd.setTextColor(WHITE, BLACK);
  // M5.Lcd.setTextSize(3);
  //M5.Lcd.printf("T:%2.2fC\r\nH:%2.2f%%", tmp, hum);
  M5.Lcd.printf("test");
  delay(2000);
}

/**
 * Maps a floating-point value from one range to another range.
 *
 * This function takes a floating-point value `x` and maps it from the input range defined by `in_min` and `in_max`
 * to the output range defined by `out_min` and `out_max`. The mapped value is returned as an integer.
 *
 * @param x The value to be mapped.
 * @param in_min The minimum value of the input range.
 * @param in_max The maximum value of the input range.
 * @param out_min The minimum value of the output range.
 * @param out_max The maximum value of the output range.
 * @return The mapped value as an integer.
 */
int mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * 100 1350
 *  90 1400
 *  80 1450
 *  70 1500
 *  60 1550
 *  50 1600
 *  40 1650
 *  30 1700
 *  20 1750
 *  10 1800
 *   0 1850
 */
int MaprawADC(int rawADC) {
  return 100 - mapfloat(rawADC, 1350, 1850, 0, 100);
}

// センサーに反応したらデータを送る関数
void sendData() {
  // valここから
  // データをつくる
  json_request["val"] = MaprawADC(rawADC);
  serializeJson(json_request, buffer, sizeof(buffer));
  HTTPClient http;
  Serial.println("1");
  // 通信開始
  http.begin(host_val);
  Serial.println("2");
  http.addHeader("Content-Type", "application/json");
  Serial.println("3");
  // ここでPOSTする
  int status_code = http.POST((uint8_t*)buffer, strlen(buffer));
  // ここでreturnうけとる
  String status_message = http.getString();
  Serial.println(status_code);
  Serial.println(status_message);
  // valここまで
  http.end();
  Serial.println("Send");
}

void sendNotice(int data) {
  // noticeここから
  // データをつくる
  json_request["notice"] = data;
  serializeJson(json_request, buffer, sizeof(buffer));
  HTTPClient http;
  Serial.println("1");
  // 通信開始
  http.begin(host_notice);
  Serial.println("2");
  http.addHeader("Content-Type", "application/json");
  Serial.println("3");
  // ここでPOSTする
  int status_code = http.POST((uint8_t*)buffer, strlen(buffer));
  // ここでreturnうけとる
  String status_message = http.getString();
  Serial.println(status_code);
  Serial.println(status_message);
  // noticeここまで
  http.end();
  Serial.println("Send");
}

void watering() {
  int timer = 0;
  int begin_time = 0;
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  begin_time = timeInfo.tm_min * 60 + timeInfo.tm_sec;
  digitalWrite(LED_PIN, LED_ON);
  digitalWrite(PUMP_PIN, true);
  sendNotice(1);
  Serial.println("ON");

  M5.Lcd.setCursor(0, 50);
  while(1) {
    delay(1000);
    getLocalTime(&timeInfo);
    timer = timeInfo.tm_min * 60 + timeInfo.tm_sec;
    rawADC = analogReadMilliVolts(INPUT_PIN);
    Serial.println("ADC: " + String(rawADC));
    Serial.println("time: " + String(timer-begin_time));
    if (rawADC < 1650) {
      break;
    } else if (timer - begin_time > 180 || timer - begin_time < 0) {
      digitalWrite(LED_PIN, LED_OFF);
      digitalWrite(PUMP_PIN, false);
      sendNotice(2);
      Serial.println("time out");
      return;
    }
  }
  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(PUMP_PIN, false);
  sendNotice(0);
  Serial.println("OFF");
}

void get_instruction() {
  HTTPClient http;
  http.begin(host_flag_last);
  int status_code = http.GET();
  String status_message = http.getString();
  Serial.println(status_code);
  Serial.println(status_message);
  http.end();
  if (status_code == 200) {
    if (status_message == "1") {
      watering();
      // 通信開始
      json_request["flag"] = 0;
      http.begin(host_flag);
      http.addHeader("Content-Type", "application/json");
      int status_code_post = http.POST((uint8_t*)buffer, strlen(buffer));
      String status_message_post = http.getString();
      Serial.println(status_code_post);
      Serial.println(status_message_post);
      http.end();
      Serial.println("Send");
    }
  }
}

//loop
void loop() {
  // 時間を取得
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  char now[20];
  sprintf(now, "%04d/%02d/%02d %02d:%02d:%02d",
    timeInfo.tm_year + 1900,
    timeInfo.tm_mon + 1,
    timeInfo.tm_mday,
    timeInfo.tm_hour,
    timeInfo.tm_min,
    timeInfo.tm_sec
  );
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.println(now);
  M5.Lcd.print("ADC: " + String(rawADC));
  Serial.println(M5.Axp.GetBatVoltage());
  // 0.9秒単位
  delay(900);
  rawADC = analogReadMilliVolts(INPUT_PIN);
  M5.update();// ボタン全体の状態更新
  // test ボタンAが押されたとき。
  if (M5.BtnA.wasPressed()) {
    watering();
    Serial.println("ON");
    while (M5.BtnA.wasReleased() == false) {
      M5.update();// ボタン全体の状態更新
      delay(1000);
    }
    Serial.println("OFF");
  }
  if (timeInfo.tm_min % 30 == 0 && timeInfo.tm_sec == 0) {
    sendData();
  }
  if (timeInfo.tm_sec == 0) {
    get_instruction();
  }
}
