#include "camera_process.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define abs_m(a,b) (a>=b ? a-b:b-a)
struct caminfo_s caminfo;
struct watch_o watch;
struct lineinfo_s lineinfo[120];
uint8_t imo[120][160];
uint8_t IsCaminfo;
uint8_t IsDisplay;

/*总钻风最大曝光时间
    15帧：2125     50帧：636     60帧：529  */
uint16_t max_int_time_us = 636;
uint16_t int_time_us;
// uint8_t exp_ki = 3;
// uint8_t LoopThres = 6;
int cam_times = 0;
uint16_t OTSU_Hz = 6;  //每10帧进行一次大津法

uint8_t SendLine[6] = {0,0,0,0,0,0x0a};
uint8_t SendFlag[6] = {0,0,0,0,0,0x0a};
uint8_t SendFlagHz = 0, SendFlagHz2 = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
int test1 = 200;
int test2 = 20;
void Caminfo_Cal()
{
    IsCaminfo = 0;
    memset(lineinfo, 0, IMG_V * sizeof(struct lineinfo_s));
    //////////////////////////////曝光时间及模拟增益调整//////////////////////////

    static int last_autogain = 30;
    if (setpara.camcfg.manbri && !(cam_times % OTSU_Hz)) //手动调节图像亮度
        set_exposure_time(setpara.camcfg.exptime);
    else if(T<1000) //自动调整亮度—O21
    {
        watch.brightness = img_otsu((uint8_t *)im, IMG_H, IMG_V, 10); //大津法调整曝光—O21
        int_time_us -= (abs(watch.brightness - 128) > 3 ? setpara.exp_ki * (watch.brightness - 128) : 0);
        int_time_us = _LIMIT(int_time_us, 1, max_int_time_us);
        set_exposure_time(int_time_us);
    }
    else if(!(cam_times % OTSU_Hz)) //自动调整亮度—O21
    {
        watch.brightness = img_otsu((uint8_t *)im, IMG_H, IMG_V, 10); //大津法调整曝光—O21
        int_time_us -= (abs(watch.brightness - 128) > 3 ? setpara.exp_ki * (watch.brightness - 128) : 0);
        int_time_us = _LIMIT(int_time_us, 1, max_int_time_us);
        set_exposure_time(int_time_us);
        cam_times = 0;
    }
    if (last_autogain != setpara.camcfg.autogain)
    {
        last_autogain = setpara.camcfg.autogain;
        set_mt9v032_reg(0x35, setpara.camcfg.autogain);
    }
    //按行搜索最佳边界
    //以特定行为基准，各自独立获得最优两组边界
    /////////////更新圆环内状态//////////////
    // watch.loop_flag = setpara.camcfg.loop_flag;
    /////////////////////////////////////////
    line_single(&lineinfo[watch.base_line], im[watch.base_line]);

    for (int y = watch.base_line-1; y >= setpara.camcfg.forward_near; y--)
        line_findnext(&lineinfo[y], im[y], &lineinfo[y + 1]);

    // int rightx = 255, leftx = 0; //圆环出口初始化
    for (int y = watch.base_line+1; y < setpara.camcfg.forward_far; y++)
    {
        lineinfo[y].y = y;
        line_findnext(&lineinfo[y], im[y], &lineinfo[y - 1]);
        watch.watch_line = y;
    }
    // int xl_last, xr_last;
    int xl, oxl, oyl;
    int xr, oxr, oyr;
    //改
////////////////
    for (int y = setpara.camcfg.forward_near; y < watch.watch_lost; y++)
    {
        xl = lineinfo[y].left;
        xr = lineinfo[y].right;

        ////////////////////圆环入口补线1-21区602////////////////////
        // 入环左前
        if (watch.InLoop == 1 && watch.InLoopAngle < watch.InLoopCirc && watch.zebra_flag == 0 &&\
            y > watch.InLoopAngle && y < 50 && watch.RLStraight == 2 && watch.InLoopAngle2 == 120)
        {
            xl = lineinfo[y].right - 135 + y;
        }
        // 开始入左环
        float slopeL;
        if (watch.InLoop == 1 && y > (watch.InLoopCirc - 5) && y < watch.InLoopAngle2) //&& y>watch.InLoopCirc
        {
            slopeL = (159.0 - lineinfo[watch.InLoopAngle2 + 2].left * 1.0) / (watch.InLoopAngle2);
            xr = (watch.InLoopAngle2 - y) * slopeL + lineinfo[watch.InLoopAngle2 + 2].left;
            xl = 0;
        }
        if (watch.InLoop == 1 && y >= watch.InLoopAngle2)
        {
            slopeL = (lineinfo[watch.InLoopAngle2 + 2].left * 1.0) / (watch.watch_lost - watch.InLoopAngle2);
            xr = lineinfo[watch.InLoopAngle2 + 2].left - slopeL * (y - watch.InLoopAngle2);
            xl = 0;
        }

        float slopeOL;
        if (watch.InLoop == 3 && watch.OutLoopRight > 60 && y > watch.OutLoopRightY && y < watch.watch_line)
        {
            slopeOL = 1.0 * watch.OutLoopRight / (watch.watch_line - watch.OutLoopRightY);
            xr = slopeOL * (watch.watch_line - y);
            xl = 0;     
        }
        // 出左环直行
        if (watch.InLoop == 4 && y < watch.OutLoopAngle && watch.zebra_flag == 0 && watch.RLStraight == 2)
        {
            xl = lineinfo[y].right - 132 + y;
            if(flag.JuncCount%2){       // 因为圆环不在三岔内，假如漏了一个三岔，则补全
                flag.JuncCount++;
            }
        }

        ///////////// 入右环前
        // if(setpara.TextLine){
        if (watch.InLoop == 5 && watch.InLoopAngle < watch.InLoopCirc && watch.zebra_flag == 0 &&\
            y > watch.InLoopAngle && y < 50 && watch.RLStraight == 1 && watch.InLoopAngle2 == 120)
        {
            xr = lineinfo[y].left + 135 - y;
        }
        // 开始入环
        // float slope; *setpara.WhenInLoop/10  &&lineinfo[watch.InLoopAngle2+2].right>10
        float slopeR;
        if (watch.InLoop == 5 && y > (watch.InLoopCirc - 5) && y < watch.InLoopAngle2) //&& y>watch.InLoopCirc
        {
            slopeR = (lineinfo[watch.InLoopAngle2 + 2].right * 1.0) / (watch.InLoopAngle2);
            xl = lineinfo[watch.InLoopAngle2 + 2].right - (watch.InLoopAngle2 - y) * slopeR;
            xr = 159;
        }
        if (watch.InLoop == 5 && y >= watch.InLoopAngle2)
        {
            slopeR = (159.0 - lineinfo[watch.InLoopAngle2 + 2].right * 1.0) / (watch.watch_lost - watch.InLoopAngle2);
            xl = lineinfo[watch.InLoopAngle2 + 2].right + slopeR * (y - watch.InLoopAngle2);
            xr = 159;
        }
        ///////////////出右环
        float slopeOR;
        if (watch.InLoop == 7 && watch.OutLoopLeft < 99 && y > watch.OutLoopLeftY && y < watch.watch_line)
        {
            slopeOR = (159.0 - watch.OutLoopLeft) / (watch.watch_line - watch.OutLoopLeftY);
            xl = 159 - slopeOR * (watch.watch_line - y);
            xr = 159;
        }
        // 出右环直行
        if (watch.InLoop == 8 && y < watch.OutLoopAngle && watch.zebra_flag == 0 && watch.RLStraight == 1)
        {
            xr = lineinfo[y].left + 132 - y;
            if(flag.JuncCount%2){       // 因为圆环不在三岔内，假如漏了一个三岔，则补全
                flag.JuncCount++;
            }
        }


        ////////////////////三岔路补线1-21区602////////////////////
        float slopeJ;
        //三岔路右
        if ( watch.Junc_flag == 2 && flag.JuncDirection == 2 && y < watch.JuncAngle && watch.JuncAngleX)
        {
            slopeJ = 1.0*watch.JuncAngleX/ watch.JuncAngle;
            xl = slopeJ * y;
            if (xr < 80)
            {
                xr = 159;
            }
            if(!gpio_get(C10))
            {
                gpio_set(C10,1);
            }
        }
        //三岔路左
        if ( watch.Junc_flag == 1 && flag.JuncDirection == 1 && y < watch.JuncAngle && watch.JuncAngleX)
        {
            slopeJ = 1.0*(159 - watch.JuncAngleX)/ watch.JuncAngle;
            xr = 159 - slopeJ * y;
            if (xl > 79)
            {
                xl = 0;
            }
            if(!gpio_get(C10))
            {
                gpio_set(C10,1);
            }
        }
        

        static float last_pitch = 0.79;
        cameraInfo.pitch = (float)setpara.camcfg.pitch / 1000;
        if (last_pitch != cameraInfo.pitch)
        {
            last_pitch = cameraInfo.pitch;
            calculate_persp();
        }
        start_persp(y, xl, &oyl, &oxl);
        oyl *= 4;
        lineinfo[y].persp_lx = oxl;
        lineinfo[y].persp_ly = oyl;

        start_persp(y, xr, &oyr, &oxr);
        oyr *= 4;
        lineinfo[y].persp_rx = oxr;
        lineinfo[y].persp_ry = oyr;
        
        ///////////////////////////////////////////////////////////////////////////
        if (ips.img_showmode == 3) //160*120
        {
            if (xl < 160)
                imo[y][xl] = 1;
            if (xr < 160)
                imo[y][xr] = 2;
        }
        else if (ips.img_showmode == 4) //128*128
        {
            imo[oyl / 4][oxl / 4] = 1;
            imo[oyr / 4][oxr / 4] = 2;
        }
        //////准备发送图像数据
        SendFlagHz ++;
        if( setpara.TextRow == 8 && SendFlagHz < 100)
        {
            SendLine[0] = y;
            SendLine[1] = xl;
            SendLine[2] = xr;
            DMA_start();
        }
        else if(setpara.TextRow == 8 && SendFlagHz > 100 && SendFlagHz2)
        {
            SendLine[0] = 155;
            SendLine[1] = watch.InLoop;
            SendLine[2] = watch.JuncLineL;
            SendLine[3] = watch.JuncLineR;
            SendLine[4] = watch.InLoopAngle2;
            DMA_start();
            SendFlagHz = 0;
            SendFlagHz2 = 0;
        }
        else if(setpara.TextRow == 8 && SendFlagHz > 100 && !SendFlagHz2)
        {
            SendLine[0] = 165;
            SendLine[1] = watch.Junc_flag;
            SendLine[2] = watch.JuncLine;
            SendLine[3] = watch.JuncAngle;
            SendLine[4] = watch.JuncAngleX;
            DMA_start();
            SendFlagHz = 0;
            SendFlagHz2 = 1;
        }
    }
    watch.cross = 0;
    watch.left_lost = 0;
    watch.right_lost = 0;

    ////////////////////////////新检测直线—21区—6-3//////////////////////////////////////
    char StrNumR = 0, StrNumL = 0;
    for (uint8_t ys = 30; ys < 80; ys += 2)
    {
        //比较右边直线
        if (!lineinfo[ys - 20].right_lost && !lineinfo[ys + 20].right_lost && !lineinfo[ys].right_lost)
        {
            char deltaR;
            deltaR = abs(lineinfo[ys - 20].right + lineinfo[ys + 20].right -lineinfo[ys].right - lineinfo[ys].right);
            if (deltaR < 3)
            {
                StrNumR++;
            }
        }
        //比较左边直线
        if (!lineinfo[ys - 20].left_lost && !lineinfo[ys + 20].left_lost && !lineinfo[ys].left_lost)
        {
            char deltaL;
            deltaL = abs(lineinfo[ys - 20].left + lineinfo[ys + 20].left -lineinfo[ys].left - lineinfo[ys].left);
            if (deltaL < 3)
            {
                StrNumL++;
            }
        }
    }
    unsigned char StrThreshold = 18, LostThreshold = 10; //直线段阈值与丢线阈值
    if (StrNumL < setpara.StrThres && StrNumR > setpara.StrThres && watch.right_lost < LostThreshold)
    {
        watch.RLStraight = 2; //右边直线左边不是
    }
    else if (StrNumL > setpara.StrThres && StrNumR < setpara.StrThres && watch.left_lost < LostThreshold)
    {
        watch.RLStraight = 1; //左边直线右边不是
    }
    else if (StrNumL > setpara.StrThres && StrNumR > setpara.StrThres && watch.left_lost < LostThreshold && watch.right_lost < LostThreshold)
    {
        watch.RLStraight = 3; //左边右边直线
    }
    else
    {
        watch.RLStraight = 0;
    }

    for (uint8_t y = setpara.camcfg.forward_near; y < setpara.camcfg.forward_far; y++)
    {
        int d_xl = lineinfo[y].left;
        int d_xr = lineinfo[y].right;

        if (d_xl >= 0 && d_xl <= 160)
            lineinfo[y].angel_left = 250 * (lineinfo[y].persp_lx - setpara.camcfg.wl * 4) / (lineinfo[y].persp_ly - setpara.camcfg.wf * 4);
        if (d_xr >= 0 && d_xr <= 160)
            lineinfo[y].angel_right = 250 * (lineinfo[y].persp_rx - setpara.camcfg.wr * 4) / (lineinfo[y].persp_ry - setpara.camcfg.wf * 4);
        // if (watch.variance_r >= 20) //右侧方差不合格错误报告
        //     caminfo.loop_process = caminfo.loop_process | 0x0040;
        ////////////////////////////////////原圆环检测//////////////////////////////
        if (y > 10)
        {
            if (lineinfo[y].left_lost && lineinfo[y].right_lost)
            {
                if (y > 7)
                {
                    watch.cross++;
                }
            }
            else if (lineinfo[y].left_lost && !lineinfo[y].right_lost)
            { //左丢线行—21区
                watch.left_lost++;
            }
            else if (!lineinfo[y].left_lost && lineinfo[y].right_lost)
            { //右丢线行—21区
                watch.right_lost++;
            }
        }
    }
    // watch.number = 0;
///////////////////////////////////大循环
    for (uint8_t y = setpara.camcfg.forward_near; y < setpara.camcfg.forward_far; y += 1)
    {
        /////////////////////////////新环岛识别2—21区602/////////////////////////////
        ////////////////左圆环
        //找左圆环前第一个角点，补一段直线
        if (!watch.Junc_flag &&\
            watch.InLoop == 0 &&\
            watch.RLStraight == 2 &&\
            y<(setpara.camcfg.forward_far-10) &&\
            caminfo.zebra_count < 2 &&\
            caminfo.apriltag_count < 2 &&\
            lineinfo[y-1].left > 19 &&\
            (lineinfo[y + 2].right-lineinfo[y].left>49) &&\
            (lineinfo[y + 2].right-lineinfo[y].left<149) &&\
            lineinfo[y + 3].left_lost  && !lineinfo[y - 3].left_lost &&\
            lineinfo[y + 2].left_lost  && !lineinfo[y - 2].left_lost &&\
            !lineinfo[y - 1].left_lost && !lineinfo[y - 4].left_lost &&\
            abs_m(lineinfo[y - 1].left,lineinfo[y - 3].left)<setpara.LoopThres&&\
            abs_m(lineinfo[y - 2].left,lineinfo[y - 4].left)<setpara.LoopThres&&\
            !lineinfo[y + 3].right_lost && !lineinfo[y - 3].right_lost &&\
            !lineinfo[y + 2].right_lost && !lineinfo[y - 2].right_lost)
        { //左圆环的第一个角点
            watch.InLoopAngle = y;
            if (watch.InLoopAngle < setpara.InLoopLine && watch.InLoopAngle > 15)
            {
                watch.InLoop = 1;
                watch.InLoopAngle = 0;
                // uart_putchar(USART_1,watch.InLoop);
            }
        }
        //左圆环上弧所在行
        if (watch.InLoop == 1 &&\
            y<(setpara.camcfg.forward_far-5)&&\
            y > setpara.WhenInLoop-30 && y > watch.InLoopAngle &&\
            caminfo.apriltag_count < 2 &&\
            watch.InLoopCirc &&\
            lineinfo[y - 1].left < 19 &&\
            (lineinfo[y - 2].right-lineinfo[y-2].left>49) &&\
            (lineinfo[y - 2].right-lineinfo[y-2].left<149) &&\
            lineinfo[y + 3].left_lost  && !lineinfo[y - 3].left_lost &&\
            lineinfo[y + 2].left_lost  && !lineinfo[y - 2].left_lost &&\
            !lineinfo[y - 1].left_lost &&\
            lineinfo[y - 1].left < lineinfo[y - 3].left &&\
            lineinfo[y].left < lineinfo[y - 2].left &&\
            !lineinfo[y + 2].right_lost && !lineinfo[y - 2].right_lost)
        { //入环点
            watch.InLoopCirc = y;
        }
        //左圆环第二个角点,连线
        if (watch.InLoop == 1 &&\
            y > (watch.InLoopCirc-5)&&\
            y < (setpara.camcfg.forward_far-5) &&\
            lineinfo[y+1].left > 19 &&\
            caminfo.apriltag_count < 2 && !lineinfo[y + 3].left_lost &&\
            !lineinfo[y + 2].left_lost && !lineinfo[y + 1].left_lost &&\
            lineinfo[y - 1].left_lost  && lineinfo[y - 3].left_lost &&\
            lineinfo[y + 1].left < lineinfo[y + 3].left &&\
            lineinfo[y + 2].left < lineinfo[y + 4].left &&\
            (lineinfo[y + 3].left - lineinfo[y + 1].left)<setpara.LoopThres &&\
            (lineinfo[y + 4].left - lineinfo[y + 2].left)<setpara.LoopThres &&\
            !lineinfo[y + 2].right_lost && !lineinfo[y - 2].right_lost)
        {
            watch.InLoopAngle2 = y;
            if(watch.InLoopAngle2<setpara.WhenInLoop)
            {
                watch.InLoopCirc=0;
            }
        }
        //成功入环,取消补线避免过度
        if (watch.InLoop == 1 && lineinfo[watch.InLoopAngle2 + 2].left > (159-setpara.InLoopRow))
        {
            watch.InLoop = 2;
            watch.InLoopCirc = 0;
            watch.InLoopAngle2 = 0;
            watch.zebra_flag = 0;
            // uart_putchar(USART_1,watch.InLoop);
        }
        //出左环找右角点横纵坐标
        if ((watch.InLoop == 3) && \
            lineinfo[y].left_lost && watch.zebra_flag == 0 && \
            lineinfo[y + 2].right > lineinfo[y].right && \
            lineinfo[y - 3].right > lineinfo[y - 1].right && \
            lineinfo[y + 4].right > lineinfo[y + 2].right && \
            lineinfo[y - 5].right > lineinfo[y - 3].right && \
            lineinfo[y].right > 30)
        {
            watch.OutLoopRight = lineinfo[y].right;
            watch.OutLoopRightY = y; //出环判断列 （左右
        }
        //出左环锁定向前
        if (watch.InLoop == 4 && \
            y<(setpara.camcfg.forward_far-10) && watch.zebra_flag == 0 &&\
            !lineinfo[y + 3].left_lost && !lineinfo[y + 4].left_lost &&\
            !lineinfo[y + 2].left_lost && !lineinfo[y + 1].left_lost &&\
            lineinfo[y - 1].left_lost  && lineinfo[y - 4].left_lost &&\
            !lineinfo[y + 3].right_lost && !lineinfo[y - 3].right_lost &&\
            !lineinfo[y + 2].right_lost && !lineinfo[y - 2].right_lost )
        {
            if((watch.OutLoopAngle = y) < setpara.OutLoopThres)
            {
                clear_all_flags();
            }
        }

        ////////////////右圆环
        //找右圆环前第一个角点，补一段直线
        if (!watch.Junc_flag &&\
            watch.InLoop == 0 &&\
            watch.RLStraight == 1 &&\
            y<(setpara.camcfg.forward_far-10)&&\
            caminfo.zebra_count < 2 &&\
            caminfo.apriltag_count < 2 &&\
            lineinfo[y-1].right < 140 &&\
            (lineinfo[y + 2].right-lineinfo[y].left>49) &&\
            (lineinfo[y + 2].right-lineinfo[y].left<149) &&\
            lineinfo[y + 3].right_lost  && !lineinfo[y - 3].right_lost &&\
            lineinfo[y + 2].right_lost  && !lineinfo[y - 2].right_lost &&\
            !lineinfo[y - 1].right_lost && !lineinfo[y - 4].right_lost &&\
            abs_m(lineinfo[y - 1].right,lineinfo[y - 3].right)<setpara.LoopThres&&\
            abs_m(lineinfo[y - 2].right,lineinfo[y - 4].right)<setpara.LoopThres&&\
            !lineinfo[y + 3].left_lost && !lineinfo[y - 3].left_lost &&\
            !lineinfo[y + 2].left_lost && !lineinfo[y - 2].left_lost)
        { //右圆环的第一个角点
            watch.InLoopAngle = y;
            if (watch.InLoopAngle < setpara.InLoopLine && watch.InLoopAngle > 15)
            {
                watch.InLoop = 5;
                watch.InLoopAngle = 0;
                // uart_putchar(USART_1, watch.InLoop);
            }
        }
        //右圆环上弧所在行
        if (watch.InLoop == 5 &&\
            y<(setpara.camcfg.forward_far-5)&&\
            y > setpara.WhenInLoop-30 && y > watch.InLoopAngle &&\
            caminfo.apriltag_count < 2 &&\
            watch.InLoopCirc &&\
            lineinfo[y - 1].right > 140 &&\
            (lineinfo[y - 2].right-lineinfo[y-2].left>50) &&\
            (lineinfo[y - 2].right-lineinfo[y-2].left<150) &&\
            lineinfo[y + 3].right_lost && !lineinfo[y - 3].right_lost &&\
            lineinfo[y + 2].right_lost && !lineinfo[y - 2].right_lost &&\
            !lineinfo[y - 1].right_lost &&\
            lineinfo[y - 1].right > lineinfo[y - 3].right &&\
            lineinfo[y].right > lineinfo[y - 2].right &&\
            !lineinfo[y + 2].left_lost && !lineinfo[y - 2].left_lost)
        { //入环点
            watch.InLoopCirc = y;
        }
        //右圆环第二个角点,连线
        if (watch.InLoop == 5 &&\
            y > (watch.InLoopCirc-5) &&\
            y<(setpara.camcfg.forward_far-5)&&\
            lineinfo[y+1].right < 139 &&\
            caminfo.apriltag_count < 2 && !lineinfo[y + 3].right_lost &&\
            !lineinfo[y + 2].right_lost && !lineinfo[y + 1].right_lost &&\
            lineinfo[y - 1].right_lost  && lineinfo[y - 3].right_lost &&\
            lineinfo[y + 1].right > lineinfo[y + 3].right &&\
            lineinfo[y + 2].right > lineinfo[y + 4].right &&\
            (lineinfo[y + 1].right - lineinfo[y + 3].right)<setpara.LoopThres &&\
            (lineinfo[y + 2].right - lineinfo[y + 4].right)<setpara.LoopThres &&\
            !lineinfo[y + 2].left_lost && !lineinfo[y - 2].left_lost)
        {
            watch.InLoopAngle2 = y;
            if(watch.InLoopAngle2<setpara.WhenInLoop)
            {
                watch.InLoopCirc=0;
            }
        }
        //成功入环,取消补线避免过度
        if (watch.InLoop == 5 && lineinfo[watch.InLoopAngle2 + 2].right < (setpara.InLoopRow - 14) &&lineinfo[watch.InLoopAngle2 + 2].right)
        {
            watch.InLoop = 6;
            watch.InLoopCirc = 0;
            watch.InLoopAngle2 = 0;
            watch.zebra_flag = 0;
            // uart_putchar(USART_1, watch.InLoop);
        }

        // //出环找右角点横纵坐标
        if ((watch.InLoop == 7) &&\
            watch.zebra_flag == 0 && \
            lineinfo[y].right_lost \
            && lineinfo[y + 2].left < lineinfo[y].left \
            && lineinfo[y - 3].left < lineinfo[y - 1].left \
            && lineinfo[y + 4].left < lineinfo[y + 2].left \
            && lineinfo[y - 5].left < lineinfo[y - 3].left \
            && lineinfo[y].left < 129)
        {
            watch.OutLoopLeft = lineinfo[y].left;
            watch.OutLoopLeftY = y; //出环判断列 （左右
        }
        // 出右环锁定向前
        if (watch.InLoop == 8 && \
            y<(setpara.camcfg.forward_far-10)&& watch.zebra_flag == 0 &&\
            !lineinfo[y + 3].right_lost && !lineinfo[y + 4].right_lost &&\
            !lineinfo[y + 2].right_lost && !lineinfo[y + 1].right_lost &&\
            lineinfo[y - 1].right_lost  && lineinfo[y - 4].right_lost &&\
            !lineinfo[y + 3].left_lost && !lineinfo[y - 3].left_lost &&\
            !lineinfo[y + 2].left_lost && !lineinfo[y - 2].left_lost)
        {
            if((watch.OutLoopAngle = y) < setpara.OutLoopThres)
            {
                clear_all_flags();
            }
            // uart_putchar(USART_1, watch.OutLoopAngle);
        }

/////////////////////////////新三岔路识别2—21区602/////////////////////////////
        if (!watch.RLStraight && !watch.InLoop) //左右都不是直线,非圆环状态
        {
            if( watch.Junc_flag != 4 &&\
                lineinfo[y + 1].edge_count == 4 &&\
                lineinfo[y + 2].edge_count == 4 &&\
                lineinfo[y + 3].edge_count == 4 &&\
                lineinfo[y + 4].edge_count == 4 &&\
                lineinfo[y - 1].edge_count == 2 &&\
                lineinfo[y - 2].edge_count == 2 &&\
                lineinfo[y - 3].edge_count == 2 &&\
                lineinfo[y + 1].black_width>lineinfo[y    ].black_width &&\
                lineinfo[y + 2].black_width>lineinfo[y + 1].black_width &&\
                lineinfo[y + 3].black_width>lineinfo[y + 2].black_width &&\
                lineinfo[y + 4].black_width>lineinfo[y + 3].black_width &&\
                abs(lineinfo[y  ].junc_angle- lineinfo[y+1].junc_angle) < 5 &&\
                abs(lineinfo[y+1].junc_angle- lineinfo[y+2].junc_angle) < 5 &&\
                abs(lineinfo[y+2].junc_angle- lineinfo[y+3].junc_angle) < 5 &&\
                abs(lineinfo[y+3].junc_angle- lineinfo[y+4].junc_angle) < 5 &&\
                
                lineinfo[y+1].junc_angle > y*2/3 && lineinfo[y+1].junc_angle < (159-y*2/3))
                {
                    watch.JuncAngle = y;
                    watch.JuncAngleX = lineinfo[y+1].junc_angle;
                }
            if (watch.Junc_flag != 4 && watch.JuncAngle<120 && y<watch.JuncAngle && lineinfo[y].right > 80 &&\
                !lineinfo[y].right_lost && !lineinfo[y+3].right_lost &&\
                !lineinfo[y+1].right_lost && !lineinfo[y+4].right_lost &&\
                !lineinfo[y-3].right_lost && !lineinfo[watch.JuncLineR+2].right_lost &&\
                !lineinfo[y-2].right_lost && !lineinfo[y-5].right_lost &&\
                !lineinfo[y-7].right_lost )
            {
                char UpRDel, DownRDel, UpRDely, DownRDely;
                UpRDel = lineinfo[y + 4].right - lineinfo[y + 1].right;
                DownRDel = lineinfo[y - 5].right - lineinfo[y - 2].right;
                UpRDely = lineinfo[y + 3].right - lineinfo[y].right;
                DownRDely = lineinfo[y - 3].right - lineinfo[y].right;
                if (UpRDel > 0 && UpRDel < 15 && DownRDel >= 0 && DownRDel < 8 &&
                    UpRDely > 0 && UpRDely < 15 && DownRDely >= 0 && DownRDely < 8) //找突起角
                {
                    watch.JuncLineR = y; //右边角的行
                }
            }
            //三岔路左边点
            if (watch.Junc_flag != 4 && watch.JuncAngle<120 && y<watch.JuncAngle && lineinfo[y].left < 79 &&\
                !lineinfo[y].left_lost && !lineinfo[y+3].left_lost &&\
                !lineinfo[y+1].left_lost && !lineinfo[y+4].left_lost &&\
                !lineinfo[y-3].left_lost && !lineinfo[watch.JuncLineL+2].left_lost &&\
                !lineinfo[y-2].left_lost && !lineinfo[y-5].left_lost &&\
                !lineinfo[y-7].left_lost ) //左边沿搜角
            {
                char UpLDel, DownLDel, UpLDely, DownLDely;
                UpLDel = lineinfo[y + 1].left - lineinfo[y + 4].left;
                // ips200_showint32(100, 14, UpLDel, 5);
                DownLDel = lineinfo[y - 2].left - lineinfo[y - 5].left;
                UpLDely = lineinfo[y].left - lineinfo[y + 3].left;   //上两行做差（3
                DownLDely = lineinfo[y].left - lineinfo[y - 3].left; //下两行做差（3
                if (UpLDel > 0 && UpLDel < 15 && DownLDel >= 0 && DownLDel < 8 &&
                    UpLDely > 0 && UpLDely < 15 && DownLDely >= 0 && DownLDely < 8) //找突起角
                {
                    watch.JuncLineL = y; //左边角的行
                }
            }

            if (abs(watch.JuncLineL - watch.JuncLineR) < 15 &&\
                lineinfo[watch.JuncLineL].right-lineinfo[watch.JuncLineL].left > 25 &&\
                lineinfo[watch.JuncLineR].right-lineinfo[watch.JuncLineR].left > 25 &&\
                lineinfo[watch.JuncLineL].right-lineinfo[watch.JuncLineL].left < 150 &&\
                lineinfo[watch.JuncLineR].right-lineinfo[watch.JuncLineR].left < 150 &&\
                !lineinfo[watch.JuncLineL+2].left_lost &&\
                !lineinfo[watch.JuncLineR+2].right_lost &&\
                !lineinfo[watch.JuncLineL-5].left_lost &&\
                !lineinfo[watch.JuncLineR-5].right_lost ) //防止十字干扰
            {
                watch.JuncLine = (watch.JuncLineL + watch.JuncLineR) / 2; //取中间
            }

            if (watch.JuncAngle < 100 && watch.JuncLine < 85 && watch.JuncLine > 20 && !watch.Junc_flag)
            {
                watch.Junc_flag = 3;
            }
            if((lineinfo[watch.JuncLineL+2].left_lost ||
                lineinfo[watch.JuncLineR+2].right_lost)&&\
                watch.Junc_flag != 4 && watch.Junc_flag != 2 && watch.Junc_flag != 1 )        //抗干扰
            {
                watch.JuncLine = 120;
                watch.JuncLineL = 120;
                watch.JuncLineR = 120;
                watch.JuncAngle = 120;
                watch.JuncAngleX = 0;
                watch.Junc_flag = 0;
            }
            if(abs(watch.JuncLine - watch.JuncAngle)>80 &&\
                watch.Junc_flag != 4 && watch.Junc_flag != 2 && watch.Junc_flag != 1){   // 互补抗干扰
                watch.JuncLine = 120;
                watch.JuncLineL = 120;
                watch.JuncLineR = 120;
                watch.JuncAngle = 120;
                watch.JuncAngleX = 0;
                watch.Junc_flag = 0;
            }
/**********************************原三岔路检测******************************/
///////////////////////////// 国赛三岔路检测
            if(watch.Junc_flag == 3 &&\
                watch.JuncAngle < 120 &&\
                (watch.JuncAngle - watch.JuncLine) > 5 &&\
                watch.JuncAngle<setpara.WhenJuncStop+30 &&\
                mycar.status == 1 &&\
                flag.JuncCount==0)  //第一次进三岔路
            {
                watch.JuncTime = mycar.RUNTIME;
                flag.JuncDirection = setpara.TempRL;
                watch.Junc_flag = flag.JuncDirection;
                // Send_ART(0x31);
                watch.zebra_flag = 0;
            }
            else if(watch.Junc_flag == 3 &&\
                watch.JuncAngle < 120 &&\
                (watch.JuncAngle - watch.JuncLine) > 5 &&\
                watch.JuncAngle<setpara.WhenJuncStop+30 &&\
                mycar.status == 1 &&\
                flag.JuncCount != 0)
            {
                watch.zebra_flag = 0;
                if(flag.JuncCount % 2 == 1)     //奇数总是出三岔路（之前走过了奇数个）
                {
                    watch.JuncTime = mycar.RUNTIME;
                    watch.Junc_flag = flag.JuncDirection;
                }
                else if(flag.JuncCount % 2 == 0 && flag.JuncCount > 0) //偶数总是入三岔路（之前走过了偶数个）
                {
                    watch.JuncTime = mycar.RUNTIME;
                    if(flag.JuncDirection == 1)
                    {
                        flag.JuncDirection = 2;
                        watch.Junc_flag = flag.JuncDirection;
                    }
                    else if(flag.JuncDirection == 2)
                    {
                        flag.JuncDirection = 1;
                        watch.Junc_flag = flag.JuncDirection;
                    }
                }
            }


            //三岔路补线时间
            if ((mycar.RUNTIME> watch.JuncTime + setpara.JuncProcess*100)&&(watch.Junc_flag==1 || watch.Junc_flag==2) && watch.JuncAngleX)
            {
                watch.Junc_flag = 0;
                watch.JuncLine = 120;
                watch.JuncLineL = 120;
                watch.JuncLineR = 120;
                watch.JuncAngle = 120;
                watch.JuncAngleX = 0;
                watch.JuncTime = 0;
                flag.JuncCount ++;  //三岔路次数
                // Send_ART(0x15);
                indata.PitchAngle = 0;
                clear_flags();
                
            }
        }
        

    }
///////////////////////////////十字处理
    ips200_showint32(90, 14, watch.cross_lost, 5);
    watch.cross_lost = 0;
    if(!watch.InLoop && !watch.Junc_flag && !watch.RLStraight &&\
        lineinfo[32].left_lost && lineinfo[21].right_lost)
    {
        for(char yc=setpara.camcfg.forward_near; yc<60; yc++)
        {
            if(lineinfo[yc].left_lost && lineinfo[yc].right_lost)
            watch.cross_lost++;
        }
        if(watch.cross_lost>25)
        {
            watch.base_line = 80;
            indata.PitchAngle = 0;
            watch.Junc_flag = 0;
            watch.JuncLine = 120;
            watch.JuncLineL = 120;
            watch.JuncLineR = 120;
            watch.JuncAngle = 120;
            watch.JuncAngleX = 0;
            // watch.JuncTime = 0;
        }
    }
    else
    {
        watch.base_line = 20;

    }

    if (watch.RLStraight == 3) //左右都是直线，所有标志清0
    {
        clear_all_flags();

        watch.zebra_flag = 0 ;
        watch.ZebraLine = 120;
        watch.ZebraRowL = 0  ;
        watch.ZebraRowR = 159;
        indata.YawAngle = 0;
        if(!caminfo.apriltag_count && !mycar.status)
            watch.April_flag = 0;
    }
    else if (watch.RLStraight == 1 || watch.RLStraight == 2)
    {
        watch.JuncLine = 120;
        watch.Junc_flag = 120;
        watch.Junc_flag = 0;
    }
    if (watch.InLoop && watch.InLoop != 1 && watch.InLoop != 5)
    {
        watch.JuncLine = 120;
        watch.Junc_flag = 120;
        watch.Junc_flag = 0;
    }

////////////////////////舵机角度计算,
    int AngleLeft = (int)0x80000000;
    int AngleRight = 0x7FFFFFFF;
    int AngleLeftLast = 0, AngleRightLast = 0;
    int target_line_y = setpara.camcfg.forward_far - 2;

    for (uint8_t y = setpara.camcfg.forward_near; y < setpara.camcfg.forward_far; y++)
    {
        if (0.3125 / 2 * ((int)lineinfo[y].persp_ly + lineinfo[y].persp_ry) < setpara.camcfg.forward_near)
        {
            continue;
        }
        if (AngleLeft < lineinfo[y].angel_left && (abs(AngleLeft - AngleLeftLast) < 400))
        {
            AngleLeft = lineinfo[y].angel_left;
            // watch.watch_ang_left = y;
        }
        if (AngleRight > lineinfo[y].angel_right && (abs(AngleRight - AngleRightLast) < 400))
        {
            AngleRight = lineinfo[y].angel_right;
            // watch.watch_ang_right = y;
        }
        if (AngleLeft > AngleRight || 0.3125 / 2 * ((int)lineinfo[y].persp_ly + lineinfo[y].persp_ry) > setpara.camcfg.forward_far)
        {
            target_line_y = y - 1;
            break;
        }
        AngleLeftLast = AngleLeft;
        AngleRightLast = AngleRight;
    }
    static int16_t last_dir = 0;

    caminfo.dir = AngleLeftLast + AngleRightLast;
    
    caminfo.ddir = caminfo.dir - last_dir;
    last_dir = caminfo.dir;
    
    indata.cam_valid_count++;
    IsCaminfo = 1;

    if (ips.img_showmode == 3) //160*120
    {
        ips200_draw_horizon(0, 119 - watch.watch_lost, 160, 119 - watch.watch_lost, WHITE);
    }
    else if (ips.img_showmode == 4) //128*128
    {
        //两轮位置
        imo[0][setpara.camcfg.wl] = 3; //55
        imo[0][setpara.camcfg.wr] = 3; //75
                                       //变化点位置
        for (uint8_t y = 0; y < target_line_y; y++)
        {
            uint16_t imo_l = (setpara.camcfg.wl * 4 + AngleLeftLast * (lineinfo[y].persp_ly - setpara.camcfg.wf * 4) / 1000);
            uint16_t imo_r = (setpara.camcfg.wr * 4 + AngleLeftLast * (lineinfo[y].persp_ry - setpara.camcfg.wf * 4) / 1000);
            if (imo_l < 4 * (IMG_H - 1))
                imo[lineinfo[y].persp_ly / 4][imo_l / 4] = 3;
            if (imo_r < 4 * (IMG_H - 1))
                imo[lineinfo[y].persp_ry / 4][imo_r / 4] = 3;
        }
    }
    cam_times ++; // 大津法计数
    // ips200_showint32(100, 14, cam_times, 5);
    if (ips.img_showmode)
        IsDisplay = 1;
    else
        IsDisplay = 0;
}

