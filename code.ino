#include <WiFi.h>           // Библиотека Wi-Fi для ESP32
#include <WebServer.h>      // Веб-сервер для ESP32
#include <Wire.h>           // I²C
#include <Adafruit_GFX.h>   // Графическая библиотека
#include <Adafruit_SSD1306.h> // Драйвер SSD1306

// --------------------- НАСТРОЙКИ ОЛЕД --------------------- //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1       // Аппаратный RESET не используем
#define SDA_PIN 21          // SDA = GPIO21
#define SCL_PIN 22          // SCL = GPIO22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --------------------- НАСТРОЙКИ Wi-Fi --------------------- //
const char* ssid = "Твой_SSID";
const char* password = "Твой_ПАРОЛЬ";

// Пример статического IP (если нужно). Можно убрать, если используете DHCP.
IPAddress local_IP(192, 168, 54, 70);
IPAddress gateway(192, 168, 54, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// --------------------- ВЕБ-СЕРВЕР --------------------- //
WebServer server(80);

// --------------------- ХРАНЕНИЕ ДАННЫХ О GPU --------------------- //
float gpuTemp = 0, gpuLoad = 0, gpuHotspot = 0, gpuMemory = 0;
float gpuMemUsed = 0, gpuMemFree = 0, gpuCoreClock = 0, gpuMemClock = 0;
float gpuPower = 0, gpuFan = 0;

// 10 характеристик
const char* labels[] = {
    "GPU Temp",    // 0
    "GPU Load",    // 1
    "GPU Hotspot", // 2
    "GPU MemLoad", // 3
    "GPU MemUsed", // 4
    "GPU MemFree", // 5
    "GPU CoreClk", // 6
    "GPU MemClk",  // 7
    "GPU Power",   // 8
    "GPU Fan"      // 9
};
int screenIndex = 0;              // какой параметр сейчас показываем
unsigned long lastSwitch = 0;     // таймер для переключения

// --------------------- ПОЛУЧАЕМ ДАННЫЕ ЧЕРЕЗ /update --------------------- //
void handleUpdate() {
  if (server.hasArg("gpu_temp"))      gpuTemp      = server.arg("gpu_temp").toFloat();
  if (server.hasArg("gpu_load"))      gpuLoad      = server.arg("gpu_load").toFloat();
  if (server.hasArg("gpu_hotspot"))   gpuHotspot   = server.arg("gpu_hotspot").toFloat();
  if (server.hasArg("gpu_memory"))    gpuMemory    = server.arg("gpu_memory").toFloat();
  if (server.hasArg("gpu_mem_used"))  gpuMemUsed   = server.arg("gpu_mem_used").toFloat();
  if (server.hasArg("gpu_mem_free"))  gpuMemFree   = server.arg("gpu_mem_free").toFloat();
  if (server.hasArg("gpu_core_clock"))gpuCoreClock = server.arg("gpu_core_clock").toFloat();
  if (server.hasArg("gpu_mem_clock")) gpuMemClock  = server.arg("gpu_mem_clock").toFloat();
  if (server.hasArg("gpu_power"))     gpuPower     = server.arg("gpu_power").toFloat();
  if (server.hasArg("gpu_fan"))       gpuFan       = server.arg("gpu_fan").toFloat();

  server.send(200, "text/plain", "OK");
}

// --------------------- РИСОВАНИЕ --------------------- //
// Получить текущее значение по индексу
float getValueByIndex(int index) {
  float values[] = {
      gpuTemp, gpuLoad, gpuHotspot, gpuMemory, gpuMemUsed,
      gpuMemFree, gpuCoreClock, gpuMemClock, gpuPower, gpuFan
  };
  return values[index];
}

// Утилита для центрированного вывода текста
void printCentered(String text, int16_t y, uint8_t textSize) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);

  int16_t x1, y1;
  uint16_t w, h;
  // Вычислим ширину текста, чтобы выровнять по центру
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int16_t xPos = (SCREEN_WIDTH - w) / 2;

  display.setCursor(xPos, y);
  display.print(text);
}

// Отрисовка текущего слайда (index)
void drawSlide(int index) {
  float val = getValueByIndex(index);

  // Очищаем буфер
  display.clearDisplay();

  // Название мелким шрифтом
  printCentered(labels[index], 5, 1);

  // Значение крупным шрифтом
  String valStr = String(val, 1);

  // Добавим единицы измерения
  if (index == 0 || index == 1 || index == 2) {
    valStr += "C";
  } else if (index == 3 || index == 4 || index == 5) {
    valStr += "MB";
  } else if (index == 6 || index == 7) {
    valStr += "MHz";
  } else if (index == 8) {
    valStr += "W";
  } else if (index == 9) {
    valStr += "RPM";
  }

  printCentered(valStr, 30, 2);

  // Выводим на экран
  display.display();
}

// --------------------- SETUP --------------------- //
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Connecting to WiFi...");

  // Статический IP (можно отключить, если не нужен)
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);

  // Подключаемся к точке доступа
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Инициализируем I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Инициализируем дисплей
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 not found! Check wiring or address.");
    while (1) { delay(1); }
  }
  Serial.println("OLED Initialized!");

  // Запускаем веб-сервер
  server.on("/update", handleUpdate);
  server.begin();

  // Отрисуем первый слайд
  drawSlide(screenIndex);
}

// --------------------- LOOP --------------------- //
void loop() {
  // Обслуживаем клиентов (принимаем /update)
  server.handleClient();

  // Проверяем, пора ли переключать слайд (каждые 10 секунд)
  if (millis() - lastSwitch > 10000) {
    lastSwitch = millis();
    screenIndex = (screenIndex + 1) % 10; // Следующий параметр из 10
    drawSlide(screenIndex);
  }

  delay(100);
}
