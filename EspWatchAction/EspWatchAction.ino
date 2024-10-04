//Код написан и распростроняется by SoDaVk (version A001)
//Не меняйте ничего, где это не нужно, если не знаете наверняка что это
//Обязательно впишите свои данные WiFi и API ключ для погоды, необязательно можно изменить время (по умолчанию время по МСК) и текст (функция отображения текста в самом низу кода)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//#include <ESP8266WiFi.h>  // 
#include <WiFi.h> для ESP32
//#include <ESP8266HTTPClient.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "MenuDisplay.h"

// Настройки Wi-Fi, впишите свои точки WIFI и их пароли - (НУЖНО ИЗМЕНИТЬ)
const char* ssidList[] = {"EspWatchAction", "wifi1", "wifi2"};  // Массив с SSID
const char* passwordList[] = {"12345678", "pass1", "pass2"};  // Массив с паролями
const int numWiFiNetworks = sizeof(ssidList) / sizeof(ssidList[0]);  // Количество сетей

// Настройки дисплея sh1306 I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Настройки кнопок
const int weatherButtonPin = 18; // Кнопка для погоды D5 / for esp32 GPIO18
const int textButtonPin = 19;    // Кнопка для текста D6 / for esp32 GPIO19
const int displayOnButtonPin = 23; // Кнопка для включения дисплея D7 /for esp32 GPIO23
bool weatherButtonPressed = false;
bool textButtonPressed = false;
bool displayOff = false; // Флаг, показывающий, отключен ли дисплей
unsigned long lastInteractionTime = 0;  // Время последнего взаимодействия
const unsigned long inactivityPeriod = 60000;  // Период бездействия 1 минута

// Настройки OpenWeatherMap - (НУЖНО ИЗМЕНИТЬ)
const String apiKey = "38498a725609bc5a11e3261b341ab7f7"; // Зарегестрируйтесь на сайте https://openweathermap.org/ и получите свой API
const String city = "Moscow";  // Название города также найдите свой город на https://openweathermap.org/ и вставте сюда
const String units = "metric"; // Для отображения в градусах Цельсия

// Глобальные переменные для отслеживания времени удерживания кнопок
unsigned long buttonPressStart = 0;
bool buttonD5Pressed = false;
bool buttonD6Pressed = false;

// Меню
bool menuDisplayed = false;  // Флаг, показывающий, отображается ли меню
int selectedMenuItem = 0;    // Текущий выбранный пункт меню
const int totalMenuItems = 6; // Количество пунктов меню

// Массив пунктов меню
const char* menuItems[] = {
  "[Back]",
  "[Info]",
  "Flashlight",
  "Telegram",
  "Deauther",
  "Sistem",
};

// NTP клиент - (НУЖНО ИЗМЕНИТЬ, только в случае если у вас время отличное от MSK)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 1800000); // 10800 секунд = 3 часа смещения (МСК), обращение каждые 30 минут

// Определите массив задержек для каждого пункта меню
const int menuDelays[] = {1, 500, 500, 500, 500, 500}; // Примерные значения в миллисекундах

void setup() {
  // Инициализация I2C с указанием пинов SDA и SCL
  Wire.begin(21, 22); // SDA -> D1, SCL -> D2  /for esp32 GPIO21 and GPIO22

  // Инициализация дисплея
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

// Показываем загрузочный экран
display.clearDisplay();

// Отображение "EWA" по центру
display.setTextSize(2); 
display.setTextColor(SSD1306_WHITE);
int16_t x1, y1;
uint16_t w, h;
display.getTextBounds("EWA", 0, 0, &x1, &y1, &w, &h);
display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 - 10);
display.println("EWA");

// Отображение "Loading..." по центру под "EWA"
display.setTextSize(1); 
display.getTextBounds("Loading...", 0, 0, &x1, &y1, &w, &h);
display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 + 10);
display.println("Loading...");

// Обновляем экран, чтобы показать начальные надписи
display.display();

// Полоска загрузки
int progressBarWidth = SCREEN_WIDTH - 10;
int progressBarHeight = 5;
int progressBarX = 5;
int progressBarY = SCREEN_HEIGHT - 10;
int progress = 0;