////////////////////// 直线清除所有标志
int clear_all_flags()
{
    //清楚圆环标志
    watch.InLoop = 0;
    watch.InLoopAngle = 120;
    watch.InLoopCirc = 120;
    watch.InLoopAngle2 = 120;

    watch.OutLoopRight = 0;
    watch.OutLoopRightY = 0;
    watch.OutLoopLeft = 160;
    watch.OutLoopLeftY = 0;
    watch.OutLoopAngle = 120;

    //清楚三岔路标志
    watch.JuncLine = 120;
    watch.JuncLineL = 120;
    watch.JuncLineR = 120;
    watch.Junc_flag = 0;
    watch.JuncAngle = 120;
    watch.JuncAngleX = 0;

    indata.PitchAngle = 0;
    // flag.stop = 0;
    // indata.YawAngle = 0;
    gpio_set(C10,0);
    // if(caminfo.apriltag_count<3)
    //     watch.AprilLine = 120 ;
}

int img_otsu(uint8_t *img, uint8_t img_v, uint8_t img_h, uint8_t step)
{
    uint8_t grayhist[_GRAY_SCALE] = {0}; //灰度直方图
    uint16_t px_sum_all = 0;             //像素点总数
    uint32_t gray_sum_all = 0;           //总灰度积分
    uint16_t px_sum = 0;                 //像素点数量
    uint32_t gray_sum = 0;               //灰度积分

    float fTemp_maxvar = 0;
    uint8_t temp_best_th = 0;
    uint8_t temp_best_th2 = 0;
    uint8_t temp_this_pixel = 0;
    float fCal_var;
    float u0, u1, w0, w1;

    //生成：1. 灰度直方图 2. 像素点总数 3. 总灰度积分
    for (int i = 0; i < img_v; i += step)
    {
        for (int j = 0; j < img_h; j += step)
        {
            temp_this_pixel = img[i * img_h + j] >> _SHIFT_FOR_GRAYSCALE;
            if (temp_this_pixel < _GRAY_SCALE)
                grayhist[temp_this_pixel]++;
            gray_sum_all += temp_this_pixel;
            px_sum_all++;
        }
    }
    //  //总平均灰度
    //  float u = 1.0*gray_sum_all/px_sum_all;
    //迭代求得最大类间方差的阈值
    for (uint8_t k = 0; k < _GRAY_SCALE; k++)
    {
        px_sum += grayhist[k];       //该灰度及以下的像素点数量
        gray_sum += k * grayhist[k]; //该灰度及以下的像素点的灰度和
        w0 = 1.0 * px_sum / px_sum_all;
        w1 = 1.0 - w0;
        if (px_sum > 0)
            u0 = 1.0 * gray_sum / px_sum;
        else
            u0 = 0.0;
        if (px_sum_all - px_sum > 0)
            u1 = 1.0 * (gray_sum_all - gray_sum) / (px_sum_all - px_sum);
        else
            u1 = 0.0;
        //fCal_var = w0*(u0-u)*(u0-u)+w1*(u1-u)*(u1-u);
        fCal_var = w0 * w1 * (u0 - u1) * (u0 - u1);
        if (fCal_var > fTemp_maxvar)
        {
            fTemp_maxvar = fCal_var;
            temp_best_th = k;
            temp_best_th2 = k;
        }
        else if (fCal_var == fTemp_maxvar)
        {
            temp_best_th2 = k;
        }
    }
    return (temp_best_th + temp_best_th2) * _Thresh_Mult;
}

