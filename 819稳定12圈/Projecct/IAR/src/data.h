#ifndef _DATA_H_
#define _DATA_H_

#include "car_global.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

//SRRAM 32MB 空间分配
//SDRAM IVT_TABLE       0x80000000 - 0x800003FF         1K
//SDRAM                 0x80000400 - 0x81DFFFFF
//*代码运行空间*        0x80000400 - 0x801FFFFF         2M - 1K
//*数据存储空间*        0x80200000 - 0x802FFFFF         1M
//*图像存储空间*        0x80300000 - 0x81D00000         27M
//SDRAM ncache          0x81E00000 - 0x81FFFFFF         2M

#define SAVEDATA_START_ADDRESS (0x80200000U)
#define SAVEIMG_START_ADDRESS (0x80300000U)
#define SDRAM_MEM_LENGTH (32 * 1024 * 1024)

#define SAVE_IMG_H 160
#define SAVE_IMG_V 120
#define IMG_BYTE (SAVE_IMG_V * SAVE_IMG_H)
#define SAVE_NUM 1
#define Paralist_Size (64 * 1) //int类型的参数，占4个字节，一页可写入64个参数

#define PIT0_MS 2
#define FLASH_SAVE_SECTOR 255
#define BAL_MS 2
#define SPD_MS 2
#define DIR_MS 2
#define ANG_MS 10
#define MAX_PWM 9900
#define NORMAL_BAT 790

struct caminfo_s
{
    int16_t dir;
    int16_t ddir;
    int16_t zebra_count;
    int16_t apriltag_count;
    int16_t loop_process;
    int16_t loop_flag;
};
/**************输入数据****************/
typedef struct indata_STRUCT
{
    int d_dir;

    int battvolt;
    int16_t Speed;

    int16_t gx;
    int16_t gy;
    int16_t gz;
    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t fps;

    /*****************************/
    //  /*mpu6050*/
    int16_t imu_acc_x;
    int16_t imu_acc_y;
    int16_t imu_acc_z;
    int16_t imu_gyr_x;
    int16_t imu_gyr_y;
    int16_t imu_gyr_z;
    int16_t gyr_bias_x;
    int16_t gyr_bias_y;
    int16_t gyr_bias_z;

    float angle_last;
    // int angle;
    float YawAngle;
    float PitchAngle;

    /*camera*/
    int16_t cam_valid_count;
    int16_t cam_fps;
    int16_t cam_dir;
    int16_t cam_ddir;

    /*speed*/
    int speedL;
    int speedR;
    int speed;
    int setspeed;
    int diff_speedL;
    int diff_speedR;
    /**********************************/

    struct caminfo_s caminfo;

    float journey;
    float target;
    int16_t DetectFlag;
} indata_STRUCT;

typedef struct outdata_STRUCT
{
    /***************/
    int steer_pwm;
    int left_pwm;
    int right_pwm;
} outdata_STRUCT;

typedef struct STATUS_CAR_STRUCT
{
    uint8_t status;   //小车运行状态
    uint32_t RUNTIME; //小车运行时间
    uint32_t save_times;
    uint32_t savenum;
    uint32_t save_img_count;
    uint32_t senddata;

} STATUS_CAR_STRUCT;

/****************原始数据*****************/
typedef struct rawdata_STRUCT
{
    int16_t Speed;
    short gyr[3];
    short acc[3];
} rawdata_STRUCT;


typedef enum STATUS_BUTTON
{
    NONE = 0,

    PRESS,
    CW,
    CCW,

    UP,
    DOWN,
    PUSH
} STATUS_BUTTON;

//！考虑Flash存储的对齐，setpara各参数的类型均为int

struct camcfg_s
{
    int wl; // 左轮位置
    int wr; // 右轮位置
    int wf; // 摄像头位置
    int forward_near; // 0~120行，near之下的行不处理
    int forward_far; // 0~120行，far之上的行不处理
    int manbri; // 手调曝光/自动曝光
    int exptime; // 手调曝光时间
    int autogain; // 
    int setedge; // 
    int pitch; // 摄像头倾角
    int rotate_x; // 逆透视变换相关
    int rotate_y; // 逆透视变换相关
    int shift; // 逆透视变换相关
    int loop_flag; // 
};
////////////////////////////参数调节定义
typedef struct setpara_STRUCT
{
    int mode;   // 起步 0直线 1左出库 2右出库
    int settime;
/////////速度相关
    int CamAveSpeed;
    int SpeedKp;
    int SpeedKi;
    int SpeedKd;
    int diffK;
    int SpeedUP;
    int SpeedDOWN;
/////////舵机相关
    int SteerMid;
    int MaxTurn;
    int CamKp;
    int CamKd;
/////////属性、视觉相关
    struct camcfg_s camcfg;
    int exp_ki;
/////////车库相关
    int garage_outtime;
    int garage_outangle;
    int ZebraCount;
    int stop_kp;
    int stop_kd;
    int StopSpeed;
/////////圆环相关
    int CamLoopKp;
    int CamLoopKd;
    int InLoopLine;
    int WhenInLoop;
    int InLoopRow;
    int LoopThres;
    int OutLoopThres;
//////////三岔路相关
    int StrThres; //直线阈值
    int TempRL; //三岔路临时变量，左1右2，后期由OpenMV发送
    int JuncProcess; //过三岔路的时间
    int WhenJuncStop;   //三岔路停车时机
    int JuncSpeed;   //三岔路停车时机
    int DetectTime;//识别时间
    int number;//识别时间
/////////二维码相关
    int AprilCount;
    int AprilStop;
    int AprilSpeed ;
    int AprilTime;
    int BackTime;
    int LaserTime;
/////////测试相关
    int testL;
    int testR;
    int TextLine;
    int TextRow;
    // int TextDistance;
/////////识别参数
    int TempDectect;
    int DectectSpeed;
    int DectectSpeed10;
    int DsP;
    int DsI;
    int DsD;

    int SlopeDown;



} setpara_STRUCT;

struct PARA_LIST_STRUCT
{
    int *para;
    int8_t label[18];
    uint8_t precision;
};
/************标志位*************/

typedef struct FLAG_STRUCT
{
    int CircCount;
    int JuncCount;
    int JuncDirection;

    int16_t DetectMode;
} FLAG_STRUCT;
//struct FLAG_STRUCT
//{
////赛道标志位
//  uint8_t loop_left;
//  uint8_t loop_right;
//  uint8_t cross;
//  uint8_t loop_left_out;
//  uint8_t loop_right_out;
//};

extern enum STATUS_BUTTON status_button;
extern struct PARA_LIST_STRUCT paralist[Paralist_Size];
extern struct setpara_STRUCT setpara;
// extern OLED_STRUCT oled; //屏幕显示

extern uint32_t T; //PIT Timer
extern struct STATUS_CAR_STRUCT mycar;
extern struct FLAG_STRUCT flag;

extern struct indata_STRUCT indata;
extern struct outdata_STRUCT outdata;

/*******************************************************************************
* API
******************************************************************************/

uint16_t Get_ADC1_Value(uint8_t chan);
uint16_t Get_ADC2_Value(uint8_t chan);
void beep();
void data_input();
void data_process();
void data_output();
void data_save();
void get_speed();
void get_angle();
int getAngleFilter(int gx, int ay, int az, int k);
void get_cam_fps();
// void data_send();
// void online_debug();
void Get_BattVolt();

// void img_save();

#endif