//ВАЖНО! Схемы подключения по образцу вы можете найти в "extras/Example schemes/ESP32 & ESP8266/" и выбрать свою плату
// Подключаем необходимые библиотеки
#include <Arduino.h>
#include "CG_RadSens.h"  // Библиотека RadSens
#include <Wire.h>        // I2C-библиотека
#include <GyverOLED.h>   // Библиотека для OLED Gyver'а идеально подойдёт для понимания методики работы с OLED-экраном, к тому же тут сразу есть русский шрифт
#define buz 19   // Устанавливаем управляющий пин пьезоизлучателя. Если вы выбрали другой управляющий пин - замените значение

CG_RadSens radSens(RS_DEFAULT_I2C_ADDRESS);   // Инициализируем RadSens
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;   // Инициализируем OLED-экран


uint32_t timer_cnt; // Таймер опроса интенсивности излучения и ипульсов для OLED-экрана
uint32_t timer_imp; // Таймер опроса импульсов для пьезоизлучателя
uint32_t timer_oled; // таймер обновления дисплея

float dynval;  // Переменная для динамического значения интенсивности
float statval; // Переменная для статического значения интенсивности
uint32_t impval;  // Переменная для кол-ва импульсов
uint32_t pulsesPrev;  // Переменная, содержащая кол-во импульсов за прошлый цикл

/*void beep(int deltime) {     // Функция, описывающая время и частоту пищания пьезоизлучателя
  ledcWriteTone(1, 500);  // Включаем на частоте 500 Гц
  delay(3);
  ledcWriteTone(1, 0);   // Выключаем
  delay(deltime);
} */

  void beep(int deltime){
    tone(buz, 5000, deltime);
  }                                             /* функция для Arduino */

void setup() {
  pinMode(buz, OUTPUT);  // Инициализируем пьезоизлучатель как получатель данных
  /*ledcSetup(1, 500, 8); // Инициализируем ШИМ (только для ESP, для Arduino это необходимо стереть)
  ledcAttachPin(buz, 1); // Задаём пин вывода пьезоизлучателя для ШИМа (только для ESP, для Arduino это необходимо стереть)*/
  oled.init();           // Инициализируем OLED в коде
  oled.flipV(1);         // Я перевернул экран для удобства
  oled.flipH(1);         // Для нормального отображения после переворота нужно инвертировать текст по горизонтали
  oled.clear();
  oled.setScale(2);      // Устанавливаем размер шрифта
  oled.print("CG_RadSens");
  delay(3000);
  radSens.init();
  oled.clear();
  delay(3000);
  oled.clear();
  pulsesPrev = radSens.getNumberOfPulses(); //Обнуляем значение перед началом работы пьезоизлучателя для предотвращения динных тресков
}

void loop() {

  if (millis() - timer_imp > 250) {  // Функция, создающая "треск" пьезоизлучателя
    timer_imp = millis();
    int pulses = radSens.getNumberOfPulses();
    if (pulses > pulsesPrev) {
      for (int i = 0; i < (pulses - pulsesPrev); i++) {
        beep(30);  // Вы можете изменить параметр, если хотите, чтобы интервал между тресками был больше или меньше
      }
      pulsesPrev = pulses;
    }
  }

  if (millis() - timer_cnt > 1000) {      // Записываем в объявленные глобальные переменные необходимые значения
    timer_cnt = millis();
    dynval = radSens.getRadIntensyDynamic(); 
    statval = radSens.getRadIntensyStatic();
    impval = radSens.getNumberOfPulses();
  }

  if (millis() - timer_oled > 1000) {  //Записываем переменные в строки и выводим их на OLED-экран
    timer_oled = millis();
    String dynint = "Дин: ";
    dynint += dynval;
    String statint = "Ст:  ";
    statint += statval;
    String nimp = "Имп: ";
    nimp += impval;
    oled.setCursor(0, 1);
    oled.print(dynint);
    oled.setCursor(0, 3);
    oled.print(statint);
    oled.setCursor(0, 5);
    oled.print(nimp);
  }
}
