#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// 📡 Sensor Pins
#define PIR_PIN 27   // PIR sensor on GPIO 27
#define SOUND_PIN 36 // Sound sensor on GPIO 36 (analog)
#define DHT_PIN 19   // DHT11 sensor on GPIO 19
#define DHT_TYPE DHT11

// 🔬 Sensor Initialization
BH1750 lightMeter;
DHT dht(DHT_PIN, DHT_TYPE);

// 🌐 WiFi Settings
const char *ssid = "Sensors_Makeathon";
const char *password = "Makeathon";

// MQTT Settings
const char *mqtt_server = "192.168.0.52";
const int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_password = "";
WiFiClient espClient;
PubSubClient client(espClient);

// ⏳ Timing Variables
unsigned long previousMillis = 0;
const long interval = 5000; // 5-second interval for sensor readings

// 🔄 Read DHT Temperature
float readDHTTemperature()
{
    float temperature = NAN;
    int attempts = 0;
    while (attempts < 5 && isnan(temperature))
    {
        temperature = dht.readTemperature();
        attempts++;
        delay(100);
    }
    return temperature;
}

// 🔄 Read DHT Humidity
float readDHTHumidity()
{
    float humidity = NAN;
    int attempts = 0;
    while (attempts < 5 && isnan(humidity))
    {
        humidity = dht.readHumidity();
        attempts++;
        delay(100);
    }
    return humidity;
}

// 📡 Connect to WiFi
void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" connected");
}

// 📡 Reconnect to MQTT Server
void reconnect()
{
    unsigned long startAttemptTime = millis(); // Start time of the connection attempt
    while (!client.connected())
    {
        if (millis() - startAttemptTime >= 10000)
        { // If 10 seconds have passed
            Serial.println("Failed to connect to MQTT server within 10 seconds.");
            return; // Exit the reconnect function after 10 seconds
        }

        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client"))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000); // Wait before trying again
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin();
    if (!lightMeter.begin())
    {
        Serial.println("Error initializing BH1750 light sensor.");
        while (1)
            ;
    }

    dht.begin(); // Start DHT

    pinMode(PIR_PIN, INPUT);
    pinMode(SOUND_PIN, INPUT);

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port); // Default MQTT port is 1883
    Serial.println("Sensors ready!");
}

void loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        // 📡 Measure light with BH1750
        uint16_t lux = lightMeter.readLightLevel();
        String lightValue = String(lux);
        Serial.print("Light (lux): ");
        Serial.println(lux);
        client.publish("room1/light", lightValue.c_str());

        // 🌡 Measure temperature
        float temperature = readDHTTemperature();
        if (!isnan(temperature))
        {
            String tempValue = String(temperature);
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
            client.publish("room1/temperature", tempValue.c_str());
        }
        else
        {
            Serial.println("Error reading DHT11 (temperature).");
        }

        // 💧 Measure humidity
        float humidity = readDHTHumidity();
        if (!isnan(humidity))
        {
            String humidityValue = String(humidity);
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.println(" %");
            client.publish("room2/humidity", humidityValue.c_str());
        }
        else
        {
            Serial.println("Error reading DHT11 (humidity).");
        }

        // 🚶‍♂️ Measure motion with PIR
        int pirValue = digitalRead(PIR_PIN);
        String motion = (pirValue == HIGH) ? "1" : "0";
        Serial.println(pirValue == HIGH ? "Motion detected!" : "No motion.");
        client.publish("room2/motion", motion.c_str());

        // 🔊 Measure noise level with KY-037
        int soundLevel = analogRead(SOUND_PIN);
        Serial.print("Noise level: ");
        Serial.println(soundLevel);
        Serial.println("-------------------------");
        client.publish("room2/noise", String(soundLevel).c_str());

        // 🚨 High noise threshold
        if (soundLevel > 800)
        {
            Serial.println("High noise detected!");
        }
    }

    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
}