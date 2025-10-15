//POor un pin del ADC del LPC1969, ingresa una tension de rangp -2 a 2 con una
//frecuencia maxima de 20kHz, obtenidos de un sensor de presión diferencial.
//Se pide almacenar la señal en la posicón 0x2008E000 y con un tamaño de 1kB, una
//vez ocupado se vuelve a desde el comienzo. En funcion del promedio de todas las
//muestras obtenidas cada 100ms de la señal capturada, se debe tomar una decicion
//sobre 2 pines de salida de GPIO que genera señales cuadradas de 3.3V en fase
//(S1 y S2) con una frecuencia de 10kHz.
//
//1)
//Si el promedio esta entre -2 a 0 la señal S1 adelanta S2 con un angulo
//proporcional al valor promedio.
//2)
//Si el promedio esta entre 0 a 2 la señal S2 adelanta S1 con un angulo
//proporcional al valor promedio.

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"


uint16_t *ADCbuffer = (uint16_t*) 0x2008E000;

uint32_t adc_value=0;
uint32_t total=0;
uint32_t paso=0;
uint32_t pasos=0;
uint32_t promedio=0;
uint32_t proporcion=0;

void configPin(void);
void configTimer(void);
void configADC(void);

int main (void){


	configPin();
	configTimer();
	configADC();

	while(1){
	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PINcfg={0};
	//GPIO P0.0
	PINcfg.Portnum=0;
	PINcfg.Pinnum=0;
	PINcfg.Funcnum=0;
	PINSEL_ConfigPin(&PINcfg);
	//GPIO P0.1
	PINcfg.Pinnum=1;
	PINSEL_ConfigPin(&PINcfg);
	//ADC0.0 P0.23
	PINcfg.Pinnum=23;
	PINcfg.Funcnum=1;
	PINSEL_ConfigPin(&PINcfg);
}

void configADC(void){
	ADC_Init(LPC_ADC, 40000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,ENABLE);
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);

	ADC_StartCmd(LPC_ADC,ADC_START_CONTINUOUS); //
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}

void ADC_IRQHandler(void)
{
	adc_value =  ADC_ChannelGetData(LPC_ADC,0);//limpia int

	*ADCbuffer = adc_value;
	ADCbuffer++;
	total+=adc_value;

	if((uint32_t)ADCbuffer>=0x200BE400){
		ADCbuffer= (uint16_t *)0x2008E000;	//mas 1024 valores
		promedio=total;	// tomo muestra parcial, al pasar 100ms divido
		total=0;	//al completar 1kB resetea total
	}

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
	MRcfg.MatchValue=0;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE, &PRcfg);
	TIM_ConfigMatch(LPC_TIM0,&MRcfg);

	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
	return;

}

void TIMER0_IRQHandler(){
	pasos++;
	paso++;
	if(pasos%100000==0){	//cada 100ms calcula promedio de ultima transferencia
		promedio=promedio/1024;
	}
	//escalo el valor promedio para que este entre 0 y 50
    proporcion=50*promedio/4096;//4096 valores posibles del ADC

	if(adc_value<2028){	// tomo 3.3/2 para cada caso

		if(paso<50){	//tomo S1 como referencia con ciclo trabajo 50%
			GPIO_SetValue(0,1<<0);
		}else{
			GPIO_ClearValue(0,1<<0);
		}

		if(paso<50+proporcion){//desfaso s1 respecto a s2
			GPIO_SetValue(0,1<<1);
		}else{
			GPIO_ClearValue(0,1<<1);
		}

	}else{
		if(paso<50){	//toma S2 como referencia con ciclo trabajo 50%
			GPIO_SetValue(0,1<<1);
		}else{
			GPIO_ClearValue(0,1<<1);
		}
		if(paso<50+proporcion){
			GPIO_SetValue(0,1<<0);
		}else{
			GPIO_ClearValue(0,1<<0);
		}
	}

	if(paso==100){//al tener int cada 1u el periodo es 100u y f=10kHz
		paso=0;
	}


}
