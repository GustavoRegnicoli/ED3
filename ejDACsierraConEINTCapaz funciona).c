/*
 * Se generar una señal en forma de diente de sierra (triangulo también)
 * Usamos timer para generar interrupciones cada x segundos y actualizar el valor de la salida del DAC
 * Al cabo de 1000 int. se reinicia la forma de onda
 * Con EINT0 aumentar frecuencia y EINT1 disminuir frecuencia (cambiar valor en timer)
 */

#include "lpc17xx_exti.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "LPC17xx.h"

uint32_t DAC_Sierra [1000]={};
//uint32_t DAC_Seno [1000] = {512,515,518,522,525,528,531,535,538,541,544,547,551,554,557,560,563,567,570,573,576,579,583,586,589,592,595,599,602,605,608,611,614,618,621,624,627,630,633,636,639,643,646,649,652,655,658,661,664,667,670,673,676,680,683,686,689,692,695,698,701,704,707,710,713,716,718,721,724,727,730,733,736,739,742,745,748,750,753,756,759,762,765,767,770,773,776,778,781,784,787,789,792,795,797,800,803,805,808,811,813,816,818,821,824,826,829,831,834,836,839,841,844,846,848,851,853,856,858,860,863,865,867,870,872,874,877,879,881,883,886,888,890,892,894,896,898,901,903,905,907,909,911,913,915,917,919,921,923,925,927,928,930,932,934,936,938,939,941,943,945,946,948,950,951,953,955,956,958,959,961,962,964,965,967,968,970,971,973,974,976,977,978,980,981,982,983,985,986,987,988,989,991,992,993,994,995,996,997,998,999,1000,1001,1002,1003,1004,1005,1006,1006,1007,1008,1009,1010,1010,1011,1012,1013,1013,1014,1014,1015,1016,1016,1017,1017,1018,1018,1019,1019,1020,1020,1020,1021,1021,1021,1022,1022,1022,1023,1023,1023,1023,1023,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1023,1023,1023,1023,1023,1022,1022,1022,1022,1021,1021,1021,1020,1020,1019,1019,1019,1018,1018,1017,1016,1016,1015,1015,1014,1014,1013,1012,1011,1011,1010,1009,1008,1008,1007,1006,1005,1004,1003,1002,1002,1001,1000,999,998,997,996,994,993,992,991,990,989,988,986,985,984,983,981,980,979,978,976,975,973,972,971,969,968,966,965,963,962,960,959,957,955,954,952,950,949,947,945,944,942,940,938,937,935,933,931,929,927,926,924,922,920,918,916,914,912,910,908,906,904,902,900,897,895,893,891,889,887,884,882,880,878,875,873,871,869,866,864,862,859,857,854,852,850,847,845,842,840,837,835,832,830,827,825,822,820,817,815,812,809,807,804,801,799,796,793,791,788,785,782,780,777,774,771,769,766,763,760,757,755,752,749,746,743,740,737,735,732,729,726,723,720,717,714,711,708,705,702,699,696,693,690,687,684,681,678,675,672,669,666,663,660,657,653,650,647,644,641,638,635,632,629,625,622,619,616,613,610,606,603,600,597,594,591,587,584,581,578,575,571,568,565,562,559,555,552,549,546,543,539,536,533,530,526,523,520,517,514,510,507,504,501,498,494,491,488,485,481,478,475,472,469,465,462,459,456,453,449,446,443,440,437,433,430,427,424,421,418,414,411,408,405,402,399,395,392,389,386,383,380,377,374,371,367,364,361,358,355,352,349,346,343,340,337,334,331,328,325,322,319,316,313,310,307,304,301,298,295,292,289,287,284,281,278,275,272,269,267,264,261,258,255,253,250,247,244,242,239,236,233,231,228,225,223,220,217,215,212,209,207,204,202,199,197,194,192,189,187,184,182,179,177,174,172,170,167,165,162,160,158,155,153,151,149,146,144,142,140,137,135,133,131,129,127,124,122,120,118,116,114,112,110,108,106,104,102,100,98,97,95,93,91,89,87,86,84,82,80,79,77,75,74,72,70,69,67,65,64,62,61,59,58,56,55,53,52,51,49,48,46,45,44,43,41,40,39,38,36,35,34,33,32,31,30,28,27,26,25,24,23,22,22,21,20,19,18,17,16,16,15,14,13,13,12,11,10,10,9,9,8,8,7,6,6,5,5,5,4,4,3,3,3,2,2,2,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,3,3,3,4,4,4,5,5,6,6,7,7,8,8,9,10,10,11,11,12,13,14,14,15,16,17,18,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,35,36,37,38,39,41,42,43,44,46,47,48,50,51,53,54,56,57,59,60,62,63,65,66,68,69,71,73,74,76,78,79,81,83,85,86,88,90,92,94,96,97,99,101,103,105,107,109,111,113,115,117,119,121,123,126,128,130,132,134,136,138,141,143,145,147,150,152,154,157,159,161,164,166,168,171,173,176,178,180,183,185,188,190,193,195,198,200,203,206,208,211,213,216,219,221,224,227,229,232,235,237,240,243,246,248,251,254,257,259,262,265,268,271,274,276,279,282,285,288,291,294,297,300,303,306,308,311,314,317,320,323,326,329,332,335,338,341,344,348,351,354,357,360,363,366,369,372,375,378,381,385,388,391,394,397,400,403,406,410,413,416,419,422,425,429,432,435,438,441,445,448,451,454,457,461,464,467,470,473,477,480,483,486,489,493,496,499,502,506,509,512};

uint32_t valorDAC =0;
uint32_t valor =0;
uint32_t indice =0;
uint32_t periodo =1000;
uint32_t inte0 =0;
uint32_t inte1 =0;
void configPin(void);
void configDAC(void);
void config_timer(void);
void configEINT(void);
void configSysTick(void);

