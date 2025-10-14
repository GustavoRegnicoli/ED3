//En un consultorio médico se desea implementar, utilizando la LPC1769, un sistema
//que registre el tiempo promedio de atención de los pacientes y controle un cartel
//indicador con dos posibles mensajes:
//“OCUPADO”, “SIGUIENTE: X”
//
//El sistema cuenta con los siguientes elementos y condiciones:
//En la puerta de la oficina del doctor hay un sensor óptico conectado al pin Cap0.0.
//La primera interrupción indica el ingreso del paciente y la siguiente la salida.
//Con estas dos capturas se debe calcular el tiempo de atención individual y
//actualizar el promedio diario en una variable global promAtencionz.
//Los tiempos de atención están en el rango de 5-10 minutos.
//
//En la puerta general del consultorio hay otro sensor conectado al Timer 2, que
//lleva la cuenta de los pacientes que ingresan al consultorio y aguardan su turno.
//
//Se dispone de tres buffers con 16 datos de 8 bits cada uno:
//Dos consecutivos en la AHB SRAM Bank 0, que contienen los mensajes predeterminados:
//buffBusy, buffNext.
//Uno adicional en la AHB SRAM Bank 1, denominado buffTransmit.
//El cartel indicador recibe los mensajes desde este buffer de manera automática,
//por lo que solo hace falta colocar en él, el mensaje adecuado.
//
//El funcionamiento requerido es el siguiente:
//Cuando un paciente ingresa a la oficina del doctor, se debe transmitir buffBusy
//por DMA hacia buffTransmit.
//Cuando el paciente sale de la oficina del doctor, se debe enviar el contenido de
//buffNext por DMA hacia buffTransmit.

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"


uint16_t *buffBusy = (uint16_t*) 0x2007C000;
uint16_t *buffNext = (uint16_t*) 0x2007C010;
uint16_t *buffTransmit = (uint16_t*) 0x20080000;
uint32_t pacienteEntra;
uint32_t pacienteSale;
uint8_t flanco=0;
uint32_t tiempoAtencion;
uint32_t tiempoTotal;
uint32_t numeroPacientes;
uint32_t promAtencion;
uint32_t pacientesAtendidos=0;


void configPin(void);
void configTimer0(void);
void configTimer2(void);
void configDMA1(void);
void configDMA2(void);

int main (void){


	configPin();
	configTimer0();
	configTimer2();
	configDMA1();
	configDMA2();

	while(1){
	}

	return 0;
}

void configPin(void){
	PINSEL_CFG_Type PINcfg;
	//CAP0.0 P1.26
	PINcfg.Portnum=1;
	PINcfg.Pinnum=26;
	PINcfg.Funcnum=3;
	PINcfg.Pinmode=0;
	PINcfg.OpenDrain=0;
	PINSEL_ConfigPin(&PINcfg);
	//CAP2.0 Contador P0.4
	PINcfg.Portnum=0;
	PINcfg.Pinnum=4;
	PINcfg.Funcnum=3;
	PINSEL_ConfigPin(&PINcfg);

}



void configTimer0(void){
	TIM_TIMERCFG_Type TIMERcfg;
	TIMERcfg.PrescaleOption=TIM_PRESCALE_USVAL;
	TIMERcfg.PrescaleValue=1000000;	//Cuenta cada 1s

	TIM_CAPTURECFG_Type CAPcfg;
	CAPcfg.CaptureChannel=0;
	CAPcfg.RisingEdge=ENABLE;
	CAPcfg.FallingEdge=ENABLE;
	CAPcfg.IntOnCaption=ENABLE;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&TIMERcfg);
	TIM_ConfigCapture(LPC_TIM0,&CAPcfg);

	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);

}

void TIMER0_IRQHandler(){
	TIM_ClearIntPending(LPC_TIM0,TIM_CR0_INT);
	if(TIM_GetIntStatus(LPC_TIM0,TIM_CR0_INT)){
		flanco++;
		if(flanco==1){
			pacienteEntra=TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
			GPDMA_ChannelCmd(0,ENABLE);
		}else{
			pacienteSale=TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
			flanco=0;
			tiempoAtencion=pacienteSale-pacienteEntra;
			tiempoTotal+=tiempoAtencion;
			numeroPacientes=TIM_GetCaptureValue(LPC_TIM2,TIM_COUNTER_INCAP0);
            pacientesAtendidos++;
			promAtencion=tiempoTotal/pacientesAtendidos;
			//numeroPacientes se incrementa al ingresar un paciente
			GPDMA_ChannelCmd(1,ENABLE);
		}
	}
}

void configTimer2(void){
	TIM_COUNTERCFG_Type COUNTERcfg;
	COUNTERcfg.CounterOption=TIM_COUNTER_INCAP0;

	TIM_Init(LPC_TIM2,TIM_COUNTER_RISING_MODE,&COUNTERcfg);

	TIM_Cmd(LPC_TIM2, ENABLE);

}



void configDMA1(void){
	GPDMA_Channel_CFG_Type DMAcfg;
	DMAcfg.ChannelNum=0;
	DMAcfg.TransferSize=16;
	DMAcfg.TransferWidth=8;
	DMAcfg.SrcMemAddr=(uint32_t)buffBusy;
	DMAcfg.DstMemAddr=(uint32_t)buffTransmit;
	DMAcfg.TransferType=GPDMA_TRANSFERTYPE_M2M;
	DMAcfg.SrcConn=0;
	DMAcfg.DstConn=0;
	DMAcfg.DMALLI=0;
	GPDMA_Init();
	GPDMA_Setup(&DMAcfg);
	GPDMA_ChannelCmd(0,DISABLE);
}
void configDMA2(void){
	GPDMA_Channel_CFG_Type DMAcfg;
	DMAcfg.ChannelNum=1;
	DMAcfg.TransferSize=16;
	DMAcfg.TransferWidth=8;
	DMAcfg.SrcMemAddr=(uint32_t)buffNext;
	DMAcfg.DstMemAddr=(uint32_t)buffTransmit;
	DMAcfg.TransferType=GPDMA_TRANSFERTYPE_M2M;
	DMAcfg.SrcConn=0;
	DMAcfg.DstConn=0;
	DMAcfg.DMALLI=0;
	GPDMA_Init();
	GPDMA_Setup(&DMAcfg);
	GPDMA_ChannelCmd(1,DISABLE);
}
