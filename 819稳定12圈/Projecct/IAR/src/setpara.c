#include "setpara.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t pushtime;
//enum STATUS_BUTTON status_button;
//struct setpara_STRUCT setpara;
struct PARA_LIST_STRUCT paralist[64] = //�ɵ�������
    {
        {&setpara.mode, "MODE", 1},
        {&setpara.settime, "RunTime", 10},
/////////�ٶ����
        {&setpara.CamAveSpeed, "S.Speed", 5},
        {&setpara.SpeedKp,     "S.SpeedKp", 1},
        {&setpara.SpeedKi,     "S.SpeedKi", 1},
        {&setpara.SpeedKd,     "S.SpeedKd", 1},
        {&setpara.diffK,       "S.diffK", 1},
        {&setpara.SpeedUP,     "S.UP", 5},
        {&setpara.SpeedDOWN,     "S.DOWN", 5},
/////////������
        {&setpara.SteerMid, "D.SteerMid", 1},
        {&setpara.MaxTurn,  "D.MaxTurn", 10},
        {&setpara.CamKp,    "D.CamKp", 1},
        {&setpara.CamKd,    "D.CamKd", 1},
/////////�������
        {&setpara.camcfg.pitch, "Pitch", 10},
        {&setpara.camcfg.forward_far, "Far", 1},
        {&setpara.camcfg.forward_near, "Near", 1},

        {&setpara.camcfg.wf, "CamWF", 1},
        {&setpara.camcfg.wl, "CamWL", 1},
        {&setpara.camcfg.wr, "CamWR", 1},

        // {&setpara.camcfg.rotate_x, "RotateX", 1},
        // {&setpara.camcfg.rotate_y, "RotateY", 1},
        // {&setpara.camcfg.shift, "Shift", 10},
/////////�Ӿ����
        {&setpara.camcfg.manbri,   "C.ManBright", 1},
        {&setpara.camcfg.exptime,  "C.ExpTime", 1},
        {&setpara.exp_ki,          "C.exp_ki", 1},
        {&setpara.camcfg.autogain, "C.AutoGain", 1},
/////////�������
        {&setpara.garage_outangle, "Z.OutAngle", 5},
        {&setpara.garage_outtime,  "Z.OutTime", 1},
        // {&setpara.ZebraCount,      "Z.ZebraCount", 1},
        {&setpara.stop_kp,         "Z.stop_kp", 1},
        {&setpara.stop_kd,         "Z.stop_kd", 1},
        // {&setpara.StopSpeed,  "Z.StopSpeed", 5},
/////////Բ�����
        {&setpara.CamLoopKp,  "O.CamLoopKp", 1},
        {&setpara.CamLoopKd,  "O.CamLoopKd", 1},
        {&setpara.InLoopLine, "O.InLoopLine", 1},
        {&setpara.WhenInLoop, "O.WhenInLoop", 1},
        {&setpara.InLoopRow, "O.InLoopRow", 1},
        {&setpara.LoopThres, "O.LoopThres", 1},
        {&setpara.OutLoopThres, "O.OutLoopThres", 1},
//////////����·���
        {&setpara.StrThres, "I.StrThres", 1},
        {&setpara.TempRL,       "Y.TempRL", 1},
        {&setpara.JuncProcess,  "Y.JuncTime", 1},
        {&setpara.WhenJuncStop, "Y.WhenJuncStop", 1},
        // {&setpara.JuncSpeed,    "Y.JuncSpeed", 1},
        // {&setpara.DetectTime,   "Y.DetectTime", 1},
        {&setpara.number,  "Y.number", 1},
//////////��ά��
        // {&setpara.AprilCount,   "#.AprilCount", 1},
        // {&setpara.AprilStop,    "#.AprilStop", 1},
        // {&setpara.AprilSpeed,   "#.AprilSpeed", 1},
        // {&setpara.AprilTime,    "#.AprilTime", 1},
        // {&setpara.BackTime,    "#.BackTime", 1},
        // {&setpara.LaserTime,    "#.LaserTime", 1},
/////////�������
        {&setpara.testL,    "T.TestL", 200},
        {&setpara.testR,    "T.TestR", 200},
        {&setpara.TextLine, "T.TextLine", 1},
        {&setpara.TextRow,  "T.TextRow", 1},
        //����·����
/////////ʶ��
        {&setpara.TempDectect,  "A.TempDect", 1},
        {&setpara.DectectSpeed,  "A.s30cm", 2},
        {&setpara.DectectSpeed10,  "A.s10cm", 2},
        {&setpara.DsP,  "A.DsP", 1},
        {&setpara.DsI,  "A.DsI", 1},
        {&setpara.DsD,  "A.DsD", 1},
/////////�µ�
        {&setpara.SlopeDown ,  "M.SlopeDown", 1},
        // {&setpara.cross_base,  "+.CrossBase", 1},
        {0}};

