/*
* Se configura el timer0.1 para que cada 100 msegundo el conversor convierta 1 vez
* Entrada sensor temperatura 0 a 100°C
* 0-40 verde
* 41-60 amarillo
* 61-100 rojo (10 muestras consecutivas)
*/

#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"


static uint32_t adc_value = 0;
uint16_t muestraRojo = 0;
#define maxVerde 1638
#define maxAmarillo 2458

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
	 * AD0.0 on P0.23
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	//Rojo
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 22;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(0, 1<<22, 1);

	//Verde
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 25;
	PinCfg.Portnum = 3;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(3, 1<<25, 1);


return;
}


void configADC(){

	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,ENABLE);	// El adc genera interrupcion con modo match (VER)
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);
	//habilitar convercion

	ADC_StartCmd(LPC_ADC,ADC_START_ON_MAT01); // El adc inicia con el MAT01

	NVIC_EnableIRQ(ADC_IRQn);
	return;
}
void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1000;//cada 1ms incrementa el timer

	struct_match.MatchChannel		=	1;
	struct_match.IntOnMatch			=	DISABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
	struct_match.MatchValue			=	50; //cada 100ms se genera flanco subida

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);

	TIM_Cmd(LPC_TIM0, ENABLE); //habilita timer0

	NVIC_EnableIRQ(TIMER0_IRQn);

	return;
}

// Se habilita la interrupcion del adc cuando se llega al modo match
void ADC_IRQHandler(void)
{
	adc_value =  ADC_ChannelGetData(LPC_ADC,0);//limpia int
	if(adc_value<=maxVerde){
		GPIO_SetValue(0, 1<<22);
		GPIO_ClearValue(3, 1<<25);
		muestraRojo=0;
	}
	else if(adc_value<=maxAmarillo){
			GPIO_ClearValue(0, 1<<22);
			GPIO_ClearValue(3, 1<<25);
			muestraRojo=0;
		}
	else if(muestraRojo++>=10){
			GPIO_ClearValue(0, 1<<22);
			GPIO_SetValue(3, 1<<25);
	}



}
