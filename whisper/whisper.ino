/*arduino安装自带*/
#include<Servo.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
/*引入本地头文件*/
#include "dht11.h"
// 0 1 2 3 4 5 6
/*oled*/
#define OLED_DI   D7  //DI   ------ 
#define OLED_D0    D5  //D0  -------
#define OLED_DC    D2  //DC  ------- 
#define OLED_CS    D8  //CS  ------- 
#define OLED_RESET D3  //RES -------
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ OLED_D0, /* data=*/ OLED_DI, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RESET);
/*可设置参数*/
int MSG_BEEP_TIMES = 5; //消息提醒次数
int MODE = 1; //1 普通 2 共享 3 静音
String APIKEY = "SdQ0BnNLNW5YY7kwX";        //API KEY
//1
//String CITY = "haerbin";
//int SWAGTIME = 2 ;//摇摆次数
//const char* clientId = "ID_whisper_zxy";
//const char* topic = "whisper_zxy";
//String NAME="张鑫屿";


//2 
String CITY = "suqian";
int SWAGTIME = 2 ;//摇摆次数
const char* clientId = "ID_whisper_lyy";
const char* topic = "whisper_lyy";
String NAME="骆媛媛";

//3
//String CITY = "beijing";
//int SWAGTIME = 2 ;//摇摆次数
//const char* clientId = "ID_whisper_lzc";
//const char* topic = "whisper_lzc";
//String NAME="梁紫晨";


//消息
String rec_msg = "";
bool isdisp_msg = false;
int show_msg_time = 0;
bool stay_block = false;

//非可自定义参数
bool switch_servo = true;
bool isbeep = true;
bool isultrasonic = true;
bool serial_free = true;


/*传感器：实例化对象*/
//DHT11
#define dht11Pin D0   //定义温湿度针脚
dht11 dht;
int dht11_temp = 0;
int dht11_humi = 0;


//蜂鸣器
int beep = D1;

//Servo
Servo servo;
#define servo_pin D4
#define swag_left_angle 107+40
#define swag_right_angle 107-40

#define swag_close_angle 6
#define swag_open_angle 107

#define swag_central_angle 107

String current_statu = "open";


/*定时任务*/
const int interval_updateTime  = 1000;// 1s
const int interval_updateWeather  = 1000 * 60 * 30; //3600s/2*10  半小时
const int interval_dht11  = 4000;
const int interval_ultrasonic  = 10;

unsigned long sys_current_millis;
//系统时间
unsigned long last_updateTime_sys_current_millis = 0;
//天气
unsigned long last_updateWeather_sys_current_millis = 0;
//DHT11
unsigned long last_updateDht11_sys_current_millis = 0;
//超声波
unsigned long last_updateUltrasonic_sys_current_millis = 0;

/*系统配置*/
#define band_rate 115200
int wifi_connect_try_count = 0;
int reset_count = 0;
bool WIFI_Status = true;


/*按键*/
//gpio0
int pinInterrupt = 0; //0 is the flash button ,so can use flash button as the config button


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
//const char* APIKEY = "Se2r7cpmAJrQZk7v_";        //API KEY
//char* CITY = "beijing";
const char* LANGUAGE = "zh-Hans";//zh-Hans 简体中文  会显示乱码
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
//const char* clientId = "ID_whisper_lzc";
//const char* topic = "whisper_lzc";

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
      client.publish(topic, buff );
      //订阅主题
      Serial.print("subscribe topic: ");
      Serial.println(topic);
      client.subscribe(topic);
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
  Serial.print("Message:");
  char a[2000] = "";
  if (length > 2000)
  {
    docode("消息太长啦！");
  }
  else {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      a[i] = (char)payload[i];
    }
    //  Serial.println(a);
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
    String payload = root["payload"];

    if (source == SOURCE ) //app
    {
      if (msg_type == "msg") //显示消息
      {
        client.publish(topic, "{\"source\":\"device\",\"status\":\"success\"}" );
        rec_msg = payload;
        isdisp_msg = true;
        show_msg_time = 1;

        //        for (int i = 0; i < 3; i++)
        //        {
        //          if (isbeep)
        //            openBeep(MSG_BEEP_TIMES);
        //          if (switch_servo)
        //            use_servo("swag");
        //
        //          Serial.println(payload);
        //          draw_msg(payload);
        //          delay(3000);
        //        }
        //        if (switch_servo)
        //          use_servo("open");
      }

      //设置模式参数
      if (msg_type == "set")
      {
        //设置收到消息提醒次数
        String device = root["device"];
        if (device == "MSG_BEEP_TIMES")
        {
          MSG_BEEP_TIMES = payload.toInt();
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"setted\"}" );
        }
        if (device == "MODE")
        {

          //1 普通 2 共享 3 静音
          MODE = payload.toInt();
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"setted\"}" );
          switchMODE();
        }
        if (device == "changewifi")
        {
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"setted\"}" );
          smartConfig();
        }
        if (device == "CITY")
        {
          CITY = String(payload);
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"setted\"}" );
          changeLoc();

        }
        if (device == "APIKEY")
        {
          APIKEY = String(payload);
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"setted\"}" );
          changeLoc();
        }
        if (device == "SWAGTIME")
        {
          SWAGTIME = payload.toInt();
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"setted\"}" );
        }
        if (device == "is_connected")
        {
          openBeep(1);
          client.publish(topic, "{\"source\":\"device\",\"status\":\"is_connected\"}" );

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
    Serial.print("DHT11-----温度: ");
    Serial.print(dht11_temp);
    Serial.print("C 湿度: ");
    Serial.print(dht11_humi);
    Serial.println("%");
  }
}

