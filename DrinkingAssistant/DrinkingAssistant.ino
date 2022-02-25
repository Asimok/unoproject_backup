#include "HX711.h"
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
int buttonPin_k4 = D2;     // 按键的管脚定义
int buttonPin_k3 = D3;
int buttonPin_k2 = D4;
int buttonPin_k1 = D5;
int recoder = D6;//语音模块 P-E
int max_times = 2;
int times = 0;

//wifi
//const char* ssid = "nova 7 5G";
//const char* password = "mzy12345";

const char* ssid = "Asimok";
const char* password = "123321123";

/*MQTT*/
const char* clientId = "jm_drinking_assistant";
const char* topic_y2m = "jm_y2m";
const char* topic_m2y = "jm_m2y";
/*wifi*/
int wifi_connect_try_count = 0;
int reset_count = 0;
bool WIFI_Status = true;

/*定时任务*/
int interval_updateTime  = 1000 * 60 * 30; // 30min
unsigned long sys_current_millis;
unsigned long last_updateTime_sys_current_millis = 0;
unsigned long stay_time = 0; //放置时间
/*压力传感器*/
float Weight = 0;

/*连接MQTT*/
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqttServer = "49.232.204.184";
const int mqttPort = 1883;
const String SOURCE = "app";


void my_timer(float weight) {
  sys_current_millis  = millis();
  stay_time = sys_current_millis - last_updateTime_sys_current_millis;
  if ( stay_time >= interval_updateTime)
  {
    times++;

    // 计时结束 响铃提示
    Serial.println("计时结束 响铃提示");
    digitalWrite(recoder, HIGH);
    delay(2000);
    digitalWrite(recoder, LOW);
    if (times > max_times)
    {
      times = 0;
      sys_current_millis  = millis();
      last_updateTime_sys_current_millis = sys_current_millis;
      Serial.println("计时清零-1");
    }
    delay(4000);
  }
  //杯子拿起  计时清零 <140g
  if (weight < 140) {
    sys_current_millis  = millis();
    last_updateTime_sys_current_millis = sys_current_millis;
  }
}

void key_button() {
  if (digitalRead(buttonPin_k4) == LOW) { // interval_updateTime =1h
    delay(10); //等待跳过按键抖动的不稳定过程
    if (digitalRead(buttonPin_k4) == LOW) // 若按键被按下
    {
      interval_updateTime = 1000 * 3600; //1h
      sys_current_millis  = millis();
      last_updateTime_sys_current_millis = sys_current_millis;
      Serial.println("打开k4 1h");
    }
  }

  if (digitalRead(buttonPin_k3) == LOW) { // interval_updateTime =1h30h
    delay(10); //等待跳过按键抖动的不稳定过程
    if (digitalRead(buttonPin_k3) == LOW) // 若按键被按下
    {
      interval_updateTime = 1000 * 3600 + 500 * 3600; //1h30m
      sys_current_millis  = millis();
      last_updateTime_sys_current_millis = sys_current_millis;
      Serial.println("打开k3 1h30m");
    }
  }

  if (digitalRead(buttonPin_k2) == LOW) { // interval_updateTime = 2h;//2h
    delay(10); //等待跳过按键抖动的不稳定过程
    if (digitalRead(buttonPin_k2) == LOW) // 若按键被按下
    {
      interval_updateTime = 2000 * 3600; //2h
      sys_current_millis  = millis();
      last_updateTime_sys_current_millis = sys_current_millis;
      Serial.println("打开k2 2h");
    }
  }

  if (digitalRead(buttonPin_k1) == LOW) { // 重新计时
    delay(10); //等待跳过按键抖动的不稳定过程
    if (digitalRead(buttonPin_k1) == LOW) // 若按键被按下
    {
      //重新计时
      sys_current_millis  = millis();
      last_updateTime_sys_current_millis = sys_current_millis;
      Serial.println("打开k1 清零");
    }
  }

}

float get_weight()
{
  Weight = Get_Weight();  //计算放在传感器上的重物重量
  float current_weight = float(Weight / 1);
//  Serial.print("weight: ");
//  Serial.print(current_weight, 3); //串口显示重量
//  Serial.print(" g\n");  //显示单位
  delay(50);        //延时100Ms
  return current_weight;
}

