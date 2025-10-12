//Segundo parcial 2023
//EJERCICIO Nº2: (45 %)
//El ADC digitalice una señal analógica cuyo ancho de banda es de 16 khz.
//La señal analógica tiene una amplitud de pico máxima positiva de 3.3 voltios.
//Los datos deben ser guardados utilizando el Hardware GDMA en la primera mitad de
//la memoria SRAM ubicada en el bloque AHB SRAM -bank 0.
//Por otro lado se tiene una forma de onda como se muestra en la imagen(Diente sierra)
//Esta señal debe ser generada por una función y debe ser reproducida por el DAC
//desde la segunda mitad de AHB SRAM - bank 0 memoria utilizando DMA de tal forma
//que se logre un periodo de 614us logrando la máxima resolución y máximo rango de
//tensión.
//Durante operación normal se debe generar por el DAC la forma de onda mencionada
//como wave_form. Se debe indicar cuál es el mínimo incremento de tensión de salida
//de esa forma de onda.
//Cuando interrumpe una extint conectada a un pin, el ADC configurado debe completar
//el ciclo de conversión que estaba realizando, y ser detenido, a continuación se
//comienzan a sacar las muestras del ADC por el DAC utilizando DMA y desde las
//posiciones de memoria originales.
//Cuando interrumpe nuevamente en el mismo pin, se vuelve a repetir la señal del
//DAC generada por la forma de onda de wave_form previamente almacenada y se arranca
//de nuevo la conversión de datos del adc.
//Se alterna así entre los dos estados del sistema con cada interrupción externa.
//Suponer una frecuencia de core cclk de 80 Mhz.
//El código debe estar debidamente comentado.

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"

//la rampa va de 512 a 1023 entonces la maxima resolucion es:
//3.3/512=6.44mV
uint32_t wave_form[1023];
uint32_t valor=512;
uint16_t *BUFFER1 = (uint16_t*) 0x2007C000; // primera mitad
uint16_t *BUFFER2 = (uint16_t*) 0x2007E000; // segunda mitad
uint16_t funcion = 1;


void configPin(void);
void configEINT(void);
void configADC(void);
void configDAC(void);
void configDMA0(void);
void configDMA1(void);
void configDMA2(void);

int main (void){

	for(volatile uint32_t i=0;i<1024;i++){
		if(i<512){
			valor++;
			wave_form[i]=valor;
		}else{
			valor--;
			wave_form[i]=valor;
		}
	}

	configPin();
	configEINT();
	configADC();
	configDAC();
	configDMA0();
	configDMA1();
	configDMA2();

	while(1){
	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PINcfg;
	//EINT0 P2.10
	PINcfg.Portnum=2;
	PINcfg.Pinnum=10;
	PINcfg.Funcnum=1;
	PINcfg.Pinmode=PINSEL_PINMODE_PULLDOWN;
	PINcfg.OpenDrain=PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&PINcfg);
	//ADC0.0 P0.23
	PINcfg.Portnum=0;
	PINcfg.Pinnum=23;
	PINcfg.Funcnum=1;
	PINcfg.Pinmode=0;
	PINSEL_ConfigPin(&PINcfg);
	//DAC.0 P0.26
	PINcfg.Pinnum=26;
	PINSEL_ConfigPin(&PINcfg);
}
void configEINT(void){
	EXTI_InitTypeDef EINTcfg;
	EINTcfg.EXTI_Line=EXTI_EINT0;
	EINTcfg.EXTI_Mode=EXTI_MODE_EDGE_SENSITIVE;
	EINTcfg.EXTI_polarity=EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	EXTI_Config(&EINTcfg);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);

}

void EINT0_IRQHandler(){
	funcion++;
	if(funcion==1){
		GPDMA_ChannelCmd(0,ENABLE);
		GPDMA_ChannelCmd(1,ENABLE);
		GPDMA_ChannelCmd(2,DISABLE);
	}
	if(funcion==2){
		GPDMA_ChannelCmd(0,DISABLE);
		GPDMA_ChannelCmd(1,DISABLE);
		GPDMA_ChannelCmd(2,ENABLE);
	}

}

void configADC(void){
	ADC_Init(LPC_ADC,32000);
	ADC_StartCmd(LPC_ADC,ADC_START_CONTINUOUS);
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,ENABLE);
	//Habilito int para apargarlo en caso necesario

}

void ADC_IRQHandler(){

	if((funcion==2)){
		if(ADC_ChannelGetStatus(LPC_ADC,0,1)){
			ADC_ChannelCmd(LPC_ADC,0,DISABLE);
			funcion=0;
		}

	}
}

