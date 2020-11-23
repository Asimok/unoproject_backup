
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); //RX=2,TX=3 ESP8266
int beep = 4;

void setup() {
  pinMode(beep, OUTPUT); //蜂鸣器

  digitalWrite(beep, HIGH);
  Serial.begin(9600);
  Serial.print("Welcome to use!\n");
  while (!Serial) {
    ;
  }
  mySerial.begin(9600);
  delay(1000);
  Serial.println("Start");
  openBeep(3);
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

    if (comdata == "a" )
    {
      openBeep(1);
    }
    else if (comdata == "b" )
    {
      openBeep(2);
    }  else if (comdata == "c" )
    {
      openBeep(3);
    }
    else if (comdata == "d" )
    {
      openBeep(4);
    }

  }
}
