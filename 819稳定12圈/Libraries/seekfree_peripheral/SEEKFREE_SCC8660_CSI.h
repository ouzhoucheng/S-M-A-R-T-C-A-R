/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		凌瞳摄像头(SCC8660) RT CSI接口
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598 & QQ2380006440)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.32.4 or MDK 5.24 或更高
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-09-04
 * @note		
					接线定义：
					------------------------------------ 
						模块管脚            单片机管脚
						SDA(摄像头的RX)     查看SEEKFREE_SCC8660_CSI.h文件中的SCC8660_CSI_COF_UART_TX宏定义
						SCL(摄像头的TX)     查看SEEKFREE_SCC8660_CSI.h文件中的SCC8660_CSI_COF_UART_RX宏定义
						场中断(VSY)         查看SEEKFREE_SCC8660_CSI.h文件中的SCC8660_CSI_VSYNC_PIN宏定义
						行中断(HREF)        不与核心板连接（悬空）
						像素中断(PCLK)      查看SEEKFREE_SCC8660_CSI.h文件中的SCC8660_CSI_PCLK_PIN宏定义
						数据口(D0-D7)       B31-B24 B31对应摄像头接口D0
					------------------------------------ 
	
					默认分辨率               160*120
					默认FPS                  50帧
 ********************************************************************************************************************/



#ifndef _SEEKFREE_SCC8660_CSI_h
#define _SEEKFREE_SCC8660_CSI_h


#include "common.h"
#include "zf_uart.h"

//--------------------------------------------------------------------------------------------------
//摄像头参数配置
//--------------------------------------------------------------------------------------------------
//原始分辨率为720*480，但由于像素颗粒并不是正方形，所以图像会有横向拉伸失真。最大无失真分辨率为 640 * 480
//当分辨率为640*480时，图像长宽比例无失真，所以如果修改图像分辨率，若行列比例与640*480不同，则图像拉伸缩放。
//默认图像分辨率为160*120，若更改图像分辨率比默认分辨率大，请在.c文件内更改图像缓冲区保存位置为SDRAM。避免编译出错。


//------------ 请仔细以上注释后再更改分辨率 ------------
//------------ 请仔细以上注释后再更改分辨率 ------------
//------------ 请仔细以上注释后再更改分辨率 ------------
#define SCC8660_CSI_PIC_W				160			//实际图像分辨率宽度	可选参数为：160 180 240 320 360 480 640 720。
#define SCC8660_CSI_PIC_H				120			//实际图像分辨率高度	可选参数为：120 160 180 240 320 360 480。

#define SCC8660_CSI_W		SCC8660_CSI_PIC_W*2		//此参数为图像数据存储宽度 请勿修改
#define SCC8660_CSI_H		SCC8660_CSI_PIC_H       //此参数为图像数据存储高度 请勿修改

//--------------------------------------------------------------------------------------------------
//引脚配置
//--------------------------------------------------------------------------------------------------
#define SCC8660_CSI_COF_UART        USART_5         //配置摄像头所使用到的串口     
#define SCC8660_CSI_COF_UART_TX     UART5_TX_C28
#define SCC8660_CSI_COF_UART_RX     UART5_RX_C29


#define SCC8660_CSI_PCLK_PIN        CSI_PIXCLK_B20  //定义像素时钟引脚
#define SCC8660_CSI_VSYNC_PIN       CSI_VSYNC_B22   //定义场信号引脚
//对于数据引脚这里不提供设置到其他引脚的定义
//由于第二组引脚由部分没有引出所以程序直接固定使用B31-B24连接摄像头的数据引脚



extern uint16  scc8660_csi1_image[SCC8660_CSI_PIC_H][SCC8660_CSI_PIC_W];
extern uint16  scc8660_csi2_image[SCC8660_CSI_PIC_H][SCC8660_CSI_PIC_W];
extern uint16  (*user_color_image)[SCC8660_CSI_PIC_W];  //图像数据

extern uint8   scc8660_csi_finish_flag;       //一场图像采集完成标志位

extern uint8   scc8660_uart_receive[3];
extern uint8   scc8660_uart_receive_num;
extern vuint8  scc8660_uart_receive_flag;


typedef enum
{
    SCC8660_INIT 			= 0x00,
    SCC8660_BRIGHT,
    SCC8660_FPS,
    SCC8660_SET_COL,
    SCC8660_SET_ROW,
    SCC8660_PCLK_DIV,
    SCC8660_PCLK_MODE,
    SCC8660_COLOR_MODE,
    SCC8660_CONFIG_FINISH,
    
    SCC8660_GET_WHO_AM_I = 0xEF,
    SCC8660_GET_STATUS 	= 0XF1,
    SCC8660_GET_VERSION	= 0xF2,
	
    SCC8660_SET_REG_ADDR	= 0xFE,
    SCC8660_SET_REG_DATA	= 0xFF,
}SCC8660_CMD;



void    scc8660_csi_init(void);
void    scc8660_set_all_config(UARTN_enum uartn, int16 buff[SCC8660_CONFIG_FINISH-1][2]);
void    scc8660_get_all_config(UARTN_enum uartn, int16 buff[SCC8660_CONFIG_FINISH-1][2]);
uint16  scc8660_get_id(UARTN_enum uartn);
uint16  scc8660_get_config(UARTN_enum uartn, uint8 config);
uint16  scc8660_get_version(UARTN_enum uartn);
uint16  scc8660_set_reg_addr(UARTN_enum uartn, uint8 reg, uint16 data);
void    csi_seekfree_sendimg_scc8660(UARTN_enum uartn, uint8 *image, uint16 width, uint16 height);


#endif
