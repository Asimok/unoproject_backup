//没加小爱同学
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#define RST_PIN         9           // 配置针脚
#define SS_PIN          10
MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;
#define TOUCH_SIG 5
unsigned long previousMillis = 0; //毫秒时间记录

Servo servo;
int servoPin = 6;
int beep = 7;

void setup() {
  pinMode(beep, OUTPUT); //蜂鸣器
  pinMode(TOUCH_SIG, INPUT);
  digitalWrite(beep, HIGH);
  Serial.begin(9600); // 设置串口波特率为9600

  SPI.begin();        // SPI开始
  mfrc522.PCD_Init(); // Init MFRC522 card
  delay(100);
  servo.attach(servoPin);
  servo.write(30);
  delay(200);
  servo.write(0);
  delay(200);
  servo.detach();
  openBeep(3);
  Serial.println("开机");
}
void loop()
{
  rc522();

  boolean touch_stat;
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis >= 20) //如果和前次时间大于等于时间间隔
  {
    touch_stat = get_touch();
    if (touch_stat == 1)
    {
      Serial.println("手指");
      openlock();
    }
    //    Serial.print(touch_stat);
  }

}

void rc522() {
  // 寻找新卡
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
//    Serial.println("没有找到卡");
    return;
  }
  // 选择一张卡
  if ( ! mfrc522.PICC_ReadCardSerial()) {
        Serial.println("没有卡可选");
    return;
  }
  // 显示卡片的详细信息
//    Serial.print(F("卡片 UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

}

/**
   将字节数组转储为串行的十六进制值
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  String temphex = "";
  for (byte i = 0; i < bufferSize; i++) {
    temphex += tohex(buffer[i]);
  }
//    Serial.println(temphex);
  delay(100);
  openBeep(1);
  check(temphex);

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

//获取状态
boolean get_touch() {
  boolean touch_stat = 0;
  touch_stat = digitalRead(TOUCH_SIG); //读入状态
  return touch_stat;
}
void openlock() {
  openBeep(2);
  servo.attach(servoPin);
  servo.write(100);
  delay(3000);
  servo.write(0);
  delay(1000);
  servo.detach();
  openBeep(3);
}
void check(String temphex)
{
    Serial.println("check");
  Serial.println(temphex);
  if (temphex == "9A685C1A")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else if (temphex == "77042A0E")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else if (temphex == "9C540D23")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else
  {
    openBeep(4);
  }

}
