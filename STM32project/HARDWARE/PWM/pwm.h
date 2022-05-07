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
void forward(int speed);    //ǰ��
void turn_left(int speed);   //��ת
void turn_right(int speed);   //��ת
void converyor_speed_set(int speed); //���ʹ��ٶ�����
void Back_speed_set(int speed);   //����ռ�ʯ�͹����ٶ�����
void Collect_speed_set(int speed);   //ǰ���ռ�����ת���ٶ�����

#endif
