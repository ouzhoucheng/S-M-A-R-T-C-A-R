#include "camera.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

//图像缓冲区。访问图像数据最好通过user_image来访问数据，最好不要直接访问缓冲区
ALIGN(64) uint8_t image_csi1[IMG_V][IMG_H];
ALIGN(64) uint8_t image_csi2[IMG_V][IMG_H];

//访问图像数据直接访问这个指针变量就可以
//访问方式非常简单，可以直接使用下标的方式访问
//例如访问第10行 50列的点，user_image[10][50]就可以了
uint8_t (*im)[IMG_H];

csi_handle_t csi_handle;
uint32_t fullCameraBufferAddr;    //采集完成的缓冲区地址
//需要配置到摄像头的数据
int16_t MT9V032_CFG[CONFIG_FINISH][2]=
{
  {AUTO_EXP,          0},   //自动曝光设置      范围1-63 0为关闭 如果自动曝光开启  EXP_TIME命令设置的数据将会变为最大曝光时间，也就是自动曝光时间的上限
                            //一般情况是不需要开启这个功能，因为比赛场地光线一般都比较均匀，如果遇到光线非常不均匀的情况可以尝试设置该值，增加图像稳定性
  {EXP_TIME,          450}, //曝光时间          摄像头收到后会自动计算出最大曝光时间，如果设置过大则设置为计算出来的最大曝光值
  {FPS,               50},  //图像帧率          摄像头收到后会自动计算出最大FPS，如果过大则设置为计算出来的最大FPS
  {SET_COL,           IMG_H}, //图像列数量        范围1-752     K60采集不允许超过188
  {SET_ROW,           IMG_V}, //图像行数量        范围1-480
  {LR_OFFSET,         0},   //图像左右偏移量    正值 右偏移   负值 左偏移  列为188 376 752时无法设置偏移    摄像头收偏移数据后会自动计算最大偏移，如果超出则设置计算出来的最大偏移
  {UD_OFFSET,         0},   //图像上下偏移量    正值 上偏移   负值 下偏移  行为120 240 480时无法设置偏移    摄像头收偏移数据后会自动计算最大偏移，如果超出则设置计算出来的最大偏移
  {GAIN,              32},  //图像增益          范围16-64     增益可以在曝光时间固定的情况下改变图像亮暗程度

  {INIT,              0}    //摄像头开始初始化
};

