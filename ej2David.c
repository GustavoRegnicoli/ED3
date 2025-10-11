/*
* ADC una señal de maximo 30khz combierte 16 muestras las guarda por dma y saca el promedio
* por DAC
*/

#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"

uint32_t bufferADC1[16];
uint32_t bufferADC2[16];
uint32_t promedio;

void configPin(void);
void configADC(void);
void configDAC(void);
void configDMA(void);

int main (void)
{
	configPin();
	configADC();
	configDAC();
	configDMA();
	while(1)
	{

	}

	return 0;
}

void configPin(){
	PINSEL_CFG_Type PinCfg;
	//AD0.2 on P0.25

	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 25;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	//Salida DAC
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);



return;
}


void configADC(){

	ADC_Init(LPC_ADC, 60000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN2,DISABLE);	// El adc genera interrupcion con modo match (VER)
	ADC_ChannelCmd(LPC_ADC,2,ENABLE);
	//habilitar convercion

	ADC_StartCmd(LPC_ADC,ADC_START_CONTINUOUS); // El adc modo burts

	return;
}

void configDAC(){

    DAC_CONVERTER_CFG_Type dac_cfg;

    dac_cfg.CNT_ENA = ENABLE;   // habilita el contador del DAC
    dac_cfg.DMA_ENA = DISABLE;  // no usamos DMA en este caso

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &dac_cfg);

	return;
}

void configDMA(void){
	GPDMA_LLI_Type DMA_LLI1;
	GPDMA_LLI_Type DMA_LLI2;
	DMA_LLI1.SrcAddr= GPDMA_CONN_ADC;
	DMA_LLI1.DstAddr= (uint32_t)bufferADC2;
	DMA_LLI1.NextLLI= (uint32_t)&DMA_LLI2;
	DMA_LLI1.Control= 16
				| (2<<18) //source width 32 bit
				| (2<<21) //dest. width 32 bit
				| (1<<25)
				;

		DMA_LLI2.SrcAddr= GPDMA_CONN_ADC;
		DMA_LLI2.DstAddr= (uint32_t)bufferADC1;
		DMA_LLI2.NextLLI= (uint32_t)&DMA_LLI1;
		DMA_LLI2.Control= 16
					| (2<<18) //source width 32 bit
					| (2<<21) //dest. width 32 bit
					| (1<<26) //source increment
					;

	//DMA: ADC a buffer
	GPDMA_Channel_CFG_Type dma_cfg;
    dma_cfg.ChannelNum = 0;
    dma_cfg.SrcMemAddr = 0;                          // memoria no es fuente
    dma_cfg.DstMemAddr = (uint32_t)bufferADC1;        // destino = buffer
    dma_cfg.TransferSize = 16;
    dma_cfg.TransferWidth = 0;
    dma_cfg.TransferType = GPDMA_TRANSFERTYPE_P2M;   // periférico a memoria
    dma_cfg.SrcConn = GPDMA_CONN_ADC;                // fuente = ADC
    dma_cfg.DstConn = 0;                             // destino no es periferico
    dma_cfg.DMALLI = (uint32_t)&DMA_LLI1;

    GPDMA_Setup(&dma_cfg);
    GPDMA_ChannelCmd(0, ENABLE);
}




void DMA_IRQHandler (){
	for(volatile int i=0;i>16;i++){
		promedio+=bufferADC1[i];//ver como usar bufer 1 o 2
	}
	promedio=promedio/16;
	DAC_UpdateValue(LPC_DAC, promedio);

}
