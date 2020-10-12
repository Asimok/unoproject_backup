#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT   //添加这一行，就能支持小爱控制

#include <Blinker.h>
 
char auth[] = "49b5881d62a5";
char ssid[] = "smart";
char pswd[] = "12345678";
 
// 新建组件对象
BlinkerButton Button1("btn-abc");
//setup()函数前添加如下代码
void miotPowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
        digitalWrite(LED_BUILTIN, LOW);

        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();
    }
    else if (state == BLINKER_CMD_OFF) {
        digitalWrite(LED_BUILTIN, HIGH);

        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();
    }
}

// 按下按键即会执行该函数
void button1_callback(const String & state) {
    BLINKER_LOG("get button state: ", state);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Blinker.vibrate();
}
 
void setup() {
    // 初始化串口，并开启调试信息
    Serial.begin(115200);    
    BLINKER_DEBUG.stream(Serial);
    // 初始化有LED的IO
    pinMode(LED_BUILTIN, OUTPUT);
    
    digitalWrite(LED_BUILTIN, HIGH);
    // 初始化blinker
    Blinker.begin(auth, ssid, pswd);
    BlinkerMIOT.attachPowerState(miotPowerState);    //添加这一行，作用是注册下面的回调函数
    Button1.attach(button1_callback);
}
 
void loop() {
    Blinker.run();
}
