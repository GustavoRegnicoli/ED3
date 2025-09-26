/*
* Se configura el timer1 para que cada 1 segundo el conversor convierta 1 vez
* Se usa MATCH y cuando se llega a este valor se habilira el ADC
*/

#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

#define _ADC_INT		ADC_ADINTEN2
#define _ADC_CHANNEL		ADC_CHANNEL_2

__IO uint32_t adc_value;

void ADC_IRQHandler(void);
void configPin(void);
void configADC(void);
void config_timer(void);
int main (void)
{

	config_timer();
	configPin();
	configADC();

	while(1)
	{

	}

	return 0;
}

void configPin(){
	PINSEL_CFG_Type PinCfg;
	/*
	 * Init ADC pin connect
	 * AD0.2 on P0.25
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 25;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
return;
}

// VER BIEN QUE FUNCIONES DE USAN EN ADC

void configADC(){

	/* Configuration for ADC:
	 *  select: ADC channel 2
	 *  ADC conversion rate = 200KHz
	 */
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,_ADC_INT,ENABLE);	// El adc genera interrupcion con modo match (VER)
	ADC_ChannelCmd(LPC_ADC,_ADC_CHANNEL,ENABLE);
	//habilitar convercion
	
	
	ADC_StartCmd(LPC_ADC,ADC_START_ON_MAT10); // El adc inicia con el MAT10
	
	NVIC_SetPriority(ADC_IRQn, (9));
	/* Enable ADC in NVIC */
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}
void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1;//cada 1us incrementa el timer

	struct_match.MatchChannel		=	0;
	struct_match.IntOnMatch			=	DISABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	struct_match.MatchValue			=	1000000;

	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM1, &struct_match);

	TIM_Cmd(LPC_TIM1, ENABLE);//habilita timer1

	NVIC_EnableIRQ(TIMER0_IRQn);

	return;
}

// Se habilita la interrupcion del adc cuando se llega al modo match
void ADC_IRQHandler(void)
{
	adc_value = 0;
	if (ADC_ChannelGetStatus(LPC_ADC,_ADC_CHANNEL,ADC_DATA_DONE))
	{
		adc_value =  ADC_ChannelGetData(LPC_ADC,_ADC_CHANNEL);
		NVIC_DisableIRQ(ADC_IRQn);
	}
}
