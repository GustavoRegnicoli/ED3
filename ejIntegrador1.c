/*
* Programar el microcontrolador LPC1769, con una frecuencia de core cclk de 80MHz,
* para que mediante una interrupción externa en el pin P2.10 por flanco ascendente,
* se comande secuencialmente las siguientes funcionalidades:
* Mediante el ADC se debe digitalizar una señal analógica cuyo ancho de banda es de 80[KHz],
*  y su amplitud de pico máxima positiva es de 3,3V.
*  • Los datos deben ser guardados, cada 1 segundo, utilizando punteros (ADC_POINTER).
*  • Los datos deben ser guardados, cada 1 segundo, utilizando el hardware GPDMA (ADC_DMA).
*  En ambos casos, desde la primera mitad de la memoria SRAM ubicada en el bloque AHB SRAM-bank
*  0 de manera tal que permita almacenar todos los datos posibles que esta memoria permita,
*  mediante un buffer circular conservando siempre las ultimas muestras.
*
*  Mediante el hardware GPDMA se debe transferir los datos desde la primera mitad de la memoria
*  SRAM ubicada en el bloque AHB SRAM-bank 0 hacia la segunda mitad de dicha memoria, en donde
*  se deben promediar todos los datos, y dicho valor digitalizado promediado debe ser
*  transmitido por la UART0 en dos tramas de 1 byte que representan la parte entera y
*  decimal del mismo.
*/
#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"



uint16_t *ADC_POINTER = (uint16_t*) 0x2007C000; // primera mitad
uint16_t *ADC_POINTER2 = (uint16_t*) 0x2007E000; // segunda mitad
uint16_t adc_value;
uint8_t funcion =1;
uint32_t promedio =0;
void configPin(void);
void configADC(void);
void configDAC(void);
void configDMA1(void);
void configDMA2(void);
void configTimer(void);
int main (void)
{
	configPin();
	configADC();
	configTimer();
	while(1)
	{

	}

	return 0;
}

void configPin(){
	PINSEL_CFG_Type PinCfg;
	//AD0.0 on P0.23

	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	//EINT0 P2.10
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 3;// R pull down
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	EXTI_InitTypeDef eint0;
	eint0.EXTI_Line = EXTI_EINT0;
	eint0.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	eint0.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	EXTI_Config(&eint0);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);


return;
}

void EINT0_IRQHandler(void) {
	LPC_SC->EXTINT |=(1<<0);		//limpia bandera
	funcion++;
	if(funcion==1){
		configADC();
	}
	if(funcion==2){
		ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,DISABLE);
		NVIC_DisableIRQ(ADC_IRQn);
		configDMA1();
		}
	if(funcion==3){
			ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,DISABLE);
			NVIC_DisableIRQ(ADC_IRQn);
			GPDMA_ChannelCmd(0, DISABLE);
			configDMA2();

			funcion=0;
			}
}
void configADC(){

	ADC_Init(LPC_ADC, 160000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,ENABLE);
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);
	//habilitar convercion

	ADC_StartCmd(LPC_ADC,ADC_START_ON_MAT01); //
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}

void ADC_IRQHandler(void)
{
	adc_value =  ADC_ChannelGetData(LPC_ADC,0);//limpia int al leer el ADC

	*ADC_POINTER = adc_value;
	ADC_POINTER++;

	if (ADC_POINTER >= (uint16_t*)0x2007E00)
	    ADC_POINTER = (uint16_t*)0x2007C000;

}

void configTimer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1000;//cada 1ms incrementa el timer

	struct_match.MatchChannel		=	1;
	struct_match.IntOnMatch			=	DISABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
	struct_match.MatchValue			=	500; //cada 1s habilita adc

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);

	TIM_Cmd(LPC_TIM0, ENABLE); //habilita timer0


	return;
}



void configDMA1(void){
	GPDMA_LLI_Type DMA_LLI1;

	DMA_LLI1.SrcAddr= (uint32_t)&LPC_ADC->ADGDR;
	DMA_LLI1.DstAddr= (uint32_t)ADC_POINTER;
	DMA_LLI1.NextLLI= (uint32_t)&DMA_LLI1;
	DMA_LLI1.Control= 4096
					| (1<<18) //source width 16 bit
					| (1<<21) //dest. width 16 bit
					| (1<<27)
					;
	//DMA: ADC a buffer
	GPDMA_Channel_CFG_Type dma_cfg;
    dma_cfg.ChannelNum = 0;
    dma_cfg.SrcMemAddr = (uint32_t)&LPC_ADC->ADGDR;             // memoria de adc
    dma_cfg.DstMemAddr = (uint32_t)ADC_POINTER;        // destino puntero
    dma_cfg.TransferSize = 4096;
    dma_cfg.TransferWidth = 16;
    dma_cfg.TransferType = GPDMA_TRANSFERTYPE_P2M;   // periférico a memoria
    dma_cfg.SrcConn = GPDMA_CONN_ADC;                // fuente = ADC
    dma_cfg.DstConn = 0;
    dma_cfg.DMALLI = (uint32_t)&DMA_LLI1;

    GPDMA_Setup(&dma_cfg);
    GPDMA_ChannelCmd(0, ENABLE);
}

void configDMA2(void){

		//DMA: buffer a buffer
		GPDMA_Channel_CFG_Type dma_cfg;
	    dma_cfg.ChannelNum = 1;
	    dma_cfg.SrcMemAddr = (uint32_t)ADC_POINTER;             // lugar de memoria de adc
	    dma_cfg.DstMemAddr = (uint32_t)ADC_POINTER2;        // destino puntero
	    dma_cfg.TransferSize = 4096;
	    dma_cfg.TransferWidth = 16;
	    dma_cfg.TransferType = GPDMA_TRANSFERTYPE_M2M;   // memoria a memoria
	    dma_cfg.SrcConn = 0;
	    dma_cfg.DstConn = 0;
	    dma_cfg.DMALLI = 0;

	    GPDMA_Setup(&dma_cfg);
	    GPDMA_ChannelCmd(1, ENABLE);
	    NVIC_EnableIRQ(DMA_IRQn);
}


void DMA_IRQHandler (void)
{
	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1))	//Verifico que la interrupcion sea del canal 0 del DMA
		{

		if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1))	//Verifico fin de DMA
			{

			GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 1);	//Limpio el flag de interrupción
			promedio=0;
			for(volatile int i=0;i<8192;i++){
				promedio+=*ADC_POINTER2;
				ADC_POINTER2++;
			}
			promedio=promedio/8192;
			ADC_POINTER2 = (uint16_t*) 0x2007E000; // segunda mitad
		}
	}
}
