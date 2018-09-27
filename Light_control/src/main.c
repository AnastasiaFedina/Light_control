#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"
#include "delay.h"
#include "I2C.h"
#include "LCD_I2C.h"
#include <stdbool.h>
#include "stm32f10x_tim.h"
#include "stdio.h"
#include "misc.h"
#include "Encoder.h"
#include "Rele.h"

#define FORWARD		0
#define BACKWARD	1

#define NOREADY		0
#define READY		1
#define INIT		3

volatile uint8_t encoder_status = INIT;
volatile uint8_t encoder_direction = FORWARD;

//Interrupt TIM3
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if (encoder_status == INIT)
            encoder_status = NOREADY;
        else
            encoder_status = READY;

        //Direction of rotation of the encoder
        encoder_direction = (TIM3->CR1 & TIM_CR1_DIR ? BACKWARD : FORWARD);
    }
}

int main()
{
    rele_init();
    encoder_init();
    LCDI2C_init(0x27,16,2);
    Delay(1000);
    LCDI2C_backlight();
    LCDI2C_write_String("Relay_1 off");
    char rele=1;
    bool stan[4] = {false};
    char str[20];
    while (1)
    {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)	//Checking the encoder click

        {
            LCDI2C_clear();
            //Relay inversion
            if (stan[rele-1]==false) stan[rele-1]=true;
            else stan[rele-1]=false;

            if(rele==1)
                GPIOA->ODR ^= GPIO_Pin_12;

            if(rele==2)
                GPIOA->ODR ^= GPIO_Pin_11;

            if(rele==3)
                GPIOA->ODR ^= GPIO_Pin_10;

            if(rele==4)
                GPIOA->ODR ^= GPIO_Pin_9;
            Delay(400);
            //Output status of the relay
            sprintf(str, "Relay_%d", rele);
            LCDI2C_write_String(str);
            if(stan[rele-1]==false)
                LCDI2C_write_String(" off");
            else
                LCDI2C_write_String(" on");
        }
//Encoder rotation
        if (encoder_status)
        {
            encoder_status = NOREADY;

            if (encoder_direction == FORWARD) {
                LCDI2C_clear();
                rele++;
                if(rele==5) rele=1;

            }
            else {
                LCDI2C_clear();
                rele--;
                if(rele==0) rele=4;

            }
            sprintf(str, "Relay_%d", rele);
            LCDI2C_write_String(str);
            if(stan[rele-1]==false)
                LCDI2C_write_String(" off");
            else
                LCDI2C_write_String(" on");
            Delay(500);
        }
    }
}
