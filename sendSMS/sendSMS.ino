// Import required libraries for GPRS
#include <Arduino.h>
#include <Wire.h>
#include <sim800.h>
#include <gprs.h>

// Import required libraries for WebServer
#include <WiFi.h>

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// Configutation of SIM800L
#define T_CALL

#if defined(T_CALL)
#define UART_TX 27
#define UART_RX 26
#define SIMCARD_RST 5
#define SIMCARD_PWKEY 4
#define SIM800_POWER_ON 23
#else
#define UART_TX 33
#define UART_RX 34
#define SIMCARD_RST 14
#define SIMCARD_PWKEY 15
#define SIM800_POWER_ON 4
#endif

#define UART_BANUD_RATE 9600

#define I2C_SDA 21
#define I2C_SCL 22

#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00

// Replace with your network credentials
const char *ssid = "...SSID...";
const char *password = "...PASSWORD...";

// I2C for SIM800
HardwareSerial hwSerial(1);
GPRS gprs(hwSerial, SIMCARD_PWKEY, SIMCARD_RST, SIM800_POWER_ON);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Pertenece al SIM800
bool setPowerBoostKeepOn(int en)
{
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en)
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  else
    Wire.write(0x35); // 0x37 is default reg value
  return Wire.endTransmission() == 0;
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

#if defined(T_CALL)
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  String info = "IP5306 KeepOn " + String((isOk ? "PASS" : "FAIL"));
  Serial.println(info);
#endif

  hwSerial.begin(UART_BANUD_RATE, SERIAL_8N1, UART_RX, UART_TX);
  Serial.println("SIM800 Setup Complete!");

  if (gprs.preInit())
  {
    Serial.println("SIM800 Begin PASS");
  }
  else
  {
    Serial.println("SIM800 Begin FAIL");
  }

  Serial.println("Test motor ...");
  int i = 3;
  while (i--)
  {
    hwSerial.print("AT+SPWM=0,1000,1\r\n");
    delay(2000);
    hwSerial.print("AT+SPWM=0,0,0\r\n");
  }

  delay(200);

  if (0 != gprs.init())
  {
    Serial.println("Not checked out to SIM card...");
    while (1)
      ;
  }

  // Switch audio channels
  hwSerial.print("AT+CHFA=1\r\n");
  delay(2);
  hwSerial.print("AT+CRSL=100\r\n");
  delay(2);
  hwSerial.print("AT+CLVL=100\r\n");
  delay(2);
  hwSerial.print("AT+CLIP=1\r\n");
  delay(2);
  Serial.println("SIM800L Init success");

  // Initialize Web Server
  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/index.html");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.on(
      "/api/message", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        //Handle body
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, (const char *)data);
        if (error)
          return;
        String telephone = doc["telephone"];
        String message = doc["message"];

        delay(200);
        hwSerial.println("AT+CMGF=1\r\n"); // Configuring TEXT mode
        delay(200);
        hwSerial.println("AT+CMGS=\"" + telephone + "\"\r\n"); //change ZZ with country code and xxxxxxxxxxx with phone number to sms
        delay(200);
        hwSerial.println(message + "\r\n"); //text content
        delay(200);
        hwSerial.write(26);
        delay(200);
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
      });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  // Start server
  server.begin();
}

void loop()
{
}
