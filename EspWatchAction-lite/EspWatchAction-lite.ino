//Код написан и распростроняется by SoDaVk
//Не меняйте ничего, где это не нужно, если не знаете наверняка что это
//Обязательно впишите свои данные WiFi и API ключ для погоды, необязательно можно изменить время (по умолчанию время по МСК) и текст (функция отображения текста в самом низу кода)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>  // или #include <WiFi.h> для ESP32
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "MenuDisplay.h"

// Настройки Wi-Fi, впишите свои точки WIFI и их пароли
const char* ssidList[] = {"wifi0", "wifi1", "wifi2"};
const char* passwordList[] = {"pass0", "pass1", "pass2"};
const int numWiFiNetworks = sizeof(ssidList) / sizeof(ssidList[0]);

// Настройки дисплея sh1306 I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// NTP клиент - настройки для Московского времени (MSK)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 1800000); // 10800 секунд = 3 часа смещения (МСК)

// Объявление функции connectToWiFi
void connectToWiFi() {
  Serial.begin(115200);
  
  for (int i = 0; i < numWiFiNetworks; i++) {
    Serial.print("Подключение к Wi-Fi: ");
    Serial.println(ssidList[i]);

    WiFi.begin(ssidList[i], passwordList[i]);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi подключен!");
      break;
    } else {
      Serial.println("\nНе удалось подключиться, пробуем другую сеть...");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Не удалось подключиться к Wi-Fi");
  }
}

void setup() {
  Wire.begin(D1, D2);  // Инициализация I2C на пинах SDA -> D1, SCL -> D2

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  // Загрузочный экран
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds("EWA", 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 - 10);
  display.println("EWA");
  
  display.setTextSize(1);
  display.getTextBounds("Loading...", 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 + 10);
  display.println("Loading...");
  display.display();

  // Полоска загрузки
  int progressBarWidth = SCREEN_WIDTH - 10;
  int progressBarHeight = 5;
  int progressBarX = 5;
  int progressBarY = SCREEN_HEIGHT - 10;
  int progress = 0;

  for (int i = 0; i <= 100; i++) {
    display.drawRect(progressBarX, progressBarY, progressBarWidth, progressBarHeight, SSD1306_WHITE);
    progress = (progressBarWidth * i) / 100;
    display.fillRect(progressBarX, progressBarY, progress, progressBarHeight, SSD1306_WHITE);
    display.display();
    delay(100);
  }

  // Подключаемся к Wi-Fi
  connectToWiFi();

  // Инициализация NTP клиента
  timeClient.begin();
}

void loop() {
  timeClient.update();
}

