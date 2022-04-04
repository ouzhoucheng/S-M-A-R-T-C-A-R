/********************************************************************
 * M8266HostIf.c
 * .Description
 *     Source file of M8266WIFI Host Interface 
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"	
#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"
#include "car_global.h"

/***********************************************************************************
 * M8266HostIf_GPIO_SPInCS_nRESET_Pin_Init                                         *
 * Description                                                                     *
 *    To initialise the GPIOs for SPI nCS and nRESET output for M8266WIFI module   *
 *    You may update the macros of GPIO PINs usages for nRESET from brd_cfg.h      *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    None                                                                         *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_GPIO_CS_RESET_Init(void)
{
	
	 /* Notes to IOMUXC_SW_PAD_CTL_PAD register
	  *    bits 31-17: reserved 0
	  *    bit  16   : 1->Hysteresis Enable
	  *    bits 15-14: b00->100KOhm Pulldown, b01->47KOhm Pullup, b10->100KOhm Pullup, b11->22KOhm PullUp
	  *    bit  13   : Select Keeper or Pull for Pad: b0-> Keeper, b1->pull
	  *    bit  12   : b1 -> Pull/Keep Enable
	  *    bit  11   : b1 -> Open Drain Enable
	  *    bits 10- 8: reserved 0
	  *    bits  7- 6: Speed Field : b00->50MHz, b01->100MHz, b10->100MHz, b11->200MHz
	  *    bits  5- 3: Driver Strength R0=260 Ohm@3.3V: b000->disabled, others = R0/value[5:3], used for impedance matching
		*    bits  2- 1: Reserived 0
		*    bit      0: b0-> Slow Slew Rate, b1->Fast Slew Rate	
	  */
	
	gpio_pin_config_t gpio_config;
	
	//Initial iMx RT1052's GPIO for M8266WIFI_SPI_nCS
  IOMUXC_SetPinMux(M8266WIFI_SPI_nCS_PORT_MUX, 0);
	IOMUXC_SetPinConfig(M8266WIFI_SPI_nCS_PORT_MUX, 0xB069);  // 0xB069=b0| 10|11 0|000 01|10 1|00|1 -> Hysteresis Disabled, 100KOhm Pullup, pull enabled, 100MHz, R0/5=52, Fast Slew Rate
	gpio_config.direction 			= kGPIO_DigitalOutput;				// Output
	gpio_config.interruptMode	  =	kGPIO_NoIntmode;						// NO Interrupt
	gpio_config.outputLogic		  =	1;										  		// Initially High
	GPIO_PinInit(M8266WIFI_SPI_nCS_GPIO, M8266WIFI_SPI_nCS_GPIO_PIN, &gpio_config); 	        						// Initialise the GPIO	
	
	//Initial iMx RT1052's GPIO for M8266WIFI_SPI_nRST
  IOMUXC_SetPinMux(M8266WIFI_nRESET_PORT_MUX, 0);			
	IOMUXC_SetPinConfig(M8266WIFI_nRESET_PORT_MUX, 0xB069);   // 0xB069=b0| 10|11 0|000 01|10 1|00|1 -> Hysteresis Disabled, 100KOhm Pullup, pull enabled, 100MHz, R0/5=52, Fast Slew Rate 
	gpio_config.direction 			= kGPIO_DigitalOutput;				// Output
	gpio_config.interruptMode	=	kGPIO_NoIntmode;							// NO Interrupt
	gpio_config.outputLogic		=	1;														// Initially High
	GPIO_PinInit(M8266WIFI_nRESET_GPIO, M8266WIFI_nRESET_GPIO_PIN, &gpio_config); 	        						// Initialise the GPIO	
}
/***********************************************************************************
 * M8266HostIf_SPI_Init                                                            *
 * Description                                                                     *
 *    To initialise the SPI Interface for M8266WIFI module                         *
 *    You may update the macros of SPI usages for nRESET from brd_cfg.h            *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    None                                                                         *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_SPI_Init(void)
{
	 ////////////////////////////////////////////////////////////////////////////////
	 // Step 1: Multiplex the IOs to be alternative as SPI functions
	 ////////////////////////////////////////////////////////////////////////////////
	 /* Notes to IOMUXC_SW_PAD_CTL_PAD register
	  *    bits 31-17: reserved 0
	  *    bit  16   : 1->Hysteresis Enable
	  *    bits 15-14: b00->100KOhm Pulldown, b01->47KOhm Pullup, b10->100KOhm Pullup, b11->22KOhm PullUp
	  *    bit  13   : Select Keeper or Pull for Pad: b0-> Keeper, b1->pull
	  *    bit  12   : b1 -> Pull/Keep Enable
	  *    bit  11   : b1 -> Open Drain Enable
	  *    bits 10- 8: reserved 0
	  *    bits  7- 6: Speed Field : b00->50MHz, b01->100MHz, b10->100MHz, b11->200MHz
	  *    bits  5- 3: Driver Strength R0=260 Ohm@3.3V: b000->disabled, others = R0/value[5:3], used for impedance matching
		*    bits  2- 1: Reserived 0
		*    bit      0: b0-> Slow Slew Rate, b1->Fast Slew Rate	
	  */
