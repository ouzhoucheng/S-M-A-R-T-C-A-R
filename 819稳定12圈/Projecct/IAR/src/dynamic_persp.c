#include "dynamic_persp.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

struct STATUS_CAMERAINFO cameraInfo; //摄像头参数
struct STATUS_IPMINFO ipmInfo;       //输出的图像数组数据

/*******************************************************************************
 * Code
 ******************************************************************************/

void init_persp()
{
    //130度镜头参数
    cameraInfo.focalLengthX = 6489.816;
    cameraInfo.focalLengthY = 6619.209;
    cameraInfo.opticalCenterX = 76.6913;
    cameraInfo.opticalCenterY = 56.3279;
    cameraInfo.cameraHeight = 315;
    cameraInfo.pitch = (float)setpara.camcfg.pitch / 1000;
    cameraInfo.yaw = 0.0;
    cameraInfo.imageWidth = 160;
    cameraInfo.imageHeight = 120;

    ipmInfo.ipmWidth = 160;
    ipmInfo.ipmHeight = 120;
    ipmInfo.ipmLeft = 1; //MATLAB代码从1开始
    ipmInfo.ipmRight = 160;
    ipmInfo.ipmTop = 1;
    ipmInfo.ipmBottom = 120;
    calculate_persp();
}

void calculate_persp()
{
    float vpp[3][1] = {0};
    float tyawp[3][3] = {0};
    float tpitchp[3][3] = {0};
    float transform[3][3] = {0};
    float t1p[3][3] = {0};
    float vp[3][1] = {0};
    float vp_X = 0;
    float vp_Y = 0;

    // vpp[0][0] = sin(cameraInfo.yaw*PI/180)/cos(cameraInfo.pitch*PI/180);
    // vpp[1][0] = cos(cameraInfo.yaw*PI/180)/cos(cameraInfo.pitch*PI/180);
    vpp[0][0] = 0;
    vpp[1][0] = 1 / cos(cameraInfo.pitch * PI / 180);
    vpp[2][0] = 0;

    //    tyawp[0][0] = cos(cameraInfo.yaw*PI/180);
    //    tyawp[0][1] = -sin(cameraInfo.yaw*PI/180);
    //    tyawp[1][0] = sin(cameraInfo.yaw*PI/180);
    //    tyawp[1][1] = cos(cameraInfo.yaw*PI/180);

    tyawp[0][0] = 1;
    tyawp[0][1] = 0;
    tyawp[1][0] = 0;
    tyawp[1][1] = 0;
    tyawp[2][2] = 1;

    tpitchp[0][0] = 1;
    tpitchp[1][1] = -sin(cameraInfo.pitch * PI / 180);
    tpitchp[1][2] = -cos(cameraInfo.pitch * PI / 180);
    tpitchp[2][1] = cos(cameraInfo.pitch * PI / 180);
    tpitchp[2][2] = -sin(cameraInfo.pitch * PI / 180);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float sum = 0;
            for (int m = 0; m < 3; m++)
            {
                sum += tyawp[i][m] * tpitchp[m][j];
            }
            transform[i][j] = sum;
        }
    }

    t1p[0][0] = cameraInfo.focalLengthX;
    t1p[0][2] = cameraInfo.opticalCenterX;
    t1p[1][1] = cameraInfo.focalLengthY;
    t1p[1][2] = cameraInfo.opticalCenterY;
    t1p[2][2] = 1;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float sum = 0;
            for (int m = 0; m < 3; m++)
            {
                sum += t1p[i][m] * transform[m][j];
            }
            transform[i][j] = sum;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 1; j++)
        {
            float sum = 0;
            for (int m = 0; m < 3; m++)
            {
                sum += transform[i][m] * vpp[m][j];
            }
            vp[i][j] = sum;
        }
    }
    vp_X = vp[0][0];
    vp_Y = vp[1][0];
    //TransformImage2Ground
    static float inPoints4[4][4] = {0};
    float inPointsr4[4] = {0};
    float inPoints3[3][4] = {0};
    float Image_matp1[4][3] = {0};
    float xfMin = 300000, xfMax = -300000, yfMin = 300000, yfMax = -300000;

    inPoints4[0][0] = inPoints3[0][0] = vp_X;
    inPoints4[0][1] = inPoints3[0][1] = ipmInfo.ipmRight;
    inPoints4[0][2] = inPoints3[0][2] = ipmInfo.ipmLeft;
    inPoints4[0][3] = inPoints3[0][3] = vp_X;
    inPoints4[1][0] = inPoints3[1][0] = ipmInfo.ipmTop;
    inPoints4[1][1] = inPoints3[1][1] = ipmInfo.ipmTop;
    inPoints4[1][2] = inPoints3[1][2] = ipmInfo.ipmTop;
    inPoints4[1][3] = inPoints3[1][3] = ipmInfo.ipmBottom;
    for (int i = 0; i < 4; i++)
        inPoints4[2][i] = inPoints3[2][i] = 1;

    ipmInfo.Image_c1 = cos(cameraInfo.pitch * PI / 180);
    ipmInfo.Image_s1 = sin(cameraInfo.pitch * PI / 180);
    ipmInfo.Image_c2 = 1;
    ipmInfo.Image_s2 = 0;

    Image_matp1[0][0] = -cameraInfo.cameraHeight * ipmInfo.Image_c2 / cameraInfo.focalLengthX;
    Image_matp1[0][1] = cameraInfo.cameraHeight * ipmInfo.Image_s1 * ipmInfo.Image_s2 / cameraInfo.focalLengthY;
    Image_matp1[0][2] = (cameraInfo.cameraHeight * ipmInfo.Image_c2 * cameraInfo.opticalCenterX / cameraInfo.focalLengthX) - (cameraInfo.cameraHeight * ipmInfo.Image_s1 * ipmInfo.Image_s2 * cameraInfo.opticalCenterY / cameraInfo.focalLengthY) - cameraInfo.cameraHeight * ipmInfo.Image_c1 * ipmInfo.Image_s2;
    Image_matp1[1][0] = cameraInfo.cameraHeight * ipmInfo.Image_s2 / cameraInfo.focalLengthX;
    Image_matp1[1][1] = cameraInfo.cameraHeight * ipmInfo.Image_s1 * ipmInfo.Image_c2 / cameraInfo.focalLengthY;
    Image_matp1[1][2] = (-cameraInfo.cameraHeight * ipmInfo.Image_s2 * cameraInfo.opticalCenterX / cameraInfo.focalLengthX) - (cameraInfo.cameraHeight * ipmInfo.Image_s1 * ipmInfo.Image_c2 * cameraInfo.opticalCenterY / cameraInfo.focalLengthY) - cameraInfo.cameraHeight * ipmInfo.Image_c1 * ipmInfo.Image_c2;
    Image_matp1[2][1] = cameraInfo.cameraHeight * ipmInfo.Image_c1 / cameraInfo.focalLengthY;
    Image_matp1[2][1] = (-cameraInfo.cameraHeight * ipmInfo.Image_c1 * cameraInfo.opticalCenterY / cameraInfo.focalLengthY) + cameraInfo.cameraHeight * ipmInfo.Image_s1;
    Image_matp1[3][1] = -ipmInfo.Image_c1 / cameraInfo.focalLengthY;
    Image_matp1[3][2] = (ipmInfo.Image_c1 * cameraInfo.opticalCenterY / cameraInfo.focalLengthY) - ipmInfo.Image_s1;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            float sum = 0;
            for (int m = 0; m < 3; m++)
            {
                sum += Image_matp1[i][m] * inPoints3[m][j];
            }
            inPoints4[i][j] = sum;
        }
    }

    for (int i = 0; i < 4; i++)
        inPointsr4[i] = inPoints4[3][i];

    for (int i = 0; i < 4; i++)
    {
        if (inPointsr4[i] == 0)
            inPointsr4[i] = 0.0000001;
        inPoints4[0][i] /= inPointsr4[i];
        if (inPoints4[0][i] > xfMax)
            xfMax = inPoints4[0][i];
        if (inPoints4[0][i] < xfMin)
            xfMin = inPoints4[0][i];
    }

    for (int i = 0; i < 4; i++)
    {
        if (inPointsr4[i] == 0)
            inPointsr4[i] = 0.0000001;
        inPoints4[1][i] /= inPointsr4[i];
        if (inPoints4[1][i] > yfMax)
            yfMax = inPoints4[1][i];
        if (inPoints4[1][i] < yfMin)
            yfMin = inPoints4[1][i];
    }

    ipmInfo.yfMax = yfMax;
    ipmInfo.yfMin = yfMin;
    ipmInfo.xfMax = xfMax;
    ipmInfo.xfMin = xfMin;
}

