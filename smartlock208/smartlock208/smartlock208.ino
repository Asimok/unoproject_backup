//去掉了小爱同学（延迟太高）
#include <SPI.h>
#include <Servo.h>
#include <SoftwareSerial.h>
//手动安装
#include <MFRC522.h>

#define RST_PIN         5           // 配置针脚
#define SS_PIN          4
#define TOUCH_SIG D8

SoftwareSerial mySerial(D0, D3);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;
Servo servo;

unsigned long previousMillis = 0; //毫秒时间记录
int servoPin = 2;//D4

void setup() {
  pinMode(TOUCH_SIG, INPUT);
  Serial.begin(9600); // 设置串口波特率为9600
  delay(100);
  mySerial.begin(9600);
  delay(100);
  SPI.begin();        // SPI开始
  mfrc522.PCD_Init(); // Init MFRC522 card
  delay(100);

  servo.attach(servoPin);
  servo.write(30);
  delay(200);
  servo.write(0);
  delay(200);
  servo.detach();

  mySerial.print("c");
  Serial.println("smatr208-lock-start");
}
void loop()
{
  rc522();
  touchbtn();
}

void touchbtn() {
  boolean touch_stat;
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis >= 20) //如果和前次时间大于等于时间间隔
  {
    //    Serial.print("\nrunning\nTouch Stat - ");
    touch_stat = get_touch();
    if (touch_stat == 1)
    {
      openlock();
    }
    //    Serial.print(touch_stat);
  }
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
  //  Serial.print("卡片 UID:");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
}

/**
   将字节数组转储为串行的十六进制值
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  String temphex = "";
  for (byte i = 0; i < bufferSize; i++) {
    //    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //    Serial.print(buffer[i], HEX);
    temphex += tohex(buffer[i]);
  }
  Serial.println(temphex);
  delay(100);
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

//获取状态
boolean get_touch() {
  boolean touch_stat = 0;
  touch_stat = digitalRead(TOUCH_SIG); //读入状态
  return touch_stat;
}
void openlock() {
  mySerial.print("a");
  delay(50);
  //  Serial.println("openlock");
  servo.attach(servoPin);
  servo.write(100);
  delay(3000);

  servo.write(0);
  delay(1000);
  servo.detach();
  mySerial.print("b");
}
void check(String temphex)
{
  //  Serial.println("check");
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
  else if (temphex == "EF9169C6")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else if (temphex == "2F3102D1")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else if (temphex == "07B5400E")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else if (temphex == "CFC519C6")
  {
    //    Serial.println(temphex);
    openlock();
  }
  else
  {
    mySerial.print("d");
  }

}