// Обновляем полоску
for (int i = 0; i <= 100; i++) {
    // Рисуем рамку полоски загрузки
    display.drawRect(progressBarX, progressBarY, progressBarWidth, progressBarHeight, SSD1306_WHITE);

    progress = (progressBarWidth * i) / 100;  // Прогресс заполняется от 0 до полной ширины
    display.fillRect(progressBarX, progressBarY, progress, progressBarHeight, SSD1306_WHITE);

    // Обновляем экран
    display.display();
    delay(100);
}

  // Инициализация кнопок
  pinMode(weatherButtonPin, INPUT_PULLUP);
  pinMode(textButtonPin, INPUT_PULLUP);
  pinMode(displayOnButtonPin, INPUT_PULLUP); // Кнопка для включения дисплея

  // Подключение к Wi-Fi
  connectToWiFi();

  // Инициализация NTP клиента
  timeClient.begin();
}

// Функция для отображения меню
void displayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Отображаем пункты меню
  for (int i = 0; i < totalMenuItems; i++) {
    if (i == selectedMenuItem) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(5, i * 10);
    display.println(menuItems[i]);
  }

  // Выводим надпись вертикально
  display.setTextSize(2);  // Размер текста для "EWA"
  
  // Устанавливаем текстовый цвет
  display.setTextColor(SSD1306_WHITE);

  // Положение для "E"
  display.setCursor(SCREEN_WIDTH - 20, 3); // Положение для "E"
  display.println("E");

  // Положение для "W"
  display.setCursor(SCREEN_WIDTH - 20, 23); // Положение для "W"
  display.println("W");

  // Положение для "A"
  display.setCursor(SCREEN_WIDTH - 20, 43); // Положение для "A"
  display.println("A");

  display.display();
}

void loop() {
  timeClient.update();

  // Функции кнопки D7 в меню
if (menuDisplayed && digitalRead(displayOnButtonPin) == LOW) {
    // Если D7 нажата, вызываем функцию отображения текста
    displayTextForMenuItem();
    delay(menuDelays[selectedMenuItem]);  // Используем значение задержки из массива 
}

  // Проверяем, прошлa ли 1 минута бездействия
  if (millis() - lastInteractionTime > inactivityPeriod && !displayOff) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displayOff = true;
  }

  // Проверяем, нажата ли кнопка для включения дисплея или для отображения меню
  if (digitalRead(displayOnButtonPin) == LOW) {
    lastInteractionTime = millis();  // Сбрасываем таймер бездействия
    if (displayOff) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      displayOff = false;
    } else {
      menuDisplayed = !menuDisplayed;  // Переключаем между меню и часами
      if (menuDisplayed) {
        displayMenu();
      } else {
        displayTimeWithWiFiStatus();
      }
    }

    delay(300);
  }

  // Обработка кнопок для навигации по меню, если меню открыто
  if (menuDisplayed) {
    // Кнопка вверх (D6)
    if (digitalRead(textButtonPin) == LOW && !textButtonPressed) {
      textButtonPressed = true;
      selectedMenuItem--;
      if (selectedMenuItem < 0) {
        selectedMenuItem = totalMenuItems - 1;  // Цикл по меню
      }
      displayMenu();  // Обновляем отображение меню
    } else if (digitalRead(textButtonPin) == HIGH && textButtonPressed) {
      textButtonPressed = false;
    }

    // Кнопка вниз (D5)
    if (digitalRead(weatherButtonPin) == LOW && !weatherButtonPressed) {
      weatherButtonPressed = true;
      selectedMenuItem++;
      if (selectedMenuItem >= totalMenuItems) {
        selectedMenuItem = 0;  // Цикл по меню
      }
      displayMenu();  // Обновляем отображение меню
    } else if (digitalRead(weatherButtonPin) == HIGH && weatherButtonPressed) {
      weatherButtonPressed = false;
    }
  }

  // Обработка кнопок на главном экране (когда меню не отображается)
  if (!menuDisplayed) {
    // Кнопка для погоды (D5)
    if (digitalRead(weatherButtonPin) == LOW && !weatherButtonPressed) {
      weatherButtonPressed = true;
      lastInteractionTime = millis();  // Обновляем время последнего взаимодействия
      displayWeather();  // Функция для отображения погоды
    } else if (digitalRead(weatherButtonPin) == HIGH && weatherButtonPressed) {
      weatherButtonPressed = false;
    }

    // Кнопка для текста (D6)
    if (digitalRead(textButtonPin) == LOW && !textButtonPressed) {
      textButtonPressed = true;
      lastInteractionTime = millis();  // Обновляем время последнего взаимодействия
      displayText();  // Функция для отображения текста
    } else if (digitalRead(textButtonPin) == HIGH && textButtonPressed) {
      textButtonPressed = false;
    }
  }

  // Проверка состояния кнопок для перезагрузки
  bool d5State = digitalRead(weatherButtonPin) == LOW;
  bool d6State = digitalRead(textButtonPin) == LOW;

  if (d5State && d6State) {
    if (!buttonD5Pressed && !buttonD6Pressed) {
      buttonPressStart = millis();
    }
    buttonD5Pressed = true;
    buttonD6Pressed = true;

    if (millis() - buttonPressStart >= 3000) {
    display.clearDisplay();
    display.setTextSize(1); // Размер текста 1
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(46, 24);
    display.println("Reboot");
    display.display();

      delay(1000);
      ESP.restart();
    }
  } else {
    buttonD5Pressed = false;
    buttonD6Pressed = false;
  }

  // Если экран включен, меню не отображается и кнопки не нажаты, показываем время и статус Wi-Fi
  if (!weatherButtonPressed && !textButtonPressed && !displayOff && !menuDisplayed) {
    displayTimeWithWiFiStatus();
  }

  delay(100);
}

