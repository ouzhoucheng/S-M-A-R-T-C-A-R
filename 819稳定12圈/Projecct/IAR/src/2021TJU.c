/*
* @file 2020TJU.c
* @rev 1.0 By Powerglide
* @brief 第十五届天津大学智能车队RT1064基础工程 
*        第十五届天津大学智能车队双车组 芯火 四轮光电代码
*        第十六届天津大学智能车队   智能视觉    405基本入环无失误（注意标志清0（非补线
*/

#include "car_global.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

//uint32_t T;                       //PIT Timer
struct STATUS_CAR_STRUCT mycar; //运行状态与时间
struct FLAG_STRUCT flag;        //标志位集合

uint32_t T = 0;              //PIT计时器
STATUS_BUTTON status_button; //梅花桩、拨轮状态
rawdata_STRUCT rawdata;      //速度、角速度、加速度原始数据
indata_STRUCT indata;        //输入量
outdata_STRUCT outdata;      //输出量
setpara_STRUCT setpara;      //设定参数
//

uint8_t example_rx_buffer;
lpuart_transfer_t example_receivexfer;
lpuart_handle_t example_g_lpuartHandle;
uint8_t uart_data;

// uint8_t distance_rx_buffer;
// lpuart_transfer_t distance_receivexfer;
// lpuart_handle_t distance_g_lpuartHandle;
// uint8_t distance_data[24],distance_num = 0;
// int last_dis = 1000, lalast_dis = 1000;
// char isDis = 0;
// uint8_t ReadDis[5]={'r','6','#','\r','\n'};
// uint8_t P_mode[7]={'s','5','-','1','#','\r','\n'};
// uint8_t A_mode[7]={'s','5','-','0','#','\r','\n'};

/*******************************************************************************
 * Code
 ******************************************************************************/
//
void example_uart_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_LPUART_RxIdle == status)
    {
        uart_data = example_rx_buffer; //将数据取出
    }

////////////////////////////art通信
    if(mycar.status == 4 && flag.DetectMode == ART_wait)
    {
        switch (uart_data)
        {
            case Rx_num_even:   // 1偶数，前行30厘米
                flag.DetectMode = ART_num_even;
            break;
            case Rx_num_odd:    // 1偶数，前行30厘米
                flag.DetectMode = ART_num_odd;
            break;
            case Rx_tag_even:   // #偶数，前行30厘米
                flag.DetectMode = ART_tag_even;
            break;
            case Rx_tag_odd:    // #偶数，前行30厘米
                flag.DetectMode = ART_tag_odd;
            break;
        }
    }


    handle->rxDataSize = example_receivexfer.dataSize; //还原缓冲区长度
    handle->rxData = example_receivexfer.data;         //还原缓冲区地址
}

// void distance_uart_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
// {
//     if (kStatus_LPUART_RxIdle == status)
//     {
//         distance_data[distance_num] = distance_rx_buffer; //将数据取出
//         distance_num ++;
//     }

//         if(distance_num>4 && distance_data[distance_num-3]=='m' && distance_data[distance_num-4]=='m')
//         {
//             if((                  distance_data[distance_num-5]>='0')&&\
//                                 ( distance_data[distance_num-5]<='9'))
//                 watch.distance =  distance_data[distance_num-5]-'0';
//             if((distance_num>5)&&(    distance_data[distance_num-6]>='0')&&\
//                                 (     distance_data[distance_num-6]<='9'))
//                     watch.distance +=(distance_data[distance_num-6]-'0')*10;
//             if((distance_num>6)&&(    distance_data[distance_num-7]>='0')&&\
//                                 (     distance_data[distance_num-7]<='9'))
//                     watch.distance +=(distance_data[distance_num-7]-'0')*100;
//             if((distance_num>7)&&(    distance_data[distance_num-8]>='0')&&\
//                                     ( distance_data[distance_num-8]<='9'))
//                     watch.distance +=(distance_data[distance_num-8]-'0')*1000;
//             distance_num = 0;
//         }

//     handle->rxDataSize = distance_receivexfer.dataSize; //还原缓冲区长度
//     handle->rxData = distance_receivexfer.data;         //还原缓冲区地址
// }

  int main(void)
{
    BOARD_ConfigMPU();
    BOARD_BootClockRUN();
    NVIC_SetPriorityGrouping(((uint32_t)PriorityGroup));
    DisableGlobalIRQ();
    Hardware_Init();

    // M8266HostIf_Init();
    // M8266WIFI_Module_Init_Via_SPI();

    example_receivexfer.dataSize = 1;
    example_receivexfer.data = &example_rx_buffer;
    uart_set_handle(USART_4, &example_g_lpuartHandle, example_uart_callback, NULL, 0, example_receivexfer.data, 1);
    
    // distance_receivexfer.dataSize = 1;
    // distance_receivexfer.data = &distance_rx_buffer;
    // uart_set_handle(USART_2, &distance_g_lpuartHandle, distance_uart_callback, NULL, 0, distance_receivexfer.data, 1);

    EnableGlobalIRQ(0);

    while (1)
    {
        key_check();
        show_ips();
        
        // if(mycar.status == 5 && watch.servo_flag){
        // uart_putbuff(USART_2, ReadDis, 5);
        // }
    }
}

