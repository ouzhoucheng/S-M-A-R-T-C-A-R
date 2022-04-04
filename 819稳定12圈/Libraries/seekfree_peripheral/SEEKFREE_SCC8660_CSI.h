/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,��ɿƼ�
 * All rights reserved.
 * ��������QQȺ��һȺ��179029047(����)  ��Ⱥ��244861897
 *
 * �����������ݰ�Ȩ������ɿƼ����У�δ��������������ҵ��;��
 * ��ӭ��λʹ�ò������������޸�����ʱ���뱣����ɿƼ��İ�Ȩ������
 *
 * @file       		��ͫ����ͷ(SCC8660) RT CSI�ӿ�
 * @company	   		�ɶ���ɿƼ����޹�˾
 * @author     		��ɿƼ�(QQ3184284598 & QQ2380006440)
 * @version    		�鿴doc��version�ļ� �汾˵��
 * @Software 		IAR 8.32.4 or MDK 5.24 �����
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-09-04
 * @note		
					���߶��壺
					------------------------------------ 
						ģ��ܽ�            ��Ƭ���ܽ�
						SDA(����ͷ��RX)     �鿴SEEKFREE_SCC8660_CSI.h�ļ��е�SCC8660_CSI_COF_UART_TX�궨��
						SCL(����ͷ��TX)     �鿴SEEKFREE_SCC8660_CSI.h�ļ��е�SCC8660_CSI_COF_UART_RX�궨��
						���ж�(VSY)         �鿴SEEKFREE_SCC8660_CSI.h�ļ��е�SCC8660_CSI_VSYNC_PIN�궨��
						���ж�(HREF)        ������İ����ӣ����գ�
						�����ж�(PCLK)      �鿴SEEKFREE_SCC8660_CSI.h�ļ��е�SCC8660_CSI_PCLK_PIN�궨��
						���ݿ�(D0-D7)       B31-B24 B31��Ӧ����ͷ�ӿ�D0
					------------------------------------ 
	
					Ĭ�Ϸֱ���               160*120
					Ĭ��FPS                  50֡
 ********************************************************************************************************************/



#ifndef _SEEKFREE_SCC8660_CSI_h
#define _SEEKFREE_SCC8660_CSI_h


#include "common.h"
#include "zf_uart.h"

//--------------------------------------------------------------------------------------------------
//����ͷ��������
//--------------------------------------------------------------------------------------------------
//ԭʼ�ֱ���Ϊ720*480�����������ؿ��������������Σ�����ͼ����к�������ʧ�档�����ʧ��ֱ���Ϊ 640 * 480
//���ֱ���Ϊ640*480ʱ��ͼ�񳤿������ʧ�棬��������޸�ͼ��ֱ��ʣ������б�����640*480��ͬ����ͼ���������š�
//Ĭ��ͼ��ֱ���Ϊ160*120��������ͼ��ֱ��ʱ�Ĭ�Ϸֱ��ʴ�����.c�ļ��ڸ���ͼ�񻺳�������λ��ΪSDRAM������������


//------------ ����ϸ����ע�ͺ��ٸ��ķֱ��� ------------
//------------ ����ϸ����ע�ͺ��ٸ��ķֱ��� ------------
//------------ ����ϸ����ע�ͺ��ٸ��ķֱ��� ------------
#define SCC8660_CSI_PIC_W				160			//ʵ��ͼ��ֱ��ʿ��	��ѡ����Ϊ��160 180 240 320 360 480 640 720��
#define SCC8660_CSI_PIC_H				120			//ʵ��ͼ��ֱ��ʸ߶�	��ѡ����Ϊ��120 160 180 240 320 360 480��

#define SCC8660_CSI_W		SCC8660_CSI_PIC_W*2		//�˲���Ϊͼ�����ݴ洢��� �����޸�
#define SCC8660_CSI_H		SCC8660_CSI_PIC_H       //�˲���Ϊͼ�����ݴ洢�߶� �����޸�

//--------------------------------------------------------------------------------------------------
//��������
//--------------------------------------------------------------------------------------------------
#define SCC8660_CSI_COF_UART        USART_5         //��������ͷ��ʹ�õ��Ĵ���     
#define SCC8660_CSI_COF_UART_TX     UART5_TX_C28
#define SCC8660_CSI_COF_UART_RX     UART5_RX_C29


#define SCC8660_CSI_PCLK_PIN        CSI_PIXCLK_B20  //��������ʱ������
#define SCC8660_CSI_VSYNC_PIN       CSI_VSYNC_B22   //���峡�ź�����
//���������������ﲻ�ṩ���õ��������ŵĶ���
//���ڵڶ��������ɲ���û���������Գ���ֱ�ӹ̶�ʹ��B31-B24��������ͷ����������



extern uint16  scc8660_csi1_image[SCC8660_CSI_PIC_H][SCC8660_CSI_PIC_W];
extern uint16  scc8660_csi2_image[SCC8660_CSI_PIC_H][SCC8660_CSI_PIC_W];
extern uint16  (*user_color_image)[SCC8660_CSI_PIC_W];  //ͼ������

extern uint8   scc8660_csi_finish_flag;       //һ��ͼ��ɼ���ɱ�־λ

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