uint8_t get_orign_edges(uint8_t *inputimg, uint8_t *edge_store)
{
#if _EDGE_STORE_SIZE % 2 != 0
#error "_EDGE_STORE_SIZE must be even!"
#endif
    //查找并存储所有跳变沿对
    uint8_t edge_store_idx = 0;
    if (inputimg[0] & 0x80) //0位置最高位是1，说明-1~0是上升沿
    {
        edge_store[edge_store_idx] = 0;
        edge_store_idx++;
    }
    for (uint8_t px = 1; px < LINE_WIDTH; px++)
    {
        if (((inputimg[px - 1] ^ inputimg[px]) & 0x80) == 0) //最高位不相等，说明px-1~px是跳变沿   分布在128两侧就认为是跳边沿。。不稳定
    //    if(!((inputimg[px-1]>watch.gray&&inputimg[px]<watch.gray)||(inputimg[px-1]<watch.gray&&inputimg[px]>watch.gray)))    //可变灰度区分值
            continue;
        if (edge_store_idx >= _EDGE_STORE_SIZE)
            break;
        edge_store[edge_store_idx] = px;
        edge_store_idx++;
    }
    if (inputimg[LINE_WIDTH - 1] & 0x80) //末位置最高位是1，说明LINE_WIDTH-1~LINE_WIDTH是下降沿
    {
        if (edge_store_idx < _EDGE_STORE_SIZE)
        {
            edge_store[edge_store_idx] = LINE_WIDTH - 1;
            edge_store_idx++;
        }
    }
    return edge_store_idx;
}

