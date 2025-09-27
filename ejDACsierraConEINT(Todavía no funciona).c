/*
 * Se generar una señal en forma de diente de sierra (triangulo también)
 * Usamos timer para generar interrupciones cada x segundos y actualizar el valor de la salida del DAC
 * Al cabo de 1000 int. se reinicia la forma de onda
 * Con EINT0 aumentar frecuencia y EINT1 disminuir frecuencia (cambiar valor en timer)
 */

#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"


uint32_t DAC_Sierra [1000]={};
uint32_t valorDAC =0;
uint32_t valor =0;
uint32_t indice =0;
uint32_t periodo =100000; // x*1e-6= T

void configPin(void);
void configDAC(void);
void config_timer(void);
void EINT0_Init(void);
void EINT1_Init(void);
int main (void)
{

	config_timer();
	configPin();
	configDAC();
	EINT0_Init();
	EINT1_Init();

	while(1)
	{

	}

	return 0;
}

void EINT0_IRQHandler(void) {

	periodo = periodo + periodo/10;
	LPC_TIM0->MR0 = periodo;
	LPC_TIM0->IR = 1;  // limpiar flag
}

void EINT1_IRQHandler(void) {

	periodo = periodo - periodo/10;
	LPC_TIM0->MR0 = periodo;
	LPC_TIM0->IR = 1;  // limpiar flag
}

// Ver si es necesario configurar este pin
void configPin(){
	PINSEL_CFG_Type PinCfg;
	// AOUT on P0.26
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
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
void TIMER0_IRQHandler(void){
	//MUESTRA SEÑAL
	//Interrupcion cada x segundos, modifica el valor del pin p0.26 ,aumentando el valor de DAC_Sierra

	DAC_Sierra [indice]=valor;
	DAC_UpdateValue(LPC_DAC, DAC_Sierra [indice]);
	indice++;
	valorDAC=DAC_Sierra [indice];
	//Para señal triangular
	if(indice<500){
		valor++;
	}
	if(indice>500){
	valor--;
	}

	if(indice==1000){
		indice=0;
		valor=0;
	}

	return;
}

void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1;//cada 1us incrementa el timer

	struct_match.MatchChannel		=	0;
	struct_match.IntOnMatch			=	ENABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	struct_match.MatchValue			=	periodo;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);

	TIM_Cmd(LPC_TIM0, ENABLE);//habilita timer0

	NVIC_EnableIRQ(TIMER0_IRQn);

	return;


}

/*
 * ERROR EN LA FORMA DE ONDA CON MUCHO RUIDO
 * VER CONFIG DE EINT (Estas sacadas de ej anteriores)
 */
void EINT0_Init(void){

	LPC_PINCON->PINSEL4 &= ~(3 << 20);  // Limpiar bits
	LPC_PINCON->PINSEL4 |=  (1 << 20);  // Función EINT0 (01)
	LPC_PINCON->PINMODE4 &= ~(3 << 20);
	LPC_PINCON->PINMODE4 |=  (3 << 20);  // Modo pull down
	LPC_SC->EXTMODE |= (1 << 0); // flanco para EINT0
    LPC_SC->EXTPOLAR |= (1 << 0); // flanco ascendente (falling edge)
	LPC_SC->EXTINT = (1 << 0);  // Limpiar cualquier interrupción pendiente
	// Habilitar EINT0 en NVIC

	NVIC_EnableIRQ(EINT0_IRQn);

}


void EINT1_Init(void) {

    // 1. Configurar P2.11 como EINT1
	LPC_PINCON->PINSEL4 &= ~(3 << 22);   // limpiar bits 23:22
    LPC_PINCON->PINSEL4 |=  (1 << 22);   // seleccionar función EINT1
    LPC_PINCON->PINMODE4 &= ~(3 << 22);  // limpiar
    LPC_PINCON->PINMODE4 |=  (3 << 22);  // Modo pull down
    LPC_SC->EXTMODE  |= (1 << 1);  // flanco
    LPC_SC->EXTPOLAR |= (1 << 1);  // flanco ascendente

    NVIC_EnableIRQ(EINT1_IRQn);

}
