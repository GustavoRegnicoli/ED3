#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"

// Array con valores precalculados 
uint32_t seno[1000] = {
   512, 515, 518, 522, /* ... hasta 1000 valores ... */ 508, 510
};

void DAC_Config(void) {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2; // P0.26 = AOUT
    PinCfg.Portnum = 0;
    PinCfg.Pinnum  = 26;
    PinCfg.Pinmode = 0;
    PinCfg.OpenDrain = 0;
    PINSEL_ConfigPin(&PinCfg);

    DAC_CONVERTER_CFG_Type dac_cfg;
    dac_cfg.CNT_ENA = ENABLE;   // usar timer interno del DAC
    dac_cfg.DMA_ENA = ENABLE;   // habilitar DMA
    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &dac_cfg);
}

void DMA_Config(void) {
    // Inicializar DMA
    GPDMA_Init();

    GPDMA_Channel_CFG_Type dma_cfg;
    dma_cfg.ChannelNum    = 0;  // usamos canal 0
    dma_cfg.SrcMemAddr    = (uint32_t)&seno;      // dirección origen: array
    dma_cfg.DstMemAddr    = (uint32_t)&(LPC_DAC->DACR); // destino: registro del DAC
    dma_cfg.TransferSize  = 1000;                 // cantidad de elementos
    dma_cfg.TransferWidth = 0;                    // tamaño (se deduce)
    dma_cfg.TransferType  = GPDMA_TRANSFERTYPE_M2P; // Memoria → Periférico
    dma_cfg.DstConn       = GPDMA_CONN_DAC;       // conexión al DAC
    dma_cfg.SrcConn       = 0;
    dma_cfg.DMALLI        = 0;

    // Configurar canal
    GPDMA_Setup(&dma_cfg);

    // Habilitar canal
    GPDMA_ChannelCmd(0, ENABLE);
}

int main(void) {
    DAC_Config();
    DMA_Config();

    while(1) {
        // CPU libre, DMA envía los valores al DAC en automático
    }
}