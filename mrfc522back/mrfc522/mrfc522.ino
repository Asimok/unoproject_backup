#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT//支持小爱同学
#include <Blinker.h>
SoftwareSerial mySerial(D0, D3);

#define RST_PIN         5           // 配置针脚
#define SS_PIN          4
MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;

#define TOUCH_SIG D8
unsigned long previousMillis = 0; //毫秒时间记录

Servo servo;
int servoPin = 2;//D4
int beep = D0;

char auth[] = "bb506eb40f3f";
char ssid[] = "女寝专用";
char pswd[] = "208208nb";
// 新建组件对象
BlinkerButton Button1("btn-abc");//注意：要和APP组件’数据键名’一致

// 按下BlinkerAPP按键即会执行该函数
void button1_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  // digitalWrite(GPIO, !digitalRead(GPIO));
  openlock();
  Blinker.vibrate();
}
//小爱电源类操作的回调函数:
//当小爱同学向设备发起控制, 设备端需要有对应控制处理函数
void miotPowerState(const String & state)
{
  BLINKER_LOG("need set power state: ", state);
  if (state == BLINKER_CMD_ON) {
    // digitalWrite(GPIO, LOW);
    openlock();
    BlinkerMIOT.powerState("on");
    BlinkerMIOT.print();
  }
  else if (state == BLINKER_CMD_OFF) {
    // digitalWrite(GPIO, HIGH);
    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();
  }
}


void setup() {
  pinMode(beep, OUTPUT); //蜂鸣器
    pinMode(TOUCH_SIG, INPUT);
  digitalWrite(beep, HIGH);
  Serial.begin(9600); // 设置串口波特率为9600
  delay(200);
  mySerial.begin(9600);
  Blinker.deleteTimer();
  BLINKER_DEBUG.stream(Serial);
  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  Button1.attach(button1_callback);

  //小爱同学务必在回调函数中反馈该控制状态
  BlinkerMIOT.attachPowerState(miotPowerState);//注册回调函数


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
  //  Serial.println("test-demo-start");
}
void loop()
{
  rc522();
  Blinker.run();

  boolean touch_stat;
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis >= 20) //如果和前次时间大于等于时间间隔
  {
    Serial.print("\nrunning\nTouch Stat - ");
    touch_stat = get_touch();
    if (touch_stat == 1)
    {
      openlock();
    }

    Serial.print(touch_stat);

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
  //  Serial.print(F("卡片 UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  //  Serial.println();
  //  Serial.print(F("卡片类型: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // 检查兼容性
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //    Serial.println(F("仅仅适合Mifare Classic卡的读写"));
    return;
  }

  MFRC522::StatusCode status;
  if (status != MFRC522::STATUS_OK) {
    //    Serial.print(F("身份验证失败？或者是卡链接失败"));
    //    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //停止 PICC
  mfrc522.PICC_HaltA();
  //停止加密PCD
  mfrc522.PCD_StopCrypto1();
  return;
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
  //  Serial.println(temphex);
  delay(100);
  //  openBeep(1);
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
  mySerial.print("a");
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
  else
  {
    mySerial.print("d");
  }

}
