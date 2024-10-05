//Код написан и распростроняется by SoDaVk (version A003)
//Не меняйте ничего, где это не нужно, если не знаете наверняка что это
#include "MenuDisplay.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
extern int selectedMenuItem;

// Устанавливаем размеры дисплея напрямую
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Функция для отображения текста в зависимости от выбранного элемента меню
void displayTextForMenuItem() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Проверяем, какой пункт меню выбран и отображаем соответствующий текст
    switch (selectedMenuItem) {
        case 0: {  // Info (Центрируем текст)
            const char* text = "";
            int16_t x1, y1;
            uint16_t w, h;
            
            // Вычисляем размер текста
            display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

            // Вычисляем координаты для центровки текста
            int x = (SCREEN_WIDTH - w) / 2;
            int y = (SCREEN_HEIGHT - h) / 2;

            // Устанавливаем курсор на вычисленные координаты
            display.setCursor(x, y);
            display.println(text);
            break;
        }

        case 1: {  // Info (Центрируем текст)
            const char* text = "by SoDaVk";
            int16_t x1, y1;
            uint16_t w, h;
            
            // Вычисляем размер текста
            display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

            // Вычисляем координаты для центровки текста
            int x = (SCREEN_WIDTH - w) / 2;
            int y = (SCREEN_HEIGHT - h) / 2;

            // Устанавливаем курсор на вычисленные координаты
            display.setCursor(x, y);
            display.println(text);
            break;
        }

        case 2:  // Flashlight
            display.fillScreen(SSD1306_WHITE);  // Включить все пиксели (фонарик)
            display.display();
            return;  // Обновляем и сразу выходим
        
        case 3:
            display.setCursor(0, 0); 
            display.println("Not available");
            break;

        case 4:
            display.setCursor(0, 0); 
            display.println("Not available");
            break;

        case 5:
            display.setCursor(0, 0); 
            display.println("Not available");
            break;

        default:
            display.setCursor(0, 0);
            display.println("Unknown item");
            break;
    }

    display.display();  // Обновить дисплей
}
