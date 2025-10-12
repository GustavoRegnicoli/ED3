//Segundo parcial 2022
//EJERCICIO Nº 1: (30 %)
//Programar el microcontrolador LPC1769 para que mediante su ADC digitalice  dos
//señales analógicas cuyos anchos de bandas son de 10 Khz cada una.
//Los canales utilizados deben ser el 2 y el 4 y los datos deben ser guardados en
//dos regiones de memorias distintas que permitan contar con los últimos 20 datos de
//cada canal. Suponer una frecuencia de core cclk de 100 Mhz.
//El código debe estar debidamente comentado.

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"

uint32_t datos2[20];
uint32_t datos4[20];

void configPin(void);
void configADC(void);
void configDMA1(void);
void configDMA2(void);
int main (void){

	configPin();
	configADC();
	configDMA1();
	configDMA2();
	while(1){
	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PinCfg;
	//AD0.2 P0.25
	PinCfg.Portnum=0;
	PinCfg.Pinnum=25;
	PinCfg.Funcnum=1;
	PinCfg.Pinmode = 0;
	PinCfg.OpenDrain = 0;
	PINSEL_ConfigPin(&PinCfg);

	//AD0.4 P1.30
	PinCfg.Portnum=1;
	PinCfg.Pinnum=30;
	PinCfg.Funcnum=3;
	PINSEL_ConfigPin(&PinCfg);
	return;
}
void configADC(void){
	ADC_Init(LPC_ADC,40000); //fm>=N2fmax
	ADC_StartCmd(LPC_ADC,ADC_START_CONTINUOUS);
	ADC_ChannelCmd(LPC_ADC,2,ENABLE);
	ADC_ChannelCmd(LPC_ADC,4,ENABLE);

	return;
}
void configDMA1(void){
	GPDMA_LLI_Type LLICfg;
	LLICfg.SrcAddr=(uint32_t)&LPC_ADC->ADDR2;
	LLICfg.DstAddr=(uint32_t)datos2;
	LLICfg.NextLLI=(uint32_t)&LLICfg;
	LLICfg.Control=20
			| (1<<18) //16 bit
			| (1<<21) //16 bit
			| (1<<27) //incremento destino
			;

	GPDMA_Channel_CFG_Type DMACfg;
	DMACfg.ChannelNum=0;
	DMACfg.TransferSize=20;
	DMACfg.TransferWidth=16;
	DMACfg.SrcMemAddr=(uint32_t)&LPC_ADC->ADDR4;
	DMACfg.DstMemAddr=(uint32_t)datos2;
	DMACfg.TransferType=GPDMA_TRANSFERTYPE_P2M;
	DMACfg.SrcConn=GPDMA_CONN_ADC;
	DMACfg.DstConn=0;
	DMACfg.DMALLI=(uint32_t)&LLICfg;
	GPDMA_Init();
	GPDMA_Setup(&DMACfg);
	GPDMA_ChannelCmd(0,ENABLE);


	return;
}

void configDMA2(void){
	GPDMA_LLI_Type LLICfg;
	LLICfg.SrcAddr=(uint32_t)&LPC_ADC->ADGDR;
	LLICfg.DstAddr=(uint32_t)datos4;
	LLICfg.NextLLI=(uint32_t)&LLICfg;
	LLICfg.Control=20
			| (1<<18) //16 bit
			| (1<<21) //16 bit
			| (1<<27) //incremento destino
			;

	GPDMA_Channel_CFG_Type DMACfg;
	DMACfg.ChannelNum=1;
	DMACfg.TransferSize=20;
	DMACfg.TransferWidth=16;
	DMACfg.SrcMemAddr=(uint32_t)&LPC_ADC->ADGDR;
	DMACfg.DstMemAddr=(uint32_t)datos4;
	DMACfg.TransferType=GPDMA_TRANSFERTYPE_P2M;
	DMACfg.SrcConn=GPDMA_CONN_ADC;
	DMACfg.DstConn=0;
	DMACfg.DMALLI=(uint32_t)&LLICfg;
	GPDMA_Init();
	GPDMA_Setup(&DMACfg);
	GPDMA_ChannelCmd(1,ENABLE);

	return;
}
