/*    im a novis to coding but do try. i have bit banged this together and works for me.
 *    hope this helps any one else that may need it.
 */
#include <DaikinHeatpumpARC480A14IR.h>
#include <HeatpumpIR.h>
#include <IRSender.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

IRSenderESP8266 irSender(14);     // IR led on Duemilanove digital pin 3, using Arduino PWM
DaikinHeatpumpARC480A14IR *heatpumpIR;

const char* ssid = "SSID";// your wifi name
const char* password = "PASS";  // your wifi password
IPAddress ip(192, 168, 0, 53); //ESP static ip
IPAddress gateway(192, 168, 0, 1);   //Set Gateway
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
const char* mqtt_server = "192.168.0.44";  // MQTT Server address

const char* client_id = "ESP8266_53"; // Must be unique on the MQTT network
const char* inTopic1 = "192.168.0.53_power";  // topic names
const char* inTopic2 = "192.168.0.53_mode";
const char* inTopic3 = "192.168.0.53_fan";
const char* inTopic4 = "192.168.0.53_temp";
const char* inTopic5 = "192.168.0.53_swing";
const char* inTopic6 = "192.168.0.53_comfort";
const char* inTopic7 = "192.168.0.53_econo";
const char* inTopic8 = "192.168.0.53_sensor";
const char* inTopic9 = "192.168.0.53_quiet";
const char* inTopic10 = "192.168.0.53_powerful";

int power;
int acmode;
int fan;
int temp;
int swing;
boolean comfort;
boolean econo;
boolean sensor;
boolean quiet;
boolean powerful;
int high = true;
int low = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(74880);
  setup_wifi();           //connect to wifi
  Serial.print("RSSI : ");
  Serial.println(WiFi.RSSI());      // wifi signal strength
  Serial.println();
  heatpumpIR = new DaikinHeatpumpARC480A14IR();
  client.setServer(mqtt_server, 1883);      
  client.setCallback(callback);

  power = POWER_OFF;        //after power outage reset remote to these settings.  does not transmit
  acmode = MODE_FAN;
  fan = FAN_1;
  temp = 26;
  swing = VDIR_AUTO;
  comfort = false;
  econo = false;
  sensor = false;
  quiet = false;
  powerful = false;
}

void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);      //set wifi to connect to your wifi and not start a AP
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {       // waits for WIFI to connect
    delay(500);
  }
  Serial.println(WiFi.localIP());
}

void reconnect() {        //waits for MQTT to connect then connects to topics
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    client.loop();
    if (client.connect(client_id)) {
      client.subscribe(inTopic1);
      client.subscribe(inTopic2);
      client.subscribe(inTopic3);
      client.subscribe(inTopic4);
      client.subscribe(inTopic5);
      client.subscribe(inTopic6);
      client.subscribe(inTopic7);
      client.subscribe(inTopic8);
      client.subscribe(inTopic9);
      client.subscribe(inTopic10);
      client.subscribe(outTopic1);
    } else {
      delay(500);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {    // lissons for MQTT updates
  if (strcmp(topic, inTopic1) == 0) {  //POWER
    if (payload[0] == '1') {
      power = POWER_ON;
    }
    else if (payload[0] == '0') {
      power = POWER_OFF;
    }
  }
  if (strcmp(topic, inTopic2) == 0) {   //MODE
    if (payload[0] == 'a') {
      acmode = MODE_AUTO;
    }
    else if (payload[0] == 'h') {
      acmode = MODE_HEAT;
    }
    else if (payload[0] == 'c') {
      acmode = MODE_COOL;
    }
    else if (payload[0] == 'd') {
      acmode = MODE_DRY;
    }
    else if (payload[0] == 'f') {
      acmode = MODE_FAN;
    }
  }
  if (strcmp(topic, inTopic3) == 0) {   //FAN
    if (payload[0] == 'a') {
      fan = FAN_AUTO;
    }
    else if (payload[0] == '1') {
      fan = FAN_1;
    }
    else if (payload[0] == '2') {
      fan = FAN_2;
    }
    else if (payload[0] == '3') {
      fan = FAN_3;
    }
    else if (payload[0] == '4') {
      fan = FAN_4;
    }
    else if (payload[0] == '5') {
      fan = FAN_5;
    }
    else if (payload[0] == 's') {
      fan = FAN_SILENT;
    }
  }
  if (strcmp(topic, inTopic4) == 0) {   //TEMP
    if (payload[0] == '1') {
      if (payload[1] == '8') {
        temp = 18;
      }
      else if (payload[1] == '9') {
        temp = 19;
      }
    }
    else if (payload[0] == '3') {
      if (payload[1] == '0') {
        temp = 30;
      }
      else if (payload[1] == '1') {
        temp = 31;
      }
    }
    else if (payload[0] == '2') {
      if (payload[1] == '0') {
        temp = 20;
      }
      else if (payload[1] == '1') {
        temp = 21;
      }
      else if (payload[1] == '2') {
        temp = 22;
      }
      else if (payload[1] == '3') {
        temp = 23;
      }
      else if (payload[1] == '4') {
        temp = 24;
      }
      else if (payload[1] == '5') {
        temp = 25;
      }
      else if (payload[1] == '6') {
        temp = 26;
      }
      else if (payload[1] == '7') {
        temp = 27;
      }
      else if (payload[1] == '8') {
        temp = 28;
      }
      else if (payload[1] == '9') {
        temp = 29;
      }
    }
  }
  if (strcmp(topic, inTopic5) == 0) {   //SWING
    if (payload[0] == 'a') {
      swing = VDIR_SWING;
      Serial.println("swing on sent");
    }
    else if (payload[0] == 'm') {
      swing = VDIR_UP;
      Serial.println("swing off sent");
    }
  }
  if (strcmp(topic, inTopic6) == 0) {    //comfort
    if (payload[0] == '1') {
      comfort = high;
    }
    else if (payload[0] == '0') {
      comfort = low;
    }
  }
  if (strcmp(topic, inTopic7) == 0) {    //econo
    if (payload[0] == '1') {
      econo = high;
    }
    else if (payload[0] == '0') {
      econo = low;
    }
  }
  if (strcmp(topic, inTopic8) == 0) {    //sensor
    if (payload[0] == '1') {
      sensor = high;
    }
    else if (payload[0] == '0') {
      sensor = low;
    }
  }
  if (strcmp(topic, inTopic9) == 0) {    //quiet
    if (payload[0] == '1') {
      quiet = high;
    }
    else if (payload[0] == '0') {
      quiet = low;
    }
  }
  if (strcmp(topic, inTopic10) == 0) {    //powerful
    if (payload[0] == '1') {
      powerful = high;
    }
    else if (payload[0] == '0') {
      powerful = low;
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(50);
}
