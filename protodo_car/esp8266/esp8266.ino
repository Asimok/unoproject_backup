/*
   ESP8266
   使用 巴法云 公众号 智能配网 一键配网
   连接mqtt 收发指令

   引脚定义：
   DHT11 5v ：D7
   软串口：RX=D3,TX=D4
   RX=D5,TX=D6 GPS
   舵机 D8

*/
#include <Servo.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>
/*引入本地头文件*/
#include "dht11.h"
//软串口
SoftwareSerial mySerial(D3, D4); //RX=D3,TX=D4
SoftwareSerial mySerial2(D5, D6); //RX=D5,TX=D6 GPS

//wifi
//const char* ssid = "Redmi10X";
//const char* password = "qwebnm123";

const char* ssid = "nova 7 5G";
const char* password = "mzy12345";

/*指令*/
const char* front = "w";
const char* back = "s";
const char* left = "a";
const char* right = "d";
const char* stop_car = "q";
const char* speed1 = "z";
const char* speed2 = "x";
const char* speed3 = "c";
const char* beep1 = "1"; //蜂鸣器提示次数
const char* beep3 = "3";
//m/n led 开关

/*可设置参数*/
int MODE = 1; //1 普通 2 静音
String BARRIER_STATUS = "o"; //避障开关 默认开
String CAR_SPEED = String(speed1);
/*系统配置*/
#define band_rate 9600
bool isbeep = true;
/*MQTT*/
const char* clientId = "car_wifi";
const char* topic_y2m = "car_y2m";
const char* topic_m2y = "car_m2y";
/*wifi*/
int wifi_connect_try_count = 0;
int reset_count = 0;
bool WIFI_Status = true;
/*定时任务*/
const int interval_dht11  = 5000;//1s
const long gps_interval = 1000;       //GPS采集时间间隔
unsigned long sys_current_millis;

//DHT11
unsigned long last_updateDht11_sys_current_millis = 0;
/*传感器：实例化对象*/
//DHT11
#define dht11Pin D7   //定义温湿度针脚
dht11 dht;
int dht11_temp = 0;
int dht11_humi = 0;

//舵机
Servo myservo; //D8
#define servo D8
int open_servo = 120;
int close_servo = 2;
/*
  GPS+北斗传感器
*/
struct
{
  char GPS_Buffer[100];
  bool isGetData;   //是否获取到GPS数据
  bool isParseData; //是否解析完成
  char UTCTime[11];   //UTC时间
  char latitude[11];    //纬度
  char N_S[2];    //N/S
  char longitude[12];   //经度
  char E_W[2];    //E/W
  bool isUsefull;   //定位信息是否有效
  char altitude[11];    //海拔
} Save_Data;
const unsigned int gpsRxBufferLength = 700;
char gpsRxBuffer[gpsRxBufferLength];
unsigned int ii = 0;
unsigned long previousMillis_gps = 0; //毫秒时间记录

/*连接MQTT*/
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqttServer = "49.232.204.184";
const int mqttPort = 1883;
const String SOURCE = "app";

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
      char buff[50];
      memset(buff, 0, sizeof(buff));
      strcpy(buff, clientId);
      const char *buff2 = " 上线";
      strcat(buff, buff2);
      client.publish(topic_m2y, buff );
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
    Serial.println();
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
      String payload = root["payload"];
      //设置收到消息提醒次数
      String device = root["device"];

      //避障开关
      if (device == "barrier_status")
      {
        BARRIER_STATUS = String(payload);
        mySerial.println(BARRIER_STATUS);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
      }
      //舵机
      if (device == "servo_open")
      {

        for (int pos = 0; pos <= 10; pos += 1) {
          myservo.write(open_servo);
          delay(15);
        }
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        Serial.println("舵机打开");
      }
      if (device == "servo_close")
      {

        for (int pos = 10; pos >= 0; pos -= 1) {
          myservo.write(close_servo);
          delay(15);
        }
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        Serial.println("舵机关闭");
      }
