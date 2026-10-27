
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ================= WIFI =================
const char* ssid = "Redmi8";
const char* password = "43215678";

// ================= MQTT =================
const char* mqtt_server = "192.168.43.77 ";   // PC/Ubuntu 192.168.43.77 IP
const int mqtt_port = 1883;

const char* mqtt_topic = "cdac/sensor/data";

// ================= UART =================
// Change pins according to your ESP32 connection
#define RXD2 16
#define TXD2 17

HardwareSerial STM32Serial(2);

WiFiClient espClient;
PubSubClient mqttClient(espClient);


// =================================================
// WiFi Connection
// =================================================
void setup_wifi()
{
    Serial.print("Connecting to WiFi");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
}


// =================================================
// MQTT Reconnection
// =================================================
void reconnectMQTT()
{
    while (!mqttClient.connected())
    {
        Serial.print("Connecting to MQTT...");

        String clientID = "ESP32_Sensor_";
        clientID += String(random(0xffff), HEX);

        if (mqttClient.connect(clientID.c_str()))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.println(mqttClient.state());

            delay(2000);
        }
    }
}


// =================================================
// SETUP
// =================================================
void setup()
{
    Serial.begin(115200);

    // UART2 communication with STM32
    STM32Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);

    setup_wifi();

    mqttClient.setServer(mqtt_server, mqtt_port);

    Serial.println();
    Serial.println("================================");
    Serial.println("ESP32 MQTT Sensor Gateway Started");
    Serial.println("================================");

}


// =================================================
// LOOP
// =================================================
void loop()
{
    // Check MQTT connection
    if (!mqttClient.connected())
    {
        reconnectMQTT();
    }

    mqttClient.loop();


    // Check data coming from STM32
    if (STM32Serial.available())
    {
        String data = STM32Serial.readStringUntil('\n');

        data.trim();

        if (data.length() > 0)
        {
            Serial.print("STM32: ");
            Serial.println(data);

            // Publish STM32 data to MQTT
            mqttClient.publish(mqtt_topic, data.c_str());

            Serial.print("MQTT Published: ");
            Serial.println(data);
        }
    }
}
