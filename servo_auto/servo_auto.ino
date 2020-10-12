#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(9, 8); // RX, TX
#define TOUCH_SIG 10
Servo servo180;

void setup() {
  pinMode(TOUCH_SIG, INPUT);      //设置10号端口为输入模式
  servo180.attach(3);
  servo180.write(60);

  Serial.begin(9600);
  Serial.println("系统启动中...");
  while (!Serial) {
    ;
  }
  mySerial.begin(9600);
  delay(1000);
  Serial.println("系统启动成功");



}
//获取状态
boolean get_touch() {
  boolean touch_stat = 0;
  touch_stat = digitalRead(TOUCH_SIG); //读入状态
  return touch_stat;
}

void loop() {
  boolean touch_stat;

  touch_stat = get_touch();
  if (touch_stat)
  {
    Serial.print("\nrunning\nTouch Stat - ");
    Serial.print(touch_stat);
     servo180.write(0); 
  }
  delay(100);

  if (mySerial.available())
  {
    String comdata = "";
    int i = 0;
    while (mySerial.available() > 0)
    {
      i++;
      char a = char(mySerial.read());
      comdata += a;
      delay(2);
    }
    Serial.println(comdata);

    servo180.write(comdata.toInt());  //控制舵机转动相应的角度。

  }


}
