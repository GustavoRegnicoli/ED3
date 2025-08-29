/*Medir tiempo de un pendulo al pasar por 2 posiciones
 */

#include "LPC17xx.h"

void configEINT(void);
static uint32_t contador=0;
static uint32_t s1=0;
static uint32_t s2=0;
static uint32_t s3=0;
static uint32_t s4=0;

int main (void){

	SysTick_Config(100000);
	configEINT();

	while(1){

	}
}


void configEINT(void){
	//EINT0
	LPC_PINCON->PINSEL4 &=~(3<<20);
	LPC_PINCON->PINSEL4 |=(1<<20);
	LPC_SC->EXTINT |=(1<<0);	//Limpia bandera int
	LPC_SC->EXTMODE |=(1<<0);	//Int por flanco
	LPC_SC->EXTPOLAR |=(1<<0);	//Int por flanco ascendente
	NVIC_EnableIRQ(EINT0_IRQn);

}

void SysTick_Handler(void){
	contador++;
}
void EINT0_IRQHandler(void){
	LPC_SC->EXTINT |=(1<<0);	//Limpia bandera int
	flanco1=contador;
}
void EINT1_IRQHandler(void){ // Inicia parpadeo
	flanco2=contador;
}
