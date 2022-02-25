#ifndef __HX711__H__
#define __HX711__H__

#include <Arduino.h>

//#define HX711_SCK 2
//#define HX711_DT 3

#define HX711_SCK D0
#define HX711_DT D1
extern void Init_Hx711();
extern void Init_Hx7112();
extern unsigned long HX711_Read(void);
extern long Get_Weight();
extern void Get_Maopi();
extern unsigned long HX711_Read2(void);
extern long Get_Weight2();
extern void Get_Maopi2();
#endif
