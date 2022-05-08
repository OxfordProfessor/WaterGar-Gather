#ifndef _SENSOR_H
#define _SENSOR_H
#include "sys.h"
void ADC_init(void);
void HCSR04_init(void);
u32 GetEchoTimer(void);
float Hcsr04GetLength(void);
void TIM7_IRQHandler(void);
void AllSensor_init(void);
float current_tds(void);
int current_ph(void);
int margin_storage_percent(void);
int margin_voltage_percent(void);
#endif
