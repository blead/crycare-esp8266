#include <MicroGear.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

// PINS
#define D0 16 
#define D6 12
#define D7 13
#define D8 15

// LEDS
#define LED_PIN D0
#define OUT_LIGHT_PIN D6

// USART
#define RX_PIN D7
#define TX_PIN D8

// WIFI
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// NETPIE
#define APPID ""
#define KEY ""
#define SECRET ""
#define SELF_ALIAS "NodeMCU1"
#define TARGET_ALIAS "Server"
#define BUFFER_SIZE 512

// SERVER
#define URL "http://192.168.1.1/api/"

WiFiClient client;
MicroGear microgear(client);
SoftwareSerial uart(RX_PIN, TX_PIN);
char data_buffer[BUFFER_SIZE*3];
int data_count = 0;

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char msg_data[msglen + 1];
  for (int i = 0; i < msglen; i++) {
    msg_data[i] = (char)msg[i];
  }
  msg_data[msglen] = 0;
  if(strcmp(msg_data, "startCrying") == 0) {
    digitalWrite(OUT_LIGHT_PIN, HIGH);
  } else if(strcmp(msg_data, "stopCrying") == 0) {
    digitalWrite(OUT_LIGHT_PIN, LOW);
  }
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE");
}

void connectWiFi() {
  Serial.println("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendData(int data_length) {
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST((uint8_t*)data_buffer, data_length);
  if(httpCode <= 0) {
    Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  pinMode(OUT_LIGHT_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Starting");
  uart.begin(115200);

  connectWiFi();

  microgear.init(KEY, SECRET, SELF_ALIAS);
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(CONNECTED, onConnected);
  microgear.connect(APPID);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(OUT_LIGHT_PIN, LOW);
}

void loop() {
  if(uart.available()) {
    char input_char = uart.read();
    if(input_char <= 80 && input_char >= 0) {
      if(data_count > 0) {
        sprintf(data_buffer, "%s %d", data_buffer, input_char);
      } else {
        sprintf(data_buffer, "%d", input_char);
      }
      data_count++;
      if(data_count == BUFFER_SIZE) {
        int data_buffer_length = strlen(data_buffer);
        Serial.println(data_buffer_length);
        sendData(data_buffer_length);
        data_count = 0;
      }
    }
  }
  if(WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if(microgear.connected()) {
    microgear.loop();
  } else {
    Serial.println("Connection lost, reconnecting");
    microgear.connect(APPID);
  }
}