void smartConfig()//配网函数
{
  WiFi.mode(WIFI_STA);//使用wifi的STA模式
  u8g2.clearBuffer();
  oled_log("打开APP", 5, 30);
  oled_log("重新配网哦！", 5, 50);

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
      oled_log(WiFi.SSID().c_str(), 5, 30);
      oled_log("配网成功", 5, 50);
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
  oled_log("正在连接WIFI...", 5, 30);
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
        oled_log("WiFi连接失败", 5, 25);
        oled_log("请用手机进行配网!", 5, 50);
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
  oled_log("WiFi连接成功", 5, 30);
  u8g2.sendBuffer();

  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  //联网成功 连接MQTT
  sys_current_millis  = millis();
  updateWeather(sys_current_millis);  //心知天气  发送http  get请求

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

void SENSOR_Init() {


  pinMode( pinInterrupt, INPUT);//设置按键管脚为输入
  pinMode(dht11Pin, OUTPUT);//温度传感器引脚
  //超声波测距
  //  pinMode(EchoPin, INPUT);
  //  pinMode(TrigPin, OUTPUT);
  //蜂鸣器
  pinMode(beep, OUTPUT);
  digitalWrite(beep, HIGH);
  //舵机
  servo.attach(servo_pin);
  use_servo("swag");
  delay(500);
  use_servo("open");
}

/*OLED相关函数*/
void mainPage() {
  u8g2.clearBuffer();

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
  u8g2.setCursor(55, 62);
  u8g2.print(String(weatherData.weather));
  u8g2.setCursor(93, 62);
  u8g2.print(String(weatherData.temp) + "°C");
  u8g2.sendBuffer();
}

void draw_msg(String s)
{
  //绘制边框
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_unifont_t_chinese1);
  u8g2.sendBuffer();

  int16_t len = s.length();
  int8_t chunk = 0;
  int8_t oneline_maxnum = 7 * 3;

  if (len % oneline_maxnum == 0)
    chunk = len / oneline_maxnum ;
  else
    chunk = len / oneline_maxnum + 1;


  for (int8_t i = 0; i < chunk; i++)
  {
    //截取文字 每行最多 21个字符
    String temp_srt = "";
    //该行完整
    if (i * oneline_maxnum + oneline_maxnum <= len )
    {
      temp_srt = sub_str(s, i * oneline_maxnum / 3, (i * oneline_maxnum + oneline_maxnum) / 3);
    }
    else
    {
      temp_srt = sub_str(s, i * oneline_maxnum / 3, len / 3);
    }

    Serial.println(temp_srt);
    int k = i % 3;
    Serial.println(k);
    if (k == 0)
    {
      u8g2.clearBuffer();
      u8g2.drawFrame(0, 0, 128, 64);
      u8g2.setFont(u8g2_font_unifont_t_chinese1);
      u8g2.sendBuffer();
    }
    if (k == 2)
    {
      u8g2.setCursor(5, 20 + k * 20);
      u8g2.print(temp_srt);
      u8g2.sendBuffer();
      delay(3000);
      u8g2.clearBuffer();
    }

    u8g2.setCursor(5, 20 + k * 20);
    u8g2.print(temp_srt);
    u8g2.sendBuffer();

  }
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

void oled_log( String msg, int x, int y)
{
  u8g2.setFont(u8g2_font_unifont_t_chinese1);
  u8g2.setCursor(x, y);
  u8g2.print(msg);
}

void button_reset()
{
  while (digitalRead(pinInterrupt) == LOW) {
    delay(200);
    reset_count ++;
    if (reset_count >= 15) { //>=3s
      Serial.println("Long Key");
      client.disconnect();
      smartConfig();
      connect_MQTT();
      break;
    }
  }
  reset_count = 0;
}

bool parseUserData(String content, struct WeatherData* weatherData) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(content);
  if (!root.success()) {
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
    Serial.print("Current date: ");
    Serial.print(currentDate);
    Serial.print("\tCurrent time: ");
    Serial.print(currentTime);
    Serial.print("\t");
    Serial.print(weekDay);
    Serial.print(" ");
    Serial.println(currentMonthName);
  }
}