//ВРЕМЯ И ДАТА
// Функция для отображения времени и статуса Wi-Fi
void displayTimeWithWiFiStatus() {
    String currentTime = timeClient.getFormattedTime();
    unsigned long epochTime = timeClient.getEpochTime();
    
    // Определяем текущий день недели
    int currentDayOfWeek = (epochTime / 86400L + 4) % 7; // 1 января 1970,четверг - начало эпохи UNIX

    // Массив полных названий и сокращений дней недели
    const char* fullDaysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* shortDaysOfWeek[] = {"Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri"};

    // Очистка дисплея
    display.clearDisplay();

    // Определяем размеры текста для центрирования
    int16_t x1, y1;
    uint16_t dayWidth, dayHeight;
    display.setTextSize(1); // Размер текста для дня недели
    display.setTextColor(WHITE); // Цвет текста

    // Корректировка индексов для предыдущего и следующего дня
    int previousDayIndex = (currentDayOfWeek - 1 + 1) % 7; // Индекс для предыдущего дня
    int nextDayIndex = (currentDayOfWeek + 1 + 8) % 7; // Индекс для следующего дня

    // Отображение предыдущего дня
    display.getTextBounds(shortDaysOfWeek[previousDayIndex], 0, 0, &x1, &y1, &dayWidth, &dayHeight);
    int previousDayX = (SCREEN_WIDTH - 100) / 2 - 1; // Центрируем предыдущий день
    display.setCursor(previousDayX, 0); // Положение для предыдущего дня
    display.print(shortDaysOfWeek[previousDayIndex]);

    // Отображение текущего дня недели по центру
    display.getTextBounds(fullDaysOfWeek[currentDayOfWeek], 0, 0, &x1, &y1, &dayWidth, &dayHeight); 
    int currentDayX = (SCREEN_WIDTH - dayWidth) / 2; // Центрируем текущий день
    display.setCursor(currentDayX, 0); // Положение для текущего дня
    display.print(fullDaysOfWeek[currentDayOfWeek]); // Выводим текущий день недели

    // Отображение следующего дня
    display.getTextBounds(shortDaysOfWeek[nextDayIndex], 0, 0, &x1, &y1, &dayWidth, &dayHeight);
    int nextDayX = (SCREEN_WIDTH - 100) / 2 + 85; // Центрируем следующий день
    display.setCursor(nextDayX, 0); // Положение для следующего дня
    display.print(shortDaysOfWeek[nextDayIndex]);

    // Отображение уровня сигнала
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(SCREEN_WIDTH - 25, SCREEN_HEIGHT - 10); // Положение для сигнала
    display.print(WiFi.RSSI()); // Отображаем уровень сигнала

    // Определяем размеры текста для центрирования времени
    int16_t xTime, yTime;
    uint16_t timeWidth, timeHeight;
    display.setTextSize(2);  // Увеличиваем размер текста для времени
    display.getTextBounds(currentTime, 0, 0, &xTime, &yTime, &timeWidth, &timeHeight);

    // Вычисляем позицию для времени по центру
    int16_t centeredX = (SCREEN_WIDTH - timeWidth) / 2;
    int16_t centeredY = (SCREEN_HEIGHT - timeHeight) / 2; // Позиция для времени

    // Вывод времени
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(centeredX, centeredY);
    display.print(currentTime);

    // Вывод даты под временем, чуть ниже
    // Вычисление текущей даты
    int currentYear = 1970;
    byte daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned long days = epochTime / 86400L; // Количество дней с начала эпохи UNIX
    while ((days >= 365 && !((currentYear % 4 == 0 && currentYear % 100 != 0) || (currentYear % 400 == 0))) || days >= 366) {
        if ((currentYear % 4 == 0 && currentYear % 100 != 0) || (currentYear % 400 == 0)) {
            days -= 366;
        } else {
            days -= 365;
        }
        currentYear++;
    }

    byte currentMonth = 0;
    for (currentMonth = 0; currentMonth < 12; currentMonth++) {
        if (currentMonth == 1 && ((currentYear % 4 == 0 && currentYear % 100 != 0) || (currentYear % 400 == 0))) {
            if (days < 29) {
                break;
            }
            days -= 29;
        } else {
            if (days < daysInMonth[currentMonth]) {
                break;
            }
            days -= daysInMonth[currentMonth];
        }
    }

    int currentDay = days + 1;
    String currentDate = String(currentDay) + "/" + String(currentMonth + 1) + "/" + String(currentYear);

    // Вывод даты чуть ниже
    display.setTextSize(1);
    uint16_t dateWidth, dateHeight;
    display.getTextBounds(currentDate, 0, 0, &xTime, &yTime, &dateWidth, &dateHeight);
    display.setCursor((SCREEN_WIDTH - dateWidth) / 2, centeredY + timeHeight + 14);
    display.print(currentDate);

    // Отображение статуса Wi-Fi
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, SCREEN_HEIGHT - 10);

    if (WiFi.status() == WL_CONNECTED) {
        display.print("Wi-Fi"); // Печатаем "Wi-Fi", если подключены
    } else {
        display.print(" X"); // Печатаем "X", если отключены
    }

    display.display();
}
