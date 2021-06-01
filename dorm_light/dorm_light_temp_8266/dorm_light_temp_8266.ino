/*
  智能开关nodemcu代码
  D0 舵机左
  D1 右边
  D2 dht11

*/
/*arduino安装自带*/
#include<Servo.h>
#include <SoftwareSerial.h>
/*自行安装*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
/*引入本地头文件*/
#include "dht11.h"
#include "ArduinoJson-v6.15.2.h"
#define dht11Pin D2   //定义温湿度针脚号为D2号引脚
/*实例化对象*/
dht11 dht;
Servo left_servo, right_servo;

/**MQTT服务器参数配置*/
const char* wifiSSID = "女寝专用";
const char* password = "208208nb";
const char* mqttServer = "39.96.68.13";
const int mqttPort = 1883;
const char* clientId = "4B2081";
const char* topic = "dorm";

WiFiClient espClient;
PubSubClient client(espClient);

/*开关状态*/
bool leftStatue = false;
bool rightStatue = false;

void setup() {
  delay(1000);
  switchlight("closeLeft");
  delay(500);
  switchlight("closeRight");
  delay(1000);

  Serial.begin(9600);
  /*连接wifi*/
  WiFi.begin(wifiSSID, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  /*连接MQTT服务器*/
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientId)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  pinMode(dht11Pin, OUTPUT);
  
  char buff[50];
  memset(buff, 0, sizeof(buff));
  strcpy(buff, clientId);
  const char *buff2 = " 上线";
  strcat(buff, buff2);
  
  //发送连接成功消息
  client.publish(topic, buff );
  //订阅主题
  client.subscribe(topic);
  delay(100);
  //  初始化开关状态
  switchlight("closeLeft");
  delay(1000);
  switchlight("closeRight");
}
void get_dht11()
{

  char msg[500];
  int tol = dht.read(dht11Pin);    //将读取到的值赋给tol
  int temp = (float)dht.temperature; //将温度值赋值给temp
  int humi = (float)dht.humidity; //将湿度值赋给humi
  
  delay(10);      //延时1秒
  StaticJsonDocument<200> temperature_data;
  temperature_data["sensor"] = "DHT11";
  temperature_data["temp"] = temp;
  temperature_data["humi"] = humi;
  serializeJson(temperature_data, msg);
  //Serial.println(msg);
  client.publish(topic, msg);
}
/**
   断开重连
*/
void reconnect_mqtt() {
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
  //  解析指令
  //  {"code":"openLeft"}
  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);
  const char* code = doc["code"];
  Serial.println();
  Serial.println(code);
  String tempcode;
  tempcode = String(code);
  if (tempcode == "get_dht11")
    get_dht11();
  else if (tempcode == "get_light_status")
    send_light_data();
  else
    switchlight(tempcode);
}

void switchlight(String tempcode)
{

  delay(50);
  if (tempcode == "openLeft")
  {
    left_servo.attach(D0);
    left_servo.write(82);
    delay(300);
    left_servo.write(45);
    delay(100);
    left_servo.detach();
    leftStatue = true;
    send_light_data();
  }
  else if (tempcode == "openRight")
  {
    right_servo.attach(D1);
    right_servo.write(8);
    delay(300);
    right_servo.write(65);
    delay(100);
    right_servo.detach();
    rightStatue = true;
    send_light_data();
  }
  else if (tempcode == "closeLeft")
  {
    left_servo.attach(D0);
    left_servo.write(2);
    delay(300);
    left_servo.write(45);
    delay(100);
    left_servo.detach();
    leftStatue = false;
    send_light_data();

  }
  else if (tempcode == "closeRight")
  {
    right_servo.attach(D1);
    right_servo.write(130);
    delay(300);
    right_servo.write(65);
    delay(100);
    right_servo.detach();
    rightStatue = false;
    send_light_data();
  }
}

void send_light_data() {
  delay(10);
  char msg[500];
  StaticJsonDocument<200> light_data;
  light_data["sensor"] = "servo";
  light_data["left"] = leftStatue;
  light_data["right"] = rightStatue;
  serializeJson(light_data, msg);
  // Serial.println(msg);
  
  client.publish(topic, msg );
  delay(100);
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
