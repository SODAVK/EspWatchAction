# EspWatchAction
This project is in alpha testing state / Данный проект находится в состоянии альфа теста 

Разработка, реализация и поддержка by SoDaVk

Введение: Данное ПО было написано на коленке в целях как можно глубже изучить возможности как arduino IDE, так и свои.
Данный код можно использовать как стационарное устройство или как переносное, например наручные часы. 
Код обновляется и со временем могут быть добавлено еще больше функций

НАЧАЛО
Необходимые компоненты: 
Плата: esp8266 или esp32 (подойдет практически любая официальная плата)
Дисплей: OLED: sh1306, sh1106 (так же могут подойти многие другие дисплеи OLED)
Копки: Любые одноконтактные кнопки, 3шт (Один контакт к GPIO + питание)
Опционально: Может быть использован аккумулятор 3.7v от 100mAh, также для обеспечения корректного отображения заряда аккумулятора
необходимы два резистора на 220kOm и 100kOm, а также плата зарядки для аккумулятора.

Разработка и обкатка ПО проводилась на wemos D1 mini (esp8266, 4мб, 80-160МГц) 
и wemos s2 esp32 mini (esp32, 4мб, 2 ядра, 160-240МГц), в качестве дисплея использовался sh1306 (0,96Д)
и sh1106 (1,3Д), в качестве кнопок для компактности были выбраны кнопки 6x6мм + использовался аккумулятор 150mAh (20x20мм, 3.7v),
плата зарядки аккумулятора и on/off переключатель 

Полные схемы и инструкцию по сборке будут написаны и выложены в ближайшее время

Установка:
Для загрузки данного кода в плату может использоваться почти любой загрузчик .bin файлов (например веб приложение: esp.huhn.me),
или же Arduino IDE (более предпочтительно, т.к. вы можете изменить любые параметры под себя, например API ключ)

Загрузчик .bin файлов: самый простой вариант обновить или загрузить код в вашу плату это использование онлайн загрузчика esp.huhn.me,
для начала необходимо зайти на их сайт (https://esp.huhn.me), после чего подключить вашу плату проводом для передачи данных к usb порту ПК,
нажать CONENCT и выбрать свой порт (плату), затем загрузить bin файл на сайт и запустить загрузку. Через некоторое время в вашу плату будет загружен код со стандартными пинами кнопок и дисплея, временем по МСК и API погоды по Московской области. А так же в списоке wifi будет одна сеть под названием "EspWatchAction" и паролем "12345678", таким образом вы должны переименовать свою точку доступа на телефоне так же, и тогда часы будут работать. 

Arduino IDE: Наиболее предпочтительный вариант загрузки или обновления кода в плате, т.к. вы можете изменить отображение времени, погоды, точек wifi и API на свои. Для этого нужно открыть в Arduino IDE файл EspWatchAction, а затем Файл -> Настройки -> Дополнительные ссылки для менеджера плат, и вставить данные ссылки: 

http://arduino.esp8266.com/stable/package_esp8266com_index.json (Если у вас esp8266)

or

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json (Если у вас esp32)

После этого нужно открыть менеджер плат: Инструменты -> Плата -> Менеджер плат и ввести в поиске "esp32" или esp8266  (необходим набор "esp 32 by Espressif Systems" или "esp8266 by ESP8266 Community") и в меню плат выбрать свою плату. После чего изменить данные в коде на свои (то что можно изменять подписано "НУЖНО ИЗМЕНИТЬ" или "МОЖНО ИЗМЕНИТЬ" 

После замены данных на свои нужно нажать на стрелку загрузки в левом верхнем углу и подождать компиляции и загрузки. (Если при загрузке кода в плату возникают ошибки с загрузкой, то экспортируйте bin файл со своими данными и загрузите по инструкции загрузки bin файлов)

На данный момент это все доступные способы загрузки и настройки кода, однако в будущем планируется добавить возможность загрузки кода по wifi, настройка важных параметров через GUI или телеграм бота. 

Подключение контактов:
Для нормальной работы нужно подключить дисплей (D1-SDA, D2-SCL, G-G, 3v3-3v3), затем подключить кнопки (G-G для всех кнопок, и D7, D6, D5 для каждой кнопки по отдельности) на этом пока все. Остается лишь сделать корпус, здесь все на ваше усмотрение.

ФУНКЦИОНАЛ:
Пока функционал не очень большой, на данный момент доступны: Отображение времени, даты, дней недели и уровня сигнала wifi. После нажатия кнопки D5 отображается погода: Текущая температура, облачность, влажность, скорость и направление ветра, UV индекс и атмосферное давление.
По нажатию кнопки D6 отображается заранее записанный текст (можно изменить в Arduino IDE). По нажатию кнопки D7 появляется меню, в котором пока имеется лишь функция фонарика.

Пока что это все, однако в будущем планируется добавить функцию вывода текста или картинки отправленной телеграм боту на дисплее, отображение уровня заряда аккумулятора и системная информация, подсчет шагов и калорий, компас, манипуляции с rfid и nfc, и wifi scaner, jammer, bad APpoint. (для rfid, шагомера и калорий потребуется отдельные модули (аксилерометр и rfid mini))  

В общем развиваться можно еще очень далеко и долго

БЛАГОДАРНОСТИ:
Создатель, редакция, поддержка и разработка: by SoDaVk
Поддержка, помощь в разработке: Salvadorial
Альфа и Бета тестеры: Salvadorial, SoDaVk

Version A002 / Версия А002
