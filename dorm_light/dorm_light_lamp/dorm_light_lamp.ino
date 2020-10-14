#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT//支持小爱同学
#include <Blinker.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "ArduinoJson-v6.15.2.h"

#define GPIO 0  //定义继电器输入引脚为D7

char auth[] = "6c8e330dd4a2";
 char ssid[] = "女寝专用";
char pswd[] = "208208nb";



const char* ssid1 = "女寝专用";
const char* password = "208208nb";
const char* mqtt_server = "39.96.68.13";
const int mqttPort = 1883;
const char* clientId = "4B208_lamp";
const char* topic = "dorm";
char* code = "";
bool lampStatue = false;
WiFiClient espClient;
PubSubClient client(espClient);


// 新建组件对象
BlinkerButton Button1("btn-abc");//注意：要和APP组件’数据键名’一致
 
// 按下BlinkerAPP按键即会执行该函数
void button1_callback(const String & state) {
 BLINKER_LOG("get button state: ", state);
 digitalWrite(GPIO, !digitalRead(GPIO));
 Blinker.vibrate();
}
 
//小爱电源类操作的回调函数:
//当小爱同学向设备发起控制, 设备端需要有对应控制处理函数 
void miotPowerState(const String & state)
{
 BLINKER_LOG("need set power state: ", state);
 if (state == BLINKER_CMD_ON) {
 digitalWrite(GPIO, LOW);
 BlinkerMIOT.powerState("on");
 BlinkerMIOT.print();
 }
 else if (state == BLINKER_CMD_OFF) {
 digitalWrite(GPIO, HIGH);
 BlinkerMIOT.powerState("off");
 BlinkerMIOT.print();
 }
}
 
void setup() {
 // 初始化串口，并开启调试信息，调试用可以删除
 Serial.begin(115200); 
 BLINKER_DEBUG.stream(Serial);
 // 初始化IO
 pinMode(GPIO, OUTPUT);
 digitalWrite(GPIO, HIGH);
 
 // 初始化blinker
 Blinker.begin(auth, ssid, pswd);
 Button1.attach(button1_callback);
 
 //小爱同学务必在回调函数中反馈该控制状态 
 BlinkerMIOT.attachPowerState(miotPowerState);//注册回调函数

 //连接wifi
  WiFi.begin(ssid1, password);
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

//  docode(a);

}
void docode(char json[2000])
{
  //  "解析指令"

  StaticJsonDocument<500> doc;
  deserializeJson(doc, json);
  const char* code = doc["code"];
  Serial.println();
  Serial.println(code);
  String tempcode;
  tempcode = String(code);
  if (tempcode == "openlamp")
   {
     digitalWrite(relay, LOW);
     lampStatue = true;
  //发送连接成功消息
 send_light_data();
     }
     else if (tempcode == "closelamp")
     {
       digitalWrite(relay, HIGH);
      lampStatue = false;
       send_light_data()}


}
void send_light_data(){
  delay(10);
char msg[500];
  StaticJsonDocument<200> light_data;
  light_data["sensor"] = "servo";
  light_data["left"] = leftStatue;
  light_data["right"] = rightStatue;
  serializeJson(light_data, msg);
  client.publish(topic, msg );
  delay(100);
  }
  void loop() {
 Blinker.run();
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