int get_best_edge(struct lineinfo_s *lineinfo, uint8_t *edge_store, struct lineinfo_s *lineinfo_ref)
{
    int min_abs_err;
    uint8_t temp_left_idx = 0, temp_right_idx = 1;
    int temp_err;
    //获得最近邻的边沿
    min_abs_err = LINE_WIDTH * 2;
    for (uint8_t k = 0; k < lineinfo->edge_count; k += 2)
    {
        temp_err = abs((int)lineinfo_ref->left - (int)edge_store[k]);
        if (temp_err < min_abs_err)
        {
            min_abs_err = temp_err;
            temp_left_idx = k;
        }
        if (edge_store[k] > lineinfo_ref->left + min_abs_err)
            break;
    }
    temp_right_idx = temp_left_idx + 1;
    min_abs_err = LINE_WIDTH * 2;
    for (uint8_t k = temp_right_idx; k < lineinfo->edge_count; k += 2)
    {
        temp_err = abs((int)lineinfo_ref->right - (int)edge_store[k]);
        if (temp_err < min_abs_err)
        {
            min_abs_err = temp_err;
            temp_right_idx = k;
        }
        if (edge_store[k] > lineinfo_ref->right + min_abs_err)
            break;
    }

    if (abs((int)edge_store[temp_right_idx] - edge_store[temp_left_idx]) > 4)
    {
        watch.watch_lost = watch.watch_line;
        // camcfg.watch_lost=  camcfg.watch_line;
    }

    //判断中间是否有杂点，障碍物，环岛等
    //累加中间的黑色部分求取比例
    int black_pix_count = 0;
    for (int k = temp_left_idx + 1; k < temp_right_idx; k += 2)
        black_pix_count += edge_store[k + 1] - edge_store[k];
    if (8 * black_pix_count > edge_store[temp_right_idx] - edge_store[temp_left_idx]) //如果有不可忽略的杂物
    {
        //选取最大的块    --可能的BUG:此处无法继续忽略噪点
        int max_width = 0;
        int max_idx = temp_left_idx;

        int dst = temp_left_idx + 1;
        uint8_t dst_flag;
        for (int i = temp_left_idx + 1; i < temp_right_idx; i += 2)
        {
            if (edge_store[i + 1] - edge_store[i] < _MIN_EDGE_WIDTH)
            {
                edge_store[dst] = edge_store[i + 2];
                lineinfo->edge_count -= 2;
                dst_flag = 1;
            }
            //  else中内容注释掉现象依然正确  但可能有坑
            else
            {
                if (dst_flag == 1) //如果之前已经出现过被忽略的黑块
                {
                    edge_store[dst + 2] = edge_store[i];
                    edge_store[dst + 3] = edge_store[i + 1];
                }
                else // 当前不可忽略的黑块为第一块
                {
                    edge_store[dst] = edge_store[i];
                    edge_store[dst + 1] = edge_store[i + 1];
                }
                dst += 2;
            }
        }

        for (int k = temp_left_idx; k < lineinfo->edge_count - 1; k += 2)
        {
            if (edge_store[k + 1] - edge_store[k] > max_width)
            {
                max_width = edge_store[k + 1] - edge_store[k];
                max_idx = k;
            }
        }
        temp_left_idx = max_idx;
        temp_right_idx = max_idx + 1;
    }

    lineinfo->left = edge_store[temp_left_idx];
    lineinfo->right = edge_store[temp_right_idx];
    if (lineinfo->right >= 158)
        lineinfo->right_lost = 1;
    else
        lineinfo->right_lost = 0;
    if (lineinfo->left <= 0)
        lineinfo->left_lost = 1;
    else
        lineinfo->left_lost = 0;
    return 0;
}