// Функция для подключения к Wi-Fi
void connectToWiFi() {
  // Сканирование доступных сетей
  int numNetworks = WiFi.scanNetworks();
  
  if (numNetworks == 0) {
    Serial.println("No networks found.");
    return;  // Если сети не найдены, выходим из функции
  }

  bool foundNetwork = false;
  for (int i = 0; i < numNetworks; i++) {
    String foundSSID = WiFi.SSID(i);
    Serial.print("Found network: ");
    Serial.println(foundSSID);
    
    // Проверяем, есть ли найденная сеть в списке предустановленных сетей
    for (int j = 0; j < numWiFiNetworks; j++) {
      if (foundSSID == ssidList[j]) {
        Serial.print("Matching network found: ");
        Serial.println(ssidList[j]);

        // Подключаемся к совпадающей сети
        WiFi.begin(ssidList[j], passwordList[j]);
        Serial.print("Connecting to ");
        Serial.println(ssidList[j]);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
          delay(500);
          Serial.print(".");
          attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nConnected to WiFi");
          foundNetwork = true;
          break;  // Выходим из цикла, если подключились
        } else {
          Serial.println("\nFailed to connect.");
        }
      }
    }

    if (foundNetwork) {
      break;  // Выходим из основного цикла, если подключились
    }
  }

  if (!foundNetwork) {
    Serial.println("No matching networks found.");
  }
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

//ПОГОДА
// Переменные для хранения последних данных о погоде
float lastTemp = 0;
String lastWeatherDesc = "N/A";
float lastWindSpeed = 0;
int lastWindDeg = 0;
int lastHumidity = 0;
float lastPressure = 0;
float lastUVIndex = 0;
bool weatherDataAvailable = false;

// Функция для преобразования угла в текстовое направление
String windDirection(float deg) {
    if (deg >= 337.5 || deg < 22.5) return "N";   // Северный
    if (deg >= 22.5 && deg < 67.5) return "NE";  // Северо-Восточный
    if (deg >= 67.5 && deg < 112.5) return "E";   // Восточный
    if (deg >= 112.5 && deg < 157.5) return "SE";  // Юго-Восточный
    if (deg >= 157.5 && deg < 202.5) return "S";   // Южный
    if (deg >= 202.5 && deg < 247.5) return "SW";  // Юго-Западный
    if (deg >= 247.5 && deg < 292.5) return "W";   // Западный
    if (deg >= 292.5 && deg < 337.5) return "NW";  // Северо-Западный
    return "N/A";  // Если по какой-то причине направление не определено
}

