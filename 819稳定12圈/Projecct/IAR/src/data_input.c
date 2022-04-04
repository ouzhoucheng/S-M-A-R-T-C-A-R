#include "data.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

const float gx_to_delta_angle = PIT_MS * 2.0 / 32768; //TimePeriod(ms)*GyrFullScale(°/ms)/DataFullScale
const float gz_to_delta_angle = PIT_MS * 2.0 / 32768; //TimePeriod(ms)*GyrFullScale(°/ms)/DataFullScale


/*******************************************************************************
 * Variables
 ******************************************************************************/

//struct indata_STRUCT indata;

/*******************************************************************************
 * Code
 ******************************************************************************/
int textspeed;
void data_input()
{
    get_speed();
    get_angle();
    get_cam_fps();
    Get_BattVolt();
    //参数传递
    if (IsCaminfo)
    {
        indata.cam_dir = caminfo.dir;
        indata.cam_ddir = caminfo.ddir;
    }
    if(mycar.status == 4){
        indata.journey += indata.speed * 0.005;
    }

}

uint16_t Get_ADC1_Value(uint8_t chan)
{
    uint16_t adcval = 0;
    ADC1->HC[0] = chan;
    while (!(ADC1->HS & (1 << 0)))
        ;
    adcval = ADC1->R[0];
    return adcval;
}

uint16_t Get_ADC2_Value(uint8_t chan)
{
    uint16_t adcval = 0;
    ADC2->HC[0] = chan;
    while (!(ADC2->HS & (1 << 0)))
        ;
    adcval = ADC2->R[0];
    return adcval;
}

void Get_BattVolt()
{
    float fBattVolt = 0;
    fBattVolt = (float)adc_convert(ADC_1, ADC1_CH5_B16) / 3350.0 * 8.1 * 16.5;
    indata.battvolt = (int)(100 * fBattVolt);
    //  if(indata.battvolt < 700) //2-27 600->700 约7v警告
    //  {
    //    if(mycar.status == 1)
    //    {
    //      gpio_set(C10,1);  //电压不足警告
    //    }
    //    else    //停车时不报警
    //    {
    //      gpio_set(C10,0);
    //    }
    //  }
}

void get_speed()
{
    //mini系数计算：
    //一米走11圈
    indata.speedL = ENC_GetPositionValue(ENC1);
    textspeed = ENC_GetPositionValue(ENC1);
    indata.speedL = (-1) * indata.speedL * 1000 / 576;
    ENC_SetInitialPositionValue(ENC1, 0);
    ENC_DoSoftwareLoadInitialPositionValue(ENC1);

    indata.speedR = ENC_GetPositionValue(ENC2);
    indata.speedR = indata.speedR * 1000 / 576;
    ENC_SetInitialPositionValue(ENC2, 0);
    ENC_DoSoftwareLoadInitialPositionValue(ENC2);

    indata.speed = (indata.speedL + indata.speedR) / 2;
}

