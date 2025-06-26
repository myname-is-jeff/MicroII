/* Includes ------------------------------*/
#include "my_lib.h"

/* Macros --------------------------------*/

/* Global Variable -----------------------*/
TIM_HandleTypeDef hbasetim; // TIM3 Handler Variable
TIM_HandleTypeDef hpwm; // TIM2 Handler Variable
ADC_HandleTypeDef hadc; // ADC Handler Variable
uint32_t debounce = 0, sampling = 0;

/* Public Function Definitions ------------*/

/* Hardware Configuration */
uint8_t Hw_Init(void){

	/* HAL Initialization	*/
	HAL_Init(); // --> Init Software

	/* CLK Configuration */
	SystemClock_Config(); // --> Init Hardware

	/* TIM3 Configuration (Base Timer) */
	TIM3_Config();

	/* TIM2 Configuration (PWM Timer) */
	TIM2_Config();

	/* GPIO Configuration */
	GPIO_Config(); // --> Init Hardware

	/* ADC Configuration */
	ADC_Config(); // --> Init Hardware

	// PWM Start
	HAL_TIM_PWM_Start(&hpwm, TIM_CHANNEL_1);

	return(1);
}

/* CLK Configuration */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  return;
}

/* GPIO Configuration */
void GPIO_Config(void){

	GPIO_InitTypeDef LED_Pin_Conf = {0};
	GPIO_InitTypeDef SWITCH_Pin_Conf = {0};
	GPIO_InitTypeDef PWM_Pin_Conf = {0};
	GPIO_InitTypeDef ADC_Pin_Conf = {0};

	/* GPIO Port Hardware Enable */
	// --> Hardware
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* LED Initial State : OFF */
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, LED_OFF);

	/* LED Pin Features Load */
	// --> Software
	LED_Pin_Conf.Pin = LED_PIN;
	LED_Pin_Conf.Mode = GPIO_MODE_OUTPUT_PP;
	LED_Pin_Conf.Pull = GPIO_NOPULL;
	LED_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;

	/* LED Pin Hardware Configuration */
	// --> Hardware
	HAL_GPIO_Init(LED_PORT, &LED_Pin_Conf);

	/* SWITCH Pin Features Load */
	// --> Software
	SWITCH_Pin_Conf.Pin = SWITCH_PIN;
	SWITCH_Pin_Conf.Mode = GPIO_MODE_INPUT;
	SWITCH_Pin_Conf.Pull = GPIO_NOPULL;
	SWITCH_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;

	/* SWITCH Pin Hardware Configuration */
	// --> Hardware
	HAL_GPIO_Init(SWITCH_PORT, &SWITCH_Pin_Conf);

	/* PWM Pin Features Load */
	// --> Software
	PWM_Pin_Conf.Pin = PWM_PIN;
	PWM_Pin_Conf.Mode = GPIO_MODE_AF_PP;
	PWM_Pin_Conf.Pull = GPIO_NOPULL;
	PWM_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;
	PWM_Pin_Conf.Alternate = GPIO_AF1_TIM2;

	/* PWM Pin Hardware Configuration */
	// --> Hardware
	HAL_GPIO_Init(PWM_PORT, &PWM_Pin_Conf);

	/* ADC Pin Hardware Configuration */
	__HAL_RCC_ADC1_CLK_ENABLE();
    /**ADC1 GPIO Configuration
	  PA1     ------> ADC1_IN1
	 */
	ADC_Pin_Conf.Pin = GPIO_PIN_1;
	ADC_Pin_Conf.Mode = GPIO_MODE_ANALOG;
	ADC_Pin_Conf.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &ADC_Pin_Conf);

	return;
}

