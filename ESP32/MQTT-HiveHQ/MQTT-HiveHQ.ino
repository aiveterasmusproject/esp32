#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define RGB_PIN   38

//---- WiFi settings
#define SSID            "dlink"
#define WiFiPASS        ""
//---- MQTT Broker settings
// replace with your broker url
#define  MQTT_SERVER    "*********.s1.eu.hivemq.cloud"
#define  MQTT_USERNAME  "*****"
#define  MQTT_PASS      "*****"
#define  MQTT_PORT      8883

WiFiClientSecure espClient;
PubSubClient client(espClient);
String clientId="ESP32Client-"+String(random(0xffff), HEX);  // Create a random client ID 

unsigned long lastMsg = 0;
unsigned long lastMillis = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

const char* subscribe_topic = "/control/led";
const char* publish_topic = "/control/mqtt";

// This certificate is needed if you want a secure connection to the
// server, In owr case, the connection is set as "Insecure"
// static const char* root_ca PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIEkjCCA3qgAwIBAgITBn+USionzfP6wq4rAfkI7rnExjANBgkqhkiG9w0BAQsF
// ADCBmDELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNj
// b3R0c2RhbGUxJTAjBgNVBAoTHFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4x
// OzA5BgNVBAMTMlN0YXJmaWVsZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1
// dGhvcml0eSAtIEcyMB4XDTE1MDUyNTEyMDAwMFoXDTM3MTIzMTAxMDAwMFowOTEL
// MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
// b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
// ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
// 9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
// IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
// VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
// 93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
// jgSubJrIqg0CAwEAAaOCATEwggEtMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/
// BAQDAgGGMB0GA1UdDgQWBBSEGMyFNOy8DJSULghZnMeyEE4KCDAfBgNVHSMEGDAW
// gBScXwDfqgHXMCs4iKK4bUqc8hGRgzB4BggrBgEFBQcBAQRsMGowLgYIKwYBBQUH
// MAGGImh0dHA6Ly9vY3NwLnJvb3RnMi5hbWF6b250cnVzdC5jb20wOAYIKwYBBQUH
// MAKGLGh0dHA6Ly9jcnQucm9vdGcyLmFtYXpvbnRydXN0LmNvbS9yb290ZzIuY2Vy
// MD0GA1UdHwQ2MDQwMqAwoC6GLGh0dHA6Ly9jcmwucm9vdGcyLmFtYXpvbnRydXN0
// LmNvbS9yb290ZzIuY3JsMBEGA1UdIAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQsF
// AAOCAQEAYjdCXLwQtT6LLOkMm2xF4gcAevnFWAu5CIw+7bMlPLVvUOTNNWqnkzSW
// MiGpSESrnO09tKpzbeR/FoCJbM8oAxiDR3mjEH4wW6w7sGDgd9QIpuEdfF7Au/ma
// eyKdpwAJfqxGF4PcnCZXmTA5YpaP7dreqsXMGz7KQ2hsVxa81Q4gLv7/wmpdLqBK
// bRRYh5TmOTFffHPLkIhqhBGWJ6bt2YFGpn6jcgAKUj6DiAdjd4lpFw85hdKrCEVN
// 0FE6/V1dN2RMfjCyVSRCnTawXZwXgWHxyvkQAiSr6w10kY17RSlQOYiypok1JR4U
// akcjMS9cmvqtmg5iUaQqqcT5NJ0hGA==
// -----END CERTIFICATE-----
// )EOF";


void setup() {

  Serial.begin(115200);
  while (!Serial)
    delay(1);

  Serial.print("\nConnecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WiFiPASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

  // This is the hiveMQ CA certificate to secure connect to the server
  // Uncomment is using the CA certificate
  // espClient.setCACert(root_ca);
  
  // Allows insecure connection (not recomended for production)
  // Comment if using CA certificate
  espClient.setInsecure();  
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(subscribeCallback);
}

void loop() {
  unsigned long millisNow = millis();

  if (!client.connected())
    reconnect();
  client.loop();

  // Every 10 seconds send a message to the /control/mqtt topic
  if (millisNow - lastMillis > 10000) {
    publishMessage(publish_topic, String(lastMsg), true);
    lastMsg += 1;
    lastMillis = millisNow;
  }
}

//=======================================================================Function=================================================================================

void reconnect() {
  // Loop until we’re reconnected
  Serial.println("Connecting");
  Serial.println(String("My ClientID is ")+clientId);

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
  
    // Attempt to connect
    int conn=client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASS);
    Serial.print(" After connect...");
    if (conn) {
      Serial.println("Connected");
      client.subscribe(subscribe_topic);  // subscribe the topics here
      //client.subscribe("****");   // subscribe more topics here
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");  // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//=======================================
// This void is called every time we have a message from the broker

void subscribeCallback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];
  
  Serial.println("Message arrived [" + String(topic) + "] " + incommingMessage);
  if (String(topic) == String(subscribe_topic)) {
    if (incommingMessage == "red")
      rgbLedWrite(RGB_PIN, 0, RGB_BRIGHTNESS, 0); // Red (GRB)
    else if (incommingMessage == "green")
      rgbLedWrite(RGB_PIN, RGB_BRIGHTNESS, 0, 0); // Green
    else if (incommingMessage == "blue")
      rgbLedWrite(RGB_PIN, 0, 0, RGB_BRIGHTNESS); // Blue
    else if (incommingMessage == "white")
      rgbLedWrite(RGB_PIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS); // White
    else if (incommingMessage == "off")
      rgbLedWrite(RGB_PIN, 0, 0, 0); // Off
  }
  // check for other commands
  /* else if( strcmp(topic,command2_topic) == 0){
      if (incommingMessage.equals(“1”)) { } // do something else
    }*/
}

//==== publising as string
void publishMessage(const char* topic, String payload, boolean retained) {
  if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message publised [" + String(topic) + "]: " + payload);
}