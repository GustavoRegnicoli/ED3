/*
 * TICKET SENSOR (Sensor de ticket):
 * (HIGH) cuando se detecta un ticket válido
 * interrupción externa (EINT) en el pin P2.10 – P2.9.
 * BARRIER (Barrera):
 * Se levanta con un nivel alto durante una duración configurable x.
 * Está controlada por el pin P0.15
 * RED LED (LED rojo):
 * Se enciende cuando se detecta un ticket inválido.
 * Pin P1.5.
 * BUTTON (Botón):
 * Después de la primera pulsación, el sistema empieza a contar 3 segundos.
 * El número total de pulsaciones determina la duración x para levantar la barrera.
 * Pin P0.0 (con interrupción por GPIO).
 * Tabla de tiempos de la barrera según pulsaciones:
 * 0_5seg
 * 1_10seg
 * 2_20seg
 * 3_40seg
 * 4_5seg
 * Frecuencia del microcontrolador: 70MHz
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

void ConfigGPIO(void);
void ConfigEINT(void);
void ConfigIntGPIO(void);
void ConfigSysTick(void);
static uint32_t contadorBarrera=0;
static uint32_t contadorPulsador=0;
static uint8_t inte0=0;
static uint8_t inteGPIO=0;
static uint8_t pulsador=0;
static uint16_t tiempo=440;

int main(void){
	ConfigGPIO();
	ConfigEINT();
	ConfigIntGPIO();
	ConfigSysTick();

	while(1){
	}
	return 0;
}

void ConfigGPIO(void){
	LPC_PINCON->PINSEL0 &=~(3<<0);	//funcion GPIO en p0.0
	LPC_GPIO0->FIODIR &=~(1<<0);	//p0.0 como entrada
	LPC_PINCON->PINMODE0 |=(3<<0);	//Rpulldown en p0.0

	LPC_PINCON->PINSEL0 &=~(3<<30);	//funcion GPIO en p0.15
	LPC_GPIO0->FIODIR |=(1<<15);	//p0.15 como salida

	LPC_PINCON->PINSEL2 &=~(3<<8);	//funcion GPIO en p1.4
	LPC_GPIO1->FIODIR |=(1<<4);		//p0.15 como salida

	LPC_PINCON->PINSEL4 &=~(3<<20);	//funcion EINT0 en p2.10
	LPC_PINCON->PINSEL4 |=(1<<20);	//funcion EINT0 en p2.10
	LPC_PINCON->PINMODE4 |=(3<<20);	//Rpulldown en p2.10

	LPC_PINCON->PINSEL4 &=~(3<<18);	//funcion GPIO en p2.9 (valida ticket)
	LPC_GPIO0->FIODIR &=~(1<<9);	//p0.0 como entrada
	LPC_PINCON->PINMODE4 |=(3<<18);	//Rpulldown en p2.9
}
void ConfigEINT(void){
	LPC_SC->EXTINT |=(1<<0);		//limpia bandera
	LPC_SC->EXTMODE &=~(1<<0);		//int por nivel
	LPC_SC->EXTPOLAR |=(1<<0);		//alto
	NVIC_EnableIRQ(EINT0_IRQn);
}
void ConfigIntGPIO(void){
	LPC_GPIOINT->IO0IntEnR |=(1<<0);//int flanco subida
	LPC_GPIOINT->IO0IntClr |=(1<<0);//limpia bandera
	NVIC_EnableIRQ(EINT3_IRQn);
}
void ConfigSysTick(void){
	SysTick->LOAD=(SystemCoreClock/10)-1;//cuenta cada 0.1seg
	SysTick->VAL=0;
	SysTick->CTRL |=(0x7<<0);
}

void EINT0_IRQHandler(void){
	LPC_SC->EXTINT |=(1<<0);		//limpia bandera
	if(LPC_GPIO2->FIOPIN>>9&&1){
		LPC_GPIO0->FIOSET |=(1<<15);
		inte0=1;

	}else{
		LPC_GPIO0->FIOCLR |=(1<<15);
	}
}

void EINT3_IRQHandler(void){
	LPC_GPIOINT->IO0IntClr |=(1<<0);//limpia bandera
	pulsador++;
	inteGPIO=1;
	contadorPulsador=0;
}

void SysTick_Handler(void){

	if(inte0==1){
		if(contadorBarrera%tiempo==0){
			LPC_GPIO0->FIOCLR |=(1<<15); // apaga la barerraa en x tiempo
			contadorBarrera=0;
			inte0=0;
		}else{
			contadorBarrera++;
		}

	}

	if(inteGPIO==1){
			if(contadorPulsador%30==0){		//ventana 3 segundos

				if(pulsador==1){
					tiempo=50;
				}
				if(pulsador==2){
					tiempo=100;
				}
				if(pulsador==3){
					tiempo=200;
				}
				if(pulsador==4){
					tiempo=400;
				}
				if(pulsador>=5){
					tiempo=50;
				}

				contadorPulsador=0;
				inteGPIO=0;
			}else{
				contadorPulsador++;
			}
		}
}












