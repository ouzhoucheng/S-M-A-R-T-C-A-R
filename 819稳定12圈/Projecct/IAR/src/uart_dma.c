#include "uart_dma.h"

#define DEMO_LPUART LPUART1

#define LPUART_TX_DMA_CHANNEL 0U                      //UART发送使用的DMA通道号
#define LPUART_RX_DMA_CHANNEL 1U                      //UART接收使用的DMA通道号
#define LPUART_TX_DMA_REQUEST kDmaRequestMuxLPUART1Tx //定义串口DMA发送请求源
#define LPUART_RX_DMA_REQUEST kDmaRequestMuxLPUART1Rx //定义串口DMA接收请求源
#define LPUART_DMAMUX_BASEADDR DMAMUX                 //定义所使用的DMA多路复用模块(DMAMUX)
#define LPUART_DMA_BASEADDR DMA0                      //定义使用的DMA
#define ECHO_BUFFER_LENGTH 8                        //UART接收和发送数据缓冲区长度

/* 收发缓冲区 */
SDK_L1DCACHE_ALIGN(uint8_t g_txBuffer[ECHO_BUFFER_LENGTH]) = {0};
SDK_L1DCACHE_ALIGN(uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH]) = {0};

/*句柄定义*/
lpuart_edma_handle_t g_lpuartEdmaHandle; //串口DMA传输句柄
edma_handle_t g_lpuartTxEdmaHandle;      //串口DMA发送句柄
edma_handle_t g_lpuartRxEdmaHandle;      //串口DMA接收句柄

lpuart_transfer_t g_sendXfer;    //定义发送传输结构体
lpuart_transfer_t g_receiveXfer; //定义接收传输结构体

/**
 * @brief 串口DMA初始化
 * @param 无
 * @retval 无
 * @note 无
  */

extern uint8_t SendLine[5];

void UART_DMA_Init(void)
{
    edma_config_t config;

    /*初始化DMAMUX */
    DMAMUX_Init(LPUART_DMAMUX_BASEADDR);
    /* 为LPUART设置DMA传输通道 */
    DMAMUX_SetSource(LPUART_DMAMUX_BASEADDR, LPUART_TX_DMA_CHANNEL, LPUART_TX_DMA_REQUEST);
    DMAMUX_SetSource(LPUART_DMAMUX_BASEADDR, LPUART_RX_DMA_CHANNEL, LPUART_RX_DMA_REQUEST);
    DMAMUX_EnableChannel(LPUART_DMAMUX_BASEADDR, LPUART_TX_DMA_CHANNEL);
    DMAMUX_EnableChannel(LPUART_DMAMUX_BASEADDR, LPUART_RX_DMA_CHANNEL);

    /* 初始化DMA */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(LPUART_DMA_BASEADDR, &config);
    /* 创建eDMA传输句柄 */
    EDMA_CreateHandle(&g_lpuartTxEdmaHandle, LPUART_DMA_BASEADDR, LPUART_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&g_lpuartRxEdmaHandle, LPUART_DMA_BASEADDR, LPUART_RX_DMA_CHANNEL);

    /* 创建 LPUART DMA 句柄 */
    LPUART_TransferCreateHandleEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, NULL, NULL, &g_lpuartTxEdmaHandle, &g_lpuartRxEdmaHandle);

    /* 启动传输 */
    // g_receiveXfer.data = g_rxBuffer;
    // g_receiveXfer.dataSize = ECHO_BUFFER_LENGTH;
    // LPUART_ReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &g_receiveXfer);

    g_sendXfer.data = SendLine;
    g_sendXfer.dataSize = 5;
    LPUART_SendEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &g_sendXfer);
}

void DMA_start()
{
    DCACHE_CleanInvalidateByRange((uint32_t)g_txBuffer, ECHO_BUFFER_LENGTH);
    g_sendXfer.data = SendLine;
    g_sendXfer.dataSize = 6;
    LPUART_SendEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &g_sendXfer);
}