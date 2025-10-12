//Segundo parcial 2022
//EJERCICIO Nº2: (45 %)
//Utilizando el timer0, un dac, interrupciones y el driver del LPC1769 , escribir un
//código que permita generar una señal triangular periódica simétrica, que tenga el
//mínimo periodo posible, la máxima excursión de voltaje pico a pico posible y el
//mínimo incremento de señal posible por el dac. Suponer una frecuencia de core
//cclk de 100 Mhz.
//El código debe estar debidamente comentado.

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"

//Para tener maxima resolucion de 0 a 3.3 tomo 1024 muestras
//entonces al tener 2 cuadrantes indice =2048
uint16_t DACtriangular[2048];
uint16_t indice=0;
uint16_t valor=0;

void configPin(void);
void configDAC(void);
void configTimer(void);

int main (void){

	configPin();
	configDAC();
	configTimer();

	while(1){
	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PinCfg;
	//AOUT P0.26
	PinCfg.Portnum=0;
	PinCfg.Pinnum=26;
	PinCfg.Funcnum=2;
	PinCfg.Pinmode = 0;
	PinCfg.OpenDrain = 0;
	PINSEL_ConfigPin(&PinCfg);

	return;
}
void configDAC(void){
	DAC_CONVERTER_CFG_Type DACcfg;
	DACcfg.DBLBUF_ENA=0;
	DACcfg.CNT_ENA=1;
	DACcfg.DMA_ENA=0;
	DAC_ConfigDAConverterControl(LPC_DAC,&DACcfg);
	DAC_Init(LPC_DAC);

	return;
}
void configTimer(void){
	TIM_TIMERCFG_Type PRcfg;
	PRcfg.PrescaleOption=TIM_PRESCALE_USVAL;
	PRcfg.PrescaleValue=1; //cada 1us

	TIM_MATCHCFG_Type MRcfg;
	MRcfg.MatchChannel=0;
	MRcfg.IntOnMatch=ENABLE;
	MRcfg.StopOnMatch=DISABLE;
	MRcfg.ResetOnMatch=ENABLE;
	MRcfg.ExtMatchOutputType=TIM_EXTMATCH_TOGGLE;
	MRcfg.MatchValue=1;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE, &PRcfg);
	TIM_ConfigMatch(LPC_TIM0,&MRcfg);

	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

//Cada 2us de cambia el valor del DAC en este caso el periodo es de 40us
void TIMER0_IRQHandler(void){
	TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);//limpio bandera
	DACtriangular[indice]=valor;

	if(indice<1024){
		valor++;
		indice++;
	}else{
		valor--;
		indice++;
	}
	if(indice>=2048){
		indice=0;
	}

	DAC_UpdateValue(LPC_DAC,DACtriangular[indice]);


}
