#include "LPC17XX.h"


void confGPIO();
void confIntGPIO();
uint8_t DC =0;

int main (void){


	confGPIO();
	confIntGPIO();
	SysTick_Config(50000);// interrupcion cada 0.5mseg
	while(1){

	}}

void SysTick_Handler(void){
	static uint8_t i = 0;

	    if(i < DC){
	        LPC_GPIO0->FIOSET = (1<<22);   // Alto mientras dure el pulso
	    } else {
	        LPC_GPIO0->FIOCLR = (1<<22);   // Bajo el resto del periodo
	    }

	    i++;
	    if(i >= 40){   // 40 * 0,5 ms = 20 ms
	        i = 0;
	    }
}



void confGPIO(){
	LPC_GPIO0->FIODIR |= (1<<22);
	LPC_GPIO2->FIODIR &=~(1<<10);
	LPC_GPIO2->FIODIR &=~(1<<11);
	LPC_GPIO2->FIODIR &=~(1<<12);
	LPC_PINCON->PINMODE4 |=(0x3F<<20);
	return;

}
void confIntGPIO(){
	LPC_GPIOINT->IO2IntEnR |=(0x7<<10);
	LPC_GPIOINT->IO2IntClr |=(0x7<<10);

	NVIC_EnableIRQ(EINT3_IRQn);
	return;
}


void EINT3_IRQHandler(void){
	if(LPC_GPIOINT->IO2IntStatR&(1<<10)){
		DC=2; //dutycycle 1mseg
		LPC_GPIOINT->IO2IntClr|=(1<<10);
		return;
	}
	if(LPC_GPIOINT->IO2IntStatR&(1<<11)){
		DC=4; //1.5mseg
		LPC_GPIOINT->IO2IntClr|=(1<<11);
		return;

	}
	if(LPC_GPIOINT->IO2IntStatR&(1<<12)){
		DC=5; //2mseg
		LPC_GPIOINT->IO2IntClr|=(1<<12);
		return;

	}
}
