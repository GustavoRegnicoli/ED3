/**
 Este programa prende led RGB
 */
#include "LPC17xx.h"

int main(void){
	uint32_t tiempo;
	LPC_PINCON ->PINSEL1 &=~(0x3<<12);
	LPC_GPIO0 ->FIODIR |=(1<<22);

	LPC_GPIO2 ->FIODIR &=~(1<<10);// P2.10 como entrada
	LPC_PINCON ->PINMODE4 |=(3<<20);

    //uint32_t relojCPU = SystemCoreClock;
	while(1){
		if((LPC_GPIO2->FIOPIN)&(1<<10)){
			tiempo=1000000;
		}else{
			tiempo=4000000;
		}

		LPC_GPIO0 ->FIOCLR |=(1<<22);
		for(volatile uint32_t i=0;i<tiempo;i++);
		LPC_GPIO0 ->FIOSET |=(1<<22);
		for(volatile uint32_t i=0;i<tiempo;i++);




		/*LPC_GPIO3 ->FIOSET |=(1<<25);
			for(volatile uint32_t i=0;i<1000000;i++);
			LPC_GPIO3 ->FIOSET |=(1<<26);
			for(volatile uint32_t i=0;i<1000000;i++);
			LPC_GPIO0 ->FIOSET |=(1<<22);
			for(volatile uint32_t i=0;i<1000000;i++);
			LPC_GPIO3 ->FIOCLR |=(1<<25);
			for(volatile uint32_t i=0;i<1000000;i++);
			LPC_GPIO3 ->FIOCLR |=(1<<26);
			for(volatile uint32_t i=0;i<1000000;i++);
			LPC_GPIO0 ->FIOCLR |=(1<<22);
			for(volatile uint32_t i=0;i<1000000;i++);

*/



	}

	return 0;
}
