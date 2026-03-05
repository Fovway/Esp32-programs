#include <M5Unified.h>
#include <WiFi.h>
#include <time.h>

// Заполни своими данными Wi-Fi.
static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// NTP настройка.
static const long GMT_OFFSET_SEC = 3 * 3600;  // UTC+3
static const int DAYLIGHT_OFFSET_SEC = 0;
static const char* NTP_SERVER_1 = "pool.ntp.org";
static const char* NTP_SERVER_2 = "time.nist.gov";

void drawClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(RED, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(8, 50);
    M5.Display.println("Time sync...");
    return;
  }

  char timeBuf[16];
  char dateBuf[24];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
  strftime(dateBuf, sizeof(dateBuf), "%d.%m.%Y", &timeinfo);

  M5.Display.fillScreen(BLACK);
  M5.Display.setTextDatum(middle_center);

  M5.Display.setTextColor(CYAN, BLACK);
  M5.Display.setTextSize(4);
  M5.Display.drawString(timeBuf, M5.Display.width() / 2, M5.Display.height() / 2 - 18);

  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.setTextSize(2);
  M5.Display.drawString(dateBuf, M5.Display.width() / 2, M5.Display.height() / 2 + 30);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(YELLOW, BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.println("M5Stick S3 Clock");
  M5.Display.println("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000) {
    delay(250);
  }

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);
}

void loop() {
  M5.update();

  // Кнопка A меняет ориентацию экрана.
  if (M5.BtnA.wasPressed()) {
    static uint8_t rotation = 1;
    rotation = (rotation + 1) % 4;
    M5.Display.setRotation(rotation);
  }

  drawClock();
  delay(1000);
}
