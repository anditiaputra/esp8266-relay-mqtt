#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define relay1Pin 4    // GPIO4 (D2)
#define relay2Pin 5    // GPIO5 (D1)

bool ledState = false;

// WiFi settings
const char *ssid = "ssidwifi";                   // WiFi name
const char *password = "passwordwifi";           // WiFi password

// MQTT Broker settings
const char *mqtt_broker = "abc.emqxsl.com";      // MQTT broker
const char *mqtt_topic = "abc/topic";            // MQTT topic
const char *mqtt_username = "username";          // MQTT username for authentication
const char *mqtt_password = "password";          // MQTT password for authentication
const int mqtt_port = 8883;                      // MQTT port (TCP)

WiFiClientSecure espClient;
PubSubClient mqtt_client(espClient);

void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    pinMode(relay1Pin, OUTPUT);
    pinMode(relay2Pin, OUTPUT);

    connectToWiFi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    espClient.setInsecure();
    connectToMQTTBroker();
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "esp8266-PCI-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic);
            // Publish message upon successful connection
            mqtt_client.publish(mqtt_topic, "Hi EMQX i'm EPS8266 Gerbang PCI ^^");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Incoming: ");
    Serial.print(topic);
    Serial.print(" - ");

    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char) payload[i];
    }
    Serial.println(message);

    if (message == "bukagerbang") {
        digitalWrite(relay1Pin, HIGH); // Turn on relay 1 (buka gerbang)
        Serial.println("Turning on relay 1 (buka)");
        delay(2000);
        digitalWrite(relay1Pin, LOW);
    } else if (message == "tutupgerbang") {
        digitalWrite(relay2Pin, HIGH); // Turn on relay 2 (tutup gerbang)
        Serial.println("Turning on relay 2 (tutup)");
        delay(2000);
        digitalWrite(relay2Pin, LOW);
    } else {
        Serial.println("Unknown command: " + message);
    }
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();
}
