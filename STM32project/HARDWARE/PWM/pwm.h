#ifndef _PWM_H
#define _PWM_H
#include "sys.h"

void TIM1_PWM_Init(u16 per,u16 psc);
void TIM2_PWM_Init(u16 per,u16 psc);
void TIM3_PWM_Init(u16 per,u16 psc);
void TIM4_PWM_Init(u16 per,u16 psc);
void TIM8_PWM_Init(u16 per,u16 psc);
void TIM9_PWM_Init(u16 per,u16 psc);
void AllMoter_init(void);
void forward(int speed);    //前进
void turn_left(int speed);   //左转
void turn_right(int speed);   //右转
void converyor_speed_set(int speed); //传送带速度设置
void Back_speed_set(int speed);   //后边收集石油滚轮速度设置
void Collect_speed_set(int speed);   //前方收集垃圾转轴速度设置

#endif
