#ifndef _CAR_GLOBAL_H_
#define _CAR_GLOBAL_H_

//SDK_drivers
#include "clock_config.h"
#include "board.h"
#include "fsl_cache.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_pit.h"
#include "fsl_adc.h"
#include "fsl_lpuart.h"
#include "fsl_enc.h"
#include "fsl_xbara.h"
#include "fsl_pwm.h"
#include "fsl_csi.h"
#include "fsl_lpi2c.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_lpuart_edma.h"
#include "zf_uart.h"
#include "zf_pwm.h"
#include "zf_gpio.h"
#include "zf_iomuxc.h"
#include "zf_uart.h"
#include "zf_adc.h"
#include "zf_spi.h"

//app_inc
#include "car_init.h"
#include "car_isr.h"
#include "ips_display.h"
#include "ips200.h"
#include "data.h"
#include "setpara.h"
#include "camera.h"
#include "camera_process.h"
//#include "mpu6050.h"
//#include "lpi2c_mpu.h"
#include "dynamic_persp.h"
#include "outputdata.h"

//common
#include "common.h"
#include "rom_api.h"
#include "vector.h"
#include "gpio_cfg.h"
#include "flash.h"
#include "seekfree_printf.h"
#include "usb_cdc.h"
#include "wave.h"

//usb_cdc
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "usb_phy.h"

//ICM
#include "SEEKFREE_ICM20602.h"

//wave
#include "uart_wave.h"
#include "uart_dma.h"

// //wave
// #include "M8266WIFI_ops.h"
// #include "M8266HostIf.h"
// #include "brd_cfg.h"
// #include "M8266WIFIDrv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define _LIMIT(data, min, max) (((int)data) < (min) ? (min) : ((data) > (max) ? (max) : (data)))
#define _MIN(x, y) (x) < (y) ? (x) : (y)

#define TREAD 15
#define WHEEL_DISTANCE 24
#define PI 3.14159265358979f

#define PIT_MS 5

#define Run 1
#define Debug 0

#define GoAhead  0
#define OutLeft  1
#define OutRight 2

#define ART_wait     0 // ͣ���ȴ�
#define ART_num_even 1 // 1ż����ǰ��30����
#define ART_num_odd  2 // 1����������30����
#define ART_tag_even 3 // #ż����ǰ��10����
#define ART_tag_odd  4 // #����������10����
#define ART_animal   5 // �������ͷ����ҡ�����Σ��Ƕȴ���90��
#define ART_fruit    6 // ˮ����������Լ��̴��

#define Rx_num_even 0x12 // 1ż����ǰ��30����
#define Rx_num_odd  0x11 // 1ż����ǰ��30����
#define Rx_tag_even 0x22 // #ż����ǰ��30����
#define Rx_tag_odd  0x21 // #ż����ǰ��30����

#define ART_num_even_back 11 // 1ż��������30cm��ԭ��
#define ART_num_odd_back  12 // 1������ǰ��30cm��ԭ��
#define ART_tag_even_back 13 // #ż��������10cm��ԭ��
#define ART_tag_odd_back  14 // #������ǰ��10cm��ԭ��
/************
0λ��ռ���ȼ�,4λ�����ȼ� : 0x7
1λ��ռ���ȼ�,3λ�����ȼ� : 0x6
2λ��ռ���ȼ�,2λ�����ȼ� : 0x5
3λ��ռ���ȼ�,1λ�����ȼ� : 0x4
4λ��ռ���ȼ�,0λ�����ȼ� : 0x3
**************/
#define PriorityGroup 0x3

typedef unsigned char         uint8_t;
typedef unsigned short int    uint16_t;
typedef unsigned int          uint32_t;
typedef   signed char         sint8_t;

typedef uint32_t		u32;
typedef uint16_t		u16;
typedef uint8_t			u8;
typedef sint8_t			s8;


/*******************************************************************************
 * API
 ******************************************************************************/

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
void status_select();
void Send_ART(uint8_t SendData);
void Send_distance(uint8_t SendData);
void clear_flags();


#endif