#include <Arduino.h>

#include <KY26HeatpumpIR.h>

IRSenderESP8266 irSender(4);

KY26HeatpumpIR heatpumpIR = KY26HeatpumpIR();

uint8_t powers[] = {
    POWER_ON,
    POWER_OFF,
};

uint8_t modes[] = {
    MODE_AUTO,
    MODE_COOL,
    MODE_DRY,
    MODE_FAN,
};

uint8_t fans[] = {
    FAN_1,
    FAN_2,
    FAN_3,
};

uint8_t power = powers[0];
uint8_t mode = modes[0];
uint8_t fan = fans[0];
uint8_t temperature = 24;
float timer = 0;

void printState() {
  Serial.print(F("\nPower: "));
  Serial.print(power == POWER_ON ? F("On") : F("Off"));
  Serial.print(F(", Mode: "));
  switch (mode) {
  case MODE_AUTO:
    Serial.print(F("Auto"));
    break;
  case MODE_COOL:
    Serial.print(F("Cool"));
    break;
  case MODE_DRY:
    Serial.print(F("Dry"));
    break;
  case MODE_FAN:
    Serial.print(F("Fan"));
    break;
  }
  Serial.print(F(", Fan: "));
  Serial.print(fan);
  Serial.print(F(", Temperature: "));
  Serial.print(temperature);
  Serial.print(F(", Timer: "));
  Serial.print(timer);
  Serial.println();
}

void promptPower() {
  Serial.println(F("\nSelect power:"));
  Serial.println(F("1: On"));
  Serial.println(F("2: Off"));

  while (true) {
    if (Serial.available() > 0) {
      int value = Serial.parseInt();
      if (value >= 1 && value <= 2) {
        power = powers[value - 1];
      }
      break;
    }
  }
}

void promptMode() {
  Serial.println(F("\nSelect mode:"));
  Serial.println(F("1: Auto"));
  Serial.println(F("2: Cool"));
  Serial.println(F("3: Dry"));
  Serial.println(F("4: Fan"));

  while (true) {
    if (Serial.available() > 0) {
      int value = Serial.parseInt();
      if (value >= 1 && value <= 4) {
        mode = modes[value - 1];
      }
      break;
    }
  }
}

void promptFan() {
  Serial.println(F("\nSelect fan:"));
  Serial.println(F("1: Low"));
  Serial.println(F("2: Medium"));
  Serial.println(F("3: High"));

  while (true) {
    if (Serial.available() > 0) {
      int value = Serial.parseInt();
      if (value >= 1 && value <= 3) {
        fan = fans[value - 1];
      }
      break;
    }
  }
}

void promptTemperature() {
  Serial.println(F("\nSelect temperature:"));

  while (true) {
    if (Serial.available() > 0) {
      int value = Serial.parseInt();
      if (value >= 15 && value <= 31) {
        temperature = value;
      }
      break;
    }
  }
}

void promptTimer() {
  Serial.println(F("\nSelect timer:"));

  while (true) {
    if (Serial.available() > 0) {
      float value = Serial.parseFloat();
      if (value >= 0 && value <= 12) {
        timer = value;
      }
      break;
    }
  }
}

void promptMenu() {
  Serial.println(F("\nSelect option:"));
  Serial.println(F("1: Power"));
  Serial.println(F("2: Mode"));
  Serial.println(F("3: Fan"));
  Serial.println(F("4: Temperature"));
  Serial.println(F("5: Timer"));

  while (true) {
    if (Serial.available() > 0) {
      int value = Serial.parseInt();
      switch (value) {
      case 1:
        promptPower();
        break;
      case 2:
        promptMode();
        break;
      case 3:
        promptFan();
        break;
      case 4:
        promptTemperature();
        break;
      case 5:
        promptTimer();
        break;
      }
      break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(500);
}

void loop() {
  printState();
  promptMenu();

  heatpumpIR.send(irSender, power, mode, fan, temperature, VDIR_AUTO, HDIR_AUTO,
                  timer);
}
