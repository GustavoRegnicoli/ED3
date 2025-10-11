//Medir con capture la frecuencia de una onda cuadrada de 1a20kHz
//Cambiar de rango si se tiene 10 muestras consecutivas
//1a5kHz
//6a10kHz
//11a15kHz
//16a20kHz
//Con DMA reproducir la onda en P0[0a3]

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"


uint32_t periodo=0;
uint32_t rango1=0;
uint32_t rango2=0;
uint32_t rango3=0;
uint32_t rango4=0;
uint8_t rangoActual=0;
void configPin(void);
void configTimer(void);
void configDMA(void);

int main (void)
{

	configPin();
	configTimer();
	configDMA();
	while(1)
	{

	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Portnum=0;
	PinCfg.Pinnum=0;
	PinCfg.Funcnum=0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum=1;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum=2;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum=3;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(0,1<<0,1);
	GPIO_SetDir(0,1<<1,1);
	GPIO_SetDir(0,1<<2,1);
	GPIO_SetDir(0,1<<3,1);
    //CAP0.0 P1.26
	PinCfg.Portnum=1;
	PinCfg.Pinnum=26;
	PinCfg.Funcnum=3;
	PINSEL_ConfigPin(&PinCfg);




	return;
}


void configTimer(void){
	TIM_TIMERCFG_Type PRCfg;
	PRCfg.PrescaleOption=TIM_PRESCALE_USVAL;
	PRCfg.PrescaleValue=1; //cada 1us

	TIM_CAPTURECFG_Type CAPCfg;
	CAPCfg.CaptureChannel=0;
	CAPCfg.RisingEdge=ENABLE;
	CAPCfg.FallingEdge=DISABLE;
	CAPCfg.IntOnCaption=ENABLE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &PRCfg);
	TIM_ConfigCapture(LPC_TIM0, &CAPCfg);

	NVIC_EnableIRQ(TIMER0_IRQn);
	return;
}

void TIMER0_IRQ(){
	TIM_ClearIntPending(LPC_TIM0,TIM_CR0_INT);//limpia bandera
	periodo=TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
	if(periodo<200){
		rango1++;
		rango2=0;
		rango3=0;
		rango4=0;
	}
	else if(periodo<100){
		rango1=0;
		rango2++;
		rango3=0;
		rango4=0;
		}
	else if(periodo<66){
		rango1=0;
		rango2=0;
		rango3++;
		rango4=0;
		}
	else{
		rango1=0;
		rango2=0;
		rango3=0;
		rango4++;
	}

	if(rango1>10){
		rangoActual=1;
	}
	if(rango2>10){
		rangoActual=2;
	}
	if(rango3>10){
		rangoActual=3;
	}
	if(rango4>10){
		rangoActual=4;
	}
	return;
}



void configDMA(void){
	GPDMA_LLI_Type DMA_LLI1;

	DMA_LLI1.SrcAddr= (uint32_t)0x4000402C;
	DMA_LLI1.DstAddr= (uint32_t)&LPC_GPIO0->FIOPIN;
	DMA_LLI1.NextLLI= (uint16_t)&DMA_LLI1;
	DMA_LLI1.Control= 1000
					| (2<<18) //source width 16 bit
					| (2<<21) //dest. width 16 bit
					| (1<<27)
					;


	GPDMA_Channel_CFG_Type DMACfg;
	DMACfg.ChannelNum=0;
	DMACfg.TransferSize=1000;// ver las muestras que tiene que transferir
	DMACfg.TransferWidth=32;
	DMACfg.SrcMemAddr=(uint32_t)0x4000402C;//posicion de cap0.0
	DMACfg.DstMemAddr=(uint32_t)&LPC_GPIO0->FIOPIN;
	DMACfg.TransferType=GPDMA_TRANSFERTYPE_M2M;
	DMACfg.SrcConn=0;
	DMACfg.DstConn=0;
	DMACfg.DMALLI=0;
	GPDMA_Init();
	GPDMA_Setup(&DMACfg);
	GPDMA_ChannelCmd(0,ENABLE);
	return;
}

