//La consigna es pasar los datos del adc a un banco de 1024 muestras, al minuto salta
//interrupcion del timer y se empiezan a sacar por DAC esas muestras

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpdma.h"


uint16_t bufferADC[1024];

void configPin(void);
void configDAC(void);
void configTIMER(void);
void configADC(void);
void configDMA(void);

int main (void)
{
    configPin();
    configTIMER();
    configADC();
    configDMA();
	while(1){
	}

	return 0;
}

void configDMA(void){
	// Config DMA: ADC → buffer
	GPDMA_Channel_CFG_Type dma_cfg;
    dma_cfg.ChannelNum = 0;
    dma_cfg.SrcMemAddr = 0;                          // no usamos memoria
    dma_cfg.DstMemAddr = (uint32_t)bufferADC;        // destino = buffer
    dma_cfg.TransferSize = 1024;
    dma_cfg.TransferWidth = 0;
    dma_cfg.TransferType = GPDMA_TRANSFERTYPE_P2M;   // periférico to memoria
    dma_cfg.SrcConn = GPDMA_CONN_ADC;                // fuente = ADC
    dma_cfg.DstConn = 0;
    dma_cfg.DMALLI = 0;

    GPDMA_Setup(&dma_cfg);
    GPDMA_ChannelCmd(0, ENABLE);
}

void configADC(void){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum   = 1; // P0.23 = AD0.0
	PinCfg.Portnum   = 0;
	PinCfg.Pinnum    = 23;
	PinCfg.Pinmode   = 0;
    PinCfg.OpenDrain = 0;
    PINSEL_ConfigPin(&PinCfg);

    ADC_Init(LPC_ADC, 200000);    // 200 kHz
    ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, ENABLE);

	ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
}

void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1000;//cada 1ms incrementa el timer

	struct_match.MatchChannel		=	0;
	struct_match.IntOnMatch			=	ENABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	struct_match.MatchValue			=	60000; // 1minuto

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);

	TIM_Cmd(LPC_TIM0, ENABLE);//habilita timer0

	NVIC_EnableIRQ(TIMER0_IRQn);

	return;

}

void TIMER0_IRQHandler(void){
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
	
	configDAC();
	return;
}

void configDAC(){

    DAC_CONVERTER_CFG_Type dac_cfg;

    dac_cfg.CNT_ENA = ENABLE;   // habilita el contador del DAC
    dac_cfg.DMA_ENA = DISABLE;  // no usamos DMA en este caso

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &dac_cfg); // pasar puntero a la estructura

	return;
}



