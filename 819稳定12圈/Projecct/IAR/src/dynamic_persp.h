#ifndef _DYNAMIC_PERSP_H_
#define _DYNAMIC_PERSP_H_

#include "car_global.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

struct STATUS_CAMERAINFO
{
  float focalLengthX;
  float focalLengthY;
  float opticalCenterX;
  float opticalCenterY;
  uint8_t cameraHeight;
  float pitch;
  float yaw;
  uint8_t imageWidth;
  uint8_t imageHeight;
  int Y_scaling;
}; 

struct STATUS_IPMINFO
{
  float ipmWidth;
  float ipmHeight;
  float ipmLeft;
  float ipmRight;
  float ipmTop;
  float ipmBottom;
  
  //计算要用的
  float Image_c1;
  float Image_s1;
  float Image_c2;
  float Image_s2;
  //计算出来的
  float yfMax;
  float yfMin;
  float xfMax;
  float xfMin;
};

extern struct STATUS_CAMERAINFO cameraInfo;   //摄像头参数
extern struct STATUS_IPMINFO ipmInfo;        //输出的图像数组数据

/*******************************************************************************
 * API
 ******************************************************************************/

void calculate_persp();
void start_persp(int img_y,int img_x,int* persp_y,int* persp_x);
void init_persp();

#endif