#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AUXHeatpumpIR.h>

// This example based on DaikinTest_MQTT example. Thanks for @nick1802 for sample

IRSenderESP8266 irSender(4); // Set GPIO pin number here. I use D2 pin on my Wemos D1 mini. See image in this example path
AUXHeatpumpIR *heatpumpIR; // This class also works with Hyundai H-AR16-09H model (rc YKR-P/002E)

// TODO: Make next params manually configurable on first launch

const char* ssid = "SSID";// your wifi name
const char* password = "SSIDPASS";  // your wifi password

IPAddress ip(192, 168, 1, 100); //ESP static ip
IPAddress gateway(192, 168, 1, 1);   //Set Gateway
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
const char* mqtt_server = "192.168.1.101";  // MQTT Server address
const int mqtt_port = 1883;  // MQTT Server port (default: 1883)

String client_id = "ESP-" + String(ESP.getChipId(), HEX);
String power_topic = "homeassistant/ac/" + client_id + "/switch";
String mode_topic = "homeassistant/ac/" + client_id + "/mode/set";
String fan_topic = "homeassistant/ac/" + client_id + "/fan_speed/set";
String temperature_topic = "homeassistant/ac/" + client_id + "/temperature/set";
String swing_topic = "homeassistant/ac/" + client_id + "/swing/set";

int power;
int acmode;
int fan;
int temp;
int swing;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(74880);

  setup_wifi();

  heatpumpIR = new AUXHeatpumpIR(); // Initiate IR receiver object

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Set callback to MQTT client which calls everytime in main loop function

  Serial.println("Topics:");
  Serial.println(power_topic);
  Serial.println(mode_topic);
  Serial.println(fan_topic);
  Serial.println(temperature_topic);
  Serial.println(swing_topic);

  // Set default AC settings
  power = POWER_OFF;
  acmode = MODE_AUTO;
  fan = FAN_AUTO;
  temp = 25;
  swing = VDIR_SWING;
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
  Serial.print("RSSI : ");
  Serial.println(WiFi.RSSI());      // wifi signal strength
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    client.loop();
    if (client.connect(client_id.c_str())) {
      client.subscribe(power_topic.c_str());
      client.subscribe(mode_topic.c_str());
      client.subscribe(fan_topic.c_str());
      client.subscribe(temperature_topic.c_str());
      client.subscribe(swing_topic.c_str());
    } else {
      delay(500);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String Payload = "";
  for (int i = 0; i < length; i++) Payload += (char)payload[i];

  if (String(topic) == power_topic) {
    if (Payload == "ON") power = POWER_ON;
    else if (Payload == "OFF") power = POWER_OFF;
  }
  if (String(topic) == mode_topic) {
    if (Payload == "heat") acmode = MODE_HEAT;
    else if (Payload == "cool") acmode = MODE_COOL;
    else if (Payload == "dry") acmode = MODE_DRY;
    else if (Payload == "fan_only") acmode = MODE_FAN;
    else if (Payload == "auto") acmode = MODE_AUTO;
  }
  if (String(topic) == fan_topic) {
    if (Payload == "auto") fan = FAN_AUTO;
    else if (Payload == "low") fan = FAN_1;
    else if (Payload == "medium") fan = FAN_2;
    else if (Payload == "high") fan = FAN_3;
  }
  if (String(topic) == temperature_topic) {
    temp = Payload.toInt();
  }
  if (String(topic) == swing_topic) {
    if (Payload == "on") swing = VDIR_AUTO;
    else if (Payload == "off") swing = VDIR_SWING;
  }

  heatpumpIR->send(irSender, power, acmode, fan, temp, swing, 0); // Last zero because my AC doesn't have horizontal swing function
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  delay(50);
}