void delay_ms(uint32_t ms)
{
    uint32_t i, j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 200000; j++)
            __NOP();
    }
}

void delay_us(uint32_t us)
{
    uint32_t i, j;

    for (i = 0; i < us; i++)
    {
        for (j = 0; j < 290; j++)
            __NOP();
    }
}

void status_select()
{

    switch (mycar.status)
    {
        case 1:     //运行
            mycar.RUNTIME += PIT_MS;
            // watch.distance = 1799;
            if (mycar.RUNTIME > setpara.settime * 100)
                mycar.status = 2;
            break;

        case 2:     //刹车
            mycar.RUNTIME += PIT_MS;
            if (mycar.RUNTIME > (setpara.settime + 10) * 100) //刹车时间1s
            {
                mycar.status = 0;
                ips.img_showmode = 3;   //停车后继续显示
                // uart_putchar(USART_2,0x15);
                // Send_ART(0x15);
                
                clear_all_flags();
                // watch.zebra_flag = 0 ;
                // watch.ZebraLine = 120;
                // watch.ZebraRowL = 0  ;
                // watch.ZebraRowR = 159;

                indata.YawAngle = 0;
                // clear_flags();
                
                save_flash();
            }
            break;

        case 3:     //起步出库
            mycar.RUNTIME += PIT_MS;
            if (setpara.mode && mycar.RUNTIME < setpara.garage_outtime * 100 && mycar.RUNTIME > 0) //起步时间舵机打角0.5s,调节/0.01s
            {
                if(setpara.mode == OutLeft){      // 左边出车库
                    outdata.steer_pwm = setpara.SteerMid + setpara.garage_outangle;
                }
                else if(setpara.mode == OutRight){ // 右边出车库
                    outdata.steer_pwm = setpara.SteerMid - setpara.garage_outangle;
                }
            }
            else
            {
                mycar.status = 1;
                clear_all_flags();
                clear_flags();
                flag.CircCount = 1;     // 第一圈
                flag.JuncCount = 0;     // 未遇到三岔路
                flag.JuncDirection = 0;
            }
            break;

        case 4:     //赛道元素,三岔路
            mycar.RUNTIME += PIT_MS;
            if (mycar.RUNTIME > setpara.settime * 100)
                mycar.status = 2;

            switch(flag.DetectMode)
            {
                case ART_num_even:          // 1偶数，前行30厘米
                    indata.target = 30;
                    if(indata.journey > 29)
                        flag.DetectMode = ART_num_even_back;
                    break;
                case ART_num_even_back:     // 1偶数，后退30cm回原地
                    indata.target = 0;
                    if(indata.journey < 1)
                        flag.DetectMode = ART_wait;
                        Send_ART(0x33);
                    break;
                case ART_num_odd:           // 1奇数，后退30厘米
                    indata.target = -30;
                    if(indata.journey < -29)
                        flag.DetectMode = ART_num_odd_back;
                    break;
                case ART_num_odd_back:      // 1奇数，前进30cm回原地
                    indata.target = 0;
                    if(indata.journey > -1){
                        flag.DetectMode = ART_wait;
                        Send_ART(0x33);
                    }
                    break;
                case ART_tag_even:          // #偶数，前行10厘米
                    indata.target = 10;
                    if(indata.journey > 9.5)
                        flag.DetectMode = ART_tag_even_back;
                    break;
                case ART_tag_even_back:     // #偶数，后退10cm回原地
                    indata.target = 0;
                    if(indata.journey < 0.5){
                        flag.DetectMode = ART_wait;
                        Send_ART(0x33);
                    }
                    break;
                case ART_tag_odd:           // #奇数，后退10厘米
                    indata.target = -10;
                    if(indata.journey < -9.5)
                        flag.DetectMode = ART_tag_odd_back;
                    break;
                case ART_tag_odd_back:      // #奇数，前进10cm回原地
                    indata.target = 0;
                    if(indata.journey > -0.5){
                        flag.DetectMode = ART_wait;
                        Send_ART(0x33);
                    }
                    break;
            }
            
            
            break;

        default:
            break;
    }
}

void Send_ART(uint8_t SendData)
{
    uart_putchar(USART_4,SendData);
}

void Send_distance(uint8_t SendData)
{
    uart_putchar(USART_2,SendData);
}

void clear_flags()
{
    indata.journey = 0;
    // watch.distance = 1799;
    // watch.servo_flag = 0;
    gpio_set(C10,0);
}
