//Mediante el DAC:
//Se debe generar una función que represente la forma de onda de la figura,
//mediante 382 muestras, para ser reproducida por el DAC desde la memoria SRAM
//ubicada en el bloque AHB SRAM-bank 1 utilizando el hardware GPDMA de tal forma que
//se logre un periodo de 1s logrando la máxima resolución y máximo rango de tensión
//permitido.
//(Se debe indicar cual es el mínimo incremento de tensión de salida de dicha
//forma de onda).
//(DAC_WAVE).

//La resolucion maxima del DAC es 3.3/1024=3.22mV
//En este caso 3.3/95=34.73mV

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"


uint16_t *DAC_WAVE = (uint16_t*) 0x2007C000; // buffer en AHB SRAM-bank 1
uint16_t DAC_WAVE1C[96];


void configPin(void);
void configDAC(void);
void configDMA(void);

int main (void)
{
	//En primer lugar genero la funcion, en el 1er cuadrante 382/4=95.5
	for(volatile uint16_t i=0;i<95;i++){
		DAC_WAVE1C [i]=i;
	}
	//Escalo en los 4 cuadrantes con máxima resolución y máximo rango de tensión
    for(volatile uint16_t i=0;i<382;i++){
    	if(i<=95){
    		DAC_WAVE[i]=512+512*DAC_WAVE1C[i]/95;
    	}
    	if(i>95&&i<=191){
    		DAC_WAVE[i]=512+512*DAC_WAVE1C[191-i]/95;
    	}
    	if(i>191&&i<=286){
    		DAC_WAVE[i]=512-512*DAC_WAVE1C[i-191]/95;
    	}
    	if(i>286){
    		DAC_WAVE[i]=512-512*DAC_WAVE1C[382-i]/95;
    	}
    	if(DAC_WAVE[i]==1024){//Me aseguro se no pasarme
    		DAC_WAVE[i]=1023;
    	}
    }

	configPin();
	configDAC();
	configDMA();
	while(1)
	{

	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Portnum=0;
	PinCfg.Pinnum=26;
	PinCfg.Funcnum=3;
	PINSEL_ConfigPin(&PinCfg);

	return;
}
void configDAC(void){
	DAC_CONVERTER_CFG_Type DACCfg;
	DACCfg.CNT_ENA=1;
	DACCfg.DMA_ENA=1;
	DAC_Init(LPC_DAC);
	DAC_ConfigDAConverterControl(LPC_DAC,&DACCfg);
	uint32_t temp = 25000000/(1*382);//25M f del DAC Cclk/4 para T=1>f=1Hz
	DAC_SetDMATimeOut(LPC_DAC,temp);
	return;
}
void configDMA(void){
	GPDMA_Channel_CFG_Type DMACfg;
	DMACfg.ChannelNum=0;
	DMACfg.TransferSize=382;
	DMACfg.TransferWidth=16;
	DMACfg.SrcMemAddr=(uint32_t)DAC_WAVE;
//	DMACfg.DstMemAddr=(uint32_t)&LPC_DAC->DACR;//VER
	DMACfg.DstMemAddr=0;//VER
	DMACfg.TransferType=GPDMA_TRANSFERTYPE_M2P;
	DMACfg.SrcConn=0;
	DMACfg.DstConn=GPDMA_CONN_DAC;
	DMACfg.DMALLI=0;
	GPDMA_Init();
	GPDMA_Setup(&DMACfg);
	GPDMA_ChannelCmd(0,ENABLE);
	return;
}