int get_max_edge(struct lineinfo_s *lineinfo, uint8_t *edge_store)
{
    //选取最大的块    --可能的BUG:此处无法忽略噪点
    int max_width = 0;
    int max_idx = 0;

    if (lineinfo->edge_count < 10)
    {
        for (int k = 0; k < lineinfo->edge_count; k += 2)
        {
            if (edge_store[k + 1] - edge_store[k] > max_width)
            {
                max_width = edge_store[k + 1] - edge_store[k];
                max_idx = k;
            }
        }
    }
    else
    {
        max_idx = 0;
    }

    lineinfo->left = edge_store[max_idx];
    lineinfo->right = edge_store[lineinfo->edge_count - 1];
    return 0;
}

int line_single(struct lineinfo_s *lineinfo, unsigned char *inputimg)
{
    //得到所有凸边沿
    uint8_t edge_store[_EDGE_STORE_SIZE] = {0};
    lineinfo->edge_count = get_orign_edges(inputimg, edge_store);

    //得到最大边沿
    get_max_edge(lineinfo, edge_store);
    return 0;
}

int line_findnext(struct lineinfo_s *lineinfo, uint8_t *inputimg, struct lineinfo_s *lineinfo_ref)
{
    //得到所有凸边沿
    uint8_t edge_store[_EDGE_STORE_SIZE] = {0};
    lineinfo->edge_count = get_orign_edges(inputimg, edge_store);
    // watch.addline_y = lineinfo->y;
    // if (lineinfo->y > 20 && lineinfo->y < 100){
    //     zebra_detect(lineinfo, edge_store, inputimg); //斑马线检测
        // apriltag_detect(lineinfo, edge_store, inputimg); //apriltag检测
    // }
    //得到最佳边沿
    get_best_edge(lineinfo, edge_store, lineinfo_ref);
    Junc_detect(lineinfo, edge_store, inputimg); //apriltag检测
    
    return 0;
}

