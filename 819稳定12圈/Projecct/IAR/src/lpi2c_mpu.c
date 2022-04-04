#include "lpi2c_mpu.h"

void init_lpi2c_mpu()
{
  lpi2c_master_config_t i2cConfig;
  uint32_t i2cclk;

  CLOCK_EnableClock(kCLOCK_Iomuxc);          /* iomuxc clock (iomuxc_clk_enable): 0x03u */

  IOMUXC_SetPinMux(IOMUXC_GPIO_B0_04_LPI2C2_SCL, 1);
  IOMUXC_SetPinMux(IOMUXC_GPIO_B0_05_LPI2C2_SDA, 1);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_04_LPI2C2_SCL, 0xD8B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_05_LPI2C2_SDA, 0xD8B0);

  /*Clock setting for LPI2C*/
  CLOCK_SetMux(kCLOCK_Lpi2cMux, 0); //USB1 PLL (480 MHz) as master lpi2c clock source  480/8 = 60 MHz
  CLOCK_SetDiv(kCLOCK_Lpi2cDiv, 5); // 60 / 6 = 10MHz

  i2cclk = ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (5 + 1));                   //  10M

  LPI2C_MasterGetDefaultConfig(&i2cConfig);
  i2cConfig.baudRate_Hz = 400000;

  LPI2C_MasterInit(MPU_LPI2C, &i2cConfig, i2cclk);
  LPI2C_MasterReset(MPU_LPI2C);
  LPI2C_MasterInit(MPU_LPI2C, &i2cConfig, i2cclk);
}

//主机等待空闲
//返回值:0,空闲
//       1,忙
uint8_t MPU_LPI2C_Master_Wait_Busy(void)
{
  uint32_t wt = 0;
  uint32_t tempreg;
  do
  { 	 
    tempreg = MPU_LPI2C->MSR;	
    wt ++;
    if(wt > LPI2C_MAX_WT)
      break;		//超时了,退出
  }while((tempreg & (1 << 25)) && (!(tempreg & (1 << 24))));    //如果BBF=1且MBF=0,则持续等待
  return 0;
}

//主机检查错误标记,并清除
//返回值:0,无错误
//    其他,错误代码
uint8_t MPU_LPI2C_Master_CheckAndClear_Error(void)
{
  uint32_t tempreg;
  uint8_t res = 0;	//返回值
  tempreg = MPU_LPI2C->MSR & (0XF << 10);	//读取NDF/ALF/FEF/PLTF等4个错误标志
	//如果发生了错误,需要先清除错误,然后才能重新开始发送下一个数据
  if(tempreg)						//有错误发生
  {
    res = tempreg >> 10;			//获取错误代码 
    MPU_LPI2C->MSR = tempreg;		//清除NDF/ALF/FEF/PLTF等4个错误标志		
    MPU_LPI2C->MCR |= 3 << 8;			//复位发送和接收FIFO	
  }
  return res;
}

//主机等待可以进行发送操作
//返回值:0,可以进行发送
//       1,不能进行发送
uint8_t MPU_LPI2C_Master_Wait_TxReady(void)
{
  uint8_t res = 0;	//返回值
  uint8_t txcount;
  uint32_t wt = 0;						//最长等待时间(防止死机)
  do
  { 	 
    txcount = MPU_LPI2C->MFSR & 0X07;	//读取使用了的FIFO个数	
    txcount = 4 - txcount;			//可用的FIFO个数(总共4个)
    res = MPU_LPI2C_Master_CheckAndClear_Error();       //读取错误标记,并清除
    if(res)
      return res;			//返回错误代码
    wt ++;
    if(wt > LPI2C_MAX_WT)			//超时了,退出
    {
      res = 0XFF;				//标记错误
      break;	
    }
  }while(txcount == 0);				//无TX FIFO可用,死等
  return 0;
}

//主机开始数据传输(仅做准备工作)
//返回值:0,成功
//    其他,失败
uint8_t MPU_LPI2C_Master_Start(void)
{
  uint8_t res = 0;
  res = MPU_LPI2C_Master_Wait_Busy();	//等待LPI2C主机空闲
  if(res == 0)
  {
    MPU_LPI2C->MSR = 0X7F00;			//清除DMF/PLTF/FEF/ALF/NDF/SDF/EPF标记
    MPU_LPI2C->MCFGR1 &= ~(1 << 8);	//AUTOSTOP=0,禁止auto stop功能 
  }
  return res; 
}	  

//主机停止数据传输
//返回值:0,成功
//    其他,失败
uint8_t MPU_LPI2C_Master_Stop(void)
{ 
  uint8_t res = 0;
  uint32_t regval = 0;
  uint32_t wt = 0;							//最长等待时间(防止死机)
  res = MPU_LPI2C_Master_Wait_TxReady();	//等待LPI2C主机可以发送
  if(res == 0)
  {
    MPU_LPI2C->MTDR = LPI2C_MSTOP_CMD;	//发送停止命令
    while(res == 0)
    {
      regval = MPU_LPI2C->MSR;	
      res = MPU_LPI2C_Master_CheckAndClear_Error();//读取错误标记,并清除
      if(regval & (1 << 9))			//STOP是否发送成功了?
      {
        MPU_LPI2C->MSR = 1 << 9;		//清除STOP发送完成标记
        break;
      } 
      wt ++;
      if(wt > LPI2C_MAX_WT)			//超时了,退出
      {
        res = 0XFF;				//标记错误
        break;	
      }
    }
  }
  return res;
}

