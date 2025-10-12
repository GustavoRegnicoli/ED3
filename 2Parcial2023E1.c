//Segundo parcial 2023
//EJERCICIO Nº 1: (35 %)
//Programar el microcontrolador LPC1769 en un código de lenguaje C para que,
//utilizando un timer y un pin de capture de esta placa sea posible demodular una
//señal PWM que ingresa por dicho pin (calcular el ciclo de trabajo y el periodo)
//y sacar una tensión continua proporcional al ciclo de trabajo a través del DAC de
//rango dinámivco 0-2V con un rate de actualización de 0,5s del promedio

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"


uint16_t DACvalue;
uint16_t flanco;
uint32_t ciclo=0;
uint32_t periodo=0;
uint32_t cicloPromedio=0;
uint32_t periodoPromedio=0;
uint32_t tiempo1=0;
uint32_t tiempo2=0;
uint32_t tiempo3=0;
uint32_t muestra=0;

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

	//CAP0.0 P1.26
	PinCfg.Portnum=1;
	PinCfg.Pinnum=26;
	PinCfg.Funcnum=4;
	PinCfg.Pinmode = 0;
	PinCfg.OpenDrain = 0;
	PINSEL_ConfigPin(&PinCfg);

	return;
}
void configDAC(void){
	DAC_CONVERTER_CFG_Type DACcfg;
	DACcfg.DBLBUF_ENA=0;
	DACcfg.CNT_ENA=0;
	DACcfg.DMA_ENA=0;
	DAC_ConfigDAConverterControl(LPC_DAC,&DACcfg);
	DAC_Init(LPC_DAC);

	return;
}
void configTimer(void){
	TIM_TIMERCFG_Type PRcfg;
	PRcfg.PrescaleOption=TIM_PRESCALE_USVAL;
	PRcfg.PrescaleValue=1; //cada 1us

	TIM_MATCHCFG_Type MATcfg;
	MATcfg.MatchChannel=0;
	MATcfg.IntOnMatch=ENABLE;
	MATcfg.StopOnMatch=DISABLE;
	MATcfg.ResetOnMatch=ENABLE;
	MATcfg.ExtMatchOutputType=TIM_EXTMATCH_TOGGLE;
	MATcfg.MatchValue=500000;//cada 0.5s

	TIM_CAPTURECFG_Type CAPcfg;
	CAPcfg.CaptureChannel=0;
	CAPcfg.RisingEdge=ENABLE;
	CAPcfg.FallingEdge=ENABLE;
	CAPcfg.IntOnCaption=ENABLE;

	TIM_Init(LPC_TIM0,TIM_TIMER_MODE, &PRcfg);
	TIM_ConfigMatch(LPC_TIM0,&MATcfg);
	TIM_ConfigCapture(LPC_TIM0,&CAPcfg);

	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}


void TIMER0_IRQHandler(void){

	if(TIM_GetIntStatus(LPC_TIM0,TIM_CR0_INT)){
		flanco++;
		if(flanco==1){
			tiempo1=TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
		}
		if(flanco==2){
			tiempo2=TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
		}
		if(flanco==3){
			tiempo3=TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
			ciclo=tiempo2-tiempo1;
			periodo=tiempo3-tiempo1;
			flanco=0;
		}



		TIM_ClearIntPending(LPC_TIM0,TIM_CR0_INT);
	}

	if(TIM_GetIntStatus(LPC_TIM0,TIM_MR0_INT)){
		//Si 3.3___1023
		//     2___620
		muestra++;
		if(muestra<10){
			ciclo+=ciclo;
			periodo+=periodo;
		}
		if(muestra>10){
			cicloPromedio=ciclo/10;
			periodoPromedio=periodo/10;
			DACvalue=620*ciclo/periodo;
			DAC_UpdateValue(LPC_DAC, DACvalue);
			cicloPromedio=0;
			periodoPromedio=0;
			muestra=0;
		}



		TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);
	}


}
