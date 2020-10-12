#include<Servo.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "ArduinoJson-v6.15.2.h"

#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         5           // 配置针脚
#define SS_PIN          4
MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;

int lock = D4, beep = D3;
long lastMsg = 0;
char msg[500];
int value = 0;

const char* ssid = "HUAWEI_B316_mqtt";
const char* password = "mqtt103mqtt103";
const char* mqtt_server = "119.45.181.212";
const int mqttPort = 1883;
const char* clientId = "entrance_230603001";
const char* topic_y2m = "chargingpile_lock_y2m";
const char* topic_m2y = "chargingpile_lock_m2y";
const char* topic_online = "chargingpile_lock_status";

char* code = "close";
String open_id = "";
WiFiClient espClient;
PubSubClient client(espClient);


void setup() {

  pinMode(lock, OUTPUT); //继电器
  pinMode(beep, OUTPUT); //蜂鸣器
  digitalWrite(beep, HIGH);
  digitalWrite(lock, LOW);
  Serial.begin(9600);
  //连接wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
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
  SPI.begin();        // SPI开始
  mfrc522.PCD_Init(); // Init MFRC522 card

  char buff[50];
  //客户端上线提醒
  memset(buff, 0, sizeof(buff));
  const char *buff2 = "_online";
  strcpy(buff, clientId);
  strcat(buff, buff2);
  //发送连接成功消息
  client.publish(topic_online, buff );
  //订阅主题
  client.subscribe(topic_y2m);
  delay(100);

  openBeep(3);
  Serial.println("system start");
}

/**
   断开重连
*/
void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("reConnecting to MQTT...");

    if (client.connect(clientId)) {
      // Serial.println("connected");
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
  // Serial.print("Message arrived in topic: ");
  //Serial.println(topic);
  //  Serial.print("Message:");
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
  //判断开锁id是不是自己
  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);
  const char* code = doc["code"];
  const char* client_id = doc["client_id"];
  const char* open_id_char = doc["open_id"];
  String tempcode, temp_client_id;
  open_id = String(open_id_char);
  tempcode = String(code);
  temp_client_id = String(client_id);
  if (temp_client_id == clientId)
  {
       Serial.println("自身客户端指令");
      
    switchlock(tempcode);
  }
  else
    Serial.println("不是属于我的开锁指令");
}

void switchlock(String tempcode)
{

  delay(10);
   Serial.println(tempcode);
  if (tempcode == "open")
  {
    //开锁

    char msg[1000];
    StaticJsonDocument<2000> entrance;
    StaticJsonDocument<200> uid_data;

    uid_data["client_id"] = clientId;
    uid_data["open_id"] = open_id;
    uid_data["status"] = "opened";
    
    entrance["entrance"] = uid_data;
    serializeJson(entrance, msg);
    Serial.println(msg);
    client.publish(topic_m2y, msg );
    digitalWrite(lock, HIGH);
    openBeep(1);

  }
  else if (tempcode == "close")
  {
    //   关锁

    char msg[1000];
    StaticJsonDocument<2000> entrance;
    StaticJsonDocument<200> uid_data;
    uid_data["client_id"] = clientId;
    uid_data["open_id"] = open_id;
    uid_data["status"] = "closed";
    entrance["entrance"] = uid_data;
    serializeJson(entrance, msg);
    Serial.println(msg);
    client.publish(topic_m2y, msg );
    digitalWrite(lock, LOW);
    openBeep(2);
  }
  else if (tempcode == "false")
  {
    //   未注册
    openBeep(4);
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
  rc522();
}

void rc522() {
  // 寻找新卡
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //Serial.println("没有找到卡");
    return;
  }

  // 选择一张卡
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    //    Serial.println("没有卡可选");
    return;
  }


  // 显示卡片的详细信息
  //  Serial.print(F("卡片 UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);


  //  Serial.println();
  //  Serial.print(F("卡片类型: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // 检查兼容性
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("仅仅适合Mifare Classic卡的读写"));
    return;
  }

  MFRC522::StatusCode status;
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("身份验证失败？或者是卡链接失败"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //停止 PICC
  mfrc522.PICC_HaltA();
  //停止加密PCD
  mfrc522.PCD_StopCrypto1();
  return;
}
//十进制转十六进制
String tohex(int n) {
  if (n == 0) {
    return "00"; //n为0
  }
  String result = "";
  char _16[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
  const int radix = 16;
  while (n) {
    int i = n % radix;          // 余数
    result = _16[i] + result;   // 将余数对应的十六进制数字加入结果
    n /= radix;                 // 除以16获得商，最为下一轮的被除数
  }
  if (result.length() < 2) {
    result = '0' + result; //不足两位补零
  }
  return result;
}
/**
   将字节数组转储为串行的十六进制值
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  String temphex;
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    temphex += tohex(buffer[i]);
  }
  //  Serial.println();
  //   Serial.println(temphex);
  //   发送数据

  char msg[500];
  StaticJsonDocument<500> card;
  StaticJsonDocument<200> uid_data;
  uid_data["client_id"] = clientId;
  uid_data["uid"] = temphex;
  card["card"] = uid_data;
  serializeJson(card, msg);
  client.publish(topic_m2y, msg );
  openBeep(1);
  Serial.println(msg);
}
