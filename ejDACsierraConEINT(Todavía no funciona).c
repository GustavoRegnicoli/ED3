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


__IO uint32_t adc_value;

uint32_t DAC_Sierra [1000]={};
uint32_t valor =0;
uint32_t indice =0;
uint32_t frecuencia =100;

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

	frecuencia=frecuencia-10;
	config_timer();
}

void EINT1_IRQHandler(void) {

    frecuencia=frecuencia+10;
    config_timer();
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
	DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, DAC_CNT_ENA);  // habilita DAC

	return;
}
void TIMER0_IRQHandler(void){
	//MUESTRA SEÑAL
	//Interrupcion cada x segundos, modifica el valor del pin p0.26 ,aumentando el valor de DAC_Sierra
	
	DAC_Sierra [indice]=valor;
	DAC_UpdateValue(LPC_DAC, DAC_Sierra [indice]);
	indice++;
	
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
	struct_match.MatchValue			=	frecuencia;

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



	LPC_SC->EXTMODE |= (1 << 0); // 1 = flanco para EINT0



	//LPC_SC->EXTPOLAR |= (1 << 0);  // 1 = flanco ascendente (rising edge)

	LPC_SC->EXTPOLAR &= ~(1 << 0); // 0 = flanco descendente (falling edge)



	LPC_SC->EXTINT = (1 << 0);  // Limpiar cualquier interrupción pendiente



	// Habilitar EINT0 en NVIC

	NVIC_EnableIRQ(EINT0_IRQn);

}



void EINT1_Init(void) {

    // 1. Configurar P2.11 como EINT1

    LPC_PINCON->PINSEL4 &= ~(3 << 22);   // limpiar bits 23:22

    LPC_PINCON->PINSEL4 |=  (1 << 22);   // seleccionar función EINT1



    // 2. Configurar resistores: sin pull-up ni pull-down

    LPC_PINCON->PINMODE4 &= ~(3 << 22);  // limpiar

    LPC_PINCON->PINMODE4 |=  (3 << 22);  // '10' = inactive (sin resistores)



    // 3. Configurar como sensible a flanco descendente

    LPC_SC->EXTMODE  |=  (1 << 1);  // 1 = edge sensitive

    LPC_SC->EXTPOLAR &= ~(1 << 1);  // 0 = falling edge



    // 4. Habilitar interrupción en NVIC

    NVIC_EnableIRQ(EINT1_IRQn);

}
