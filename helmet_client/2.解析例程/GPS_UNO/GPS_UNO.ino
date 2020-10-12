/******************************************************************
	作者：神秘藏宝室
	店铺：ILoveMCU.taobao.com
	最终解释权归原作者所有，鄙视其他店铺不劳而获的抄袭行为！
******************************************************************/

 #include <SoftwareSerial.h>
struct
{
	char GPS_Buffer[200];
	bool isGetData;		//是否获取到GPS数据
	bool isParseData;	//是否解析完成
	char UTCTime[11];		//UTC时间
	char latitude[11];		//纬度
	char N_S[2];		//N/S
	char longitude[12];		//经度
	char E_W[2];		//E/W
	bool isUsefull;		//定位信息是否有效
  char altitude[11];    //海拔
} Save_Data;

const unsigned int gpsRxBufferLength = 900;
char gpsRxBuffer[gpsRxBufferLength];
unsigned int ii = 0;
SoftwareSerial mySerial2(12, 13); //RX=12,TX=13

void setup()	//初始化内容
{
	Serial.begin(9600);			//定义波特率9600，和我们店铺的GPS模块输出的波特率一致
	mySerial2.begin(9600);
	Serial.println("ILoveMCU.taobao.com");
	Serial.println("Wating...");

	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
}

void loop()		//主循环
{
	gpsRead();	//获取GPS数据
	parseGpsBuffer();//解析GPS数据
	printGpsBuffer();//输出解析后的数据
	// Serial.println("\r\n\r\nloop\r\n\r\n");
}

void errorLog(int num)
{
	Serial.print("ERROR");
	Serial.println(num);

}

void printGpsBuffer()
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		Serial.print("Save_Data.UTCTime = ");
		Serial.println(Save_Data.UTCTime);

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
			Serial.print("Save_Data.latitude = ");
			Serial.println(Save_Data.latitude);
			Serial.print("Save_Data.N_S = ");
			Serial.println(Save_Data.N_S);
			Serial.print("Save_Data.longitude = ");
			Serial.println(Save_Data.longitude);
			Serial.print("Save_Data.E_W = ");
			Serial.println(Save_Data.E_W);
      Serial.print("Save_Data.altitude = ");
      Serial.println(Save_Data.altitude);
		}
		else
		{
			Serial.println("GPS DATA is not usefull!");
     Serial.println(Save_Data.altitude);
		}
		
	}
}

void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		Serial.println("**************");
		Serial.println(Save_Data.GPS_Buffer);

		
		for (int i = 0 ; i <= 9 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);;break;	//获取UTC时间
//						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);Serial.println(subString);break;	//获取UTC时间
						case 2:memcpy(Save_Data.latitude, subString, subStringNext - subString);;break;	//获取纬度信息
						case 3:memcpy(Save_Data.N_S, subString, subStringNext - subString);;break;	//获取N/S
						case 4:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 5:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W
            case 9:memcpy(Save_Data.altitude, subString, subStringNext - subString);break; //获取海拔

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
         Save_Data.isUsefull = true;
//					if(usefullBuffer[0] == 'A')
//						Save_Data.isUsefull = true;
//					else if(usefullBuffer[0] == 'V')
//						Save_Data.isUsefull = false;

				}
				else
				{
					errorLog(2);	//解析错误
				}
			}

delay(100);
		}
	}
}


void gpsRead() {
	while (mySerial2.available())
	{
		gpsRxBuffer[ii++] = mySerial2.read();
		if (ii == gpsRxBufferLength)clrGpsRxBuffer();
	}

	char* GPS_BufferHead;
	char* GPS_BufferTail;
	if ((GPS_BufferHead = strstr(gpsRxBuffer, "$GNGGA,")) != NULL || (GPS_BufferHead = strstr(gpsRxBuffer, "$GNGGA,")) != NULL )
	{
		if (((GPS_BufferTail = strstr(GPS_BufferHead, "\r\n")) != NULL) && (GPS_BufferTail > GPS_BufferHead))
		{
			memcpy(Save_Data.GPS_Buffer, GPS_BufferHead, GPS_BufferTail - GPS_BufferHead);
			Save_Data.isGetData = true;

			clrGpsRxBuffer();
		}
	}
}

void clrGpsRxBuffer(void)
{
	memset(gpsRxBuffer, 0, gpsRxBufferLength);      //清空
	ii = 0;
}
