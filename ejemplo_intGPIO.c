/*Parpadear LED rojo (P0.22) cada 0.5seg. con EIT2 por flanco ascendente detiene el
 * parpadeo. Con int de GPIO en P2.10 sigue el parpadeo
 */

#include "LPC17xx.h"

void configGPIO(void);
void configST(void);
void configIntGPIO(void);
void configEINT(void);

int main (void){

	configGPIO();
	configST();
	configIntGPIO();
	configEINT();

	while(1){

	}
}

void configGPIO(void){
	//Se supone que por defecto estan en funcion gpio
	LPC_GPIO0->FIODIR |=(1<<22);	//P0.22 como salida
	LPC_GPIO2->FIODIR &=~(1<<10);	//P2.10 como entrada
	LPC_PINCON->PINMODE4 |=(0x3<<20);	//Rpullup
	LPC_PINCON->PINSEL4 |=(1<<24);	//EINT2 P2.12
	LPC_GPIO0->FIOSET |=(1<<22);
}
void configST(void){
	SysTick_Config(10000000);
}
void configIntGPIO(void){
	LPC_GPIOINT->IO2IntEnR |=(1<<10);	//Int flanco subida en P2.11
	LPC_GPIOINT->IO2IntClr |=(1<<10);	//Limpio bandera
	NVIC_EnableIRQ(EINT3_IRQn);
}
void configEINT(void){
	LPC_SC->EXTINT |=(1<<2);	//Limpia bandera int
	LPC_SC->EXTMODE |=(1<<2);	//Int por flanco
	LPC_SC->EXTPOLAR &=~(0<<2);	//Int por flanco ascendente
	NVIC_EnableIRQ(EINT2_IRQn);
}
void SysTick_Handler(void){
	static int i =0;

	if(i<6){
		LPC_GPIO0->FIOCLR |=(1<<22);
	}else{
		LPC_GPIO0->FIOSET |=(1<<22);
	}
	i++;
	if(i==10){
		i=0;
	}
}
void EINT2_IRQHandler(void){ //Al presionar este boton detiene parpadeo
	SysTick->CTRL &=~(0x7<<0);
	LPC_GPIO0->FIOSET |=(1<<22);
	LPC_SC->EXTINT |=(1<<2);
}
void EINT3_IRQHandler(void){ // Inicia parpadeo
	SysTick->CTRL |=(0x7<<0);
	LPC_GPIOINT->IO2IntClr |=(1<<10);	//Limpio bandera
}