/* TIM3 Configuration */
void TIM3_Config(void){

	TIM_ClockConfigTypeDef CLK_Source = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/*  TIM Features Load */
	// --> Software
	hbasetim.Instance = BASE_TIMER;
	hbasetim.Init.Prescaler = BASE_PRESCALER-1;
	hbasetim.Init.CounterMode = TIM_COUNTERMODE_UP;
	hbasetim.Init.Period = _BasePeriod(Timer_Period);
	hbasetim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hbasetim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	/* TIM Hardware Configuration */
	// --> Hardware
	HAL_TIM_Base_Init(&hbasetim);

	/* CLK Source Selection */
	CLK_Source.ClockSource = TIM_CLOCKSOURCE_INTERNAL; // --> Software
	HAL_TIM_ConfigClockSource(&hbasetim, &CLK_Source); // --> Hardware

	/* TIM OVF Interrupt Config */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // --> Software
	HAL_TIMEx_MasterConfigSynchronization(&hbasetim, &sMasterConfig); // --> Hardware

	/* TIMER RUN (Interruption)	*/
	HAL_TIM_Base_Start_IT(&hbasetim); // --> Hardware

	return;
}

/* TIM2 Configuration */
void TIM2_Config(void){

	TIM_ClockConfigTypeDef CLK_Source = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef PWM_Config = {0};

	/*  TIM Features Load */
	// --> Software
	hpwm.Instance = PWM_TIMER;
	hpwm.Init.Prescaler = PWM_PRESCALER-1;
	hpwm.Init.CounterMode = TIM_COUNTERMODE_UP;
	hpwm.Init.Period = _PWMPeriod(Carrier_Period);
	hpwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hpwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	/* TIM Initialize */
	HAL_TIM_Base_Init(&hpwm); // --> Software

	/* CLK Source Selection */
	CLK_Source.ClockSource = TIM_CLOCKSOURCE_INTERNAL; // --> Software
	HAL_TIM_ConfigClockSource(&hpwm, &CLK_Source); // --> Hardware

	/* PWM Initialize */
	HAL_TIM_PWM_Init(&hpwm); // --> Software

	/* TIM OVF Interrupt Config */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // --> Software
	HAL_TIMEx_MasterConfigSynchronization(&hpwm, &sMasterConfig); // --> Hardware

	/* PWM Features Load */
	// --> Software
	PWM_Config.OCMode = TIM_OCMODE_PWM1;
	PWM_Config.Pulse = 0; // Stopped
	PWM_Config.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWM_Config.OCFastMode = TIM_OCFAST_DISABLE;

	/* PWM Hardware Configuration */
	// --> Software
	HAL_TIM_PWM_ConfigChannel(&hpwm, &PWM_Config, TIM_CHANNEL_1);

	return;
}

/* Filtered Read Pin */
Click_State  CLICK_Detect(GPIO_TypeDef* GPIO_PORT, uint16_t GPIO_PIN){

	Button_State button_state = Non_Pressed;
	static uint8_t state = 0;

	/* Rising Edge Detection FSM */
	switch(state){
		case 0: // Non Pressed
			button_state = (HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN)==SWITCH_ON)?	Pressed:Non_Pressed;
			if(button_state == Pressed){
				debounce = DeBounce_Delay;
				state = 1;
			}
			break;
		case 1: // Pressed
			if(debounce == 0){
				button_state = (HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN)==SWITCH_ON)?	Pressed:Non_Pressed;
				if(button_state == Non_Pressed){
					state = 0;
					return(Clicked);
				}
			}
			break;
		default:
			state = 0;
			break;
	}

	return(Non_Clicked);
}

/* ADC Configuration */
void ADC_Config(void){

	ADC_ChannelConfTypeDef sConfig = {0};

	hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.ScanConvMode = DISABLE;
	hadc.Init.ContinuousConvMode = ENABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.NbrOfConversion = 1;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	HAL_ADC_Init(&hadc);

	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	return;
}

/* ADC Read */
float ADC_Read(void){
	uint32_t counts = 0;

	HAL_ADC_Start(&hadc);
	// Poll for 100us
	HAL_ADC_PollForConversion(&hadc, 100);
	// Read Value
	counts = HAL_ADC_GetValue(&hadc);
	// ADC Stop
	HAL_ADC_Stop(&hadc);
	// Vooltage
	return(counts * (ADC_RES));

}

/* TIM3 IRQ Definition */
void TIM3_IRQHandler(void)
{
	/* SAMPLING */
  if(sampling > 0) sampling--;
  else sampling = ADC_Sampling_Period;

  /* TIM2 IT CATCH */
  HAL_TIM_IRQHandler(&hbasetim); // --> Hardware

  return;
}

/* Private Function Definitions ------------*/