void use_servo(String tempcode)
{
  //  openBeep(3);
  Serial.print("舵机指令:   ");
  Serial.println(tempcode);
  delay(10);
  if (tempcode == "swag")
  {
    //swag
    for (int i = 0; i < SWAGTIME; i++)
    {
      servo.write(swag_left_angle);
      delay(500);
      servo.write(swag_right_angle);
      delay(500);
    }
    servo.write(swag_central_angle);
    current_statu = "central";

  }

  else if (tempcode == "close" && current_statu != "close")
  {
    current_statu = "close";
    servo.write(swag_close_angle);
    delay(50);
  }
  else if (tempcode == "open" && current_statu != "open")
  {
    current_statu = "open";
    servo.write(swag_open_angle);
    delay(50);
  }
}

void openBeep(int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(beep, LOW);
    delay(100);
    digitalWrite(beep, HIGH);
    delay(100);
  }
}

void wait_msg()
{
  if (Serial.available())
  {
    serial_free = false;
    String comdata = "";
    int i = 0;
    while (Serial.available() > 0)
    {
      i++;
      char a = char(Serial.read());
      comdata += a;
      delay(2);
    }
    Serial.println(comdata);
    if (comdata.indexOf("closeservo") != -1)
    {
      Serial.println(comdata);
      stay_block = true;
      if (switch_servo)
        use_servo("close");
    }
    if (comdata.indexOf("openservo") != -1)
    {
      stay_block = false;
      Serial.println(comdata);
      if (switch_servo)
        use_servo("open");
    }
    refrash_screen();
    delay(200);
  }
  else
    serial_free = true;
}

void switchMODE()
{
  //   MODE  1 普通 2 共享 3 静音

  //非可自定义参数
  switch_servo = true;
  isbeep = true;
  isultrasonic = true;
  if (MODE == 1)
  {
    bool switch_servo = true;
    bool isbeep = true;
    bool isultrasonic = true;
  }
  if (MODE == 2)
  {
    switch_servo = false;
    isbeep = true;
    isultrasonic = false;
  }
  if (MODE == 3)
  {
    switch_servo = false;
    isbeep = false;
    isultrasonic = false;
  }
}

void refrash_screen() {
  if (!stay_block)
  {
    if (isdisp_msg)
    {

      if (show_msg_time <= 3)
      {
        show_msg_time++;
        //显示消息
        if (isbeep)
          openBeep(MSG_BEEP_TIMES);
        if (switch_servo)
          use_servo("swag");

        Serial.println(rec_msg);
        draw_msg(rec_msg);
        delay(10);
      }
      else {

        show_msg_time = 0;
        isdisp_msg = false;
        rec_msg = "";
      }
    }
    else
    {
      delay(1000);
      mainPage();
    }
  }
  else {
    u8g2.clearBuffer();
    oled_log("不许偷看啊！！！", 5, 35);
    u8g2.sendBuffer();
  }
}

void setup()
{
  Serial.begin(band_rate);
  while (!Serial) {
    ;
  }
  OLED_Init();
  u8g2.clearBuffer();
  oled_log(NAME, 5, 15);
  oled_log("讲悄悄话给你听", 5, 35);
  oled_log("by 没有胡子的猫", 5, 55);
  u8g2.sendBuffer();
  delay(5200);


//  u8g2.clearBuffer();
//  oled_log("生日快乐呀!!!", 1, 25);
//   u8g2.setFont(u8g2_font_7x13B_tr);
//     u8g2.setCursor(1, 45);
//  u8g2.print("Designed in 2021.5");
//  u8g2.sendBuffer();
//  delay(5200);
  
  u8g2.clearBuffer();
  oled_log("系统正在启动...", 5, 30);
  u8g2.sendBuffer();
  delay(1000);
  SENSOR_Init();
  WIFI_Init();//调用WIFI函数
  timeClient.begin();
  //更新天气
  HTTP_Init();
  delay(100);
  first_updateWeather();
  openBeep(3);

}

void changeLoc()
{

  HTTP_Init();
  delay(100);
  first_updateWeather();
}

void loop()
{
  //重连机制
  if (!client.connected()) {
    //    reconnect_MQTT();
    connect_MQTT();
  }
  client.loop();
  //检测flash按键是否触发长按操作
  button_reset();

  if (isultrasonic)
    wait_msg();//串口等待数据

  if (serial_free) {

    sys_current_millis  = millis();
    updateTime(sys_current_millis);
    updateWeather(sys_current_millis);  //心知天气  发送http  get请求
    get_dht11(sys_current_millis);
    delay(10);

    refrash_screen();

  }



}