/*******************************************************************************
 * Code
 ******************************************************************************/

void init_key()
{
    gpio_pin_config_t gpioPinConfigStruct;
    enc_config_t EncConfigStruct;

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_14_GPIO1_IO14, 0); //UP
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_15_GPIO1_IO15, 0); //PUSH
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_03_GPIO1_IO03, 0); //DOWN
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_02_GPIO1_IO02, 0); //PRESS
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_00_XBAR1_INOUT14, 0);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_01_XBAR1_INOUT15, 0);

    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_14_GPIO1_IO14, 0x1F0B0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_15_GPIO1_IO15, 0x1F0B0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_03_GPIO1_IO03, 0x1F0B0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_02_GPIO1_IO02, 0x1F0B0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_00_XBAR1_INOUT14, 0x1B0B1);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_01_XBAR1_INOUT15, 0x1B0B1);

    gpioPinConfigStruct.direction = kGPIO_DigitalInput;
    gpioPinConfigStruct.outputLogic = 1; //Invalid
    gpioPinConfigStruct.interruptMode = kGPIO_IntFallingEdge;

    GPIO_PinInit(GPIO1, 14, &gpioPinConfigStruct);
    GPIO_PinInit(GPIO1, 15, &gpioPinConfigStruct);
    GPIO_PinInit(GPIO1, 3, &gpioPinConfigStruct);
    GPIO_PinInit(GPIO1, 2, &gpioPinConfigStruct);

    GPIO_PortEnableInterrupts(GPIO1, 1 << 15);
    GPIO_PortEnableInterrupts(GPIO1, 1 << 14);
    GPIO_PortEnableInterrupts(GPIO1, 1 << 3);
    GPIO_PortEnableInterrupts(GPIO1, 1 << 2);

    NVIC_SetPriority(GPIO1_Combined_0_15_IRQn, NVIC_EncodePriority(PriorityGroup, 0, 0));
    EnableIRQ(GPIO1_Combined_0_15_IRQn);

    //XBARA_Init(XBARA1);
    //����mini��ʼ��ʱ��ʼ����XBARA1

    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputIomuxXbarInout14, kXBARA1_OutputEnc3PhaseAInput);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputIomuxXbarInout15, kXBARA1_OutputEnc3PhaseBInput);

    ENC_GetDefaultConfig(&EncConfigStruct);
    EncConfigStruct.decoderWorkMode = kENC_DecoderWorkAsNormalMode; //AB����������

    ENC_Init(ENC3, &EncConfigStruct);
    ENC_DoSoftwareLoadInitialPositionValue(ENC3);
}

