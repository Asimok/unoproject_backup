
#include <SoftwareSerial.h>

//超声波测距
unsigned int EchoPin = 4;
unsigned int TrigPin = 5;
unsigned long Time_Echo_us = 0;
unsigned long Len_mm_X100  = 0;
unsigned long Len_Integer = 0;
unsigned int Len_Fraction = 0;
int ultrasonic_dis = 0;

const int interval_ultrasonic  = 10;//10ms
const int interval_ultrasonic_send  = 300;
unsigned long sys_current_millis;
//系统时间
unsigned long last_updateTime_sys_current_millis = 0;
//超声波
unsigned long last_updateUltrasonic_sys_current_millis = 0;
//上次发送时间
unsigned long last_updateUltrasonic_sys_current_millis_send = 0;

bool issend = false;
void setup() {

  Serial.begin(115200);
  Serial.print("Welcome to use!\n");
  while (!Serial) {
    ;
  }

  //超声波测距
  pinMode(EchoPin, INPUT);
  pinMode(TrigPin, OUTPUT);

  delay(1000);
  Serial.println("Start");
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
      //      Serial.print("距离: ");
      //      Serial.print(ultrasonic_dis);
      //      Serial.println("cm");
    }
  }
}

void senCode_close(int temp_sys_current_millis_send,String code)

{

  if (temp_sys_current_millis_send - last_updateUltrasonic_sys_current_millis_send >= interval_ultrasonic_send)
  {
    last_updateUltrasonic_sys_current_millis_send = temp_sys_current_millis_send;

    Serial.print(code);
    issend=true;
  }
}
void senCode_open(int temp_sys_current_millis_send,String code)

{

  if (temp_sys_current_millis_send - last_updateUltrasonic_sys_current_millis_send >= interval_ultrasonic_send)
  {
    last_updateUltrasonic_sys_current_millis_send = temp_sys_current_millis_send;

    Serial.print(code);
    issend=false;
    
  }
}
void loop() {
  sys_current_millis  = millis();
  getDistance(sys_current_millis);
  if (ultrasonic_dis < 12)
  {
    senCode_close(sys_current_millis,"closeservo");
    delay(200);
  }
  else if(issend)
  {
    senCode_open(sys_current_millis,"openservo");
    delay(200);
  }
  delay(100);


}
