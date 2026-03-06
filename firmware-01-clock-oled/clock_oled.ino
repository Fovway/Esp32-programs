#include <WiFi.h>
#include <time.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// -----------------------------
// OLED SSD1306 (I2C 128x64)
// -----------------------------
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// Пользовательское подключение I2C:
// SDA -> GPIO 3, SCL -> GPIO 4
constexpr uint8_t SDA_PIN = 3;
constexpr uint8_t SCL_PIN = 4;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------
// Wi-Fi + NTP
// -----------------------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";

constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr unsigned long NTP_SYNC_TIMEOUT_MS = 10000;

// -----------------------------
// Web настройки
// -----------------------------
WebServer server(80);
Preferences preferences;

// Пароль для входа в веб-интерфейс
const char* WEB_AUTH_USER = "admin";
const char* WEB_AUTH_PASSWORD = "clock123";

// Часовой пояс по умолчанию: Москва (UTC+3)
long gmtOffsetSeconds = 3 * 3600;
int daylightOffsetSeconds = 0;

bool isTimeSynced = false;
unsigned long lastDotBlinkMs = 0;
bool showDots = true;

// Заглушка датчика температуры/влажности (до подключения реального датчика)
float mockTemperatureC = 24.5f;
int mockHumidity = 52;
unsigned long lastClimateUpdateMs = 0;
int climateStep = 0;

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

  display.setCursor(2, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi");
  } else {
    display.print("----");
  }

  display.setCursor(92, 0);
  display.print(isTimeSynced ? "NTP" : "...");

  display.drawLine(0, 10, SCREEN_WIDTH - 1, 10, SSD1306_WHITE);
}


void updateMockClimate() {
  if (millis() - lastClimateUpdateMs < 2000) {
    return;
  }

  lastClimateUpdateMs = millis();
  climateStep = (climateStep + 1) % 12;

  // Небольшие плавные изменения, чтобы выглядело как "живые" данные
  static const float tempPattern[12] = {24.5f, 24.6f, 24.7f, 24.8f, 24.9f, 25.0f, 24.9f, 24.8f, 24.7f, 24.6f, 24.5f, 24.4f};
  static const int humPattern[12] = {52, 53, 54, 54, 55, 56, 55, 54, 53, 52, 51, 52};

  mockTemperatureC = tempPattern[climateStep];
  mockHumidity = humPattern[climateStep];
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

  if (!showDots) {
    timeBuffer[2] = ' ';
  }

  char dateBuffer[11];
  strftime(dateBuffer, sizeof(dateBuffer), "%d.%m.%Y", &timeinfo);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  drawHeader();
  drawCenteredText(String(timeBuffer), 18, 3);

  display.setTextSize(1);
  display.setCursor(8, 44);
  display.print("T:");
  display.print(mockTemperatureC, 1);
  display.print("C  H:");
  display.print(mockHumidity);
  display.print("%");

  drawCenteredText(String(dateBuffer), 54, 1);

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

void applyTimezoneAndSync() {
  configTime(gmtOffsetSeconds, daylightOffsetSeconds, NTP_SERVER_1, NTP_SERVER_2);

  struct tm timeinfo;
  unsigned long start = millis();
  while (!getLocalTime(&timeinfo, 100) && millis() - start < NTP_SYNC_TIMEOUT_MS) {
    delay(200);
  }

  isTimeSynced = getLocalTime(&timeinfo, 100);
}

bool ensureAuthenticated() {
  if (server.authenticate(WEB_AUTH_USER, WEB_AUTH_PASSWORD)) {
    return true;
  }

  server.requestAuthentication(BASIC_AUTH, "ESP32 Clock", "Введите пароль веб-доступа");
  return false;
}

void handleRoot() {
  if (!ensureAuthenticated()) {
    return;
  }

  String html =
    "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>ESP32 Clock</title>"
    "<style>body{font-family:Arial,sans-serif;max-width:480px;margin:20px auto;padding:0 12px;}"
    "label{display:block;margin:12px 0 6px;}input{width:100%;padding:10px;font-size:16px;}"
    "button{margin-top:14px;padding:10px 14px;font-size:16px;}small{color:#555;}</style></head><body>"
    "<h2>Настройка часового пояса</h2>"
    "<p><small>Текущее смещение: GMT=" + String(gmtOffsetSeconds) + " сек, DST=" + String(daylightOffsetSeconds) + " сек</small></p>"
    "<form method='POST' action='/timezone'>"
    "<label for='gmt'>GMT offset (секунды)</label>"
    "<input id='gmt' name='gmt' type='number' value='" + String(gmtOffsetSeconds) + "' required>"
    "<label for='dst'>DST offset (секунды)</label>"
    "<input id='dst' name='dst' type='number' value='" + String(daylightOffsetSeconds) + "' required>"
    "<button type='submit'>Сохранить и синхронизировать</button>"
    "</form>"
    "<p><small>Пример: Москва UTC+3 = 10800, DST = 0</small></p>"
    "</body></html>";

  server.send(200, "text/html; charset=utf-8", html);
}

void handleTimezoneSave() {
  if (!ensureAuthenticated()) {
    return;
  }

  if (!server.hasArg("gmt") || !server.hasArg("dst")) {
    server.send(400, "text/plain; charset=utf-8", "Отсутствуют параметры gmt/dst");
    return;
  }

  long newGmt = server.arg("gmt").toInt();
  int newDst = server.arg("dst").toInt();

  gmtOffsetSeconds = newGmt;
  daylightOffsetSeconds = newDst;

  preferences.putLong("gmt", gmtOffsetSeconds);
  preferences.putInt("dst", daylightOffsetSeconds);

  applyTimezoneAndSync();

  String response =
    "<!doctype html><html><head><meta charset='utf-8'><meta http-equiv='refresh' content='2;url=/'></head><body>"
    "<h3>Сохранено</h3><p>GMT=" + String(gmtOffsetSeconds) + ", DST=" + String(daylightOffsetSeconds) + "</p>"
    "<p>Время пересинхронизировано. Возврат...</p></body></html>";

  server.send(200, "text/html; charset=utf-8", response);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/timezone", HTTP_POST, handleTimezoneSave);
  server.onNotFound([]() {
    if (!ensureAuthenticated()) {
      return;
    }
    server.send(404, "text/plain", "Not found");
  });
  server.begin();
}

void loadTimezoneSettings() {
  preferences.begin("clock", false);
  gmtOffsetSeconds = preferences.getLong("gmt", 3 * 3600);
  daylightOffsetSeconds = preferences.getInt("dst", 0);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

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

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  }

  loadTimezoneSettings();
  applyTimezoneAndSync();
  setupWebServer();
}

void loop() {
  if (millis() - lastDotBlinkMs >= 1000) {
    lastDotBlinkMs = millis();
    showDots = !showDots;
  }

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    applyTimezoneAndSync();
  }

  server.handleClient();
  updateMockClimate();
  drawClockFace();
  delay(100);
}
