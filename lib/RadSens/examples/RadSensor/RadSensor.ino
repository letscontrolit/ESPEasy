// Инициализируем библиотеки
#include <Wire.h>
#include <CG_RadSens.h>
#include <GyverOLED.h>

#define ADC_pin A0 // задаём значение пина АЦП
#define buz_pin 14 // Задаём значения пина для пищалки

GyverOLED<SSH1106_128x64> oled; // Инициализируем 1.3" OLED-экран
CG_RadSens radSens(RS_DEFAULT_I2C_ADDRESS); // Инициализируем RadSens

uint16_t ADC; // Переменная для значений АЦП
uint32_t timer_cnt; // Таймер для измерений дозиметра
uint32_t timer_bat; // Таймер для измерения заряда батареи
uint32_t timer_imp; // Таймер опроса импульсов для пьезоизлучателя
uint32_t pulsesPrev; // Число импульсов за предыдущую итерацию

//Функция аудиоприветствия
void hello() {
  for (int i = 1; i < 5; i++) {
    tone(buz_pin, i * 1000);
    delay(100);
  }
  tone(buz_pin, 0);
  delay(100);
  oled.setScale(2);
  oled.setCursor(10, 3);
  oled.print("Radsensor");
  oled.update();  
  delay(3000);
  oled.clear(); 
}

//Функция, которая создаёт "трески" пьезоизлучателя при появлении импульсов
void beep() {     // Функция, описывающая время и частоту пищания пьезоизлучателя
  tone(buz_pin, 3500);
  delay(13);
  tone(buz_pin, 0);
  delay(40);
}

//Функция предупреждения при превышении порога излучения
void warning() {
  for (int i = 0; i < 3; i++) {
    tone(buz_pin, 1500);
    delay(250);
    tone(buz_pin, 0);
    delay(250);
  }
}

void setup() {
  Wire.begin();
  oled.init(); // Инициализируем OLED в коде
  oled.clear(); 
  oled.update();  
  pinMode(ADC_pin, OUTPUT); // Инициализируем АЦП как получатель данных
  hello();  // Приветствуем пищанием  
  oled.update();  // Обновляем экран
  pulsesPrev = radSens.getNumberOfPulses(); // Записываем значение для предотвращения серии тресков на старте
}

void loop() {
  // Раз в 250 мс происходит опрос счётчика импульсов для создания тресков, если число импульсов за 250 мс превысит 5, раздастся предупреждение
  if (millis() - timer_imp > 250) {  
    timer_imp = millis();
    int pulses = radSens.getNumberOfPulses();
    if (pulses - pulsesPrev > 5 ) {
      pulsesPrev = pulses;
      warning();
    }
    if (pulses > pulsesPrev) {
      for (int i = 0; i < (pulses - pulsesPrev); i++) {
        beep();
      }
      pulsesPrev = pulses;
    }
  }
  // Снимаем показания с дозиметра и выводим их на экран
  if (millis() - timer_cnt > 1000) { 
    timer_cnt = millis();
    char buf1[50];
    char buf2[50];
    char buf3[50];
    sprintf(buf1, "%.1f мкр/ч", radSens.getRadIntensyDynamic()); // Собираем строку с показаниями динамической интенсивности
    sprintf(buf2, "Стат: %.1f мкр/ч ", radSens.getRadIntensyStatic()); // Собираем строку с показаниями средней интенсивности за период работы
    oled.setCursor(0, 2);
    oled.setScale(2);
    oled.print(buf1);
    oled.setCursor(0, 6);
    oled.setScale(1);
    oled.print(buf2);
  }
  // Считываем показание с АЦП, рисуем батарею и создаём индикацию заряда, показания АЦП вы можете подстроить под своё удобство
  if (millis() - timer_bat > 5000) { 
    timer_bat = millis();
    ADC = analogRead(ADC_pin); 
    oled.rect(110, 0, 124, 8, OLED_STROKE); 
    oled.rect(125, 3, 126, 5, OLED_FILL);
    if (ADC >= 350) {
      oled.rect(112, 2, 114, 6, OLED_FILL);
      oled.rect(116, 2, 118, 6, OLED_FILL);
      oled.rect(120, 2, 122, 6, OLED_FILL);
    }
    if (ADC < 350 && ADC >= 335) {
      oled.rect(112, 2, 114, 6, OLED_FILL);
      oled.rect(116, 2, 118, 6, OLED_FILL);
    }
    if (ADC < 335 && ADC >= 320) {
      oled.rect(112, 2, 114, 6, OLED_FILL);
    }
    if (ADC < 320){
      oled.rect(110, 0, 124, 8, OLED_STROKE);
      oled.rect(125, 3, 126, 5, OLED_FILL);
    }
  }
  oled.update(); // Обновляем экран в конце цикла
}
