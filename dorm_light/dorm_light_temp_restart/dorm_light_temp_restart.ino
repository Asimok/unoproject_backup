
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "ArduinoJson-v6.15.2.h"
#define relay 0    //定义继电器针脚号0号引脚

long lastMsg = 0;
char msg[500];
int value = 0;

const char* ssid = "女寝专用";
const char* password = "208208nb";
const char* mqtt_server = "39.96.68.13";
const int mqttPort = 1883;
const char* clientId = "4B208_restart";
const char* topic = "dorm";
char* code = "";
WiFiClient espClient;
PubSubClient client(espClient);



void setup() {
   Serial.begin(9600);
  //连接wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //连接MQTT服务器
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientId)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  
  char buff[50];
  memset(buff, 0, sizeof(buff));
  const char *buff2 = " 上线";
  strcpy(buff, clientId);
  strcat(buff, buff2);
  //发送连接成功消息
  client.publish(topic, buff );
  //订阅主题
  client.subscribe(topic);

}

void loop() {
  //重连机制
  if (!client.connected()) {
    reconnect_mqtt();
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    reconnect_wifi();
  }
  client.loop();

}
/**
   断开重连
*/
void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("reConnecting to MQTT...");

    if (client.connect(clientId)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
void reconnect_wifi()
{
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("reConnected to the WiFi network");
}

void callback(char* topic, byte* payload, unsigned int length) {
  //收到消息
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  char a[2000] = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    a[i] = (char)payload[i];
  }

  docode(a);

}
void docode(char json[2000])
{
  //  "解析指令"

  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);
  const char* code = doc["code"];
  Serial.println();
  Serial.println(code);
  String tempcode;
  tempcode = String(code);
  if (tempcode == "restart")
   {
     digitalWrite(relay, HIGH);
     delay(1000);
      digitalWrite(relay, LOW);
      char buff[50];
  memset(buff, 0, sizeof(buff));
  const char *buff2 = " 正在重启";
  strcpy(buff, clientId);
  strcat(buff, buff2);
  //发送连接成功消息
  client.publish(topic, buff );
     }


}