void connect_MQTT()
{
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqtt_callback);
  Serial.println("Connecting to MQTT...");
  while (!client.connected()) {
    Serial.print(".");
    delay(1000);
    if (client.connect(clientId)) {
      Serial.println("\nMQTT connected");
      //发送连接成功消息

       String data = "{\"Weight\":" + String(Weight) + ",\"stay_time\":" + String(stay_time) + ",\"interval_updateTime\":" + String(interval_updateTime) + "\}";
      char data_char[150];
      strcpy(data_char, data.c_str());
      client.publish(topic_m2y,  data_char );
      
      //订阅主题
      Serial.print("subscribe topic: ");
      Serial.println(topic_y2m);
      client.subscribe(topic_y2m);
      Serial.println("MQTT连接成功");
    } else {
      Serial.print("\nfailed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
/**
   断开重连
*/
void reconnect_MQTT() {
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
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //收到消息
  Serial.println("Received Message: ");
  char a[2000] = "";
  if (length <= 2000) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      a[i] = (char)payload[i];
    }
    //    Serial.println();
    //    Serial.println(a);
    docode(a);
  }
}
void docode(char json[2000])
{
  StaticJsonDocument<200> root;
  deserializeJson(root, json);

  //接受消息文字
  String source = root["source"];
  String msg_type = root["type"];
  if (source == SOURCE ) //app
  {

    //设置模式参数
    if (msg_type == "set")
    {
      //      {"source":"app","type":"set","device":"getmaopi"}

      //      String payload = root["payload"];
      //设置收到消息提醒次数
      String device = root["device"];
      //重量清零
      if (device == "getmaopi")
      {
        Get_Maopi();
        //              client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        //              Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
      }
      //时间清零
      if (device == "time_clean")
      {
        //      {"source":"app","type":"set","device":"time_clean"}
      sys_current_millis  = millis();
      last_updateTime_sys_current_millis = sys_current_millis;
      }
      if (device == "time_long")
      {
        //        {"source":"app","type":"set","device":"time_long","payload":2500030}
        unsigned long  payload = root["payload"];
        interval_updateTime = payload; //自定义时长
        sys_current_millis  = millis();
        last_updateTime_sys_current_millis = sys_current_millis;
        Serial.print("自定义时长");
      }

    }
    if (msg_type == "get_weight")
    {
      //      {"source":"app","type":"get_weight"}


      String data = "{\"Weight\":" + String(Weight) + ",\"stay_time\":" + String(stay_time) + ",\"interval_updateTime\":" + String(interval_updateTime) + "\}";
      char data_char[150];
      strcpy(data_char, data.c_str());
      client.publish(topic_m2y,  data_char );
      Serial.println("updating...");
      Serial.println(data);
    }

  }
}
void smartConfig()//配网函数
{
  WiFi.mode(WIFI_STA);//使用wifi的STA模式

  Serial.println("\nWait for Smartconfig...");//串口打印
  WiFi.beginSmartConfig();//等待手机端发出的名称与密码
  //死循环，等待获取到wifi名称和密码
  while (1)
  {
    //等待过程中一秒打印一个.
    Serial.print(".");
    delay(1000);
    if (WiFi.smartConfigDone())//获取到之后退出等待
    {
      WiFi.setAutoConnect(true);
      Serial.println("SmartConfig Success");
      //打印获取到的wifi名称和密码
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      delay(1000);
      break;
    }
  }
}
void WIFI_Init()
{
  Serial.println("\n正在连接wifi...");

  //当设备没有联网的情况下，执行下面的操作
  while (WiFi.status() != WL_CONNECTED)
  {
    if (WIFI_Status) //WIFI_Status为真,尝试使用flash里面的信息去 连接路由器
    {
      Serial.print(".");
      delay(1000);
      wifi_connect_try_count++;
      if (wifi_connect_try_count >= 20)
      {
        WIFI_Status = false;
        Serial.println("\nWiFi连接失败，请用手机进行配网\n");
      }
    }
    else//使用flash中的信息去连接wifi失败，执行
    {
      smartConfig();  //smartConfig技术配网
    }
  }
  //串口打印连接成功的IP地址
  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  //联网成功 连接MQTT
  connect_MQTT();
}

void wifi_with_password()
{
  //连接wifi
  int connect_times = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && connect_times < 30) {
    connect_times++;
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  //连接MQTT服务器
  if (WiFi.status() == WL_CONNECTED)
  { connect_MQTT();
    Serial.println("Connected to the WiFi network");
  }
  else
    Serial.println("Connected to the WiFi fail!!!");
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

void setup() {
  Serial.begin(9600);

  pinMode(buttonPin_k4, INPUT_PULLUP); //设置按键管脚上拉输入模式
  pinMode(buttonPin_k3, INPUT_PULLUP);
  pinMode(buttonPin_k2, INPUT_PULLUP);
  pinMode(buttonPin_k1, INPUT_PULLUP);


  Init_Hx711();
  Get_Maopi();    //获取毛重
  //  WIFI_Init();//调用WIFI函数
  wifi_with_password();

  pinMode(recoder, OUTPUT);//喇叭
  digitalWrite(recoder, HIGH);
  delay(1500);
  digitalWrite(recoder, LOW);
  Serial.println("系统启动成功");

}
void loop() {
  //重连机制
  if (WiFi.status() == WL_CONNECTED)
    if (!client.connected()) {
      connect_MQTT();
    }

  client.loop();
  key_button();
  float weight = get_weight();
  my_timer(weight);

}
