#ifndef _LPI2C_MPU_H_
#define _LPI2C_MPU_H_

//#include "car_global.h"

/*******************************************************************************
* Definitions
******************************************************************************/

#define MPU_LPI2C LPI2C2

#define LPI2C_MTX_CMD				0<<8		//LPI2C主机发送数据命令
#define LPI2C_MRX_CMD				1<<8		//LPI2C主机接收数据命令
#define LPI2C_MSTOP_CMD				2<<8		//LPI2C主机停止命令
#define LPI2C_MSTART_ADDR_CMD		4<<8		//LPI2C主机发送起始信号+发送地址

#define LPI2C_MAX_WT			0xFFFFF	//0X1FFFFF	//最大等待超时时间

/*******************************************************************************
* API
******************************************************************************/

void init_lpi2c_mpu();
uint8_t MPU_LPI2C_Master_Wait_Busy(void);				//LPI2C主机等待空闲
uint8_t MPU_LPI2C_Master_CheckAndClear_Error(void);		//LPI2C主机检查错误标记并清零
uint8_t MPU_LPI2C_Master_Wait_TxReady(void);			//LPI2C主机等待可以发送数据
uint8_t MPU_LPI2C_Master_Start(void);					//LPI2C主机准备开始数据传输
uint8_t MPU_LPI2C_Master_Stop(void);					//LPI2C主机停止数据传输
uint8_t MPU_LPI2C_Master_Send(uint8_t *txbuf,uint8_t size,uint16_t cmd);//LPI2C主机发送数据
uint8_t MPU_LPI2C_Master_Read(uint8_t *rxbuf,uint8_t size);		//LPI2C主机接收数据 
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);
uint8_t MPU_Write_Byte(uint8_t addr,uint8_t reg,uint8_t data);
uint8_t MPU_Read_Byte(uint8_t addr,uint8_t reg);

#endif