#if (M8266WIFI_SPI_INTERFACE_NO == 1)
	#if 0 
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI, 1);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK,0x3069);  //0x3069=b0| 00|11| 0|000 01|10 1|00|1 -> Hysteresis Disabled, 100KOhm Pulldown, pull enabled, 100MHz, R0/5=52, Fast Slew Rate
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO,0x3069);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI,0x3069);
	#elif 1
    // IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_27_LPSPI1_SCK, 0);                                    
    // IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_28_LPSPI1_SDO, 0);                                    
    // IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_29_LPSPI1_SDI, 0);                                    

    // IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_27_LPSPI1_SCK,0x3069);  //0x3069=b0| 00|11| 0|000 01|10 1|00|1 -> Hysteresis Disabled, 100KOhm Pulldown, pull enabled, 100MHz, R0/5=52, Fast Slew Rate
    // IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_28_LPSPI1_SDO,0x3069);                                                                              
    // IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_29_LPSPI1_SDI,0x3069);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI, 1);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK,0x3069);  //0x3069=b0| 00|11| 0|000 01|10 1|00|1 -> Hysteresis Disabled, 100KOhm Pulldown, pull enabled, 100MHz, R0/5=52, Fast Slew Rate
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO,0x3069);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI,0x3069);
  #endif	
#elif(M8266WIFI_SPI_INTERFACE_NO == 2)
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_07_LPSPI2_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_08_LPSPI2_SD0, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_09_LPSPI2_SDI, 0);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_07_LPSPI2_SCK,0x10B0);                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_08_LPSPI2_SD0,0x10B0);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_09_LPSPI2_SDI,0x10B0);	
#elif(M8266WIFI_SPI_INTERFACE_NO == 3)
  #if 1
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_15_LPSPI3_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_14_LPSPI3_SDO, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_13_LPSPI3_SDI, 0);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_15_LPSPI3_SCK,0x10B0);                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_14_LPSPI3_SDO,0x10B0);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_13_LPSPI3_SDI,0x10B0);
 #else
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_00_LPSPI3_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_01_LPSPI3_SDO, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_02_LPSPI3_SDI, 0);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_00_LPSPI3_SCK,0x10B0);                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_01_LPSPI3_SDO,0x10B0);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_02_LPSPI3_SDI,0x10B0); 
 #endif 
#elif(M8266WIFI_SPI_INTERFACE_NO == 4)
  #if 1
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_07_LPSPI4_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_06_LPSPI4_SDO, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_05_LPSPI4_SDI, 0);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_07_LPSPI4_SCK,0x10B0);                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_06_LPSPI4_SDO,0x10B0);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_05_LPSPI4_SDI,0x10B0);
 #else
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_03_LPSPI4_SCK, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_02_LPSPI4_SDO, 0);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_01_LPSPI4_SDI, 0);                                    

    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_03_LPSPI4_SCK,0x10B0);                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_02_LPSPI4_SDO,0x10B0);                                                                              
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_01_LPSPI4_SDI,0x10B0); 
 #endif 
