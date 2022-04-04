#ifndef _CAMERA_PROCESS_H_
#define _CAMERA_PROCESS_H_

#include "car_global.h"

#define _GRAY_SCALE 64
#define _SHIFT_FOR_GRAYSCALE 2
#define _Thresh_Mult 2 //1/2*(256/_GRAY_SCALE)

#define LINE_WIDTH 160

#define WIDTH 88 //�����������

#define _EDGE_STORE_SIZE 22 //����Ϊż��
#define _MIN_EDGE_WIDTH 20  //������С�������

#define _LEFT_MARGIN 3
#define _RIGHT_MARGIN (LINE_WIDTH - 4)

struct lineinfo_s
{
    unsigned char y;          //0~119
    unsigned char left;       //0~160
    unsigned char right;      //0~160
    unsigned char edge_count; //0~160
    // unsigned char edge_store;   //����û��0612
    uint16_t persp_lx; //0~255
    uint16_t persp_ly; //0~255
    uint16_t persp_rx; //0~255
    uint16_t persp_ry; //0~255
    int angel_left;
    int angel_right;
    int left_lost;
    int right_lost;
    unsigned char zebra_flag;
    unsigned char apriltag_flag;
    
    unsigned char black_width;
    unsigned char junc_angle;
};

struct watch_o
{
    int base_line;
    
    uint8_t brightness;
    int watch_line;
    int watch_lost;
    
    int loop_flag;
    
    short int gray;
    int cross;

    //���Ҷ���������21��
    int left_lost;
    int right_lost;

    //Բ����־λ
    int RLStraight;   //ֱ�߱�־������ֱ��0��1��2��3����
    int InLoopAngle;  //����Բ����һ����������
    int InLoopCirc;   //����Բ����͹��
    int InLoopAngle2; //�뻷ǰ�����ڶ����ǵ�
    int OutLoopAngle; //�����󿴼��ڶ����ǵ�


    int InLoop; //Բ��״̬0�޻�
    //1��׼�����󻷣����ڶ����� 2��׼�����󻷣�����
    //5��׼�����һ������ڶ����� 6��׼�����һ�������
    int OutLoopRight;  //�����ҽǵ������
    int OutLoopRightY; //�����ҽǵ�������
    int OutLoopLeft;   //������ǵ������
    int OutLoopLeftY;  //������ǵ�������

    //�Ƿ��������·��־����21��
    int Junc_flag; //0��1��2��3���뵫��֪������
    int DeltaR;    //��ʱ������ʾ
    int DeltaL;    //��ʱ������ʾ

    int JuncLineL; //����·�ǵ���������
    int JuncLineR; //����·�ǵ���������
    int JuncLine;  //ȡƽ��
    int JuncAngle;
    int JuncAngleX;  //ȡƽ��
    int JuncTime;  //����·ʱ��
    
    int zebra_flag;
    int ZebraLine;
    int ZebraRowL;
    int ZebraRowR;

    uint32_t DectectStart;

    int April_flag;
    int AprilLine;
    uint32_t AprilStart;

    uint32_t FruitStart;
    uint32_t LaserStart;
    uint32_t AnimalStart;

    char servo_flag;
    uint32_t ServoStart;

    int distance;

    int cross_lost;
    
};

extern uint16_t max_int_time_us;
extern struct watch_o watch;
extern uint8_t imo[120][160];
extern struct caminfo_s caminfo;
extern uint8_t IsCaminfo;
extern uint8_t IsDisplay;

int img_otsu(uint8_t *img, uint8_t img_v, uint8_t img_h, uint8_t step);
uint8_t get_orign_edges(uint8_t *inputimg, uint8_t *edge_store);
int get_best_edge(struct lineinfo_s *lineinfo, uint8_t *edge_store, struct lineinfo_s *lineinfo_ref);
int get_max_edge(struct lineinfo_s *lineinfo, uint8_t *edge_store);
int line_single(struct lineinfo_s *lineinfo, unsigned char *inputimg);
int line_findnext(struct lineinfo_s *lineinfo, uint8_t *inputimg, struct lineinfo_s *lineinfo_ref);
int zebra_detect(struct lineinfo_s *lineinfo, uint8_t *edge_store, uint8_t *inputimg);
int apriltag_detect(struct lineinfo_s *lineinfo, uint8_t *edge_store, uint8_t *inputimg);
int Junc_detect(struct lineinfo_s *lineinfo, uint8_t *edge_store, uint8_t *inputimg);

void Caminfo_Cal(void);
int clear_all_flags();

#endif