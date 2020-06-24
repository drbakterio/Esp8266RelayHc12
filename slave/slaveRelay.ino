#include <ESP8266WiFi.h>
#include <espnow.h>
#include <WifiEspNow.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint8_t mymac[] = {0xEC, 0xFA, 0xBC, 0x9B, 0xF5, 0x62};
uint8_t mac[] = {0xEC, 0xFA, 0xBC, 0x9B, 0xF5, 0x61};

const int relay = D5;
int heartBeat;

typedef struct struct_message {
  bool power;
  bool readRelay;
} struct_message;

struct_message message;

void initEspNow() {
  wifi_set_macaddr(STATION_IF, &mymac[0]);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  esp_now_register_recv_cb(OnDataRecv);
}

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t len) {
  String deviceMac = convertMac(mac_addr);

  memcpy(&message, data, sizeof(message));

  Serial.print("Message received from device: ");
  Serial.print(deviceMac);
  Serial.println();
  Serial.printf(message.power ? "Relay true" : "Relay false");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("power ");
  lcd.print(message.power);
  if (message.power) {
    digitalWrite(relay, LOW);
  } else {
    digitalWrite(relay, HIGH);

  }
  if (message.readRelay) {
    sendMessage(mac_addr);
  }
  Serial.println();
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  String deviceMac = convertMac(mac_addr);
  Serial.print("Sent to device: ");
  Serial.print(deviceMac);
  Serial.println();

  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Setup init");
  lcd.begin(16, 2);

  // Inicializar el LCD
  lcd.init();

  WiFi.mode(WIFI_STA);
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());

  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  initEspNow();
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH); //default power off
  Serial.println("Setup done");

}

void loop() {
  if (millis() - heartBeat > 30000) {
    Serial.println("Waiting for ESP-NOW messages...");
    lcd.setCursor(0, 0);
    lcd.print("Waiting for ESP-NOW messages...");
    heartBeat = millis();
    lcd.setCursor(0, 1);
    lcd.print("power ");
    lcd.print(digitalRead(relay));
  }
}

void sendMessage(uint8_t * mac_addr) {
  message.readRelay = digitalRead(relay);
  Serial.println("Sending message");
  lcd.setCursor(0, 1);
  lcd.print("Sending message");

  // Send message via ESP-NOW
  esp_now_send(mac_addr, (uint8_t *)&message, sizeof(message));

  delay(2000);
}

String convertMac(uint8_t * mac) {
  String deviceMac = "";
  deviceMac += String(mac[0], HEX);
  deviceMac += String(mac[1], HEX);
  deviceMac += String(mac[2], HEX);
  deviceMac += String(mac[3], HEX);
  deviceMac += String(mac[4], HEX);
  deviceMac += String(mac[5], HEX);
  return deviceMac;
}
