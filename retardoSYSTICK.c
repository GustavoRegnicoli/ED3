/* Parpadear LED con systick
 *
 */


#include "LPC17xx.h"

void confGPIO(void);



int main (void){
	confGPIO();
	SysTick_Config(10000000);	//Int cada 100mseg

	while(1){

	}
}

void SysTick_Handler(void){
	static int i=0;
	if(i == 0){
	       LPC_GPIO0->FIOSET |= (1<<22);   // LED ON
    }
    if(i == 10){
        LPC_GPIO0->FIOCLR |= (1<<22);   // LED OFF
	   }

    i++;
    if(i > 20){
        i = 0;
	   }

}

void confGPIO(void){
	LPC_GPIO0 -> FIODIR |=(1<<22);
}
