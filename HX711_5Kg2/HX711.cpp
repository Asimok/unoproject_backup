#include "hx711.h"

long HX711_Buffer = 0;
long Weight_Maopi = 0,Weight_Shiwu = 0;
long HX711_Buffer2 = 0;
long Weight_Maopi2 = 0,Weight_Shiwu2 = 0;
#define GapValue 430

//****************************************************
//初始化HX711
//****************************************************
void Init_Hx711()
{
	pinMode(HX711_SCK, OUTPUT);	
	pinMode(HX711_DT, INPUT);
 pinMode(HX711_SCK2, OUTPUT); 
  pinMode(HX711_DT2, INPUT);
}


//****************************************************
//获取毛皮重量
//****************************************************
void Get_Maopi()
{
	Weight_Maopi = HX711_Read();		
} 
void Get_Maopi2()
{
  Weight_Maopi2 = HX711_Read2();    
} 
//****************************************************
//称重
//****************************************************
long Get_Weight()
{
	HX711_Buffer = HX711_Read();
	Weight_Shiwu = HX711_Buffer;
	Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//获取实物的AD采样数值。
	Weight_Shiwu = (long)((float)Weight_Shiwu/GapValue); 	
	return Weight_Shiwu;
}
long Get_Weight2()
{
  HX711_Buffer2 = HX711_Read2();
  Weight_Shiwu2 = HX711_Buffer2;
  Weight_Shiwu2 = Weight_Shiwu2 - Weight_Maopi2;       //获取实物的AD采样数值。
  Weight_Shiwu2 = (long)((float)Weight_Shiwu2/GapValue);  
  return Weight_Shiwu2;
}
//****************************************************
//读取HX711
//****************************************************
unsigned long HX711_Read(void)	//增益128
{
	unsigned long count; 
	unsigned char i;
	bool Flag = 0;

	digitalWrite(HX711_DT, HIGH);
	delayMicroseconds(1);

	digitalWrite(HX711_SCK, LOW);
	delayMicroseconds(1);

  	count=0; 
  	while(digitalRead(HX711_DT)); 
  	for(i=0;i<24;i++)
	{ 
	  	digitalWrite(HX711_SCK, HIGH); 
		delayMicroseconds(1);
	  	count=count<<1; 
		digitalWrite(HX711_SCK, LOW); 
		delayMicroseconds(1);
	  	if(digitalRead(HX711_DT))
			count++; 
	} 
 	digitalWrite(HX711_SCK, HIGH); 
	count ^= 0x800000;
	delayMicroseconds(1);
	digitalWrite(HX711_SCK, LOW); 
	delayMicroseconds(1);
	
	return(count);
}
unsigned long HX711_Read2(void)  //增益128
{
  unsigned long count; 
  unsigned char i;
  bool Flag = 0;

  digitalWrite(HX711_DT2, HIGH);
  delayMicroseconds(1);

  digitalWrite(HX711_SCK2, LOW);
  delayMicroseconds(1);

    count=0; 
    while(digitalRead(HX711_DT2)); 
    for(i=0;i<24;i++)
  { 
      digitalWrite(HX711_SCK2, HIGH); 
    delayMicroseconds(1);
      count=count<<1; 
    digitalWrite(HX711_SCK2, LOW); 
    delayMicroseconds(1);
      if(digitalRead(HX711_DT2))
      count++; 
  } 
  digitalWrite(HX711_SCK2, HIGH); 
  count ^= 0x800000;
  delayMicroseconds(1);
  digitalWrite(HX711_SCK2, LOW); 
  delayMicroseconds(1);
  
  return(count);
}
