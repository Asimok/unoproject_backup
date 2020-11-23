int  beep = D3;

void setup() {
 
  pinMode(beep, OUTPUT); //蜂鸣器
    digitalWrite(beep, HIGH);

     openBeep(3);
}

void loop() {
  
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