void init_setpara()
{
    setpara.mode        = 1     ; // 0ǰ��  1�����2�ҳ���
    setpara.settime     = 1210    ;
/////////�ٶ����                          
    setpara.CamAveSpeed = 200   ;
    setpara.SpeedKp     = 150   ;
    setpara.SpeedKi     = 30    ;
    setpara.SpeedKd     = 75    ;
    setpara.diffK       = 40    ;
    setpara.SpeedUP     = 75   ;
    setpara.SpeedDOWN   = 5     ;
/////////������                          
    setpara.SteerMid    = 3700  ;
    setpara.MaxTurn     = 430   ;
    setpara.CamKp       = 75    ;
    setpara.CamKd       = 16    ;
/////////�������                          
    setpara.camcfg.pitch        = 540   ;
    setpara.camcfg.forward_far  = 110   ;
    setpara.camcfg.forward_near = 3     ;
    setpara.camcfg.wf           = -6    ;
    setpara.camcfg.wl           = 76    ;
    setpara.camcfg.wr           = 97    ; // ����λ�ã����ܵ���Ӱ���뻷
    setpara.camcfg.rotate_x     = -4    ;
    setpara.camcfg.rotate_y     = 2     ;
    setpara.camcfg.shift        = 270   ;
/////////�Ӿ����                          
    setpara.camcfg.manbri       = 0     ;
    setpara.camcfg.exptime      = 153   ;
    setpara.exp_ki              = 1     ;
    setpara.camcfg.autogain     = 30    ;
/////////�������                          
    setpara.garage_outangle     = 250   ; // ����Ƕȣ�+��-�ң����Ż����һ��12��־������š��ô��
    setpara.garage_outtime      = 3     ;   // �������Ƕ�ʱ�䣬��0.01s
    // setpara.ZebraCount          = 6    ;
    setpara.stop_kp             = 100   ;
    setpara.stop_kd             = 25    ;
    // setpara.StopSpeed           = 70    ;
/////////Բ�����                          
    setpara.CamLoopKp    = 65    ;
    setpara.CamLoopKd    = 13     ;
    setpara.InLoopLine   = 60    ;
    setpara.WhenInLoop   = 83    ;
    setpara.InLoopRow    = 67    ;
    setpara.LoopThres    = 8     ;
    setpara.OutLoopThres = 40    ;
//////////����·���                          
    setpara.StrThres     = 16    ;
    setpara.TempRL       = 1     ; //����·��ʱ��������1��2��������OpenMV����
    setpara.JuncProcess  = 2     ;
    setpara.WhenJuncStop = 62    ;
    // setpara.JuncSpeed    = -2    ;
    // setpara.DetectTime   = 30    ;
    setpara.number       = 1     ;    
//////////��ά�����            
    // setpara.AprilCount   = 5     ;
    // setpara.AprilStop    = 78    ;
    // setpara.AprilSpeed   = 25    ;
    // setpara.AprilTime    = 110   ;  
    // setpara.BackTime     = 15    ;  
    // setpara.LaserTime    = 80    ;  
/////////�������                           
    setpara.testL        = 0     ;
    setpara.testR        = 0     ;
    setpara.TextLine     = 33    ;
    setpara.TextRow      = 0     ;
    // setpara.TextDistance = 0     ;
/////////ʶ��                         
    setpara.TempDectect     = 0     ;
    setpara.DectectSpeed     = 52     ;
    setpara.DectectSpeed10    = 22     ;
    setpara.DsP = 200;
    setpara.DsI = 30;
    setpara.DsD = 131;
/////////�µ�
    setpara.SlopeDown = 3;
    // setpara.cross_base = 80;

}

void init_paranum()
{
    while (paralist[ips.para_num].precision)
        ips.para_num++;
}

void init_readpara()
{
    uint32_t data_to_read[64];
    uint8_t times = Paralist_Size / 64;

    //��ҳ��ȡ
    for (uint8_t i = 0; i < times; i++)
    {
        flash_read_page(FLASH_SAVE_SECTOR, i, data_to_read, 64);
        for (uint8_t j = 0; j < 64; j++)
            *paralist[i * 64 + j].para = data_to_read[j];
    }
}

