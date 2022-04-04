#include "ips_display.h"

struct IPS_STRUCT ips;

void show_upperpage(uint8_t page)
{
    static int lastpage;
    if (lastpage != page)
    {
        ips200_address_set(0, 120, 159, 239);
        for (uint8_t j = 0; j < 120; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
        lastpage = page;
    }
    switch (page)
    {
    case 0:
        ips200_showstr(0, 8, "brightness:");
        ips200_showint32(100, 8, watch.brightness, 3);

        ips200_showstr(0, 9, "dir:");
        ips200_showint32(35, 9, indata.cam_dir, 5);

        // ips200_showstr(0, 10, "dSteerPWM:");
        // ips200_showint32(100, 10, setpara.SteerMid - outdata.steer_pwm, 4);

        //      ips200_showstr(0, 11, "Distance:");
        //      ips200_showint32(100, 11, indata.dataa, 4);

        ips200_showstr(5, 10, "M:");
        ips200_showint32(30, 10, (int)(indata.PitchAngle), 4);

        ips200_showstr(5, 11, "O:");
        ips200_showint32(30, 11, watch.InLoop, 5);
        //
        ips200_showstr(70, 11, "I:");
        ips200_showint32(95, 11, watch.RLStraight, 5);

        ips200_showstr(5, 12, "Y:");
        ips200_showint32(30, 12, watch.Junc_flag, 5);


        ips200_showstr(5, 13, "J:");
        ips200_showint32(30, 13, (int)(indata.YawAngle), 5);

        ips200_showstr(70, 12, "A:");
        ips200_showint32(95, 12, flag.DetectMode, 5);

        ips200_showstr(70, 13, "+:");
        ips200_showint32(95, 13, watch.base_line, 5);

        // ips200_showint32(0, 14, indata.PitchAngle, 5);
        // ips200_showint32(50, 14, indata.dz, 5);
        // ips200_showint32(90, 14, indata.az, 5);
        break;

    case 1:
        //      ips200_showstr(0, 8, "dir:");
        //      ips200_showint32(35, 8, indata.cam_dir, 5);

        ips200_showstr(0, 9, "Set:");
        ips200_showint32(40, 9, indata.diff_speedL, 3);
        ips200_showint32(70, 9, indata.diff_speedR, 3);
        ips200_showint32(100, 9, indata.setspeed, 3);

        ips200_showstr(0, 10, "Spe:");
        ips200_showint32(40, 10, indata.speedL, 3);
        ips200_showint32(100, 10, indata.speedR, 3);
        ips200_showint32(70, 10, indata.speed, 3);

        //      ips200_showstr(0, 11, "outL:");
        //      ips200_showint32(45, 11, outdata.left_pwm, 4);
        //
        // ips200_showstr(0, 11, "Ang:");
        ips200_showint32(40 , 11, indata.gx, 3);
        ips200_showint32(70 , 11, indata.gy, 3);
        ips200_showint32(100, 11, indata.gz, 3);

        ips200_showstr(0, 12, "pL");
        ips200_showint32(15, 12, outdata.left_pwm, 4);

        ips200_showstr(70, 12, "pR");
        ips200_showint32(85, 12, outdata.right_pwm, 4);
        break;

    default:
        break;
    }
}

void show_image(uint8_t showmode)
{
    switch (showmode)
    {
    case 1:
        ips200_displayimage032((uint8_t *)im, 160, 120);
        ips200_draw_vertical(77, 0, 77, 119, RED);
        ips200_draw_vertical(82, 0, 82, 119, RED);
        break;

    case 2:
        ips200_showbwpic((uint8_t *)im, 160, 120);
        break;

    case 3:
        IPS_ShowEdge2((uint8_t *)imo,(uint8_t *)im, 160, 120);
        memset(imo, 0, 120 * 160);

        switch(setpara.TextRow)
        {
            case 1:
    //////////////////Ô²»·
                ips200_draw_horizon(0, 119 - watch.InLoopAngle, 160, 119 - watch.InLoopAngle, PURPLE);
                ips200_draw_horizon(0, 119 - watch.InLoopCirc, 160, 119 - watch.InLoopCirc, BLUE);
                ips200_draw_horizon(0, 119 - watch.InLoopAngle2, 160, 119 - watch.InLoopAngle2, YELLOW);
                ips200_draw_horizon(0, 119 - setpara.WhenInLoop, 160, 119 - setpara.WhenInLoop, YELLOW);
                ips200_draw_horizon(0, 119 - watch.OutLoopAngle, 160, 119 - watch.OutLoopAngle, YELLOW);
                break;
            case 2:
    //////////////////Èý²íÂ·
                ips200_draw_horizon(0, 119 - watch.JuncLine, 160, 119 - watch.JuncLine, PURPLE);
                ips200_draw_horizon(0, 119 - watch.JuncAngle, 160, 119 - watch.JuncAngle, BLUE);
                ips200_draw_vertical(watch.JuncAngleX, 0, watch.JuncAngleX, 119, BLUE);
                ips200_draw_horizon(0, 119 - watch.JuncLineR, 160, 119 - watch.JuncLineR, YELLOW);
                ips200_draw_horizon(0, 119 - watch.JuncLineL, 160, 119 - watch.JuncLineL, BROWN);
                // ips200_draw_horizon(0, 119 - setpara.WhenJuncStop, 160, 119 - setpara.WhenJuncStop, BROWN);
                break;
            case 3:
    //////////////////¶þÎ¬Âë
                ips200_draw_horizon(0, 119 - watch.AprilLine, 160, 119 - watch.AprilLine, PURPLE);
                ips200_draw_horizon(0, 119 - setpara.AprilStop, 160, 119 - setpara.AprilStop, BLUE);
                break;
            case 4:
    //////////////////³µ¿â
                // ips200_draw_horizon(0, 119 - setpara.cross_base, 160, 119 - setpara.cross_base, BROWN);
                // ips200_draw_vertical(watch.ZebraRowL, 0, watch.ZebraRowL, 119, PURPLE);
                // ips200_draw_vertical(watch.ZebraRowR, 0, watch.ZebraRowR, 119, YELLOW);
            default:
                break;
        }

        ips200_draw_horizon(0, 119 - setpara.TextLine, 160, 119 - setpara.TextLine, WHITE);
        ips200_draw_vertical(setpara.TextRow, 0, setpara.TextRow, 119, WHITE);
        break;

    case 4:
        IPS_ShowEdge((uint8_t *)imo, 160, 120);
        memset(imo, 0, 120 * 160);
        break;

    default:
        break;
    }
}

void show_static()
{
    ips200_showstr(165, 0, "T:");
    ips200_showint32(185, 0, T / 1000, 4);
    ips200_showint32(230, 0, (T % 1000) / 100, 1);

    ips200_showstr(165, 1, "RUNTIME:");
    ips200_showint32(235, 1, mycar.RUNTIME / 1000, 4);
    ips200_showint32(270, 1, (mycar.RUNTIME % 1000) / 100, 1);

    ips200_showstr(165, 2, "status:");
    ips200_showint32(225, 2, mycar.status, 1);

    ips200_showstr(165, 3, "FPS:");
    ips200_showint32(205, 3, indata.cam_fps, 3);

    ips200_showstr(265, 3, "Yc:");
    ips200_showint32(295, 3, flag.JuncCount, 3);

    ips200_showstr(265, 4, "C :");
    ips200_showint32(295, 4, flag.CircCount, 3);

    ips200_showstr(165, 4, "gz:");
    ips200_showint32(205, 4, indata.gz, 5);

    ips200_showstr(165, 5, "Speed:");
    ips200_showint32(215, 5, indata.speed, 3);

    //  ips200_showstr(165, 6, "Voltage:");
    //  ips200_showint32(235, 6, indata.battvolt/100, 4);
    //  ips200_showint32(270, 6, indata.battvolt % 100, 1);

    ips200_showstr(165, 6, "V:");
    ips200_showint32(190, 6, indata.battvolt / 100, 4);
    ips200_showstr(210, 6, ".");
    ips200_showint32(225, 6, indata.battvolt % 100, 1);

    ips200_draw_horizon(159, 120, 319, 120, YELLOW);
    ips200_draw_vertical(159, 120, 159, 239, YELLOW);

    if (ips.page_mode == 0)
    {
        ips200_address_set(0, 0, 7, 7);
        for (uint8_t i = 0; i < 8; i++)
            for (uint8_t j = 0; j < 8; j++)
                ips200_wr_data16(BLUE);
    }
}

void show_changeable()
{
    int temp_para_select = ips.para_select; //ç”±äºŽæ½œåœ¨çš„ä¸­ï¿??å¹²æ‰°ï¼Œå¿…é¡»å…ˆå­˜å‚¨å‚æ•°åºå·ä»¥é¿å…å±ï¿??
    static int last_para_select = 0;

    if (temp_para_select != last_para_select)
    {
        ips200_address_set(160, 128, 319, 239);
        for (uint8_t j = 0; j < 112; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
        last_para_select = temp_para_select;
    }
    if (temp_para_select > 2)
    {
        ips200_showint32(160, 8, temp_para_select - 3, 2);
        ips200_showstr(190, 8, paralist[temp_para_select - 3].label);
        ips200_showint32(280, 8, *paralist[temp_para_select - 3].para, 4);
    }
    else
    {
        ips200_address_set(160, 128, 319, 143);
        for (uint8_t j = 0; j < 16; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
    }
    if (temp_para_select > 1)
    {
        ips200_showint32(160, 9, temp_para_select - 2, 2);
        ips200_showstr(190, 9, paralist[temp_para_select - 2].label);
        ips200_showint32(280, 9, *paralist[temp_para_select - 2].para, 4);
    }
    else
    {
        ips200_address_set(160, 144, 319, 159);
        for (uint8_t j = 0; j < 16; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
    }
    if (temp_para_select > 0)
    {
        ips200_showint32(160, 10, temp_para_select - 1, 2);
        ips200_showstr(190, 10, paralist[temp_para_select - 1].label);
        ips200_showint32(280, 10, *paralist[temp_para_select - 1].para, 4);
    }
    else
    {
        ips200_address_set(160, 160, 319, 175);
        for (uint8_t j = 0; j < 16; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
    }
    if (ips.changepara)
    {
        ips200_showint32(160, 11, temp_para_select, 2);
        ips200_showstr(190, 11, paralist[temp_para_select].label);
        ips200_showint32W(280, 11, *paralist[temp_para_select].para, 4);
    }
    else
    {
        ips200_showint32W(160, 11, temp_para_select, 2);
        ips200_showstrW(190, 11, paralist[temp_para_select].label);
        ips200_showint32(280, 11, *paralist[temp_para_select].para, 4);
    }
    if (temp_para_select < ips.para_num - 1)
    {
        ips200_showint32(160, 12, temp_para_select + 1, 2);
        ips200_showstr(190, 12, paralist[temp_para_select + 1].label);
        ips200_showint32(280, 12, *paralist[temp_para_select + 1].para, 4);
    }
    else
    {
        ips200_address_set(160, 192, 319, 207);
        for (uint8_t j = 0; j < 16; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
    }
    if (temp_para_select < ips.para_num - 2)
    {
        ips200_showint32(160, 13, temp_para_select + 2, 2);
        ips200_showstr(190, 13, paralist[temp_para_select + 2].label);
        ips200_showint32(280, 13, *paralist[temp_para_select + 2].para, 4);
    }
    else
    {
        ips200_address_set(160, 208, 319, 223);
        for (uint8_t j = 0; j < 16; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
    }
    if (temp_para_select < ips.para_num - 3)
    {
        ips200_showint32(160, 14, temp_para_select + 3, 2);
        ips200_showstr(190, 14, paralist[temp_para_select + 3].label);
        ips200_showint32(280, 14, *paralist[temp_para_select + 3].para, 4);
    }
    else
    {
        ips200_address_set(160, 224, 319, 239);
        for (uint8_t j = 0; j < 16; j++)
            for (uint8_t i = 0; i < 160; i++)
                ips200_wr_data16(BACK_COLOR);
    }
}

void show_ips()
{
    show_upperpage(ips.showpage);
    if (ips.img_showmode)
    {
        if (IsDisplay)
        {
            show_image(ips.img_showmode);
            //      usb_cdc_send_char(0x00);
            //      usb_cdc_send_char(0xFF);
            //      usb_cdc_send_char(0x01);
            //      usb_cdc_send_char(0x01);
            //
            //      usb_cdc_send_buff((uint8_t *)im, 19200);
            IsDisplay = 0;
        }
    }
    else
        IsDisplay = 0;

    show_static();
    show_changeable();
}