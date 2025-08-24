/*
/*
 1. Una famosa empresa de calzados a incorporado a sus zapatillas 10 luces leds
comandadas por un microcontrolador LPC1769 y ha pedido a su grupo de
ingenieros que diseñen 2 secuencias de luces que cada cierto tiempo se vayan
intercalando (secuencia A - secuencia B- secuencia A- ... ). Como todavía no se
ha definido la frecuencia a la cual va a funcionar el CPU del microcontrolador, las
funciones de retardos que se incorporen deben tener como parametros de
entrada variables que permitan modificar el tiempo de retardo que se vaya a
utilizar finalmente. Se pide escribir el código que resuelva este pedido,
considerando que los leds se encuentran conectados en los puertos P0,0 al P0.9
 */

#include "LPC17xx.h"

void configGPIO();
void retardo();

uint32_t secA = 1111111111;
uint32_t secB = 0000000000;


int main(void){
	configGPIO();


	while(1){
		LPC_GPIO0 ->FIOPIN = secA;
		retardo();
		LPC_GPIO0 ->FIOPIN = secB;
		retardo();
	}
	return 0;
}


void configGPIO(){
	LPC_PINCON -> PINSEL0 &= ~(0x3FFFFF<<0);//Selecciono P0.0 a p0.10 funcion gpio
	LPC_GPIO0 -> FIODIR |= (0x3FF<<0);//Selecciono P0.0 a p0.10 como salidas
}
void retardo(){
	for(volatile int i =0;i<1000000;i++){}
}