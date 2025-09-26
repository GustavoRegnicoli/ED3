/*
 * Se generar una señal en forma de diente de sierra (triangulo también)
 * Usamos timer para generar interrupciones cada x segundos y actualizar el valor de la salida del DAC
 * Al cabo de 1000 int. se reinicia la forma de onda
 * Con EINT0 aumentar frecuencia y EINT1 disminuir frecuencia (cambiar valor en timer)
 */

#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "LPC17xx.h"

uint32_t DAC_Sierra [1000]={};
uint32_t valor =0;
uint32_t indice =0;
uint32_t frecuencia =1000;

void configPin(void);
void configDAC(void);
void config_timer(void);
int main (void)
{

	config_timer();
	configPin();
	configDAC();


	while(1)
	{

	}

	return 0;
}

// Ver si es necesario configurar este pin
void configPin(){
	PINSEL_CFG_Type PinCfg;
	// AOUT on P0.26
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
return;
}

void configDAC(){
	DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, DAC_CNT_ENA);  // habilita DAC



	return;
}
void TIMER0_IRQHandler(void){
	//MUESTRA LA SEÑAL
	//Interrupcion cada x segundos, modifica el valor del pin p0.26 ,aumentando el valor de DAC_Sierra
	DAC_Sierra [indice]=valor;
	DAC_UpdateValue(LPC_DAC, DAC_Sierra [indice]);
	indice++;
	
	//Para señal triangular
	if(indice<500){
		valor++;
	}
	if(indice>500){
	valor--;
	}
	if(indice==1000){
		indice=0;
		valor=0;
	}

	return;
}

void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1;//cada 1us incrementa el timer

	struct_match.MatchChannel		=	0;
	struct_match.IntOnMatch			=	ENABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	struct_match.MatchValue			=	frecuencia;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);

	TIM_Cmd(LPC_TIM0, ENABLE);//habilita timer0

	NVIC_EnableIRQ(TIMER0_IRQn);

	return;


}

