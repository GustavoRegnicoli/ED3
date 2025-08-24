#include "LPC17XX.h"


void confGPIO();
void confIntGPIO();
void retardo(uint32_t tiempo);
uint8_t inte =0;

int main (void){


	uint32_t tiempo;
	confGPIO();
	confIntGPIO();

	while(1){

		if(inte%2){
			tiempo=1000000;
		}else {
			tiempo=500000;
		}

		LPC_GPIO0 ->FIOCLR =(1<<22);
		retardo(tiempo);
		LPC_GPIO0 ->FIOSET =(1<<22);
		retardo(tiempo);

	}}

void confGPIO(){
	LPC_GPIO0->FIODIR |= (1<<22);
	LPC_GPIO2->FIODIR &=~(1<<10);
	LPC_PINCON->PINMODE4 |=(3<<20);
	return;

}
void confIntGPIO(){
	LPC_GPIOINT->IO2IntEnR |=(1<<10);
	LPC_GPIOINT->IO2IntClr |=(1<<10);
	NVIC_EnableIRQ(EINT3_IRQn);
	return;
}
void retardo(uint32_t tiempo){
	for(volatile uint32_t i =0;i<tiempo;i++){}
	return;
}

void EINT3_IRQHandler(void){
	if(LPC_GPIOINT->IO2IntStatR&(1<<10)){
		inte++;
	}
	LPC_GPIOINT->IO2IntClr|=(1<<10);

}
