#include <OneWire.h>

#include <DallasTemperature.h>

#include <Wire.h>

#include <CG_RadSens.h>

#include <HardwareSerial.h>

uint8_t dallas_pin = 40; //Пин данных, к которому подключен датчик температуры


HardwareSerial Serial2(1); 
CG_RadSens rads(RS_DEFAULT_I2C_ADDRESS);
OneWire oneWire(dallas_pin);
DallasTemperature sensors(&oneWire);
//Данные для подключения модуля SIM800 к GPRS. лучше узнавать у оператора, необходима поддержка оператором 2G сетей 
String apn = "internet";                    //APN
String apn_u = "gdata";                     //APN-Username
String apn_p = "gdata";                     //APN-Password
String url = "http://narodmon.ru"; 
String mac;  //Уникальный MAC-адрес для регистрации устройства на народмоне, без него сервер не примет показания. можно взять мак адрес Wi-Fi станции ESP
String name;  //Имя станции (необязательно)
String owner;  //Логин владельца станции нужен для привязки датчика к аккаунту некоего пользователя (необязательно)
String lat; //Широта (необязательно)
String lon; //Долгота (необязательно)
String alt; //Высота над уровнем моря (необязательно)

adc_attenuation_t ADC_ATTEN_DB_11; //Калибровочная таблица для корректного считывания напряжения на АЦП

void setup()
{
  pinMode(8, OUTPUT); 
  delay(1000);
  digitalWrite(8, 0); //Включение питания на шине i2c
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 5, 4);
  Wire.begin(7, 6);
  rads.init(); //Инициализация библиотеки модуля RadSens , можно считать булево значение 
  delay(15000);
  //rads.setLPmode(true); //Включение режима низкого энергопотребления на датчике радиации (снижает потребление, отключает светодиод и интервально питает генератор) 
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  delay(5000);
  analogSetPinAttenuation(9, ADC_ATTEN_DB_11); //Устанавливаем калибровочную таблицу на пин АЦП
  gsm_config_gprs(); //Настраиваем GPRS 
}

//Формируем GET-запрос на Народмон
void loop() {
  sensors.requestTemperatures();
  gsm_http_post("http://narodmon.ru/get?ID=" + mac + "&rad=" + String(rads.getRadIntensyStatic()) +"&temp=" + String(sensors.getTempC(0))+"&vcc=" + String(analogReadMilliVolts(9)) + "&name=" + name + "&owner=" + owner + "&lat=" + lat + "&lon=" + lon + "&alt=" + alt);
  delay(600000);
}

void gsm_http_post( String postdata) {
  Serial.println("post gprs"); //AT-команды для управления передачей данных с модуля SIM800 на народмон
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + postdata);
  gsm_send_serial("AT+HTTPACTION=0");
  delay(3000);
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}

void gsm_config_gprs() {
  Serial.println("config gprs"); //Настройка GPRS подключения на SIM800 с помощью AT-команд
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_send_serial(String command) { //Функция для обмена сообщениями с модулем SIM800
  Serial.println(command);
  Serial2.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (Serial2.available()) {
      Serial.write(Serial2.read());
    }
  }
  Serial.println();
}

void sysInfo() { //Функция для вывода информации о модуле и показаний
  uint8_t chipId = rads.getChipId();
  uint8_t firmWare = rads.getFirmwareVersion();
  uint16_t sens = rads.getSensitivity();
  bool hvGen = rads.getHVGeneratorState();
  bool ledState = rads.getLedState();
  Serial.println("system info");
  Serial.print(" chip id ");
  Serial.print(chipId);
  Serial.print(" firmware version ");
  Serial.print(firmWare);
  Serial.print(" sensitivity  ");
  Serial.print(sens);
  Serial.print(" hvGenerator on ");
  Serial.print(hvGen);
  Serial.print(" led on  ");
  Serial.print(ledState);
  Serial.println(rads.getRadIntensyStatic());
}
