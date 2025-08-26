// Utilizando interrupciones por GPIO realizar un código en C que permita, mediante 4 pines de 
// entrada GPIO, leer y guardar un número compuesto por 4 bits. Dicho número puede ser 
// cambiado por un usuario mediante 4 switches, los cuales cuentan con sus respectivas 
// resistencias de pull up externas. El almacenamiento debe realizarse en una variable del tipo 
// array de forma tal que se asegure tener disponible siempre los últimos 10 números elegidos 
// por el usuario, garantizando además que el número ingresado más antiguo, de este conjunto 
// de 10, se encuentre en el elemento 9 y el número actual en el elemento 0 de dicho array. La 
// interrupción por GPIO empezará teniendo la máxima prioridad de interrupción posible y cada 
// 200 números ingresados deberá disminuir en 1 su prioridad hasta alcanzar la mínima posible. 
// Llegado este momento, el programa deshabilitará todo tipo de interrupciones producidas por 
// las entradas GPIO. Tener en cuenta que el código debe estar debidamente comentado.

#include "lpc17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_exti.h"

#define TRUE 1
#define FALSE 0

void pinConfig();
uint8_t readValue();
void addArray(uint8_t *arr, uint8_t newNumber);

uint8_t count = 0;
uint8_t currentPriority = 0;
uint8_t array[10] = {0,0,0,0,0,0,0,0,0,0};

int main(){
    SystemInit();
    pinConfig();
    while (TRUE)
    {
        
    }
    return 0;
}

void pinConfig(){
    LPC_GPIO0->FIODIR &= ~((uint32_t)0xF);              //Pin 0.0 al 0.3 como entradas
    LPC_GPIOINT->IO0IntEnF |= ((uint32_t)0xF);          //Pin 0.0 al 0.3 con interr de bajada
    NVIC_SetPriority(EINT3_IRQn, currentPriority);
    NVIC_EnableIRQ(EINT3_IRQn);
    return;
}

uint8_t readValue(){
    return GPIO_ReadValue(PINSEL_PORT_0) & (uint32_t)0xF;
}

void addArray(uint8_t *arr, uint8_t newNumber){         //Funcion que coloca el nuevoNum en la pos 0
    for(int i = 8; i>=0 ; i--){
        arr[i+1] = arr[i];
    }
    arr[0] = newNumber;
    return;
} 

void EINT3_IRQHandler(){
    addArray(array, readValue());                   //Añade al array el nuevo num ingresado
    count ++;
    if(count == 200){
        count = 0;
        currentPriority++;
        if(currentPriority == 31){
            NVIC_DisableIRQ(EINT3_IRQn);
        }else{
            NVIC_SetPriority(EINT3_IRQn, currentPriority);
        }
    }
    return;
}