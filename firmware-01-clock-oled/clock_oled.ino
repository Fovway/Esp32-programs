#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -----------------------------
// OLED SSD1306 (I2C 128x64)
// -----------------------------
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------
// Wi-Fi + NTP
// -----------------------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Europe/Moscow = UTC+3 круглый год
constexpr long GMT_OFFSET_SECONDS = 3 * 3600;
constexpr int DAYLIGHT_OFFSET_SECONDS = 0;
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";

constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr unsigned long NTP_SYNC_TIMEOUT_MS = 10000;

bool isTimeSynced = false;
unsigned long lastDotBlinkMs = 0;
bool showDots = true;

void drawCenteredText(const String& text, int y, int textSize) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(textSize);
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - static_cast<int>(w)) / 2;
  display.setCursor(max(0, x), y);
  display.print(text);
}

void drawHeader() {
  display.setTextSize(1);

  // Wi-Fi индикатор
  display.setCursor(2, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi");
  } else {
    display.print("----");
  }

  // Статус времени
  display.setCursor(92, 0);
  display.print(isTimeSynced ? "NTP" : "...");

  // Линия-разделитель
  display.drawLine(0, 10, SCREEN_WIDTH - 1, 10, SSD1306_WHITE);
}

void drawClockFace() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    drawCenteredText("NO TIME", 24, 2);
    drawCenteredText("check WiFi/NTP", 48, 1);
    display.display();
    return;
  }

  char timeBuffer[6];
  strftime(timeBuffer, sizeof(timeBuffer), "%H:%M", &timeinfo);

  // Мигающее двоеточие для живости интерфейса
  if (!showDots) {
    timeBuffer[2] = ' ';
  }

  char dateBuffer[11];
  strftime(dateBuffer, sizeof(dateBuffer), "%d.%m.%Y", &timeinfo);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  drawHeader();

  // Крупное время
  drawCenteredText(String(timeBuffer), 18, 3);

  // Дата ниже
  drawCenteredText(String(dateBuffer), 52, 1);

  display.display();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
  }
}

void syncTimeWithNtp() {
  configTime(GMT_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, NTP_SERVER_1, NTP_SERVER_2);

  struct tm timeinfo;
  unsigned long start = millis();
  while (!getLocalTime(&timeinfo, 100) && millis() - start < NTP_SYNC_TIMEOUT_MS) {
    delay(200);
  }

  isTimeSynced = getLocalTime(&timeinfo, 100);
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawCenteredText("ESP32 CLOCK", 20, 2);
  drawCenteredText("Booting...", 48, 1);
  display.display();

  connectWiFi();
  syncTimeWithNtp();
}

void loop() {
  if (millis() - lastDotBlinkMs >= 1000) {
    lastDotBlinkMs = millis();
    showDots = !showDots;
  }

  // Если связь потерялась — пробуем восстановить Wi‑Fi и NTP
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    syncTimeWithNtp();
  }

  drawClockFace();
  delay(100);
}
