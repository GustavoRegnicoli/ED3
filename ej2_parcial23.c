/*
 * Utilizando interrupciones por GPIO realizar un código en C que permita, mediante 4 pines de
entrada GPIO, leer y guardar un número compuesto por 4 bits. Dicho número puede ser
cambiado por un usuario mediante 4 switches, los cuales cuentan con sus respectivas
resistencias de pull up externas. El almacenamiento debe realizarse en una variable del tipo
array de forma tal que se asegure tener disponible siempre los últimos 10 números elegidos
por el usuario, garantizando además que el número ingresado más antiguo, de este conjunto
de 10, se encuentre en el elemento 9 y el número actual en el elemento 0 de dicho array. La
interrupción por GPIO empezará teniendo la máxima prioridad de interrupción posible y cada
200 números ingresados deberá disminuir en 1 su prioridad hasta alcanzar la mínima posible.
Llegado este momento, el programa deshabilitará todo tipo de interrupciones producidas por
las entradas GPIO. Tener en cuenta que el código debe estar debidamente comentado.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

void ConfigGPIO(void);
void ConfigIntGPIO(void);


static uint8_t num [10] = {};
static uint32_t total =0;
static uint32_t prio =0;


int main (void){
	ConfigGPIO();
	ConfigIntGPIO();


	while(1){

	}
	return 0;
}

void ConfigGPIO(void){
	LPC_PINCON->PINSEL0 &=~(0xFF<<0);//funcion GPIO para P0[0:3]

}
void ConfigIntGPIO(void){
	LPC_GPIOINT->IO0IntEnR |=(0xF<<0);//int por flanco descendiente
	LPC_GPIOINT->IO0IntClr |=(0xF<<0);//limpio bandera
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_SetPriority(EINT3_IRQn,0);

}

void EINT3_IRQHandler(void){
	LPC_GPIOINT->IO0IntClr |=(0xF<<0);//limpio bandera

	for (int i=9; i>0; i--) {
	    num[i] = num[i-1];
	}
	num[0]= LPC_GPIO0->FIOPIN & (0xF<<0);
	total++;
	if(total%200==0){
		prio++;
		if(prio<32){
			NVIC_SetPriority(EINT3_IRQn,prio);
		}else{
			NVIC_DisableIRQ(EINT3_IRQn);
		}
	}
}