/**************
//从摄像头内部获取到的配置数据
int16_t GET_CFG[CONFIG_FINISH - 1][2]=
{
  {AUTO_EXP,          0},   //自动曝光设置      
  {EXP_TIME,          0},   //曝光时间          
  {FPS,               0},   //图像帧率          
  {SET_COL,           0},   //图像列数量        
  {SET_ROW,           0},   //图像行数量        
  {LR_OFFSET,         0},   //图像左右偏移量    
  {UD_OFFSET,         0},   //图像上下偏移量    
  {GAIN,              0},   //图像增益          
};
**************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void csi_isr(CSI_Type *base, csi_handle_t *handle, status_t status, void *userData)
{
  if(csi_get_full_buffer(&csi_handle, &fullCameraBufferAddr))
  {
    //csi_add_empty_buffer(&csi_handle, (uint8_t *)fullCameraBufferAddr);
    if(IsDisplay == 0)
    {
      if(fullCameraBufferAddr == (uint32_t)image_csi1[0])
        im = image_csi1; //image_csi1采集完成
      else if(fullCameraBufferAddr == (uint32_t)image_csi2[0])
        im = image_csi2; //image_csi2采集完成

      Caminfo_Cal();
    }
    csi_add_empty_buffer(&csi_handle, (uint8_t *)fullCameraBufferAddr);
  }
}

void init_camera(void)
{
  uint16_t temp, i;
  uint8_t send_buffer[4];
  uint8_t receive[3];
  csi_config_t csi_config;
  uint16_t width = IMG_H;
  uint16_t height = IMG_V;
  uint16_t temp_width = 0, temp_height = 0;
  uint32_t pixel_num = 0;

  init_camera_uart();

  //等待摄像头上电初始化成功
  delay_ms(1000);

  //开始配置摄像头并重新初始化
  for(i = 0; i < CONFIG_FINISH; i ++)
  {
    send_buffer[0] = 0xA5;
    send_buffer[1] = MT9V032_CFG[i][0];
    temp = MT9V032_CFG[i][1];
    send_buffer[2] = temp >> 8;
    send_buffer[3] = (uint8_t)temp;

    LPUART_WriteBlocking(LPUART5, send_buffer, 4);

    delay_ms(2);
  }    
  //以上部分对摄像头配置的数据全部都会保存在摄像头上51单片机的eeprom中
  //利用set_exposure_time函数单独配置的曝光数据不存储在eeprom中

  LPUART_ReadBlocking(LPUART5, receive, 3);

  //判断初始化是否成功
  if(receive[0] != 0xA5 || receive[1] != 0xFF || receive[2] != 0xFF)
  {
    //ipsprintf(IPS_WHITE,IPS_BLACK,0,0,"Camera is Failed!");
    while(1);                                 //摄像头初始化失败，进入死循环，手动复位程序（危险）
  }
  else
  {
    //ipsprintf(IPS_WHITE,IPS_BLACK,0,0,"Camera is OK!");
  }
  
  //Vertical flip
  set_mt9v032_reg(0x0D, 0x032A);
  
  //CSI采集初始化
  init_camera_pins();
  CLOCK_SetMux(kCLOCK_CsiMux, 2);
  CLOCK_SetDiv(kCLOCK_CsiDiv, 0);
  
  pixel_num = width * height;
  if(pixel_num % 8)
    assert(0); //像素点不是8的倍数 无法采集
  if(width % 8)
  {
    temp_width = width - width % 8;  //储存临时宽度
    i = 1;
    while(pixel_num > (temp_width + (i * 8)))
    {
      if(!(pixel_num % ((temp_width + (i * 8))))) 
      {
        temp_width += (i * 8);//储存临时宽度
        temp_height = pixel_num / temp_width;
        break;
      }
      i ++;
    }
    if(!temp_height)
    {//如果没有找到则从临时宽度往下找
      i = 1;
      while((temp_width - (i * 8)))
      {
        if(!(pixel_num % ((temp_width - (i * 8)))))
        {
          temp_width -= (i * 8);//储存临时宽度
          temp_height = pixel_num / temp_width;
          break;
        }
        i ++;
      }
    }
    if(!temp_height)
    {//如果还是没有找到
      temp_width = pixel_num;
      temp_height = 1;
    }
  }
  else
  {
    temp_width = width;
    temp_height = height;
  }

  NVIC_SetPriority(CSI_IRQn, NVIC_EncodePriority(PriorityGroup, 3, 0));
  
  CSI_GetDefaultConfig(&csi_config);
  csi_config.width = temp_width;
  csi_config.height = temp_height;
  csi_config.polarityFlags = kCSI_DataLatchOnRisingEdge;
  csi_config.bytesPerPixel = 1U;
  csi_config.linePitch_Bytes = temp_width;
  csi_config.workMode = kCSI_NonGatedClockMode;
  csi_config.dataBus = kCSI_DataBus8Bit;
  csi_config.useExtVsync = true;
  CSI_Init(CSI, &csi_config);
  CSI_TransferCreateHandle(CSI, &csi_handle, csi_isr, NULL);
  
  csi_add_empty_buffer(&csi_handle, image_csi1[0]);
  csi_add_empty_buffer(&csi_handle, image_csi2[0]);
  
  CSI_TransferStart(CSI, &csi_handle);
  
  im = image_csi1;
}

void init_camera_uart()
{
  uint32_t freq = 0;
  lpuart_config_t lpuart5_config;

  CLOCK_EnableClock(kCLOCK_Lpuart5);
  CLOCK_SetMux(kCLOCK_UartMux, 0);
  CLOCK_SetDiv(kCLOCK_UartDiv, 0);

  IOMUXC_SetPinMux(IOMUXC_GPIO_B1_12_LPUART5_TX, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_B1_13_LPUART5_RX, 0);

  IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_12_LPUART5_TX, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_13_LPUART5_RX, 0x10B0);

  freq = LPUART_SrcFreqGet();

  LPUART_GetDefaultConfig(&lpuart5_config);
  lpuart5_config.baudRate_Bps = 9600;
  lpuart5_config.enableRx = true;
  lpuart5_config.enableTx = true;
	
  LPUART_Init(LPUART5, &lpuart5_config, freq);
  LPUART_Deinit(LPUART5);
  LPUART_Init(LPUART5, &lpuart5_config, freq);
}

//对摄像头内部寄存器进行写操作
void set_mt9v032_reg(uint8_t addr, uint16_t data)
{
  uint16_t temp;
  uint8_t  send_buffer[4];

  send_buffer[0] = 0xA5;
  send_buffer[1] = SET_ADDR;
  temp = addr;
  send_buffer[2] = temp >> 8;
  send_buffer[3] = (uint8_t)temp;

  LPUART_WriteBlocking(LPUART5, send_buffer, 4);
  //delay_ms(10);

  send_buffer[0] = 0xA5;
  send_buffer[1] = SET_DATA;
  temp = data;
  send_buffer[2] = temp >> 8;
  send_buffer[3] = (uint8_t)temp;

  LPUART_WriteBlocking(LPUART5, send_buffer, 4);
}

//单独设置摄像头曝光时间
void set_exposure_time(uint16_t light)
{
  uint16_t temp;
  uint8_t  send_buffer[4];

  send_buffer[0] = 0xA5;
  send_buffer[1] = SET_EXP_TIME;
  temp = light;
  send_buffer[2] = temp>>8;
  send_buffer[3] = (uint8_t)temp;

  LPUART_WriteBlocking(LPUART5, send_buffer, 4);
}

void init_camera_pins()
{
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_08_CSI_DATA09, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_09_CSI_DATA08, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_10_CSI_DATA07, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_11_CSI_DATA06, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_12_CSI_DATA05, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_13_CSI_DATA04, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_14_CSI_DATA03, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_15_CSI_DATA02, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_04_CSI_PIXCLK, 0);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_06_CSI_VSYNC, 0);

  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_08_CSI_DATA09, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_09_CSI_DATA08, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_10_CSI_DATA07, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_11_CSI_DATA06, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_12_CSI_DATA05, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_13_CSI_DATA04, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_14_CSI_DATA03, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_15_CSI_DATA02, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_04_CSI_PIXCLK, 0x10B0);
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_06_CSI_VSYNC, 0x10B0);
}

uint8_t csi_get_full_buffer(csi_handle_t *handle, uint32_t *buffaddr)
{
  if(kStatus_Success == CSI_TransferGetFullBuffer(CSI, handle, (uint32_t *)buffaddr))
    return 1;
  return 0;
}

void csi_add_empty_buffer(csi_handle_t *handle, uint8_t *buff)
{
  CSI_TransferSubmitEmptyBuffer(CSI, handle, (uint32_t)buff);
}