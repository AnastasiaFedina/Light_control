#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "Encoder.h"

//Initialization encoder
void encoder_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpio_cfg;
    gpio_cfg.GPIO_Mode = GPIO_Mode_IPU;
    gpio_cfg.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    gpio_cfg.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio_cfg);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    //Configure TIM3
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_Period = 3;
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up | TIM_CounterMode_Down;
    TIM_TimeBaseInit(TIM3, &TIMER_InitStructure);

    //Configure Encoder Interface, interrupt
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    NVIC_EnableIRQ(TIM3_IRQn);

    GPIO_InitTypeDef port;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_IPU;
    port.GPIO_Pin = GPIO_Pin_5;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &port);
}
