#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library
#include <PubSubClient.h> // MQTT library

// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
CCS811 ccs811(D3); // nWAKE on D3

float val1, val2;

const char* ssid = "debugging";  // Enter SSID here
const char* password = "";  // Enter Password here
const char* mqtt_server = "192.168.211.155";  // Replace with your MQTT broker IP
const int mqtt_port = 1883;  // Default MQTT port
const char* mqtt_user = "";  // Replace with your MQTT username (if required)
const char* mqtt_password = "";  // Replace with your MQTT password (if required)

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("setup: Starting CCS811 basic demo");
  Serial.print("setup: ccs811 lib  version: ");
  Serial.println(CCS811_VERSION);

  Wire.begin();
  ccs811.set_i2cdelay(50);
  bool ok = ccs811.begin();
  if (!ok) Serial.println("setup: CCS811 begin FAILED");

  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(), HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(), HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(), HEX);

  ok = ccs811.start(CCS811_MODE_1SEC);
  if (!ok) Serial.println("setup: CCS811 start FAILED");

  // WiFi setup
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  // MQTT setup
  client.setServer(mqtt_server, mqtt_port);
  
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw);

  if (errstat == CCS811_ERRSTAT_OK) {
    val1 = eco2;
    val2 = etvoc;

    Serial.print("CCS811: ");
    Serial.print("eco2="); Serial.print(val1); Serial.print(" ppm  ");
    Serial.print("etvoc="); Serial.print(val2); Serial.print(" ppb  ");
    Serial.println();

    // Publish to MQTT
    char eco2Str[8];
    char etvocStr[8];
    dtostrf(val1, 6, 2, eco2Str);
    dtostrf(val2, 6, 2, etvocStr);
    
    client.publish("school/co2", eco2Str);
    client.publish("school/tvoc", etvocStr);
  }
  else if (errstat == CCS811_ERRSTAT_OK_NODATA) {
    Serial.println("CCS811: waiting for (new) data");
  }
  else if (errstat & CCS811_ERRSTAT_I2CFAIL) {
    Serial.println("CCS811: I2C error");
  }
  else {
    Serial.print("CCS811: errstat="); Serial.print(errstat, HEX);
    Serial.print("="); Serial.println(ccs811.errstat_str(errstat));
  }

  delay(1000);
  server.handleClient();
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(val1, val2));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float val1, float val2) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Measured Air Quality</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "<script>\n";
  ptr += "setInterval(loadDoc,1000);\n";
  ptr += "function loadDoc() {\n";
  ptr += "var xhttp = new XMLHttpRequest();\n";
  ptr += "xhttp.onreadystatechange = function() {\n";
  ptr += "if (this.readyState == 4 && this.status == 200) {\n";
  ptr += "document.body.innerHTML =this.responseText}\n";
  ptr += "};\n";
  ptr += "xhttp.open(\"GET\", \"/\", true);\n";
  ptr += "xhttp.send();\n";
  ptr += "}\n";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>Measured Air Quality</h1>\n";
  ptr += "<p>CO2: "; ptr += val1; ptr += " ppm</p>";
  ptr += "<p>TVOC: "; ptr += val2; ptr += " ppb</p>";
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