void displayWeather() {
    // Отображаем индикатор загрузки
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Loading weather...");
    display.display();

    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        // Основной запрос на получение погоды
        String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + units + "&appid=" + apiKey;
        http.begin(client, url);  // Используем новую версию begin с объектом WiFiClient
        
        int httpCode = http.GET();

        if (httpCode == 200) {
            String payload = http.getString();
            DynamicJsonDocument doc(3072); 
            deserializeJson(doc, payload);

            // Извлекаем данные о погоде
            lastTemp = doc["main"]["temp"];
            lastWeatherDesc = doc["weather"][0]["description"].as<const char*>();
            lastWindSpeed = doc["wind"]["speed"];
            lastWindDeg = doc["wind"]["deg"];  // Получаем направление ветра
            lastHumidity = doc["main"]["humidity"];
            lastPressure = doc["main"]["pressure"];

            // Извлекаем координаты для получения UV Index
            float lat = doc["coord"]["lat"];
            float lon = doc["coord"]["lon"];

            // Запрос UV Index на основе координат
            String uvUrl = "http://api.openweathermap.org/data/2.5/uvi?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + apiKey;
            http.begin(client, uvUrl);
            int uvHttpCode = http.GET();
            
            if (uvHttpCode == 200) {
                String uvPayload = http.getString();
                DynamicJsonDocument uvDoc(1024);
                deserializeJson(uvDoc, uvPayload);
                lastUVIndex = uvDoc["value"];
            }

            // Устанавливаем флаг, что данные о погоде доступны
            weatherDataAvailable = true;

        } else {
            Serial.println("Error getting weather data");
        }

        http.end();
    }

    if (weatherDataAvailable) {
        // Очистка дисплея и вывод сохранённых данных о погоде
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        
        // Сдвигаем строки вверх для оптимального отображения
        display.setCursor(0, 5);
        display.print("Temp: ");
        display.print(lastTemp);
        display.println(" C");
        
        display.setCursor(0, 15);
        display.print("More: ");
        display.println(lastWeatherDesc);
        
        display.setCursor(0, 25);
        display.print("Wind: ");
        display.print(lastWindSpeed);
        display.print(" m/s ");
        display.print("(");
        display.print(windDirection(lastWindDeg));
        display.println(")");

        display.setCursor(0, 35);
        display.print("Humidity: ");
        display.print(lastHumidity);
        display.println(" %");
        
        display.setCursor(0, 45);
        display.print("Pressure: ");
        display.print(lastPressure);
        display.println(" hPa");
        
        display.setCursor(0, 55);
        display.print("UV Index: ");
        display.print(lastUVIndex);
        
        display.display();
    } else {
        // Если данных о погоде нет, вывести сообщение
        display.clearDisplay();
        display.setTextSize(1);

        // Определяем размеры текста для центрирования
        int16_t x1, y1;
        uint16_t textWidth, textHeight;
        String message = "No weather data";

        // Получаем размеры текста
        display.getTextBounds(message, 0, 0, &x1, &y1, &textWidth, &textHeight);

        // Вычисляем координаты для центрирования текста по горизонтали
        int16_t centeredX = (SCREEN_WIDTH - textWidth) / 2;
        int16_t centeredY = (SCREEN_HEIGHT - textHeight) / 2;

        // Устанавливаем курсор для вывода текста
        display.setCursor(centeredX, centeredY);
        display.setTextColor(WHITE);
        display.println(message);
        display.display();
    }
}


// Функция для отображения текста
void displayText() {
  // Очистка дисплея
  display.clearDisplay();
  
  // Устанавливаем размер текста и цвет
  display.setTextSize(2); // Размер текста - (МОЖНО ИЗМЕНИТЬ, 1 - мелкий, 2 - крупный)
  display.setTextColor(WHITE);

  // Определяем размеры текста для его центрирования
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  String text = "v.A002"; // Текст для отображения - (МОЖНО ИЗМЕНИТЬ НА СВОЙ ТЕКСТ)
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  
  // Вычисляем позицию по центру экрана
  int16_t centeredX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t centeredY = (SCREEN_HEIGHT - textHeight) / 2;
  
  // Устанавливаем курсор на центр экрана и выводим текст
  display.setCursor(centeredX, centeredY);
  display.print(text);
  display.display();
}
