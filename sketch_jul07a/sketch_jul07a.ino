
#include <ESP8266WiFi.h>






#include <ESP8266WiFi.h>      //加入ESP8266wifi头文件

#define LEDPIN 2                   //GPIO2
WiFiClient client;

const char *ssid     = "HEXBOT";        //ssid，你家的wifi名称
const char *password = "HEXBOT01?02?03";//wifi密码
const char *host = "192.168.0.107";   //改为上图所示的IP地址（改为你自己的）
const int tcpPort = 8266;//改为上图所示的本地端口号（改为你自己的）


void setup()
{
    Serial.begin(115200);    
    pinMode(LEDPIN,OUTPUT);
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");//会通过usb转tll模块发送到电脑，通过ide集成的串口监视器可以获取数据。
    Serial.println(ssid);

    WiFi.begin(ssid, password);//启动

     //在这里检测是否成功连接到目标网络，未连接则阻塞。
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("WiFi connection failed......");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());  //WiFi本地IP
}


void loop()
{
    while (!client.connected())   //若未连接到电脑服务端，则esp8266客户端进行连接。
    {
        if (!client.connect(host, tcpPort))//实际上这一步就在连接服务端，如果连接上，该函数返回true
        {
            Serial.println("connection....");
            delay(500);

        }
    }

    while (client.available())//available()表示是否可以获取到数据
    {
        char val = client.read();//read()表示从网络中读取数据。
        Serial.println(val);    //打印出来接收到的值
        if(val=='a'){          //pc端发送a和b来控制
           digitalWrite(LEDPIN, LOW);
        }
        if(val=='b')
        {
            digitalWrite(LEDPIN, HIGH);
        }
    }
}
