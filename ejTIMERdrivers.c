/*
 * Parpadear un LED. 1s en alto y 1s en bajo
 */

#include "LPC17xx.h"


#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
// TODO: insert other definitions and declarations here

#define	OUTPUT	    (uint8_t) 1
#define INPUT	    (uint8_t) 0

#define PIN_22		((uint32_t)(1<<22))
#define PORT_ZERO	(uint8_t)	0
#define PORT_ONE	(uint8_t)	1
#define PORT_TWO	(uint8_t)	2
#define PORT_THREE	(uint8_t)	3

#define _ADC_INT		ADC_ADINTEN2
#define _ADC_CHANNEL		ADC_CHANNEL_2

void config_GPIO(void);
void config_timer(void);
void config_ADC(void);


int main(void) {

	config_GPIO();
	config_timer();
	config_ADC();
	GPIO_SetValue(PORT_ZERO,PIN_22);

	while(1) {}
    return 0 ;
}

void config_ADC(void){
	/*LPC_SC->PCONP |= (1 << 12);

	LPC_SC->PCLKSEL0 |= (3<<24);  //CCLK/8
	LPC_ADC->ADCR &=~(255 << 8);  //[15:8] CLKDIV
	//LPC_ADC->ADCR |=(255 << 8);  //[15:8] CLKDIV
	LPC_ADC->ADCR |= (1 << 0);   // channel
	LPC_ADC->ADCR |= (1 << 16);   // burst
	LPC_PINCON->PINMODE1 |= (1<<15); //neither pull-up nor pull-down.
	LPC_PINCON->PINSEL1 |= (1<<14);	 //seleccionar función ADC en pinconnect block
	LPC_ADC->ADINTEN |= (1<<0);
	LPC_ADC->ADINTEN &=~ (1<<8);
	NVIC_EnableIRQ(ADC_IRQn);
	*/
	
	/* Configuration for ADC:
		 *  select: ADC channel 2
		 *  ADC conversion rate = 200KHz
		 */
		ADC_Init(LPC_ADC, 200000);
		ADC_IntConfig(LPC_ADC,_ADC_INT,ENABLE);
		ADC_ChannelCmd(LPC_ADC,_ADC_CHANNEL,ENABLE);

		NVIC_SetPriority(ADC_IRQn, (9));
		/* Enable ADC in NVIC */
		NVIC_EnableIRQ(ADC_IRQn);
		return;
}
void ADC_IRQHandler(void)
{
}

}

void TIMER0_IRQHandler(void){
	//habilitar convercion
	ADC_StartCmd(LPC_ADC,ADC_START_NOW);
	return;
}
void config_GPIO(){
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
void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1;//cada 1ms incrementa el timer

	struct_match.MatchChannel		=	0;
	struct_match.IntOnMatch			=	ENABLE;
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
