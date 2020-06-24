#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial HC12(D4, D3); // HC-12 TX Pin, HC-12 RX Pin

uint8_t mymac[] = {0xEC, 0xFA, 0xBC, 0x9B, 0xF5, 0x61};
uint8_t mac[] = {0xEC, 0xFA, 0xBC, 0x9B, 0xF5, 0x62};

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
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  memcpy(&message, data, sizeof(message));

  Serial.print("Message received from device: ");
  Serial.print(macStr);
  Serial.println();
  Serial.printf(message.power ? "Relay true" : "Relay false");
  Serial.println();
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  Serial.print("Sent to device: ");
  Serial.print(macStr);
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

  WiFi.mode(WIFI_AP);
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());

  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  initEspNow();

  HC12.begin(9600);               // Serial port to HC12
  Serial.println("Setup done");
}

void loop() {

  if (millis() - heartBeat > 30000) {
    Serial.println("Waiting for ESP-NOW messages...");
    lcd.setCursor(0, 0);
    lcd.print("Waiting for ESP-NOW messages...");
    lcd.setCursor(0, 1);
    lcd.print("                                 ");
    heartBeat = millis();
  }


  if (HC12.available() > 0) {             // If HC-12 has data
    lcd.clear();
    message.power = HC12.read();
    lcd.setCursor(0, 0);
    lcd.print("power ");
    lcd.print(message.power);

    sendMessage(mac);
  }
}

void sendMessage(uint8_t * mac_addr) {
  Serial.println("Sending message");
  lcd.setCursor(0, 1);
  lcd.print("Sending message");

  // Send message via ESP-NOW
  esp_now_send(mac_addr, (uint8_t *)&message, sizeof(message));

  delay(2000);
}

char* convertMac(const uint8_t * mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  return macStr;
}
