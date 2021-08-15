/*
  引脚定义：
  DHT11 5v ：D7
  触摸按键 3.3v ：D8 
  OLED 5v ：D15--SCL D14--SDA
  软串口：RX=D8,TX=D9
*/



#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
/*引入本地头文件*/
#include "dht11.h"

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#include <SoftwareSerial.h>
//软串口
SoftwareSerial mySerial(D8, D9); //RX=D8,TX=D9
/*指令*/
const char* front = "w";
const char* back = "s";
const char* left = "a";
const char* right = "d";
const char* stop_car = "q";
const char* speed1 = "z";
const char* speed2 = "x";
const char* speed3 = "c";
const char* beep1 = "1";
const char* beep3 = "3";
const char* auto_light_on = "n";//小夜灯开
const char* auto_light_off = "l";
/*可设置参数*/
int MSG_BEEP_TIMES = 3; //消息提醒次数
int MODE = 1; //1 普通 2 静音
String APIKEY = "SdQ0BnNLNW5YY7kwX";        //API KEY
String CITY = "huhehaote";
String NIGHT_LIGHT_STATUS = "n"; //小夜灯默认开
/*系统配置*/
const char* clientId = "jm_wifi";
const char* topic_y2m = "jm_y2m";
const char* topic_m2y = "jm_m2y";

#define band_rate 115200
int wifi_connect_try_count = 0;
int reset_count = 0;
bool WIFI_Status = true;
//备忘录模式
bool KEY_CLEAN = true;
//非可自定义参数
bool isbeep = true;

/*定时任务*/
const int interval_updateTime  = 1000;// 1s
const int interval_updateWeather  = 1000 * 60 * 30; //3600s/2*10  半小时
const int interval_dht11  = 4000;

unsigned long sys_current_millis;
//系统时间
unsigned long last_updateTime_sys_current_millis = 0;
//天气
unsigned long last_updateWeather_sys_current_millis = 0;
//DHT11
unsigned long last_updateDht11_sys_current_millis = 0;
//按键
unsigned long previousMillis_key = 0; //毫秒时间记录
/*传感器：实例化对象*/
//DHT11
#define dht11Pin D7   //定义温湿度针脚
dht11 dht;
int dht11_temp = 0;
int dht11_humi = 0;

//按键
#define TOUCH_SIG D6
bool is_touch = false;
//消息
String rec_msg1 = "";
String rec_msg2 = "";
String rec_msg3 = "";
bool isdisp_msg = false;
int show_msg_time = 0;
// 按钮可以清楚保持状态
bool stay_block = false;



/*NTP获取时间*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String currentTime = "";
String currentDate = "";
String weekDay = "";

/*http获取天气*/
const char* HOST = "http://api.seniverse.com";
const char* LANGUAGE = "zh-Hans"; //zh-Hans 简体中文  会显示乱码
const unsigned long HTTP_TIMEOUT = 5000;               // max respone time from server
// 天气：提取的数据的类型
struct WeatherData {
  char city[16];//城市名称
  char weather[32];//天气介绍（多云...）
  char temp[16];//温度
  char udate[32];//更新时间
};
HTTPClient http;
String GetUrl;
String response;
WeatherData weatherData;

