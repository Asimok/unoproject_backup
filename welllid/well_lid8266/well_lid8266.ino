#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson-v6.15.2.h"

int port0 = D0;//连接光线传感器
int value0 = 0;//测量的模拟数值
int port1 = D1;
int value1 = 0;
int port2 = D2;
int value2 = 0;

const char* ssid = "test";
const char* password = "12345678";
const char* mqtt_server = "39.96.68.13";
const int mqttPort = 1883;
const char* clientId = "well_lid";
const char* topic = "well_lid";
char* code = "close1";
WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
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
//  client.setCallback(callback);
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
  pinMode(port0, INPUT);
  pinMode(port1, INPUT);
    pinMode(port2, INPUT);

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
void loop()
{
 //重连机制
  if (!client.connected()) {
    reconnect_mqtt();
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    reconnect_wifi();
  }
  client.loop();
  delay(1000);//间隔1秒读取数据
  readvalue();
  // printvalue();
  delay(100);
  sendvalue();

}


void readvalue()
{
  value0 = digitalRead(port0);
  delay(10);
  value1 = digitalRead(port1);
  delay(10);
  value2 = digitalRead(port2);
  delay(10);
}
void sendvalue()
{
  //返回服务器格式 axxbxxcxxd

char msg[2000];
   StaticJsonDocument<200> light_data;
  light_data["w1"] = value0;
  light_data["w2"] = value1;
  light_data["w3"] = value2;
  serializeJson(light_data, msg);
  client.publish(topic, msg );

  
}
void printvalue()
{
  
  Serial.print("value:  ");
  Serial.print(value0);//打印出来便于调试
  Serial.print("  ");
  Serial.print(value1);
  Serial.print("  ");
  Serial.println(value2);
}


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