//斑马线探测
// int zebra_detect(struct lineinfo_s *lineinfo, uint8_t *edge_store, uint8_t *inputimg)
// {
//     uint8_t white_width, zobra_white_count, edge_now;
//     zobra_white_count = 0;
//         if (lineinfo->edge_count > 14)
//         {
//             for (uint8_t k = 0; k < lineinfo->edge_count; k += 2)
//             {
//                 edge_now = edge_store[k];
//                 if (edge_now < LINE_WIDTH)
//                 {
//                     if ((inputimg[edge_now] & 0x80) == 0x80)
//                     {
//                         white_width = edge_store[k + 1] - edge_store[k];
//                         if (white_width > 1 && white_width < 14)
//                         {
//                             zobra_white_count++;
//                         }
//                     }
//                 }
//             }
//             if (zobra_white_count > 6)
//             {
//                 lineinfo->zebra_flag = 1;
//                 // if(!mycar.status)
//                 //     ips200_draw_horizon(0, 119 - lineinfo->y, 160, 119 - lineinfo->y, BLUE);
//             }
//         }
//         else
//         {
//             lineinfo->zebra_flag = 0;
//         }
//     return 0;
// }

// int apriltag_detect(struct lineinfo_s *lineinfo, uint8_t *edge_store, uint8_t *inputimg)
// {
//     // if(lineinfo->y == setpara.TextLine)
//     //     ips200_showint32(100, 14, lineinfo->edge_count, 5);
//     uint8_t Awhite_width, Awhite_count, Aedge_now;
//     Awhite_count = 0;
//         if (lineinfo->edge_count > 8 && lineinfo->edge_count < 16)
//         {
//             for (uint8_t k = 0; k < lineinfo->edge_count; k += 2)
//             {
//                 Aedge_now = edge_store[k];
//                 if (Aedge_now < LINE_WIDTH && Aedge_now>(lineinfo->y/2) && Aedge_now<(159-lineinfo->y/2))
//                 {
//                     if ((inputimg[Aedge_now] & 0x80) == 0x80)
//                     {
//                         Awhite_width = edge_store[k + 1] - edge_store[k];
//                         if (Awhite_width > 1 && Awhite_width < 14)
//                         {
//                             Awhite_count++;
//                             // lineinfo->apriltag_flag = 1;
//                         }
//                     }
//                     // ips200_drawpoint(lineinfo->y/2,119 - lineinfo->y,BLUE);
//                     // ips200_drawpoint(159-lineinfo->y/2,119 - lineinfo->y,BLUE);
//                 }
                