int ctimes;
int8_t d_ctimes;
void key_check()
{
    static int last_ctimes = 0;
    static int cT = 0;
    ctimes = ENC_GetPositionValue(ENC3); //��ȡ��ת������AB���������
    if (ctimes % 4 == 0)                 //�������Ϊ4�ı���
    {
        cT = T;
        d_ctimes = (ctimes - last_ctimes) / 4;
        if (d_ctimes > 0)
        {
            //status_button = CW;
            //��ת��������Ӧ
            if (ips.changepara)
                *paralist[ips.para_select].para += paralist[ips.para_select].precision * d_ctimes; //��������
            else                                                                                   //ѡ������������
            {
                ips.para_select = (ips.para_select + d_ctimes) % ips.para_num;
            }
        }
        else if (d_ctimes < 0)
        {
            //status_button = CCW;
            //��ת��������Ӧ
            if (ips.changepara)
                *paralist[ips.para_select].para += paralist[ips.para_select].precision * d_ctimes; //��������
            else                                                                                   //ѡ������������
            {
                if (ips.para_select >= abs(d_ctimes))
                    ips.para_select += d_ctimes;
                else
                    ips.para_select += (ips.para_num + d_ctimes);
            }
        }
        last_ctimes = ctimes;
    }
    else
    {
        if (T - cT > 1000)
        {
            ENC_SetInitialPositionValue(ENC3, 0);
            ENC_DoSoftwareLoadInitialPositionValue(ENC3);
            ctimes = 0;
            last_ctimes = 0;
        }
    }

    //��¼����ʱ��
    pushtime = T;

    //���ְ��²�������Ļ��ʼ�������޸�����
    if (status_button == PUSH || status_button == PRESS)
        ;

    switch (status_button)
    {
    case PRESS:
        while (!GPIO_PinRead(GPIO1, 2))
            ;

        if (T - pushtime < 500)
        {
            ips.changepara ^= 1; //״̬ȡ��
        }
        else //������ťΪ����
        {
            paraclear();
            // save_flash();
            // if(setpara.mode == 3)
            //     ips.img_showmode = 3;
            // else
                ips.img_showmode = 0;

            int start_time = T;
            while (T - start_time < 1000);
            if(setpara.mode == 3){
                mycar.status = 4;
                // flag.DetectMode = setpara.TempDectect;
                flag.DetectMode = ART_wait;
                Send_ART(0x33);
                indata.journey = 0;
            }
            else if(setpara.mode == 4){
                mycar.status = 4;
                flag.DetectMode = setpara.TempDectect;
                indata.journey = 0;
            }
            else{
                mycar.status = 3;
            }
        }

        break;

    case PUSH:
        while (!GPIO_PinRead(GPIO1, 15))
            ;

        if (T - pushtime < 500)
        {
            ips.page_mode ^= 1; //״̬ȡ��
        }
        else
        {
            mycar.senddata = 1;
        }

        break;

    case UP:
        while (!GPIO_PinRead(GPIO1, 14))
            ;

        if (T - pushtime < 300)
        {
            if (ips.page_mode)
            {
                if (ips.showpage > SHOWPAGE_MIN)
                    ips.showpage--;
                else
                    ips.showpage = SHOWPAGE_MAX;
            }
            else
            {
                if (ips.img_showmode > SHOWMODE_MIN)
                    ips.img_showmode--;
                else
                    ips.img_showmode = SHOWMODE_MAX;
            }
        }
        else
        {
        }
        break;

    case DOWN:
        while (!GPIO_PinRead(GPIO1, 3))
            ;

        if (T - pushtime < 300)
        {
            if (ips.page_mode)
            {
                if (ips.showpage < SHOWPAGE_MAX)
                    ips.showpage++;
                else
                    ips.showpage = SHOWPAGE_MIN;
            }
            else
            {
                if (ips.img_showmode < SHOWMODE_MAX)
                    ips.img_showmode++;
                else
                    ips.img_showmode = SHOWMODE_MIN;
            }
        }

        else
        {
        }

        break;

    default:
        break;
    }

    setpara.camcfg.autogain = _LIMIT(setpara.camcfg.autogain, 16, 64);
    setpara.camcfg.exptime = _LIMIT(setpara.camcfg.exptime, 1, max_int_time_us);

    //�������״̬
    status_button = NONE;
}

void paraclear()
{
    mycar.RUNTIME = 0;
    mycar.save_times = 0;
    mycar.savenum = 0;
    mycar.save_img_count = 0;
    mycar.senddata = 0;
}

void save_flash()
{
    uint32_t data_to_write[64];
    uint8_t times = Paralist_Size / 64;
    uint8_t status;
    status = flash_erase_sector(FLASH_SAVE_SECTOR);
    if (status)
        while (1)
            ; //����ʧ��

    //��ҳд��
    for (uint8_t i = 0; i < times; i++)
    {
        for (uint8_t j = 0; j < 64; j++)
            data_to_write[j] = *paralist[i * 64 + j].para;
        flash_page_program(FLASH_SAVE_SECTOR, i, data_to_write, 64);
    }
}
