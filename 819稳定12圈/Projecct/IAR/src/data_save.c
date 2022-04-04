#include "data.h"

int32_t *sdram_data = (int32_t *)SAVEDATA_START_ADDRESS;
uint8_t *sdram_img = (uint8_t *)SAVEIMG_START_ADDRESS;

void data_save()
{
    if (mycar.status == 1 || mycar.status == 2)
    {
        int32_t save_sdram[SAVE_NUM] = {0};
        save_sdram[0] = (int32_t)indata.speed;
        for (int i = 0; i < SAVE_NUM; i++)
            sdram_data[mycar.save_times * SAVE_NUM + i] = save_sdram[i];
        mycar.save_times++;
        mycar.savenum = mycar.save_times * SAVE_NUM;
    }
}

// void img_save()
// {
//     if (mycar.status == 1 || mycar.status == 2)
//     {
//         if (mycar.save_img_count > 1000)
//             return;
//         for (uint8_t i = 0; i < SAVE_IMG_V; i++)
//             for (uint8_t j = 0; j < SAVE_IMG_H; j++)
//             {
//                 if (setpara.save_img == 1)
//                 {
//                     //Vertical flip
//                     sdram_img[mycar.save_img_count * IMG_BYTE + i * SAVE_IMG_H + j] = im[SAVE_IMG_H - i - 1][j];
//                 }
//                 else if (setpara.save_img == 2)
//                 {
//                     //save imo
//                 }
//             }
//         mycar.save_img_count++;
//     }
// }

// void data_send()
// {
//     if (mycar.status == 0 && mycar.senddata == 1)
//     {
//         //DisableGlobalIRQ();
//         //USB虚拟串口使用需要打开中断
//         if (setpara.save_img == 0)
//         {
//             printf("%s\n", "Tags"); //print tags
//             for (int i = 0; i < (mycar.savenum / SAVE_NUM); i++)
//             {
//                 //匿名四轴上位机，前五个通道有效
//                 Uart1_Send_AF(0, 0, 0, 0, 0, 0, 0, 0);
//                 //Uart1_Send_AF(sdram[i * SAVE_NUM], sdram[i * SAVE_NUM + 1], 0, 0, 0, 0, 0, 0);
//                 send_wave();
//             }
//         }
//         else if (setpara.save_img == 1 || setpara.save_img == 2)
//         {
//             for (int i = 0; i < mycar.save_img_count; i++)
//             {
//                 //发送图像至逐飞上位机
//                 usb_cdc_send_char(0x00);
//                 usb_cdc_send_char(0xFF);
//                 usb_cdc_send_char(0x01);
//                 usb_cdc_send_char(0x01);

//                 usb_cdc_send_buff((uint8_t *)(sdram_img + i * IMG_BYTE), IMG_BYTE);
//                 delay_ms(10);
//             }
//         }
//         mycar.senddata = 0;
//         //EnableGlobalIRQ(0);
//     }
// }
