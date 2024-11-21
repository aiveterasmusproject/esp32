#include <WiFi.h>
#include <PubSubClient.h>

#define RGB_PIN   38

//define the constants of the code
const char* ssid = "dlink";
const char* password = "";
const char* mqtt_server = "192.168.1.**";  //mqtt server IP address
String mqtt_topic = "/led/control";    // feel free to change this

//create the MQTT client (the connection details are below)
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // initiates the serial communication
  Serial.begin(115200);
  // initiates the WiFi using station mode (normal WiFi node), and waits until
  // the connection is successfull
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

  // connects to the MQTT server (without specifying any topic), and sets the
  // callback function that will handle the messages received from MQTT broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp="";
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  // The message is received in byte* and this for will convert the message 
  // to a String(char by char until length) 
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // If the topic is correct, the message is interpreted, and the LED turns
  // on or off. The validation of topic is important if there are multiple
  // topic subscriptions
  if (String(topic) == mqtt_topic) {
    if (messageTemp == "red")
      rgbLedWrite(RGB_PIN, 0, RGB_BRIGHTNESS, 0); // Red (GRB)
    else if (messageTemp == "green")
      rgbLedWrite(RGB_PIN, RGB_BRIGHTNESS, 0, 0); // Green
    else if (messageTemp == "blue")
      rgbLedWrite(RGB_PIN, 0, 0, RGB_BRIGHTNESS); // Blue
    else if (messageTemp == "white")
      rgbLedWrite(RGB_PIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS); // White
    else if (messageTemp == "off")
      rgbLedWrite(RGB_PIN, 0, 0, 0); // Off
  }
}

//this function will be used to connect to MQTT broker
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // the client name (in this case ‘ESP8266Client_sub’ must be unique
    if (client.connect(String("ESP32Client-")+String(random(0xffff), HEX))) {
      Serial.println("connected");
      // to subscribe to multiple topics, just repeat the following line
      client.subscribe(mqtt_topic.c_str());
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    Serial.println("Disconnected :)...");
    connectMQTT();
  }
  //keep the client connected
  client.loop();
  delay(5);
}