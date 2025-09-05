/*
 * Convertir cada 1 segundo en A0.0, 0a1 led azul, 1a2 led verde, 2a3 led rojo
 * mayor a 3 parpadean los 3
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void confGPIO(void);
void confADC(void);


uint16_t ADC0Value = 0;
static uint32_t contador = 0;
static uint32_t mayor3 = 0;
int main(void) {
confGPIO();
confADC();
SysTick_Config(SystemCoreClock / 10);
LPC_GPIO0->FIOSET = (1<<22);
LPC_GPIO3->FIOSET = (1<<26);
LPC_GPIO3->FIOSET = (1<<25);
while(1){}
return 0;
}

void confGPIO(void){

	LPC_GPIO0->FIODIR |= (1<<22);	//led rojo
	LPC_GPIO3->FIODIR |= (1<<26);	//led azul
	LPC_GPIO3->FIODIR |= (1<<25);	//led verde
return;
}

void SysTick_Handler(void){		//interrumpe cada 0.1 seg
	contador++;

	if(contador%10==0){
		LPC_ADC->ADCR &= ~(7<<24);  // limpiar START
		LPC_ADC->ADCR |= (1<<24);   // start now
	}


	if(mayor3==1){
		if(contador%2){
					LPC_GPIO0->FIOSET = (1<<22);
					LPC_GPIO3->FIOSET = (1<<26);
					LPC_GPIO3->FIOSET = (1<<25);
					}else{
						LPC_GPIO0->FIOCLR = (1<<22);
						LPC_GPIO3->FIOCLR = (1<<26);
						LPC_GPIO3->FIOCLR = (1<<25);
					}
	}
	return;
}


void confADC(void){
	LPC_SC->PCONP |= (1 << 12);
	LPC_ADC->ADCR |= (1 << 21); //habilita el ADC
	LPC_SC->PCLKSEL0 |= (3<<24);  //CCLK/8
	LPC_ADC->ADCR &=~(255 << 8);  //[15:8] CLKDIV
	//LPC_ADC->ADCR |=(255 << 8);  //[15:8] CLKDIV
	LPC_ADC->ADCR |= (1 << 0);   // channel
	LPC_ADC->ADCR &= ~(1<<16);
	//LPC_ADC->ADCR |= (1 << 16);   // burst
	LPC_PINCON->PINMODE1 |= (1<<15); //neither pull-up nor pull-down.
	LPC_PINCON->PINSEL1 |= (1<<14);	 //seleccionar función ADC en pinconnect block
	LPC_ADC->ADINTEN |= (1<<0);
	LPC_ADC->ADINTEN &=~ (1<<8);
	NVIC_EnableIRQ(ADC_IRQn);


return;
}
void ADC_IRQHandler(void)
{
	ADC0Value = ((LPC_ADC->ADDR0)>>4) & 0xFFF; //Variable auxiliar para observar el valor del registro de captura

	if (ADC0Value<1241){	// 0a1 prende el led azul
		LPC_GPIO0->FIOSET = (1<<22);
		LPC_GPIO3->FIOCLR = (1<<26);
		LPC_GPIO3->FIOSET = (1<<25);
	}

	if (ADC0Value>1241 && ADC0Value<2482){	// 1a2 prende el led verde
		LPC_GPIO0->FIOSET = (1<<22);
		LPC_GPIO3->FIOSET = (1<<26);
		LPC_GPIO3->FIOCLR = (1<<25);
	}
	if (ADC0Value>2482 && ADC0Value<3723){	// 2a3 prende el led rojo
		LPC_GPIO0->FIOCLR = (1<<22);
		LPC_GPIO3->FIOSET = (1<<26);
		LPC_GPIO3->FIOSET = (1<<25);
		}

	if (ADC0Value>=3723){	// mayor a 3 parpadea cada 0.1seg
		mayor3=1;
	}else{
		mayor3=0;
	}


return;
}
