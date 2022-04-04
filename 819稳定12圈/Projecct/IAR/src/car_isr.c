#include "car_isr.h"


void PIT_IRQHandler(void)
{
    if ((PIT_GetStatusFlags(PIT, kPIT_Chnl_0) & kPIT_TimerFlag) == kPIT_TimerFlag)
    {
        T += PIT_MS;

        status_select();
        data_input();
        data_process();
        data_output();

        switch(setpara.TextRow)
        {
            case 1:
//////////////////圆环
                SendF2(watch.InLoop, watch.InLoopAngle, watch.InLoopCirc, watch.InLoopAngle2, 0);
                break;
            case 2:
//////////////////三岔路
                SendF2(watch.Junc_flag, watch.JuncLine, watch.JuncAngle, watch.JuncAngleX,flag.JuncCount);
                break;
            case 3:
//////////////////二维码
                SendF2(watch.April_flag, watch.AprilLine, mycar.status, watch.DeltaL,indata.speed);
                break;
            case 4:
//////////////////三岔路
                SendF2(flag.JuncCount, flag.JuncDirection,watch.Junc_flag,watch.JuncLine,watch.JuncAngle);
                break;
            case 5:
//////////////////调方向
                SendF2(indata.cam_dir, outdata.steer_pwm, outdata.left_pwm,outdata.right_pwm,indata.speed);
                break;
            case 6:
//////////////////调速度
                SendF2(indata.setspeed, flag.DetectMode,indata.speedR,indata.speed,(int)indata.journey);
                break;
            case 7:
//////////////////标志
                // SendF2(watch.distance, watch.journey, 0, watch.servo_flag,watch.April_flag);
                break;
            default:
                break;
        }
        // if( setpara.TextRow == 8)
        // {
        //     SendFlag[0] = 155;
        //     SendFlag[1] = watch.InLoop;
        //     SendFlag[2] = watch.InLoopAngle;
        //     SendFlag[3] = watch.InLoopCirc;
        //     SendFlag[4] = watch.InLoopAngle2;
        //     DMA_start();
        //     SendFlag[0] = 165;
        //     SendFlag[1] = watch.Junc_flag;
        //     SendFlag[2] = watch.JuncLine;
        //     SendFlag[3] = watch.JuncAngle;
        //     SendFlag[4] = watch.JuncAngleX;
        //     DMA_start();
        // }
    }

    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    __DSB();
}

void CSI_IRQHandler(void)
{
    CSI_DriverIRQHandler();
    __DSB();
}

uint32_t K;

void GPIO1_Combined_0_15_IRQHandler(void)
{
    delay_ms(1);
    if (GPIO_GetPinsInterruptFlags(GPIO1) & (1 << 2))
    {
        if (!GPIO_PinRead(GPIO1, 2))
        {
            K++;
            status_button = PRESS;
        }

        GPIO_PortClearInterruptFlags(GPIO1, 1 << 2);
    }

    if (GPIO_GetPinsInterruptFlags(GPIO1) & (1 << 14))
    {
        if (!GPIO_PinRead(GPIO1, 14))
        {
            K++;
            status_button = UP;
        }
        GPIO_PortClearInterruptFlags(GPIO1, 1 << 14);
    }

    if (GPIO_GetPinsInterruptFlags(GPIO1) & (1 << 15))
    {
        if (!GPIO_PinRead(GPIO1, 15))
        {
            K++;
            status_button = PUSH;
        }
        GPIO_PortClearInterruptFlags(GPIO1, 1 << 15);
    }

    if (GPIO_GetPinsInterruptFlags(GPIO1) & (1 << 3))
    {
        if (!GPIO_PinRead(GPIO1, 3))
        {
            K++;
            status_button = DOWN;
        }
        GPIO_PortClearInterruptFlags(GPIO1, 1 << 3);
    }
    __DSB();
}