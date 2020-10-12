#include <Servo.h>

#define   forwardSpeed 1000
#define   inversionSpeed 2000
//int port1 = A8;//连接火焰传感器
//int value1 = 0;//测量的模拟数值
Servo servo360;
Servo servo180;
void setup() {
  servo180.attach(3);
  servo180.write(60);
  //Serial.begin(9600);
  //  while (!Serial) {
  //    ;
  //  }
  // pinMode(4,OUTPUT);
  //digitalWrite(4, HIGH);
}

void loop() {
  // digitalWrite(4, HIGH);
  //  delay(1000);
  // value1 = analogRead(port1);
  //  Serial.print("    value: ");
  //  Serial.println(value1);
  // servo180.writeMicroseconds(forwardSpeed);
  // delay(300);
  // servo180.detach();
  //  delay(1000);

  //  servo360.attach(6);
  //  servo360.writeMicroseconds(inversionSpeed);
  //   delay(500);
  //    servo360.detach();
    servo180.write(60);
    delay(2000);
    servo180.write(85);
    delay(2000);
    servo180.write(110);
    delay(2000);
     servo180.write(85);
    delay(2000);

  //detectDistance();
  delay(1000);
  //  digitalWrite()
}

void detectDistance() {
  for (int i = 85; i <= 110; i += 5)
  {
    //左转
    servo180.write(i);
    delay(100);
    if (i == 110)
    {
      for (int i = 110; i >= 60; i -= 5)
      {
        //右转
        servo180.write(i);
        delay(100);
        if (i == 60)
        {
          for (int i = 60; i <= 85; i += 5)
          {
            //左转回归到正向
            servo180.write(i);
            delay(100);
            if (i == 85)
              break;
          }
        }
      }

    }
  }
  servo180.write(85);
}





void detectDistance(int angleLeft,int angleRight,int angelCenter,int speed) {
  for (int i = angelCenter; i <= angleLeft; i += 5)
  {
    //左转
    servo180.write(i);
    delay(50);
    writeAroundDistance(i, getDistance());
    delay(speed);
    if (i == angleLeft)
    {
      for (int i = angleLeft; i >= angleRight; i -= 5)
      {
        //右转
        servo180.write(i);
        delay(50);
        writeAroundDistance(i, getDistance());
        delay(speed);
        if (i == angleRight)
        {
          for (int i = angleRight; i <= 85; i += 5)
          {
            //左转回归到正向

            servo180.write(i);
            delay(50);
            writeAroundDistance(i, getDistance());
            delay(speed);
            if (i == angelCenter)
              break;
          }
        }
      }
    }
  }

}
