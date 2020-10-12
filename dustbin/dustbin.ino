    #include <SoftwareSerial.h>
    #include <Servo.h> //引入lib
    SoftwareSerial mySerial(10, 11); // RX, TX
      Servo myservoleft;
      Servo wetservoleft;
      Servo reservoleft;
      Servo hzservoleft ;
void setup() {
 Serial.begin(9600);
      while (!Serial) {
        ;
      }
      mySerial.begin(9600);
      delay(2000);
      Serial.println("Start connecting");
myservoleft.attach(6);
myservoleft.write(100);
wetservoleft.attach(3);
wetservoleft.write(100);
reservoleft.attach(5);
reservoleft.write(100);
hzservoleft.attach(9);
hzservoleft.write(100);
}

void loop() {
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
         if (comdata == "A" )
        {
        //干垃圾开
          delay(100);
          myservoleft.write(20);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
          mySerial.print("openDryTrush");
          
        }
         else if (comdata == "a" )
        {
         //干垃圾关
          delay(100);
          myservoleft.write(100);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
         mySerial.print("closeDryTrush");
        }
         else if (comdata == "B" )
        {
         //湿垃圾开
          delay(100);
          wetservoleft.write(20);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
          mySerial.print("openwettrush");
        }
         else if (comdata == "b" )
        {
        //湿垃圾关
          delay(100);
          wetservoleft.write(100);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
         mySerial.print("closewettrush");
        }
        else if (comdata == "C" )
        {
         //可回收垃圾开
          delay(100);
          reservoleft.write(20);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
          mySerial.print("openrecycletrush");
        }
         else if (comdata == "c" )
        {
        //可回收垃圾关
          delay(100);
          reservoleft.write(100);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
         mySerial.print("closerecycletrush");
        }
        else if (comdata == "D" )
        {
         //有害垃圾开
          delay(100);
          hzservoleft.write(20);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
          mySerial.print("openhazaroustrush");
        }
         else if (comdata == "d" )
        {
        //有害垃圾关
          delay(100);
          hzservoleft.write(100);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
         mySerial.print("closehazaroustrush");
        }


        
         else if (comdata == "R" )
        {
        //有害垃圾关
          delay(100);
          myservoleft.write(20);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
         mySerial.print("open1");
        }
         else if (comdata == "G" )
        {
        //有害垃圾关
          delay(100);
          myservoleft.write(100);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
         mySerial.print("open2");
        }
      }
}