#else
#error Invalid M8266WIFI_SPI_INTERFACE_NO defined in brd_cfg.h 
#endif

	 ////////////////////////////////////////////////////////////////////////////////
	 // Step 2: Configure the SPI fynctions
	 ////////////////////////////////////////////////////////////////////////////////
	 /* Notes:
	  *    1. Two different clocks configured
		*       - lpspi_clk   : clocks for lpspi function module. Here, we set to 192MHz, or aound 1/5.2ns
		*       - spi_baudrate: spi_sck frequency, initially to be 4MHz. No more than lpspi_clk/2, and better be lpspi_clk/2^n
		*    2. Datasheet says, pcsToSckDelay, lastSckToPcsDelay, betweenTransferDelayInNanoSec should be at least 1cycle of lpspi_clk
    */

 	  u32 lpspi_clk;
	  u8  lpspi_clk_divid_def =  2; // Default Clock divider = 2, therefore SPI Source Clock =192MHz as default

    u32 spi_baudrate=8000000;    //SCK->8MHz Baud=8Mbps. Noted:  SCK frequency different from LPSPI Module clock(lpspiclk)
	  lpspi_master_config_t lpspi_config;
	
	 //Setup the SPI Module Clock sources to be PLL3_PFD0/(1+5)=576Mhz/(1+5) = 96MHz
    CLOCK_SetMux(kCLOCK_LpspiMux, 1);     								// Select USB1 PLL PFD0(576MHz) as LPSPI Clock Sources
    CLOCK_SetDiv(kCLOCK_LpspiDiv, lpspi_clk_divid_def);    // Default Clock divider = 35, therefore SPI Clock = 16MHz Initally


    LPSPI_Enable(M8266WIFI_INTERFACE_SPI, false);		
    lpspi_clk=(CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk)/(lpspi_clk_divid_def+1));
    lpspi_config.baudRate												=	spi_baudrate;
    lpspi_config.whichPcs												=	kLPSPI_Pcs1;
    lpspi_config.pcsActiveHighOrLow							=	kLPSPI_PcsActiveLow;
    lpspi_config.bitsPerFrame										=	8;
    lpspi_config.cpol														=	kLPSPI_ClockPolarityActiveHigh; // Require Idle Low, so Active High
    lpspi_config.cpha														=	kLPSPI_ClockPhaseFirstEdge;     // kLPSPI_ClockPhaseSecondEdge;
    lpspi_config.direction											=	kLPSPI_MsbFirst;
    lpspi_config.pinCfg													=	kLPSPI_SdiInSdoOut;
    lpspi_config.dataOutConfig									= kLpspiDataOutRetained;					// kLpspiDataOutTristate;
    lpspi_config.pcsToSckDelayInNanoSec					=  5; // The delay is (SCKPCS+1)cycles of lpspi_clk. Therefore, at least 5.02ns @ 192MHz
    lpspi_config.lastSckToPcsDelayInNanoSec			=  5; // The delay is (PCSSCK+1)cycles of lpspi_clk. Therefore, at least 5.02ns @ 192MHz
    lpspi_config.betweenTransferDelayInNanoSec	= 10; // The delya is (DBT   +2)cycles of lpspi_clk. Therefore, at least 5.02ns @ 192MHz 

    LPSPI_MasterInit(M8266WIFI_INTERFACE_SPI, &lpspi_config, lpspi_clk);
    LPSPI_Enable(M8266WIFI_INTERFACE_SPI, true);
	
} 

void M8266HostIf_SPI_SetSpeed(u32 SPI_BaudRatePrescaler)
{
	M8266WIFI_INTERFACE_SPI->CR  &= ~(1<<0);	   							  // MEN=0,禁止LPSPIx	 	 
	M8266WIFI_INTERFACE_SPI->CCR &= ~(0XFF<<0);  							  // SCKDIV=0X00,清除SCKDIV原来的设置
	M8266WIFI_INTERFACE_SPI->CCR |= SPI_BaudRatePrescaler&0xFF;	// SCKDIV=sckdiv,设置SCKDIV
	M8266WIFI_INTERFACE_SPI->CR  |= 1<<0;											  // MEN=0,使能LPSPIx	
}