/*连接MQTT*/
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqttServer = "39.96.68.13";
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

      u8g2.clearBuffer();
      u8g2.firstPage();
      do {
        oled_log("MQTT连接成功", 5, 30);
      } while ( u8g2.nextPage() );
      u8g2.sendBuffer();

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
  Serial.println("Message: ");
  char a[1200] = "";
  if (length <= 1200) {
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(String(json));
  if (!root.success()) {
    Serial.println("JSON parsing failed!");
  }
  else {
    //接受消息文字
    String source = root["source"];
    String msg_type = root["type"];


    if (source == SOURCE ) //app
    {
      if (msg_type == "msg") //显示消息
      {
        //判断是否设置为按键清除
        if (KEY_CLEAN)
          stay_block = true;
        else
          stay_block = false;

        client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"success\"}" );
        Serial.println("{\"source\":\"device\",\"status\":\"success\"}");
        String temp_rec_msg1 = root["msg1"];
        String temp_rec_msg2 = root["msg2"];
        String temp_rec_msg3 = root["msg3"];
        rec_msg1 = temp_rec_msg1;
        rec_msg2 = temp_rec_msg2;
        rec_msg3 = temp_rec_msg3;
        isdisp_msg = true;
        show_msg_time = 1;
      }

      //设置模式参数
      if (msg_type == "set")
      {
        String payload = root["payload"];

        //设置收到消息提醒次数
        String device = root["device"];
        if (device == "MSG_BEEP_TIMES")
        {
          MSG_BEEP_TIMES = payload.toInt();
          mySerial.println(beep1);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
        }
        if (device == "night_light")
        {
          NIGHT_LIGHT_STATUS = String(payload);
          mySerial.println(NIGHT_LIGHT_STATUS);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
        }
        if (device == "MODE")
        {
          //1 普通 2 静音
          MODE = payload.toInt();
          mySerial.println(beep1);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
          switchMODE();
        }
        if (device == "changewifi")
        {
          mySerial.println(beep1);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
          smartConfig();
        }
        if (device == "CITY")
        {
          CITY = String(payload);
          mySerial.println(beep1);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
          changeLoc();

        }
        if (device == "APIKEY")
        {
          APIKEY = String(payload);
          mySerial.println(beep1);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"setted\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"setted\"}");
          changeLoc();
        }

        if (device == "is_connected")
        {
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"is_connected\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"is_connected\"}");
        }
      }
      if (msg_type == "update")
      {
        String data = "{\"temp\":" + String(dht11_temp) + ",\"humi\":" + String(dht11_humi) + ",\"night_light_status\":\"" + NIGHT_LIGHT_STATUS +  "\"}";
        char data_char[80];
        strcpy(data_char, data.c_str());
        client.publish(topic_m2y,  data_char );
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
        }
        else if (directions == "speed2")
        {
          mySerial.println(speed2);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
        }
        else if (directions == "speed3")
        {
          mySerial.println(speed3);
          client.publish(topic_m2y, "{\"source\":\"device\",\"status\":\"received\"}" );
          Serial.println("{\"source\":\"device\",\"status\":\"received\"}");
        }
        
      }
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
//    Serial.print("DHT11-----温度: ");
//    Serial.print(dht11_temp);
//    Serial.print("C 湿度: ");
//    Serial.print(dht11_humi);
//    Serial.println("%");
  }
}

void smartConfig()//配网函数
{
  WiFi.mode(WIFI_STA);//使用wifi的STA模式

  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("打开APP", 5, 30);
    oled_log("重新配网哦！", 5, 50);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();

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

      u8g2.clearBuffer();
      u8g2.firstPage();
      do {
        oled_log(WiFi.SSID().c_str(), 5, 30);
        oled_log("配网成功", 5, 50);
      } while ( u8g2.nextPage() );
      u8g2.sendBuffer();
      delay(2000);
      break;
    }
  }
}

void OLED_Init()
{
  u8g2.begin();
  u8g2.enableUTF8Print();   // enable UTF8 support for the Arduino print() function}
}

void WIFI_Init()
{
  Serial.println("\n正在连接...");
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("正在连接WIFI...", 5, 30);
  } while ( u8g2.nextPage() );

  u8g2.sendBuffer();

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
        u8g2.clearBuffer();
        u8g2.firstPage();
        do {
          oled_log("WiFi连接失败", 5, 25);
          oled_log("请用手机进行配网!", 5, 50);
        } while ( u8g2.nextPage() );
        u8g2.sendBuffer();
        Serial.println("\nWiFi连接失败，请用手机进行配网\n");
      }
    }
    else//使用flash中的信息去连接wifi失败，执行
    {
      smartConfig();  //smartConfig技术配网
    }
  }
  //串口打印连接成功的IP地址

  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("WiFi连接成功", 5, 30);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();

  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  //联网成功 连接MQTT
  sys_current_millis  = millis();
  updateWeather(sys_current_millis);  //心知天气  发送http  get请求

  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("连接MQTT...", 5, 30);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();
  connect_MQTT();
}

void HTTP_Init()
{
  GetUrl = String(HOST) + "/v3/weather/now.json?key=";
  GetUrl += APIKEY;
  GetUrl += "&location=";
  GetUrl += CITY;
  GetUrl += "&language=";
  GetUrl += LANGUAGE;
  //设置超时
  http.setTimeout(HTTP_TIMEOUT);
  //设置请求url
  http.begin(GetUrl);
  //以下为设置一些头  其实没什么用 最重要是后端服务器支持
  http.setUserAgent("esp8266");//用户代理版本
  http.setAuthorization("esp8266", "boge"); //用户校验信息
}

void SENSOR_Init()
{
  pinMode(dht11Pin, OUTPUT);//温度传感器引脚
  //按键
  pinMode(TOUCH_SIG, INPUT);
}

