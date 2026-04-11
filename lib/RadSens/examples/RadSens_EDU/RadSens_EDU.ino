/* RadSensor EDU V1.1
   by ClimateGuard, 2022.
*/


// НАСТРОЙКИ
#define SOUND_ON_ENABLE 1   // Звуковое приветствие при старте

#define R1 100    // Верхнее плечо делителя (R1 на плате) [КОм]
#define R4 100    // Нижнее плечо делителя (R4 на плате) [КОм]

#define ADC_pin A0 // Пин АЦП
#define buz_pin 14 // Пин пьезоизлучателя

// ПОДКЛЮЧЕНИЕ БИБЛИОТЕК
#include <CG_RadSens.h>                     // RadSens
CG_RadSens radSens(RS_DEFAULT_I2C_ADDRESS); // Конструктор RadSens

#include <GyverOLED.h>          // OLED
GyverOLED<SSH1106_128x64> oled; // Конструктор экрана

// ПЕРЕМЕННЫЕ
float Voltage;       // Переменная напряжения
uint32_t timer_cnt;  // Таймер для измерений дозиметра
uint32_t timer_bat;  // Таймер для измерения заряда батареи
uint32_t timer_imp;  // Таймер опроса импульсов для пьезоизлучателя
uint32_t pulsesPrev; // Число импульсов за предыдущую итерацию

// ФУНКЦИИ
void splash_screen(); // Метод: приветственный дисплей
void disp_refresh();  // Метод: обновление дисплея
void pulse_notify();  // Метод: звуковая индикация
void bat_measure();   // Метод: Обновление заряда аккумулятора

// ГЛАВНЫЙ МЕТОД SETUP (вызывается один раз)
void setup() {
  //Wire.begin();
  Serial.begin(115200);
  oled.init(); // Инициализируем OLED в коде
  oled.clear();
  oled.update();
  pinMode(ADC_pin, OUTPUT); // Инициализируем АЦП как получатель данных
  splash_screen();  // Приветствуем пищанием
  oled.update();  // Обновляем экран
  pulsesPrev = radSens.getNumberOfPulses(); // Записываем значение для предотвращения серии тресков на старте
  oled.clear();
  oled.update();
}

// ГЛАВНЫЙ МЕТОД LOOP (вызывается каждый раз)
void loop() {
  pulse_notify();
  disp_refresh();
  bat_measure();
}

// Приветственная нотификация
void splash_screen() {
  tone(buz_pin, 0);
  delay(100);
  oled.setScale(2);
  oled.setCursor(10, 3);
  oled.print("Radsensor");
  oled.update();
#if SOUND_ON_ENABLE     // Если настроен звук при включении
  tone(buz_pin, 500);
  delay(400);
  tone(buz_pin, 600);
  delay(500);
  tone(buz_pin, 900);
  delay(1100);
  tone(buz_pin, 0);
#endif
  delay(2000);
  oled.clear();
}

void disp_refresh() {
  // Снимаем показания с дозиметра и выводим их на экран
  if (millis() - timer_cnt > 1000) {
    oled.clear();
    timer_cnt = millis();
    char buf1[50];
    char buf2[50];
    sprintf(buf1, "%.1f мкр/ч   ", radSens.getRadIntensyDynamic()); // Собираем строку с показаниями динамической интенсивности
    sprintf(buf2, "Стат: %.1f мкр/ч ", radSens.getRadIntensyStatic()); // Собираем строку с показаниями средней интенсивности за период работы
    oled.setCursor(0, 2);
    oled.setScale(2);
    oled.print(buf1);
    oled.setCursor(0, 6);
    oled.setScale(1);
    oled.print(buf2);

    oled.rect(110, 0, 124, 8, OLED_STROKE);
    oled.rect(125, 3, 126, 5, OLED_FILL);
   //выводим на экран уровеь заряда аккумулятора ввиде прямоугольников
  if (Voltage > 3.0f){ 
    int cell_count = ceil((Voltage - 3.0) / 0.4); //интересующий нас диапозон напряжений от 3.0 до 4.2
    if(cell_count > 3)//максимальное доступное колличество прямоугольников символизирующих заряд - 3
       cell_count = 3;
    int start_pos = 112; //начало координат первого прямоугольника по оси х
    for (int i = 0; i < cell_count; i++)
     {
       oled.rect(start_pos, 2, start_pos+2, 6, OLED_FILL);
       start_pos += 4;  
     }
   }
    oled.update(); // Обновляем экран
  }
}

//Функция предупреждения при превышении порога излучения
void alarm_notify() {
  // 3 раза выдаём "мигалку"
  for (int i = 0; i < 3; i++) {
    tone(buz_pin, 1500);
    delay(150);
    tone(buz_pin, 800);
    delay(150);
    tone(buz_pin, 0);
    delay(150);
  }
}

//Функция, которая создаёт "трески" пьезоизлучателя при появлении импульсов
void sound_notify() {
  tone(buz_pin, 300);
  delay(13);
  tone(buz_pin, 0);
  delay(40);
}

void pulse_notify() {
  if (millis() - timer_imp > 250) {
    timer_imp = millis();
    int pulses = radSens.getNumberOfPulses();
    if (pulses - pulsesPrev > 5 ) {
      pulsesPrev = pulses;
      alarm_notify();
    }
    if (pulses > pulsesPrev) {
      for (int i = 0; i < (pulses - pulsesPrev); i++) {
        sound_notify();
      }
      pulsesPrev = pulses;
    }
  }
}

void bat_measure() {

  if (millis() - timer_bat > 500) {
    timer_bat = millis();

    float voltage = 3.3f / 1023.0f * analogRead(ADC_pin);
    Voltage = (voltage * (R1 + R4)) / R4;
  }
}