void configDAC(void){
	DAC_CONVERTER_CFG_Type DACcfg;
	DACcfg.DBLBUF_ENA=0;
	DACcfg.CNT_ENA=1;
	DACcfg.DMA_ENA=1;
	DAC_Init(LPC_DAC);
	DAC_ConfigDAConverterControl(LPC_DAC,&DACcfg);
	//Para un T=614us_f=1629Hz
	//Para una resolucion maxima necesito 1024 muestras
	//time_out=(Cclk/4)/(1629*1024)=15
	DAC_SetDMATimeOut(LPC_DAC,15);

}
void configDMA0(void){
	//ADC a memoria SRAM0 1parte
	GPDMA_LLI_Type LLIcfg;
	LLIcfg.SrcAddr=(uint32_t)&LPC_ADC->ADDR0;
	LLIcfg.DstAddr=(uint32_t)BUFFER1;
	LLIcfg.NextLLI=(uint32_t)&LLIcfg;
	LLIcfg.Control=4096
			| (1<<18) //16 bit
			| (1<<21) //16 bit
			| (1<<27) //incremento destino
			;

	GPDMA_Channel_CFG_Type DMAcfg;
	DMAcfg.ChannelNum=0;
	DMAcfg.TransferSize=4096;	//SRAM01p de 8192 pero necesito 2 pos por dato
	DMAcfg.TransferWidth=16;	// ADC de 12bits
	DMAcfg.SrcMemAddr=(uint32_t)&LPC_ADC->ADDR0;
	DMAcfg.DstMemAddr=(uint32_t)BUFFER1;
	DMAcfg.TransferType=GPDMA_TRANSFERTYPE_P2M;
	DMAcfg.SrcConn=GPDMA_CONN_ADC;
	DMAcfg.DstConn=0;
	DMAcfg.DMALLI=(uint32_t)&LLIcfg;
	GPDMA_Init();
	GPDMA_Setup(&DMAcfg);
	GPDMA_ChannelCmd(0,ENABLE);
}
void configDMA1(void){
	//SRAM0 2parte a DAC
	GPDMA_LLI_Type LLIcfg;
	LLIcfg.SrcAddr=(uint32_t)BUFFER2;
	LLIcfg.DstAddr=(uint32_t)&LPC_DAC->DACR;
	LLIcfg.NextLLI=(uint32_t)&LLIcfg;
	LLIcfg.Control=4096
			| (1<<18) //16 bit
			| (1<<21) //16 bit
			| (1<<27) //incremento destino
			;

	GPDMA_Channel_CFG_Type DMAcfg;
	DMAcfg.ChannelNum=1;
	DMAcfg.TransferSize=4096;
	DMAcfg.TransferWidth=16;	// DAC de 10bits
	DMAcfg.SrcMemAddr=(uint32_t)BUFFER2;
	DMAcfg.DstMemAddr=(uint32_t)&LPC_DAC->DACR;
	DMAcfg.TransferType=GPDMA_TRANSFERTYPE_M2P;
	DMAcfg.SrcConn=GPDMA_CONN_DAC;
	DMAcfg.DstConn=0;
	DMAcfg.DMALLI=(uint32_t)&LLIcfg;
	GPDMA_Init();
	GPDMA_Setup(&DMAcfg);
	GPDMA_ChannelCmd(1,ENABLE);
}
void configDMA2(void){
	////SRAM0 1parte a DAC
	GPDMA_LLI_Type LLIcfg;
	LLIcfg.SrcAddr=(uint32_t)BUFFER1;
	LLIcfg.DstAddr=(uint32_t)&LPC_DAC->DACR;
	LLIcfg.NextLLI=(uint32_t)&LLIcfg;
	LLIcfg.Control=4096
			| (1<<18) //16 bit
			| (1<<21) //16 bit
			| (1<<26) //incremento fuente
			;

	GPDMA_Channel_CFG_Type DMAcfg;
	DMAcfg.ChannelNum=2;
	DMAcfg.TransferSize=4096;
	DMAcfg.TransferWidth=16;	// DAC de 10bits
	DMAcfg.SrcMemAddr=(uint32_t)BUFFER1;
	DMAcfg.DstMemAddr=(uint32_t)&LPC_DAC->DACR;
	DMAcfg.TransferType=GPDMA_TRANSFERTYPE_M2P;
	DMAcfg.SrcConn=GPDMA_CONN_DAC;
	DMAcfg.DstConn=0;
	DMAcfg.DMALLI=(uint32_t)&LLIcfg;
	GPDMA_Init();
	GPDMA_Setup(&DMAcfg);
	GPDMA_ChannelCmd(2,DISABLE);
}
