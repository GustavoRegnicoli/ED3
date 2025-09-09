/*
 * Utilizando Systick e interrupciones externas escribir un código en C que cuente indefinidamente de 0
a 9. Un pulsador conectado a Eint0 reiniciará la cuenta a 0 y se mantendrá en ese valor mientras el
pulsador se encuentre presionado. Un pulsador conectado a Eint1 permitirá detener o continuar la
cuenta cada vez que sea presionado. Un pulsador conectado a Eint2 permitirá modificar la velocidad
de incremento del contador. En este sentido, cada vez que se presione ese pulsador el contador pasará
a incrementar su cuenta de cada 1 segundo a cada 1 milisegundo y viceversa. Considerar que el
microcontrolador se encuentra funcionando con un reloj (cclk) de 16 Mhz. El código debe estar
debidamente comentado y los cálculos realizados claramente expresados. En la siguiente figura se
muestra una tabla que codifica el display y el esquema del hardware sobre el que funcionará el
programa.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

void ConfigGPIO(void);
void ConfigEINT(void);
void ConfigSysTick(void);

static uint8_t numero [10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67};
static uint32_t contador =0;
static uint32_t inte2 =0;
static uint8_t i = 0;
static char velocidad =0;

int main (void){
	ConfigGPIO();
	ConfigEINT();
	ConfigSysTick();

	while(1){

	}
	return 0;
}

void ConfigGPIO(void){
	LPC_PINCON->PINSEL4 &=~(0x3FFF<<0);//funcion GPIO para P2[0:6]
	LPC_GPIO2->FIODIR |=(0x7F<<0);//salida P2[0:6]
	LPC_PINCON->PINSEL4 &= ~(0x3F<<20);
	LPC_PINCON->PINSEL4 |=(0b010101<<20);//funcion EINT012
	LPC_PINCON->PINMODE4 |=(3<<20);//R pulldown
	LPC_PINCON->PINMODE4 &=~(0xF<<22);//R pullup
}
void ConfigEINT(void){
	//EINT0
	LPC_SC->EXTINT |=(1<<0);//limpia bandera
	LPC_SC->EXTMODE &=~(1<<0);//int por nivel
	LPC_SC->EXTPOLAR |=(1<<0);//por nivel alto
	NVIC_EnableIRQ(EINT0_IRQn);

	//EINT1
	LPC_SC->EXTINT |=(1<<1);//limpia bandera
	LPC_SC->EXTMODE |=(1<<1);//int por flanco
	LPC_SC->EXTPOLAR &=~(1<<1);//por flanco bajada
	NVIC_EnableIRQ(EINT1_IRQn);

	//EINT2
	LPC_SC->EXTINT |=(1<<2);//limpia bandera
	LPC_SC->EXTMODE |=(1<<2);//int por flanco
	LPC_SC->EXTPOLAR &=~(1<<2);//por flanco bajada
	NVIC_EnableIRQ(EINT2_IRQn);

}
void ConfigSysTick(void){
	SysTick -> LOAD = (SystemCoreClock/10)-1;//cuenta cada 0.1 segundo
	SysTick -> VAL = 0;
	SysTick -> CTRL = (7<<0);

}

void SysTick_Handler(void){

	contador++;

	if(velocidad==0){
		if(contador%10==0){//para velocidad0 cambia cada 1 segundo
			LPC_GPIO2->FIOPIN = numero[i];
			i++;
		}
	}else{
		LPC_GPIO2->FIOPIN = numero[i];
		i++;
	}

	if(i==10){
		i=0;
	}

}

void EINT0_IRQHandler(void){
	LPC_SC->EXTINT |=(1<<0);//limpia bandera
	i = 0;
	LPC_GPIO2->FIOPIN = numero[i];
}

void EINT1_IRQHandler(void){
	LPC_SC->EXTINT |=(1<<1);//limpia bandera
	if(SysTick -> CTRL & (0x7<<0)){
		SysTick -> CTRL = (0<<0);
	}else{
		SysTick -> CTRL = (7<<0);
	}
}

void EINT2_IRQHandler(void){
	LPC_SC->EXTINT |=(1<<2);//limpia bandera
	inte2++;
	if(inte2%2){
		velocidad=0;
	}else{
		velocidad=1;
	}
}