void get_angle()
{
    //获取数据-21O

    struct
    {
        uint8 reg;
        uint8 dat[6];
    } bufG;
    bufG.reg = ICM20602_GYRO_XOUT_H | ICM20602_SPI_R;

    icm_spi_r_reg_bytes(&bufG.reg, 7);
    indata.imu_gyr_x = (int16)(((uint16)bufG.dat[0] << 8 | bufG.dat[1]));
    indata.imu_gyr_y = (int16)(((uint16)bufG.dat[2] << 8 | bufG.dat[3]));
    indata.imu_gyr_z = (int16)(((uint16)bufG.dat[4] << 8 | bufG.dat[5]));

    struct
    {
        uint8 reg;
        uint8 dat[6];
    } bufA;
    bufA.reg = ICM20602_ACCEL_XOUT_H | ICM20602_SPI_R;

    icm_spi_r_reg_bytes(&bufA.reg, 7);
    indata.imu_acc_x = (int16)(((uint16)bufA.dat[0] << 8 | bufA.dat[1]));
    indata.imu_acc_y = (int16)(((uint16)bufA.dat[2] << 8 | bufA.dat[3]));
    indata.imu_acc_z = (int16)(((uint16)bufA.dat[4] << 8 | bufA.dat[5]));
    indata.ax = indata.imu_acc_x;
    indata.ay = indata.imu_acc_y;
    indata.az = indata.imu_acc_z;
////////////////////////陀螺仪矫正相关////////////////////////////////////
    static int16_t gx_h[10] = {0};
    static int16_t gy_h[10] = {0};
    static int16_t gz_h[10] = {0};

    for (uint8_t i = 9; i > 0; i--)
        gx_h[i] = gx_h[i - 1];
    for (uint8_t i = 9; i > 0; i--)
        gy_h[i] = gy_h[i - 1];
    for (uint8_t i = 9; i > 0; i--)
        gz_h[i] = gz_h[i - 1];

    /*偏移校正*/
    gx_h[0] = indata.imu_gyr_x - indata.gyr_bias_x;
    gy_h[0] = indata.imu_gyr_y - indata.gyr_bias_y;
    gz_h[0] = indata.imu_gyr_z - indata.gyr_bias_z;

    indata.gx = (gx_h[0] + gx_h[1] + gx_h[2] + gx_h[3] + gx_h[4] + gx_h[5] + gx_h[6] + gx_h[7] + gx_h[8] + gx_h[9]) / 10;
    indata.gy = (gy_h[0] + gy_h[1] + gy_h[2] + gy_h[3] + gy_h[4] + gy_h[5] + gy_h[6] + gy_h[7] + gy_h[8] + gy_h[9]) / 10;
    indata.gz = (gz_h[0] + gz_h[1] + gz_h[2] + gz_h[3] + gz_h[4] + gz_h[5] + gz_h[6] + gz_h[7] + gz_h[8] + gz_h[9]) / 10;

    //陀螺仪自动校准
    if (T > 1000 && T < 2000)
    {
        static int times = 1;
        static int gx_adj = 0;
        static int gy_adj = 0;
        static int gz_adj = 0;
        times++;
        gx_adj += indata.gx;
        gy_adj += indata.gy;
        gz_adj += indata.gz;
        if (times == (1000 / PIT_MS))
        {
            indata.gyr_bias_x = gx_adj / times;
            indata.gyr_bias_y = gy_adj / times;
            indata.gyr_bias_z = gz_adj / times;
            times = 0;
        }
    }
////////////////////////陀螺仪矫正相关////////////////////////////////////


    // if (caminfo.loop_flag)
    // {
    //     setpara.camcfg.loop_flag = caminfo.loop_flag;
    //     //loop_angle(indata.caminfo.loop_flag);      //圆环中对gz积分
    //     //beep();
    // }
/************** 华北赛 入库
    if(watch.zebra_flag && flag.CircCount == 3)    // 斑马线积分角
    {
        indata.YawAngle += gz_to_delta_angle*indata.gz;
        if((indata.YawAngle > 75 || indata.YawAngle < -75)&&mycar.status == 1)
        {
            // flag.stop = 1;   // 刹车
            mycar.status =2;
        }
    }
*******************/
    indata.PitchAngle += gz_to_delta_angle*indata.gy;

    int LoopPreOut = 200;
    int LoopOut = 270;
    if( watch.InLoop == 2 || watch.InLoop == 6 ||\
        watch.InLoop == 3 || watch.InLoop == 7)
    {
        indata.YawAngle += gz_to_delta_angle*indata.gz;
        if(indata.YawAngle > LoopPreOut && indata.YawAngle < LoopOut)
        {
            watch.InLoop = 3;
        }
        else if(indata.YawAngle < -LoopPreOut && indata.YawAngle > -LoopOut)
        {
            watch.InLoop = 7;
        }
        // else if(indata.YawAngle > LoopOut || indata.YawAngle < -LoopOut)
        // {
        //     clear_all_flags();
        //     indata.YawAngle = 0;
        // }
        else if(indata.YawAngle > LoopOut)
        {
            watch.InLoop = 4;
            indata.YawAngle = 0;
        }
        else if(indata.YawAngle < -LoopOut)
        {
            watch.InLoop = 8;
            indata.YawAngle = 0;
        }
    }


}

int getAngleFilter(int gx, int ay, int az, int k)
{
    float f_k = k * 0.0001;
    float angle_acc = 57.29577951 * atan2(ay, -az);
    indata.angle_last = (1 - f_k) * (indata.angle_last - gx_to_delta_angle * gx) + f_k * angle_acc;
    return (int)(100 * indata.angle_last);
}

void get_cam_fps()
{
    static int16_t last_cam_valid_count[6] = {0};
    if (T % 200 == 0)
    {
        for (uint8_t i = 5; i > 0; i--)
            last_cam_valid_count[i] = last_cam_valid_count[i - 1];
        last_cam_valid_count[0] = indata.cam_valid_count;
        indata.cam_fps = last_cam_valid_count[0] - last_cam_valid_count[5];
    }
}
