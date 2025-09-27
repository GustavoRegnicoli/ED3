#include "lpc17xx_gpio.h"
    void GPIO_Config(void) {
        // P0.22 como salida
        GPIO_SetDir(0, (1<<22), 1);  // (puerto, máscara, 1=output)

        // P2.10 como entrada
        GPIO_SetDir(2, (1<<10), 0);
    }

#include "lpc17xx_gpio.h"
    void INTGPIO_Config(void) {
        // Config P2.10 para generar interrupción por flanco
        GPIO_IntCmd(2, (1<<10), 1);   // Habilita int. en puerto 2, pin 10, flanco
        NVIC_EnableIRQ(EINT3_IRQn);   // Todas las interrupciones GPIO caen en EINT3_IRQn
    }
    void EINT3_IRQHandler(void) {
        if (GPIO_GetIntStatus(2, 10, 1)) {  // chequea si fue flanco en P2.10
            GPIO_ClearInt(2, (1<<10));      // limpiar flag
            // acción
        }
    }

#include "lpc17xx_exti.h"
#include "lpc17xx_pinsel.h"
    void EINT_Config(void) {
        // Configura pin P2.10 como EINT0
        PINSEL_CFG_Type PinCfg;
        PinCfg.Funcnum = 1;
        PinCfg.Portnum = 2;
        PinCfg.Pinnum  = 10;
        PinCfg.Pinmode = 0;
        PinCfg.OpenDrain = 0;
        PINSEL_ConfigPin(&PinCfg);

        EXTI_InitTypeDef exti_cfg;
        exti_cfg.EXTI_Line = EXTI_EINT0;
        exti_cfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
        exti_cfg.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
        EXTI_Config(&exti_cfg);

        NVIC_EnableIRQ(EINT0_IRQn);
    }
    void EINT0_IRQHandler(void) {
        EXTI_ClearEXTIFlag(EXTI_EINT0);
        // acción
    }

#include "lpc17xx_timer.h"
    void Timer0_Config(void) {
        TIM_TIMERCFG_Type cfg;
        cfg.PrescaleOption = TIM_PRESCALE_USVAL;
        cfg.PrescaleValue  = 1;  // 1us por tick

        TIM_MATCHCFG_Type match;
        match.MatchChannel = 0;
        match.IntOnMatch   = ENABLE;
        match.ResetOnMatch = ENABLE;
        match.StopOnMatch  = DISABLE;
        match.MatchValue   = 1000;  // 1 ms

        TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfg);
        TIM_ConfigMatch(LPC_TIM0, &match);

        NVIC_EnableIRQ(TIMER0_IRQn);
        TIM_Cmd(LPC_TIM0, ENABLE);
    }
    void TIMER0_IRQHandler(void) {
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
        // acción cada 1ms
    }

#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"

    void ADC_Config(void) {
        PINSEL_CFG_Type PinCfg;
        PinCfg.Funcnum   = 1; // P0.23 = AD0.0
        PinCfg.Portnum   = 0;
        PinCfg.Pinnum    = 23;
        PinCfg.Pinmode   = 0;
        PinCfg.OpenDrain = 0;
        PINSEL_ConfigPin(&PinCfg);

        ADC_Init(LPC_ADC, 200000);    // 200 kHz
        ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
        ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, ENABLE);

        NVIC_EnableIRQ(ADC_IRQn);
        ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
    }

    void ADC_IRQHandler(void) {
        uint16_t val = ADC_ChannelGetData(LPC_ADC, 0);
    }

#include "lpc17xx_dac.h"
    void DAC_Config(void) {
        PINSEL_CFG_Type PinCfg;
        PinCfg.Funcnum   = 2; // P0.26 = AOUT
        PinCfg.Portnum   = 0;
        PinCfg.Pinnum    = 26;
        PinCfg.Pinmode   = 0;
        PinCfg.OpenDrain = 0;
        PINSEL_ConfigPin(&PinCfg);

        DAC_CONVERTER_CFG_Type dac_cfg;
        dac_cfg.CNT_ENA = DISABLE;
        dac_cfg.DMA_ENA = DISABLE;
        DAC_Init(LPC_DAC);
        DAC_ConfigDAConverterControl(LPC_DAC, &dac_cfg);
    }
    void DAC_Write(uint32_t value) {
        DAC_UpdateValue(LPC_DAC, value & 0x3FF); // 10 bits
    }
#include "lpc17xx_gpdma.h"

    void DMA_Config(void) {
        GPDMA_Init();

        GPDMA_Channel_CFG_Type dma_cfg;
        dma_cfg.ChannelNum = 0;
        dma_cfg.SrcMemAddr = (uint32_t)myBuffer;
        dma_cfg.DstMemAddr = (uint32_t)&(LPC_DAC->DACR);
        dma_cfg.TransferSize = 1000;
        dma_cfg.TransferWidth = 0;
        dma_cfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
        dma_cfg.DstConn = GPDMA_CONN_DAC;
        dma_cfg.SrcConn = 0;
        dma_cfg.DMALLI = 0;

        GPDMA_Setup(&dma_cfg);
        GPDMA_ChannelCmd(0, ENABLE);
    }

#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

    void UART0_Config(void) {
        PINSEL_CFG_Type PinCfg;
        PinCfg.Funcnum = 1; // P0.2 TXD0, P0.3 RXD0
        PinCfg.Portnum = 0;
        PinCfg.Pinmode = 0;
        PinCfg.OpenDrain = 0;
        PinCfg.Pinnum = 2;
        PINSEL_ConfigPin(&PinCfg);
        PinCfg.Pinnum = 3;
        PINSEL_ConfigPin(&PinCfg);

        UART_CFG_Type uartCfg;
        UART_ConfigStructInit(&uartCfg);
        UART_Init(LPC_UART0, &uartCfg);
        UART_TxCmd(LPC_UART0, ENABLE);
    }