void start_persp(int img_y, int img_x, int *persp_y, int *persp_x)
{
    float x_Grid = 0;
    float y_Grid = 0;
    float Y_x_Grid = 0;
    float Y_y_Grid = 0;
    float stepRow = 0;
    float stepCol = 0;
    float Image_matp2[3][3] = {0};

    stepRow = (ipmInfo.yfMax - ipmInfo.yfMin) / IMG_V;
    stepCol = (ipmInfo.xfMax - ipmInfo.xfMin) / IMG_H;

    Image_matp2[0][0] = cameraInfo.focalLengthX * ipmInfo.Image_c2 + ipmInfo.Image_c1 * ipmInfo.Image_s2 * cameraInfo.opticalCenterX;
    Image_matp2[0][1] = -cameraInfo.focalLengthX * ipmInfo.Image_s2 + ipmInfo.Image_c1 * ipmInfo.Image_c2 * cameraInfo.opticalCenterX;
    Image_matp2[0][2] = -ipmInfo.Image_s1 * cameraInfo.opticalCenterX;
    Image_matp2[1][0] = ipmInfo.Image_s2 * (-cameraInfo.focalLengthY * ipmInfo.Image_s1 + ipmInfo.Image_c1 * cameraInfo.opticalCenterY);
    Image_matp2[1][1] = ipmInfo.Image_c2 * (-cameraInfo.focalLengthY * ipmInfo.Image_s1 + ipmInfo.Image_c1 * cameraInfo.opticalCenterY);
    Image_matp2[1][2] = -cameraInfo.focalLengthY * ipmInfo.Image_c1 - ipmInfo.Image_s1 * cameraInfo.opticalCenterY;
    Image_matp2[2][0] = ipmInfo.Image_c1 * ipmInfo.Image_s2;
    Image_matp2[2][1] = ipmInfo.Image_c1 * ipmInfo.Image_c2;
    Image_matp2[2][2] = -ipmInfo.Image_s1;

    float x = 0;
    float y = 0;
    y = ipmInfo.yfMax - 0.5 * stepRow;
    img_y += setpara.camcfg.rotate_y;
    img_x += setpara.camcfg.rotate_x;
    if (img_x < 1)
        img_x = 1;
    for (int i = 1; i < img_y + 1; i++)
    {
        x = ipmInfo.xfMin + 0.5 * stepCol;
        for (int j = 0; j < img_x; j++)
        {
            x_Grid = x;
            y_Grid = y;
            x = x + stepCol;
        }
        y = y - stepRow;
    }
    //y放缩
    float Y_x = 0;
    float Y_y = 0;
    Y_y = ipmInfo.yfMax - 0.5 * stepRow;
    for (int i = 1; i < 116; i++)
    {
        Y_x = ipmInfo.xfMin + 0.5 * stepCol;
        for (int j = 1; j < 2; j++)
        {
            Y_x_Grid = Y_x;
            Y_y_Grid = Y_y;
            Y_x = Y_x + stepCol;
        }
        Y_y = Y_y - stepRow;
    }

    cameraInfo.Y_scaling = (int)((Image_matp2[1][0] * Y_x_Grid + Image_matp2[1][1] * Y_y_Grid + Image_matp2[1][2] * (-cameraInfo.cameraHeight)) /
                                 (Image_matp2[2][0] * Y_x_Grid + Image_matp2[2][1] * Y_y_Grid + Image_matp2[2][2] * (-cameraInfo.cameraHeight)));

    *persp_x = (int)((Image_matp2[0][0] * x_Grid + Image_matp2[0][1] * y_Grid + Image_matp2[0][2] * (-cameraInfo.cameraHeight)) /
                     (Image_matp2[2][0] * x_Grid + Image_matp2[2][1] * y_Grid + Image_matp2[2][2] * (-cameraInfo.cameraHeight)));
    *persp_x = (int)(*persp_x + setpara.camcfg.shift);

    if (*persp_x < 0)
        *persp_x = 0;
    if (*persp_x > 620)
        *persp_x = 620;

    *persp_y = (int)((Image_matp2[1][0] * x_Grid + Image_matp2[1][1] * y_Grid + Image_matp2[1][2] * (-cameraInfo.cameraHeight)) /
                     (Image_matp2[2][0] * x_Grid + Image_matp2[2][1] * y_Grid + Image_matp2[2][2] * (-cameraInfo.cameraHeight)));
    *persp_y = (int)(*persp_y * 114 / cameraInfo.Y_scaling);
}