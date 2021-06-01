/*arduino安装自带*/
#include<Servo.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/*引入本地头文件*/
#include "dht11.h"


/*传感器：实例化对象*/
//DHT11
#define dht11Pin D2   //定义温湿度针脚
dht11 dht;
int dht11_temp = 0;
int dht11_humi = 0;

//超声波测距
unsigned int EchoPin = D4;
unsigned int TrigPin = D5;
unsigned long Time_Echo_us = 0;
unsigned long Len_mm_X100  = 0;
unsigned long Len_Integer = 0;
unsigned int Len_Fraction = 0;
int ultrasonic_dis = 0;

//蜂鸣器
int beep = D6;

//Servo
Servo servo;
#define servo_pin D7
#define SWAGTIME 3 //摇摆次数
#define swag_left_angle 0
#define swag_right_angle 60
#define swag_close_angle 90
#define swag_central_angle 30


/*定时任务*/
const int interval_updateTime  = 1000;// 1s
const int interval_updateWeather  = 1000 * 10; //1s*10
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
//CITY http获取天气中配置


/*按键*/
//gpio0
int pinInterrupt = 0; //0 is the flash button ,so can use flash button as the config button


/*NTP获取时间*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String currentTime = "";
String currentDate = "";

/*http获取天气*/
const char* HOST = "http://api.seniverse.com";
const char* APIKEY = "Se2r7cpmAJrQZk7v_";        //API KEY
char* CITY = "daqing";
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
const char* clientId = "whisper_001";
const char* topic = "whisper";


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
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    a[i] = (char)payload[i];
  }
  Serial.println(a);
  docode(a);
}
void docode(char json[2000])
{
  //  解析指令
  //  StaticJsonDocument<200> doc;
  //  deserializeJson(doc, json);
  //  const char* code = doc["code"];
  //  Serial.println();
  //  Serial.println(code);
  //  String tempcode;
  //  tempcode = String(code);
  //  if (tempcode == "get_dht11")
  //    get_dht11();
  //  else if (tempcode == "get_light_status")
  //    send_light_data();
  //  else
  //    switchlight(tempcode);
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
    //  StaticJsonDocument<200> temperature_data;
    //  temperature_data["sensor"] = "DHT11";
    //  temperature_data["temp"] = temp;
    //  temperature_data["humi"] = humi;
    //  serializeJson(temperature_data, msg);
    //  //Serial.println(msg);
    //  client.publish(topic, msg);
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

      break;
    }
  }
}
void WIFI_Init()
{
  Serial.println("\n正在连接...");
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
  Serial.println("连接成功");
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
  //联网成功 连接MQTT
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
  pinMode(EchoPin, INPUT);
  pinMode(TrigPin, OUTPUT);
  //蜂鸣器
  pinMode(beep, OUTPUT); 
  digitalWrite(beep, LOW);
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
    currentTime = timeClient.getFormattedTime();
    //获取时间戳
    unsigned long epochTime = timeClient.getEpochTime();
    //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
    String weekDay = weekDays[timeClient.getDay()];
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
void getDistance(int temp_sys_current_millis)
{
  if (temp_sys_current_millis - last_updateUltrasonic_sys_current_millis >= interval_ultrasonic)
  {
    last_updateUltrasonic_sys_current_millis = temp_sys_current_millis;

    digitalWrite(TrigPin, HIGH);
    delayMicroseconds(50);
    digitalWrite(TrigPin, LOW);
    Time_Echo_us = pulseIn(EchoPin, HIGH);
    if ((Time_Echo_us < 60000) && (Time_Echo_us > 1))
    {
      Len_mm_X100 = (Time_Echo_us * 34) / 20;
      Len_Integer = Len_mm_X100 / 100;
      Len_Fraction = Len_mm_X100 % 100;
      ultrasonic_dis = Len_Integer + Len_Fraction / 100.0;
      Serial.print("距离: ");
      Serial.print(ultrasonic_dis);
      Serial.println("cm");
    }
  }
}

void use_servo(String tempcode)
{
  Serial.print("舵机指令: ");
  Serial.println(tempcode);
  delay(50);
  if (tempcode == "swag")
  {

    servo.attach(servo_pin);
    for (int i = 0; i < SWAGTIME; i++)
    {
      servo.write(swag_left_angle);
      delay(500);
      servo.write(swag_right_angle);
      delay(500);
    }
    servo.write(swag_central_angle);
    servo.detach();
  }
  else if (tempcode == "close")
  {
    servo.attach(servo_pin);
    servo.write(swag_close_angle);
    servo.detach();
  }
  else if (tempcode == "open")
  {
    servo.attach(servo_pin);
    servo.write(swag_central_angle);
    servo.detach();
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
void setup()
{
  Serial.begin(band_rate);
  SENSOR_Init();
  WIFI_Init();//调用WIFI函数
  timeClient.begin();
  HTTP_Init();
openBeep(3);
}

void loop()
{
  //重连机制
  if (!client.connected()) {
    reconnect_MQTT();
  }
  client.loop();
  //检测flash按键是否触发长按操作
  button_reset();

  sys_current_millis  = millis();
  updateTime(sys_current_millis);
  updateWeather(sys_current_millis);  //心知天气  发送http  get请求
  get_dht11(sys_current_millis);
  getDistance(sys_current_millis);
//  if (ultrasonic_dis < 8)
//    use_servo("swag");

}
