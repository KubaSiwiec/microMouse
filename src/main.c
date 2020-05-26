/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f1xx.h"
			
UART_HandleTypeDef uart;

TIM_HandleTypeDef tim2;


//CREATE THE MOTOR CLASS AND USE IT TO CONTROL THE FUNCTIONS
 struct Motor{
	GPIO_TypeDef *port;
	uint16_t pinPhase;
	uint16_t pinEnable;
	TIM_HandleTypeDef tim;
	__IO uint16_t channel;
};

/*
 * My functions
 */



void setMotorDir(struct Motor motor, GPIO_PinState direction){
	//set for forward, reset for backwards
	HAL_GPIO_WritePin(motor.port, motor.pinPhase, direction);
}

//REEEEEEEE coś tu robię nie tak
//void setMotorSpeed(struct Motor motor, uint16_t pwm){
//	motor.channel = pwm;
//}

void send_string(char* s){
	HAL_UART_Transmit(&uart, (uint8_t*)s, strlen(s), 1000);
}



/*
 *PWM
 */



void TIM2_IRQHandler(void)
{
 HAL_TIM_IRQHandler(&tim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
 if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
 else
 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}




int main(void)
{

	SystemCoreClock = 8000000;
	HAL_Init();

	// enable the port A clock
	__HAL_RCC_GPIOA_CLK_ENABLE();
	//USART clock
	__HAL_RCC_USART2_CLK_ENABLE();

	__HAL_RCC_TIM2_CLK_ENABLE();

	//USART lines configuration
	GPIO_InitTypeDef gpioUSART;
	gpioUSART.Mode = GPIO_MODE_AF_PP;
	gpioUSART.Pin = GPIO_PIN_2;
	gpioUSART.Pull = GPIO_NOPULL;
	gpioUSART.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpioUSART);

	gpioUSART.Mode = GPIO_MODE_AF_INPUT;
	gpioUSART.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOA, &gpioUSART);

	//USART config

	uart.Instance = USART2;
	uart.Init.BaudRate = 115200;
	uart.Init.WordLength = UART_WORDLENGTH_8B;
	uart.Init.Parity = UART_PARITY_NONE;
	uart.Init.StopBits = UART_STOPBITS_1;
	uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart.Init.OverSampling = UART_OVERSAMPLING_16;
	uart.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&uart);




	//gpio object, which stores parameters of the ports being set
	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_5 | GPIO_PIN_1; //1 - ph, 0 - enb (pwm 2/1)
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;

	//initialize port A acording to the gpio object fields
	HAL_GPIO_Init(GPIOA, &gpio);

	//change frequency for pin 0 to high, for pwm
	gpio.Pin = GPIO_PIN_0;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOA, &gpio);


	tim2.Instance = TIM2;
	tim2.Init.Period = 500 - 1;
	tim2.Init.Prescaler = 800 - 1;	//dzielenie przez 4000 - częstotliwość 2kHz
	tim2.Init.ClockDivision = 0;
	tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim2.Init.RepetitionCounter = 0;
	tim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_Base_Init(&tim2);

	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_TIM_Base_Start_IT(&tim2);


	/*
	 * Channels settings
	 */
	 TIM_OC_InitTypeDef oc;
	 oc.OCMode = TIM_OCMODE_PWM1;
	 oc.Pulse = 0;	//0 for min speed, 20 for max
	 oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	 oc.OCNPolarity = TIM_OCNPOLARITY_LOW;
	 oc.OCFastMode = TIM_OCFAST_ENABLE;
	 oc.OCIdleState = TIM_OCIDLESTATE_SET;
	 oc.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	 HAL_TIM_PWM_ConfigChannel(&tim2, &oc, TIM_CHANNEL_1);

	 HAL_TIM_PWM_Start(&tim2, TIM_CHANNEL_1);	//channel 1 connected to pin A0



	 struct Motor motor = {GPIOA, GPIO_PIN_1, GPIO_PIN_0, tim2, tim2.Instance->CCR1};


	 HAL_GPIO_WritePin(motor.port, motor.pinPhase, GPIO_PIN_SET);
	for(;;){

		HAL_GPIO_WritePin(motor.port, GPIO_PIN_5, GPIO_PIN_SET);

		send_string("Diodes have been set!\n");
//		motor.tim.Instance->CCR1 = 499;	//sets motor speed
//		setMotorSpeed(motor, 499);
		setMotorDir(motor, GPIO_PIN_SET);

		HAL_Delay(4000);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		send_string("Diodes have been reset!\n");
		setMotorDir(motor, GPIO_PIN_RESET);
		motor.tim.Instance->CCR1 = 0; //makes it able to control the PWM percentage
		HAL_Delay(4000);
	}


}