//主机发送数据
//txbuf:发送缓冲区
//size:发送数据长度(最大一次发送255字节)
//cmd:
//LPI2C_MTX_CMD(0X000):发送写数据命令
//LPI2C_MRX_CMD(0X100):发送读数据命令 
//LPI2C_MSTART_ADDR_CMD(0X400):发送起始信号+地址命令
//注意:当cmd非0时,建议设置size=1!!
//返回值:0,成功
//    其他,失败	  
uint8_t MPU_LPI2C_Master_Send(uint8_t *txbuf,uint8_t size,uint16_t cmd)
{                        
  uint8_t res = 0;
  uint8_t t = 0;
  while(size --)
  {
    res = MPU_LPI2C_Master_Wait_TxReady();	//等待LPI2C1主机可以发送
    if(res)
      return res;
    MPU_LPI2C->MTDR = cmd | txbuf[t ++];		//发送数据
  }
  return res;
} 	    

//主机接收数据
//rxbuf:接收缓冲区
//size:接收数据长度(最大一次发送255字节)
//返回值:0,成功
//    其他,失败	 
uint8_t MPU_LPI2C_Master_Read(uint8_t *rxbuf,uint8_t size)
{
  uint8_t res = 0;
  uint8_t t = 0;
  uint16_t rdata = 0;
  uint32_t wt = 0;								//最长等待时间(防止死机)
  res = MPU_LPI2C_Master_Wait_TxReady();		//等待LPI2C主机可以发送
  if(res)
    return res;
  MPU_LPI2C->MTDR = LPI2C_MRX_CMD | (size - 1);	//发送接收命令和此次接收数据长度 
  while(size --)
  {
    wt = 0;								//计时器清零
    do
    { 
      res = MPU_LPI2C_Master_CheckAndClear_Error();//读取错误标记,并清除
      if(res)
        return res; 
      rdata =  MPU_LPI2C->MRDR;				//读取接收FIFO
      wt ++;
      if(wt > LPI2C_MAX_WT)				//超时了,退出
      {
        res = 0XFF;					//标记错误
        break;	
      }
    }while(rdata & (1 << 14));				//等待FIFO里面有数据 
    rxbuf[t ++] = rdata & 0XFF;				//存储数据
  }
  return res; 
}
//IIC连续写
//addr:器件地址 
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
  uint8_t sdata = 0;
  MPU_LPI2C_Master_Start();  
  sdata = (addr << 1) | 0;									//发送器件地址+写命令
  MPU_LPI2C_Master_Send(&sdata, 1, LPI2C_MSTART_ADDR_CMD);	//发送起始信号+器件地址    
  MPU_LPI2C_Master_Send(&reg, 1, LPI2C_MTX_CMD);			//写寄存器地址 
  MPU_LPI2C_Master_Send(buf, len, LPI2C_MTX_CMD);			//写入len个数据    
  MPU_LPI2C_Master_Stop();								//停止LPI2C传输 
  return 0;       
}

//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{ 
  uint8_t sdata = 0;
  MPU_LPI2C_Master_Start();  
  sdata = (addr << 1) | 0;									//发送器件地址+写命令
  MPU_LPI2C_Master_Send(&sdata, 1, LPI2C_MSTART_ADDR_CMD);	//发送起始信号+器件地址    
  MPU_LPI2C_Master_Send(&reg, 1, LPI2C_MTX_CMD);			//写寄存器地址 
  sdata = (addr << 1) | 1;									//发送器件地址+读命令
  MPU_LPI2C_Master_Send(&sdata, 1, LPI2C_MSTART_ADDR_CMD);	//发送起始信号+器件地址 
  MPU_LPI2C_Master_Read(buf, len);						//读取len个字节数据
  MPU_LPI2C_Master_Stop();								//停止LPI2C传输 
  return 0;       
}

//IIC写一个字节 
//devaddr:器件IIC地址
//reg:寄存器地址
//data:数据
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Write_Byte(uint8_t addr,uint8_t reg,uint8_t data)
{
  uint8_t sdata = 0;
  MPU_LPI2C_Master_Start();  
  sdata = (addr << 1) | 0;									//发送器件地址+写命令
  MPU_LPI2C_Master_Send(&sdata, 1, LPI2C_MSTART_ADDR_CMD);	//发送起始信号+器件地址    
  MPU_LPI2C_Master_Send(&reg, 1, LPI2C_MTX_CMD);			//写寄存器地址 
  MPU_LPI2C_Master_Send(&data, 1, LPI2C_MTX_CMD);			//写入1个字节数据    
  MPU_LPI2C_Master_Stop();								//停止LPI2C传输 
  return 0;
}

//IIC读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
uint8_t MPU_Read_Byte(uint8_t addr,uint8_t reg)
{
  uint8_t sdata = 0;
  MPU_LPI2C_Master_Start();  
  sdata = (addr << 1) | 0;									//发送器件地址+写命令
  MPU_LPI2C_Master_Send(&sdata, 1, LPI2C_MSTART_ADDR_CMD);	//发送起始信号+器件地址    
  MPU_LPI2C_Master_Send(&reg, 1, LPI2C_MTX_CMD);			//写寄存器地址 
  sdata = (addr << 1) | 1;									//发送器件地址+读命令
  MPU_LPI2C_Master_Send(&sdata, 1, LPI2C_MSTART_ADDR_CMD);	//发送起始信号+器件地址 
  MPU_LPI2C_Master_Read(&sdata, 1);						//读取len个字节数据
  MPU_LPI2C_Master_Stop();								//停止LPI2C传输 
  return sdata;
}