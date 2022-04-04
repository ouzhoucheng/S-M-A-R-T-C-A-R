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
  
  //����Ҫ�õ�
  float Image_c1;
  float Image_s1;
  float Image_c2;
  float Image_s2;
  //���������
  float yfMax;
  float yfMin;
  float xfMax;
  float xfMin;
};

extern struct STATUS_CAMERAINFO cameraInfo;   //����ͷ����
extern struct STATUS_IPMINFO ipmInfo;        //�����ͼ����������

/*******************************************************************************
 * API
 ******************************************************************************/

void calculate_persp();
void start_persp(int img_y,int img_x,int* persp_y,int* persp_x);
void init_persp();

#endif