//                 if (Awhite_count > 1 && Awhite_count < 4)
//                 {
//                     lineinfo->apriltag_flag = 1;
//                     if(!mycar.status)
//                         ips200_draw_horizon(0, 119 - lineinfo->y, 160, 119 - lineinfo->y, BROWN);
//                 }
//                 // if(lineinfo->apriltag_flag && !mycar.status){
//                 //     ips200_draw_horizon(0, 119 - lineinfo->y, 160, 119 - lineinfo->y, BROWN);
//                 // }
//             }
//         }
//         else
//         {
//             lineinfo->apriltag_flag = 0;
//         }
// }

int Junc_detect(struct lineinfo_s *lineinfo, uint8_t *edge_store, uint8_t *inputimg)
{
    if(lineinfo->y == setpara.TextLine)
    {
        // ips200_showint32(50, 14, edge_store[2]-edge_store[1], 5);
        // ips200_showint32(0, 14, edge_store[1], 5);

    }
    uint8_t JBlack_width,wb_count,bw_count;
    if(lineinfo->edge_count==4 && edge_store[2]>39 && edge_store[1]<120){
        JBlack_width = edge_store[2]-edge_store[1];
        if(JBlack_width<70)
            lineinfo->black_width = JBlack_width;
            lineinfo->junc_angle = (edge_store[2]+edge_store[1])/2;
    }
    else
        JBlack_width = 100;
    // if(lineinfo->y == setpara.TextLine)
    //     ips200_showint32(0, 14, lineinfo->black_width, 5);
}

