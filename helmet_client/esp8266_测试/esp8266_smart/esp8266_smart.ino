#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "ArduinoJson-v6.15.2.h"

SoftwareSerial mySerial(D7, D8); // RX, TX
long lastMsg = 0;
char msg[500];
int value = 0;

//const char* ssid = "CMCC-Asimov";
//const char* password = "m913919m";
const char* mqtt_server = "39.96.68.13";
const int mqttPort = 1883;
const char* clientId = "HELMET_1";
const char* topic = "helmet_topic";
WiFiClient espClient;
PubSubClient client(espClient);
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);                       // wait for a second
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}
void setup() {

  Serial.begin(115200);
  //连接wifi
 delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  smartConfig();  //微信智能配网
  delay(500);
  Serial.println("");
  Serial.println("WiFi connected");
  
  
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.println("Connecting to WiFi..");
//  }
//  Serial.println("Connected to the WiFi network");
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

  char buff[1024];
  memset(buff, 0, sizeof(buff));
  const char *buff2 = " 上线";
  strcpy(buff, clientId);
  strcat(buff, buff2);
  //发送连接成功消息
  client.publish(topic, buff );
  //订阅主题
  client.subscribe(topic);

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
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

}

void loop() {
  //重连机制
  if (!client.connected()) {
    reconnect_mqtt();
  }
//  if (WiFi.status() != WL_CONNECTED)
//  {
//    reconnect_wifi();
//  }

    //读取串口数据
    char a[500] = ""; //定义字符数组，接受来自上位机的数据
    int i = 0;
    if(Serial.available()){
//    while (!Serial.available()); //等待数据传送过来，若没有数据，一直等待，即执行到本行不向下执行
    while (Serial.available()) //当发现缓存中有数据时，将数据送至字符数组a中
    {
      a[i] = Serial.read();
      i++;
      delay(3);
    }
    Serial.println(a);
    //转发数据到服务器
    client.publish(topic, a);
    
  }
  client.loop();
}