/***********************************************************************************
 * M8266HostIf_Init                                                                *
 * Description                                                                     *
 *    To initialise the Host interface for M8266WIFI module                        *
 * Parameter(s):                                                                   *
 *    baud: baud rate to set                                                       *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/ 
void M8266HostIf_Init(void)
{
	 M8266HostIf_GPIO_CS_RESET_Init();
	 M8266HostIf_SPI_Init();

}

//////////////////////////////////////////////////////////////////////////////////////
// BELOW FUNCTIONS ARE REQUIRED BY M8266WIFIDRV.LIB. 
// PLEASE IMPLEMENTE THEM ACCORDING TO YOUR HARDWARE
//////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************************
 * M8266HostIf_Set_nRESET_Pin                                                      *
 * Description                                                                     *
 *    To Outpout HIGH or LOW onto the GPIO pin for M8266WIFI nRESET                *
 *    You may update the macros of GPIO PIN usages for nRESET from brd_cfg.h       *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    1. level: LEVEL output to nRESET pin                                         *
 *              0 = output LOW  onto nRESET                                        *
 *              1 = output HIGH onto nRESET                                        *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_Set_nRESET_Pin(u8 level)
{
	  if(level!=0)
		{
			GPIO_PinWrite(M8266WIFI_nRESET_GPIO, M8266WIFI_nRESET_GPIO_PIN, 1);
		}
		else
		{
			GPIO_PinWrite(M8266WIFI_nRESET_GPIO, M8266WIFI_nRESET_GPIO_PIN, 0);
		}
}
/***********************************************************************************
 * M8266HostIf_Set_SPI_nCS_PIN                                                     *
 * Description                                                                     *
 *    To Outpout HIGH or LOW onto the GPIO pin for M8266WIFI SPI nCS               *
 *    You may update the macros of GPIO PIN usages for SPI nCS from brd_cfg.h      *
 *    You are not recommended to modify codes below please                         *
 * Parameter(s):                                                                   *
 *    1. level: LEVEL output to SPI nCS pin                                        *
 *              0 = output LOW  onto SPI nCS                                       *
 *              1 = output HIGH onto SPI nCS                                       *
 * Return:                                                                         *
 *    None                                                                         *
 ***********************************************************************************/
void M8266HostIf_Set_SPI_nCS_Pin(u8 level)
{
	  if(level!=0)
		{
			GPIO_PinWrite(M8266WIFI_SPI_nCS_GPIO, M8266WIFI_SPI_nCS_GPIO_PIN, 1);
		}
		else
		{
			GPIO_PinWrite(M8266WIFI_SPI_nCS_GPIO, M8266WIFI_SPI_nCS_GPIO_PIN, 0);
		}
}

/***********************************************************************************
 * M8266WIFIHostIf_delay_us                                                        *
 * Description                                                                     *
 *    To loop delay some micro seconds.                                            *
 * Parameter(s):                                                                   *
 *    1. nus: the micro seconds to delay                                           *
 * Return:                                                                         *
 *    none                                                                         *
 ***********************************************************************************/
void M8266HostIf_delay_us(u8 nus)
{
   delay_us(nus);
}

/***********************************************************************************
 * M8266HostIf_SPI_ReadWriteByte                                                   *
 * Description                                                                     *
 *    To write a byte onto the SPI bus from MCU MOSI to the M8266WIFI module       *
 *    and read back a byte from the SPI bus MISO meanwhile                         *
 *    You may update the macros of SPI usage from brd_cfg.h                        *
 * Parameter(s):                                                                   *
 *    1. TxdByte: the byte to be sent over MOSI                                    *
 * Return:                                                                         *
 *    1. The byte read back from MOSI meanwhile                                    *                                                                         *
 ***********************************************************************************/
u8 M8266HostIf_SPI_ReadWriteByte(u8 TxdByte)
{
	u8 RxData = 0;
  u8 TxData = TxdByte;

  lpspi_transfer_t spi_tranxfer;
    
  spi_tranxfer.configFlags=kLPSPI_MasterPcs1 | kLPSPI_MasterPcsContinuous;     //PCS1
  spi_tranxfer.txData=&TxData;
  spi_tranxfer.rxData=&RxData;
  spi_tranxfer.dataSize=1;
  LPSPI_MasterTransferBlocking(M8266WIFI_INTERFACE_SPI, &spi_tranxfer);	
  return RxData;

}
