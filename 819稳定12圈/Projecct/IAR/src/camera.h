#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "car_global.h"

/*******************************************************************************
* Definitions
******************************************************************************/

//配置摄像头参数(Horizon Vertical)
#define IMG_H               160             //图像宽度  范围1-752       RT102X RT105X RT106X 采集时列宽度必须为4的倍数
#define IMG_V               120             //图像高度	范围1-480

//摄像头命令枚举
typedef enum
{
  INIT = 0,               //摄像头初始化命令
  AUTO_EXP,               //自动曝光命令
  EXP_TIME,               //曝光时间命令
  FPS,                    //摄像头帧率命令
  SET_COL,                //图像列命令
  SET_ROW,                //图像行命令
  LR_OFFSET,              //图像左右偏移命令
  UD_OFFSET,              //图像上下偏移命令
  GAIN,                   //图像偏移命令
  CONFIG_FINISH,          //非命令位，主要用来占位计数

  SET_EXP_TIME = 0XF0,    //单独设置曝光时间命令
  GET_STATUS,             //获取摄像头配置命令
  GET_VERSION,            //固件版本号命令
	
  SET_ADDR = 0XFE,        //寄存器地址命令
  SET_DATA                //寄存器数据命令
}CMD;

extern csi_handle_t csi_handle;
extern uint8_t (*im)[IMG_H];
extern uint32_t fullCameraBufferAddr;

/*******************************************************************************
* API
******************************************************************************/

void init_camera(void);
void init_camera_uart();
void init_camera_pins();
void set_exposure_time(uint16_t light);
void set_mt9v032_reg(uint8_t addr, uint16_t data);
uint8_t csi_get_full_buffer(csi_handle_t *handle, uint32_t *buffaddr);
void csi_add_empty_buffer(csi_handle_t *handle, uint8_t *buff);

#endif