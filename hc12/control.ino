#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "";
const char* password = "";

SoftwareSerial HC12(D4, D3, false); // HC-12 TX Pin, HC-12 RX Pin

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html {
        font-family: Arial;
        display: inline-block;
        text-align: center;
      }

      h2 {
        font-size: 3.0rem;
      }

      p {
        font-size: 3.0rem;
      }

      body {
        max-width: 600px;
        margin: 0px auto;
        padding-bottom: 25px;
      }

      .switch {
        position: relative;
        display: inline-block;
        width: 120px;
        height: 68px
      }

      .switch input {
        display: none
      }

      .slider {
        position: absolute;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        border-radius: 34px
      }

      .slider:before {
        position: absolute;
        content: "";
        height: 52px;
        width: 52px;
        left: 8px;
        bottom: 8px;
        background-color: #fff;
        -webkit-transition: .4s;
        transition: .4s;
        border-radius: 68px
      }

      input:checked+.slider {
        background-color: #2196F3
      }

      input:checked+.slider:before {
        -webkit-transform: translateX(52px);
        -ms-transform: translateX(52px);
        transform: translateX(52px)
      }

    </style>
  </head>

  <body>
    <h2>ESP Web Server</h2>
    <h4>Prender Bomba de Agua</h4>
    <label class="switch">
      <input type="checkbox" onchange="toggleCheckbox(this)" id="waterpump">
      <span class="slider"></span>
    </label>
    <script>
      function toggleCheckbox(element) {
        var xhr = new XMLHttpRequest();
        if (element.checked) {
          xhr.open("GET", "/update?relay=" + element.id + "&state=1", true);
        } else {
          xhr.open("GET", "/update?relay=" + element.id + "&state=0", true);
        }
        xhr.send();
      }

    </script>
  </body>

</html>
)rawliteral";

void wifiConnect() {
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting to "); Serial.print(ssid);
  
while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  } 
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 
  lcd.setCursor(0, 0);
  lcd.print(WiFi.localIP());
}

void initServer(){
  
Serial.println("initServer...");
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("request init...");
    request->send_P(200, "text/html", index_html, NULL);
  });
  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam("relay") & request->hasParam("state")) {
      inputMessage = request->getParam("relay")->value();
      inputParam = "relay";
      inputMessage2 = request->getParam("state")->value();
      inputParam2 = "state";
      
      int power = inputMessage2.toInt();
      sendMessage(power);
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();
  
  }

void setup() {
  Serial.begin(115200);
  Serial.println("Init...");  
  lcd.begin(16, 2);

  // Inicializar el LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  
  wifiConnect();
  initServer();
  
  HC12.begin(9600);               // Serial port to HC12
  Serial.println("Setup done");
}
void loop() {  
}

void sendMessage(int power)
{
  Serial.println("Sending message");
  Serial.println("HC12");
Serial.println(HC12.available());

  if (HC12.available() >0) {
    lcd.setCursor(0, 1);
    lcd.print("power ");
    lcd.print(power);
    HC12.write(power);
  }
}
