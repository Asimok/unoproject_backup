#include <Servo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 5           // 配置针脚
#define SS_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;

Servo servo;
int servoPin = 2;//D4
int beep = D3;

void setup() {


  Serial.begin(9600);
  // 初始化IO
  digitalWrite(beep, HIGH);
   delay(100);
  SPI.begin();        // SPI开始
  mfrc522.PCD_Init(); // Init MFRC522 card
  
 
//  servo.attach(servoPin);
//  servo.write(20);
//  delay(200);
//  servo.write(0);
//  servo.detach();
//  openBeep(4);

 Serial.println("system start");
}
void loop() {
  rc522();
}

void rc522() {
  // 寻找新卡
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //    Serial.println("没有找到卡");
    return;
  }

  // 选择一张卡
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    //    Serial.println("没有卡可选");
    return;
  }


  // 显示卡片的详细信息
  Serial.print(F("卡片 UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

  //  Serial.println();
  //  Serial.print(F("卡片类型: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //  Serial.println(mfrc522.PICC_GetTypeName(piccType));

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
    //    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //    Serial.print(buffer[i], HEX);
    temphex += tohex(buffer[i]);
  }
  //  Serial.println();
  //   Serial.println(temphex);
  //   发送数据
   Serial.println(temphex);
  openBeep(1);
//  check(temphex);
 
 
}
void openBeep(int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(beep, LOW);
    openBeep(2);
    delay(100);
    digitalWrite(beep, HIGH);
    openBeep(3);
    delay(100);
  }
}
void openlock() {
  servo.attach(servoPin);
  servo.write(100);
  delay(1000);
  servo.write(0);
  delay(1000);
  servo.detach();
}
//void check(String uids)
//{
//  if (uids == "9A685C1A")
//    openlock();
//   if(uids=="77042A0E")
//   openlock();
//}
