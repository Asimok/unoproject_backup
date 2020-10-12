void setup()
{
     Serial.begin(9600);
     pinMode(13, OUTPUT);
     pinMode(12, OUTPUT);
     pinMode(11, OUTPUT);
     pinMode(10, OUTPUT);
}

void loop()
{
    while(Serial.available()){
        char c = Serial.read();
        Serial.println(c);
//灯1     
        if (c == 'A') {
            //Serial.println("BLE get char A.");
            //关
            digitalWrite(13, HIGH);
        }
        if (c == 'a') {
          
            //开
            digitalWrite(13, LOW);
        }

        //灯2     
        if (c == 'B') {
          
            //关
            digitalWrite(12, HIGH);
        }
        if (c == 'b') {
          
            //开
            digitalWrite(12, LOW);
        }
        //灯3     
        if (c == 'C') {
     
            //关
            digitalWrite(11, HIGH);
        }
        if (c == 'c') {
          
            //开
            digitalWrite(11, LOW);
        }
     //灯4    
        if (c == 'D') {
          
            //关
            digitalWrite(10, HIGH);
        }
        if (c == 'd') {
          
            //开
            digitalWrite(10, LOW);
        }   
    }
}