//led
      if (device == "led_open")
      {

       mySerial.println('m');
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        Serial.println("led打开");
      }
      if (device == "led_close")
      {
mySerial.println('n');
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        Serial.println("led关闭");
      }
      
      if (device == "MODE")
      {
        //1 普通 2 静音
        MODE = payload.toInt();
        mySerial.println(beep1);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        switchMODE();
      }
      //修改wifi
      if (device == "changewifi")
      {
        mySerial.println(beep1);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
        smartConfig();
      }
      //连接状态
      if (device == "is_connected")
      {
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"is_connected\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"is_connected\"}");
      }
    }
    if (msg_type == "update")
    {
      String data = "{\"temp\":" + String(dht11_temp) + ",\"barrier_status\":\"" + BARRIER_STATUS +
                    "\",\"humi\":" + String(dht11_humi) + ",\"car_speed\":\"" + CAR_SPEED + "\",\"MODE\":" + String(MODE) + "\}";
      char data_char[150];
      strcpy(data_char, data.c_str());
      client.publish(topic_m2y,  data_char );
      Serial.println("updating...");
      Serial.println(data);
    }

    if (msg_type == "car")
    {
      String directions = root["directions"];
      if (directions == "front")
      {
        mySerial.println(front);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
      }
      else if (directions == "back")
      {
        mySerial.println(back);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
      }
      else if (directions == "left")
      {
        mySerial.println(left);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
      }
      else if (directions == "right")
      {
        mySerial.println(right);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
      }
      else if (directions == "stop")
      {
        mySerial.println(stop_car);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
      }
      else if (directions == "speed1")
      {
        mySerial.println(speed1);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
        CAR_SPEED = String(speed1);
      }
      else if (directions == "speed2")
      {
        mySerial.println(speed2);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
        CAR_SPEED = String(speed2);
      }
      else if (directions == "speed3")
      {
        mySerial.println(speed3);
        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
        CAR_SPEED = String(speed3);
      }

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
void get_dht11(int temp_sys_current_millis)
{
  if (temp_sys_current_millis - last_updateDht11_sys_current_millis >= interval_dht11)
  {
    last_updateDht11_sys_current_millis = temp_sys_current_millis;
    char msg[500];
    int tol = dht.read(dht11Pin);    //将读取到的值赋给tol
    dht11_temp = (float)dht.temperature; //将温度值赋值给temp
    dht11_humi = (float)dht.humidity; //将湿度值赋给humi
    //        Serial.print("DHT11-----温度: ");
    //        Serial.print(dht11_temp);
    //        Serial.print("C 湿度: ");
    //        Serial.print(dht11_humi);
    //        Serial.println("%");
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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //连接MQTT服务器
  connect_MQTT();

}
void SENSOR_Init()
{
  pinMode(dht11Pin, OUTPUT);//温度传感器引脚
}
void switchMODE()
{
  //   MODE  1 普通 2 静音

  if (MODE == 1)
  {
    isbeep = true;
  }
  if (MODE == 2)
  {
    isbeep = false;
  }

}


/*--------------------gps---------------------*/
void errorLog(int num)
{
  Serial.print("ERROR");
  Serial.println(num);

}

void printGpsBuffer()
{
  if (Save_Data.isParseData)
  {
    Save_Data.isParseData = false;

    if (Save_Data.isUsefull)
    {
      Save_Data.isUsefull = false;
      //发送数据
      StaticJsonDocument<200> gps_data;
      gps_data["sensor"] = "gps";
      gps_data["longitude"] = Save_Data.longitude;
      gps_data["latitude"] = Save_Data.latitude;
      gps_data["N_S"] = Save_Data.N_S;
      gps_data["E_W"] = Save_Data.E_W;
      gps_data["altitude"] = Save_Data.altitude;
      char gps_data_str[200];
      serializeJson(gps_data, gps_data_str);
      client.publish(topic_m2y, gps_data_str );
      Serial.println("GPS...");
      Serial.println(gps_data_str);
      //      Serial.println("重新获取GPS数据");
      delay(500);
    }
    else
    {
      Serial.println("GPS DATA is not usefull!");
    }
  }

}

void parseGpsBuffer()
{
  char *subString;
  char *subStringNext;
  if (Save_Data.isGetData)
  {
    Save_Data.isGetData = false;
    //     Serial.println("**************");
    //     Serial.println(Save_Data.GPS_Buffer);

    delay(100);

    for (int i = 0 ; i <= 9 ; i++)
    {
      if (i == 0)
      {
        if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
          errorLog(1);  //解析错误
      }
      else
      {
        subString++;
        if ((subStringNext = strstr(subString, ",")) != NULL)
        {
          char usefullBuffer[40];
          switch (i)
          {
            case 1: memcpy(Save_Data.UTCTime, subString, subStringNext - subString);; break; //获取UTC时间
            //            case 2:memcpy(usefullBuffer, subString, subStringNext - subString);Serial.println(subString);break; //获取UTC时间
            case 2: memcpy(Save_Data.latitude, subString, subStringNext - subString);; break; //获取纬度信息
            case 3: memcpy(Save_Data.N_S, subString, subStringNext - subString);; break; //获取N/S
            case 4: memcpy(Save_Data.longitude, subString, subStringNext - subString); break; //获取纬度信息
            case 5: memcpy(Save_Data.E_W, subString, subStringNext - subString); break; //获取E/W
            case 9: memcpy(Save_Data.altitude, subString, subStringNext - subString); break; //获取海拔

            default: break;
          }

          subString = subStringNext;
          Save_Data.isParseData = true;
          Save_Data.isUsefull = true;
        }
        else
        {
          errorLog(2);  //解析错误
        }
      }

      delay(200);
    }
  }
}

void gpsRead() {
  while (mySerial2.available())
  {
    gpsRxBuffer[ii++] = mySerial2.read();
    if (ii == gpsRxBufferLength)clrGpsRxBuffer();
  }

  char* GPS_BufferHead;
  char* GPS_BufferTail;
  //  if ((GPS_BufferHead = strstr(gpsRxBuffer, "$GPRMC,")) != NULL || (GPS_BufferHead = strstr(gpsRxBuffer, "$GNRMC,")) != NULL )
  if ((GPS_BufferHead = strstr(gpsRxBuffer, "$GNGGA,")) != NULL || (GPS_BufferHead = strstr(gpsRxBuffer, "$GNGGA,")) != NULL )
  {
    if (((GPS_BufferTail = strstr(GPS_BufferHead, "\r\n")) != NULL) && (GPS_BufferTail > GPS_BufferHead))
    {
      memcpy(Save_Data.GPS_Buffer, GPS_BufferHead, GPS_BufferTail - GPS_BufferHead);
      Save_Data.isGetData = true;

      clrGpsRxBuffer();
    }
  }
}

void clrGpsRxBuffer(void)
{
  memset(gpsRxBuffer, 0, gpsRxBufferLength);      //清空
  ii = 0;
}

void  get_gps_sensor(int temp_sys_current_millis)
{

  gpsRead();  //获取GPS数据
  parseGpsBuffer();//解析GPS数据
  if (temp_sys_current_millis - previousMillis_gps >= gps_interval)
  {
    previousMillis_gps = temp_sys_current_millis; //更新时间记录
    printGpsBuffer();//输出解析后的数据
  }

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

void setup(void)
{
  Serial.begin(band_rate);
  mySerial.begin(band_rate);
  mySerial2.begin(band_rate);
  while (!Serial) {
    ;
  }
  while (!mySerial) {
    ;
  }
  while (!mySerial2) {
    ;
  }
  delay(100);
  Save_Data.isGetData = false;
  Save_Data.isParseData = false;
  Save_Data.isUsefull = false;
  SENSOR_Init();

  //  WIFI_Init();//调用WIFI函数
  wifi_with_password();
  //舵机
  myservo.attach(servo);
  for (int pos = 0; pos <= 10; pos += 1) {
    myservo.write(open_servo);
    delay(15);
  }
  delay(1000);
  for (int pos = 10; pos >= 0; pos -= 1) {
    myservo.write(close_servo);
    delay(15);
  }
  mySerial.println(beep3);
  Serial.println("系统启动成功!!");
}
void loop(void)
{

  //重连机制
  if (!client.connected()) {
    //    reconnect_MQTT();
    connect_MQTT();
  }
  client.loop();
  sys_current_millis  = millis();
  get_dht11(sys_current_millis);
  get_gps_sensor(sys_current_millis);
  delay(10);


}