int main (void)
{

	config_timer();
	configPin();
	configDAC();
	configEINT();
	configSysTick();

	while(1)
	{

	}

	return 0;
}



void configEINT(void){
	/*LPC_SC->EXTINT |=(1<<0);		//limpia bandera
	LPC_SC->EXTMODE |=(1<<0);		//int por flanco
	LPC_SC->EXTPOLAR |=(1<<0);		//alto
	NVIC_EnableIRQ(EINT0_IRQn);

	LPC_SC->EXTINT |=(1<<1);		//limpia bandera
	LPC_SC->EXTMODE |=(1<<1);		//int por flanco
	LPC_SC->EXTPOLAR |=(1<<0);		//alto
	NVIC_EnableIRQ(EINT1_IRQn);
*/
    EXTI_InitTypeDef exti_cfg;

    // ---------- EINT0 ----------
    exti_cfg.EXTI_Line = EXTI_EINT0;
    exti_cfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    exti_cfg.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
    EXTI_Config(&exti_cfg);
    EXTI_ClearEXTIFlag(EXTI_EINT0);
    NVIC_EnableIRQ(EINT0_IRQn);

    // ---------- EINT1 ----------
    exti_cfg.EXTI_Line = EXTI_EINT1;
    exti_cfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    exti_cfg.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
    EXTI_Config(&exti_cfg);
    EXTI_ClearEXTIFlag(EXTI_EINT1);
    NVIC_EnableIRQ(EINT1_IRQn);

}

void configSysTick(void){
	/*
	 * 1/Cclk______1cuenta
	 * 20e-3_______x=Cclk*0.02
	 */
	SysTick->LOAD=(SystemCoreClock/50)-1;//cuenta cada 0.02seg
	SysTick->VAL=0;
	SysTick->CTRL |=(0x7<<0);
}
//Probar antirebote
void SysTick_Handler(void){
	inte0=0;
	inte1=0;
}


void EINT0_IRQHandler(void) {
	LPC_SC->EXTINT |=(1<<0);//limpia bandera
	inte0++;
	if(inte0==1){

	periodo = periodo + 10;
	LPC_TIM0->MR0 = periodo;
	LPC_TIM0->IR = 1;  // limpiar flag
	if(periodo>=4294967295){//2^32 ver frecuencia limite
		periodo=4294967295;
	}

	}
}

void EINT1_IRQHandler(void) {
	LPC_SC->EXTINT |=(1<<1);		//limpia bandera
	inte1++;
	if(inte1==1){

	periodo = periodo - 10;
	LPC_TIM0->MR0 = periodo;
	LPC_TIM0->IR = 1;  // limpiar flag
	if(periodo<=100){
			periodo=100;
		}

		}
}

void configPin(){
	PINSEL_CFG_Type PinCfg;
	// AOUT on P0.26
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	PINSEL_CFG_Type EINT0Cfg;
		// EINT0 on P2.10
	EINT0Cfg.Funcnum = 1;
	EINT0Cfg.OpenDrain = 0;
	EINT0Cfg.Pinmode = 3;
	EINT0Cfg.Pinnum = 10;
	EINT0Cfg.Portnum = 2;
	PINSEL_ConfigPin(&EINT0Cfg);

	PINSEL_CFG_Type EINT1Cfg;
		// EINT1 on P2.11
	EINT1Cfg.Funcnum = 1;
	EINT1Cfg.OpenDrain = 0;
	EINT1Cfg.Pinmode = 3;
	EINT1Cfg.Pinnum = 11;
	EINT1Cfg.Portnum = 2;
	PINSEL_ConfigPin(&EINT1Cfg);
return;
}

void configDAC(){

    DAC_CONVERTER_CFG_Type dac_cfg;

    dac_cfg.CNT_ENA = ENABLE;   // habilita el contador del DAC
    dac_cfg.DMA_ENA = DISABLE;  // no usamos DMA en este caso

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &dac_cfg); // pasar puntero a la estructura

	return;
}
void TIMER0_IRQHandler(void){
	//MUESTRA LA SEÑAL
	//Interrupcion cada x segundos, modifica el valor del pin p0.26 ,aumentando el valor de DAC_Sierra
	DAC_Sierra [indice]=valor;
	DAC_UpdateValue(LPC_DAC, DAC_Sierra [indice]);
	/*
	DAC_Seno [indice]=valor;
	DAC_UpdateValue(LPC_DAC, DAC_Seno [indice]);
    */
	valorDAC=DAC_Sierra [indice];
	indice++;

	//Para señal triangular
	if(indice<500){
		valor++;
	}
	if(indice>500){
	valor--;
	}
	if(indice==1000){
		indice=0;
		valor=0;
	}

	return;
}

void config_timer(){
	TIM_TIMERCFG_Type	struct_config;
	TIM_MATCHCFG_Type	struct_match;

	struct_config.PrescaleOption	=	TIM_PRESCALE_USVAL;//mide en ms o ticks
	struct_config.PrescaleValue		=	1;//cada 1us incrementa el timer

	struct_match.MatchChannel		=	0;
	struct_match.IntOnMatch			=	ENABLE;
	struct_match.ResetOnMatch		=	ENABLE;
	struct_match.StopOnMatch		=	DISABLE;//cuando hay un match se detiene o no el controlador
	struct_match.ExtMatchOutputType	=	TIM_EXTMATCH_NOTHING;
	struct_match.MatchValue			=	periodo;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);

	TIM_Cmd(LPC_TIM0, ENABLE);//habilita timer0

	NVIC_EnableIRQ(TIMER0_IRQn);

	return;


}
