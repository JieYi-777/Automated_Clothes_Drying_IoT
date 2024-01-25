// Include the libraries
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include "DHT.h"

// Replace these values with your own
const char *WIFI_SSID = "BaskinRobbins";    // Change to your WiFi SSID
const char *WIFI_PASSWORD = "What7YouSay?"; // Change to your password
const char *MQTT_SERVER = "34.121.176.166"; // Chgange to your VM instance public (external) IP address
const int MQTT_PORT = 1883;                 // It needs to be same with MQTT port number allowed by your firewall configuration
const char *MQTT_TOPIC = "iot";         // Change to your MQTT topic

const int DHT_PIN = A4;          // Change to your humidity sensor's input pin
const int rainPin = 42;          // Change to your rain sensor's input pin
const int relayPin = 39;         // Change to your relay output pin
const int servoPin = 21;         // Change to your servo motor output pin

const int DHT_TYPE = DHT11;      // Your humidity sensor's type
DHT dht(DHT_PIN, DHT_TYPE);      // Initialize humidity sensor
Servo myservo;                   // Initialize servo motor

WiFiClient espClient;
PubSubClient client(espClient);

// Function to set up WiFi connection
void setup_wifi()
{
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Set up inputs, outputs, wifi and MQTT client
void setup()
{
  Serial.begin(115200);

  // Input initialization
  dht.begin();
  pinMode(rainPin, INPUT);

  // Output initialization
  pinMode(relayPin, OUTPUT);
  myservo.attach(servoPin);
  myservo.write(0);

  // Connect wifi and MQTT server
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

// Function to handle MQTT reconnection
void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("Connected to MQTT server");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Main loop for sensor readings and MQTT communication
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();
  delay(1000); // adjust the delay according to your requirements

  // Get sensors reading
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int rainValue = !digitalRead(rainPin);

  // Control relay (with motor/fan) and servo based on sensor readings
  if (rainValue == 1 && humidity > 60 && temperature > 25) {
    // It is raining, humidity is greater than 60%, and temperature is greater than 25°C

    // Activate the relay
    digitalWrite(relayPin, HIGH);

    // Rotate the servo to max position (180 degrees)
    myservo.write(180);
  }
  else{
    // Stop the relay
    digitalWrite(relayPin, LOW);

    // Rotate the servo to original position (0 degrees)
    myservo.write(0);
  }

  // Send data to MQTT broker under the topic
  char payload[40]; // Adjusted payload size
  sprintf(payload, "{\"rain\":%d, \"humidity\":%.2f, \"temperature\":%.2f}", rainValue, humidity, temperature);
  client.publish(MQTT_TOPIC, payload);
}