/*OLED相关函数*/
void mainPage() {
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {

    //  室温 湿度
    u8g2.setFont(u8g2_font_unifont_t_chinese1);//9px
    u8g2.setFontDirection(0);
    u8g2.setCursor(0, 13);
    u8g2.print("感知:  " + String(dht11_temp) + "°C");
    u8g2.setCursor(95, 13);
    u8g2.print(String(dht11_humi) + "%");

    //  时间
    u8g2.setFont(u8g2_font_luBS18_tf);//18px
    u8g2.setFontDirection(0);
    u8g2.setCursor(22, 12 + 18 + 5);
    u8g2.print(String(currentTime));
    //  u8g2.setFont(u8g2_font_lubB08_tn);//9px
    u8g2.setFont(u8g2_font_7x13B_tr);

    u8g2.setFontDirection(0);
    //  u8g2.setCursor(35, 12 + 18 + 5 + 11);
    u8g2.setCursor(17, 12 + 18 + 5 + 11);
    u8g2.print(String(currentDate) + " " + String(weekDay));

    //网络天气
    u8g2.setFont(u8g2_font_unifont_t_chinese1);//14px
    u8g2.setCursor(0, 62);
    u8g2.print(String(weatherData.city));
    u8g2.setCursor(62, 62);
    u8g2.print(String(weatherData.weather));
    u8g2.setCursor(93, 62);
    u8g2.print(String(weatherData.temp) + "°C");
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();

}

void draw_msg(String s1, String s2, String s3)
{

  //在微信小程序端 分割成3个字符串
  //绘制边框
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setFont(u8g2_font_unifont_t_chinese1);
    oled_log(s1, 5, 20);
    oled_log(s2, 5, 40);
    oled_log(s3, 5, 60);

  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();
  delay(1000);

}

String deal_str(String temp_text1, int num )
{

  char temp_text[100];
  strcpy(temp_text, temp_text1.c_str());

  char Set[100];
  int mp = 0;
  int iCut = 0;
  int i = 0;
  int len = strlen(temp_text);
  for (i = 0; i < len; i++)
  {

    if (temp_text[i] >> 7 & 1) //如果最高位右移&运算后==1
    {

      Set[mp++] = i + 3;
      i += 2; //中文字符占2个字节 i++ 避免下次循环又读取同一个字符的第2个字节
    }
    else
    {

      Set[mp++] = i + 1;
    }
  }

  /*
    关键在于Set[mp++] 存储结束字符的位置 ，循环结束后直接读取结束位置的字符设置为'\0'即可把
  */
  iCut = Set[num - 1];
  temp_text[iCut] = '\0';

  //  Serial.println(temp_text);
  return String(temp_text);
}
String sub_str(String buf , int8_t start_index , int8_t end_index)
{

  if (start_index == 0)
    return deal_str(buf, end_index);
  else

  {
    String start_str = deal_str(buf, start_index);
    String end_str = deal_str(buf, end_index);
    end_str.remove(0, start_str.length());
    return end_str;
  }
}
bool parseUserData(String content, struct WeatherData* weatherData)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(content);
  if (!root.success())
  {
    Serial.println("JSON parsing failed!");
    return false;
  }

  //复制我们感兴趣的字符串
  strcpy(weatherData->city, root["results"][0]["location"]["name"]);
  strcpy(weatherData->weather, root["results"][0]["now"]["text"]);
  strcpy(weatherData->temp, root["results"][0]["now"]["temperature"]);
  strcpy(weatherData->udate, root["results"][0]["last_update"]);
  return true;
}
// 打印从JSON中提取的数据
void printUserData(const struct WeatherData* weatherData) {

  Serial.print("City:");
  Serial.print(weatherData->city);
  Serial.print(", ");
  Serial.print("Weather:");
  Serial.print(weatherData->weather);
  Serial.print(", ");
  Serial.print("Temp:");
  Serial.print(weatherData->temp);
  Serial.print("C");
  Serial.print(", ");
  Serial.print("Last Updata:");
  Serial.println(weatherData->udate);

}
void first_updateWeather()
{

  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    //判断请求是否成功
    if (httpCode == HTTP_CODE_OK) {
      //读取响应内容
      response = http.getString();
      Serial.println("Get the data from Internet!");
      //        Serial.println(response);
      //解析响应内容
      if (parseUserData(response, &weatherData)) {
        //打印响应内容
        printUserData(&weatherData);
      }
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
void updateWeather(int temp_sys_current_millis)
{
  //心知天气  发送http  get请求
  if (temp_sys_current_millis - last_updateWeather_sys_current_millis >= interval_updateWeather)
  {
    last_updateWeather_sys_current_millis = temp_sys_current_millis;
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //判断请求是否成功
      if (httpCode == HTTP_CODE_OK) {
        //读取响应内容
        response = http.getString();
        Serial.println("Get the data from Internet!");
        //        Serial.println(response);
        //解析响应内容
        if (parseUserData(response, &weatherData)) {
          //打印响应内容
          printUserData(&weatherData);
        }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}
void updateTime(int temp_sys_current_millis)
{
  if (temp_sys_current_millis - last_updateTime_sys_current_millis >= interval_updateTime)
  {
    last_updateTime_sys_current_millis = temp_sys_current_millis;

    timeClient.update();
    currentTime = timeClient.getFormattedTime().substring(0, 5);
    //    currentTime = String(timeClient.getHours()) + ":" + String(timeClient.getMinutes());
    //获取时间戳
    unsigned long epochTime = timeClient.getEpochTime();
    //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
    weekDay = weekDays[timeClient.getDay()];
    String currentMonthName = months[currentMonth - 1];
    //    Serial.print("Current date: ");
    //    Serial.print(currentDate);
    //    Serial.print("\tCurrent time: ");
    //    Serial.print(currentTime);
    //    Serial.print("\t");
    //    Serial.print(weekDay);
    //    Serial.print(" ");
    //    Serial.println(currentMonthName);
  }
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
void refrash_screen()
{

  if (isdisp_msg)
  {

    if (show_msg_time <= MSG_BEEP_TIMES )
    {
      show_msg_time++;
      //显示消息
      if (isbeep)

        mySerial.println(beep3);
      draw_msg(rec_msg1, rec_msg2, rec_msg3);
      delay(10);
    }
    else {
      show_msg_time = 1;
      isdisp_msg = false;
      rec_msg1 = "";
      rec_msg2 = "";
      rec_msg3 = "";
    }
  }

}
void changeLoc()
{

  HTTP_Init();
  delay(100);
  first_updateWeather();
}
void oled_log( String msg, int x, int y)
{
  u8g2.setFont(u8g2_font_unifont_t_chinese1);
  u8g2.setFontDirection(0);
  u8g2.setCursor(x, y);
  u8g2.print(msg);

}
//获取状态
boolean get_touch()
{
  boolean touch_stat = 0;
  delay(5);
  touch_stat = digitalRead(TOUCH_SIG); //读入状态
  return touch_stat;
}

void setup(void)
{
  Serial.begin(band_rate);
  mySerial.begin(band_rate);
  while (!Serial) {
    ;
  }
  while (!mySerial) {
    ;
  }

  OLED_Init();
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("姜萌的小车车", 5, 25);
    oled_log("by 没有胡子的猫", 5, 45);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();
  delay(1500);
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("系统正在启动...", 5, 30);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();

  delay(100);
  SENSOR_Init();
  WIFI_Init();//调用WIFI函数
  timeClient.begin();
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("更新本地天气...", 5, 30);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();

  delay(100);
  //更新天气
  HTTP_Init();
  delay(100);
  first_updateWeather();

  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    oled_log("系统启动成功!", 5, 30);
  } while ( u8g2.nextPage() );
  u8g2.sendBuffer();
  delay(100);
  mySerial.println(beep3);
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
  updateTime(sys_current_millis);
  updateWeather(sys_current_millis);  //心知天气  发送http  get请求
  get_dht11(sys_current_millis);
  delay(10);
  if (stay_block)
  {
    refrash_screen();
  }
  else
    mainPage();

  //长按机制
  unsigned long currentMillist_key = millis();         //读取当前时间
  boolean touch_stat;
  touch_stat = get_touch();
  if (touch_stat == 1 && is_touch == false)
  {
    is_touch = true;
    previousMillis_key = currentMillist_key;
  }
  else if (touch_stat == 0)
  {
    is_touch = false;
    previousMillis_key = currentMillist_key;
  }
  if (is_touch)
  {
    Serial.print("长按：");
    Serial.println(currentMillist_key - previousMillis_key);
    if (currentMillist_key - previousMillis_key >= 10000) //长按10秒重新配网
    {
      smartConfig();
    }
    else if (currentMillist_key - previousMillis_key >= 20) //如果和前次时间大于等于时间间隔
    {
      //清除屏幕内容
      stay_block = false;
    }
  }
}
