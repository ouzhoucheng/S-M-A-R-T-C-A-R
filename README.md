# smart-car-1st-2021
we get 1st prize in national finals -- 16th chinese intelligent car racing

# 16届智能视觉组-摄像头算法
底层搜线算法是往届学长写的,我们的主要工作在于各个元素的检测,这也是本篇重点讲解部分,代码很多很乱很复杂,建议跟随本篇的顺序来阅读,移植.

出库入库处理部分参考文件`国赛改规则前805准备移植wifi`,其他参考文件`国赛819稳定12圈（RT1064总钻风巡线）`,最关键的文件是`camera_process.c`,建议结合代码阅读.

**注意**这套代码是根据我们的装车情况写的,各个元素可调参数的经验值见文件`setpara.c`,摄像头高31.5cm,倾角可通过逆透视结果来校准(左右边沿平行).参数有前缀分类(如O.开头是圆环相关参数,Y.开头与三岔路有关).

有疑问可联系 16届视觉组 区梓川 QQ 572729767

![请添加图片描述](https://img-blog.csdnimg.cn/9386d9af6b89482b9a7c03ae48e2dc70.gif)

![请添加图片描述](https://img-blog.csdnimg.cn/5c6614c41a9843099355fafd2b7a67df.gif)

![请添加图片描述](https://img-blog.csdnimg.cn/0934b4a37761413c82b0c40871866d85.gif)

# 1.视觉处理相关文件
![请添加图片描述](https://img-blog.csdnimg.cn/2d4182954ca94b3db5fdcfa80ac70c12.png)



- 2021TJU.c:主函数，初始化
- camera_process.c  .h:底层搜线、元素处理算法
- camera.c  .h:摄像头初始化与属性配置
- dynamic_persp.c  .h:逆透视变换

# 2.摄像头相关函数
## camera_process.c
```C
void Caminfo_Cal() // 摄像头中断函数，处理圆环、三岔，补线等
int clear_all_flags() // 清除所有标志位
int img_otsu(~) // 大津法处理，自动获取曝光值
uint8_t get_orign_edges(~) // 得到原始跳变沿对
int get_best_edge(~) // 获取最近邻边沿
int get_max_edge(~) // 获取最佳边沿
int line_single(~) // 得到基准行边沿
int line_findnext(~) // 得到延申行边沿
int zebra_detect(~) // 斑马线检测
int Junc_detect(~) // 三岔路检测
```
![请添加图片描述](https://img-blog.csdnimg.cn/187dee9cf43f4719be8b84e48c1f0030.png)

## camera.c
```C
int16_t MT9V032_CFG[CONFIG_FINISH][2]=
{
  {AUTO_EXP,          0},   //自动曝光设置      
  // 范围1-63 0为关闭 如果自动曝光开启  
  // EXP_TIME命令设置的数据将会变为最大曝光时间
  // 也就是自动曝光时间的上限
  //一般情况是不需要开启这个功能
  // 因为比赛场地光线一般都比较均匀
  // 如果遇到光线非常不均匀的情况可以尝试设置该值
  // 增加图像稳定性
  {EXP_TIME,          450}, //曝光时间          
  // 摄像头收到后会自动计算出最大曝光时间
  // 如果设置过大则设置为计算出来的最大曝光值
  {FPS,               50},  //图像帧率          
  // 摄像头收到后会自动计算出最大FPS
  // 如果过大则设置为计算出来的最大FPS
  {SET_COL,           IMG_H}, //图像列数量 范围1-752     
  {SET_ROW,           IMG_V}, //图像行数量 范围1-480
  {LR_OFFSET,         0},   //图像左右偏移量    
  {UD_OFFSET,         0},   //图像上下偏移量    正值 上偏移   负值 下偏移  
  {GAIN,              32},  //图像增益 范围16-64     
  // 增益可以在曝光时间固定的情况下改变图像亮暗程度
  {INIT,              0}    //摄像头开始初始化
};
void csi_isr(~) // csi(RT1064的摄像头模块)中断函数
void init_camera(void) // 总钻风摄像头初始化
```

## dynamic_persp.c
```C
void init_persp() // 动态逆透视所需摄像头内参初始化
void calculate_persp() // 逆透视相关参数计算
void start_persp(~) // 逆透视变换
``` 
[逆透视作用与原理1](https://blog.csdn.net/east_fence/article/details/106953970)
[逆透视作用与原理2](https://zhuanlan.zhihu.com/p/391535081)
摄像头算法2(二值化&搜线&逆透视&方向).md

# 3.算法思路
## 底层搜线
见文件camera_process.c

### 初始图象
我们先初始化参数为160x120,从总钻风摄像头得到每帧灰度图像大小为160x120.
![请添加图片描述](https://img-blog.csdnimg.cn/db7ac8cd6a924ecf873b092c2353091a.png)

### 大津法自动曝光
因为赛道上各处亮度可能不一样,我们用大津法找阈值`watch.brightness = img_otsu((uint8_t *)im, IMG_H, IMG_V, 10);`,再计算得到一个曝光时间`int_time_us -= (abs(watch.brightness - 128) > 3 ? setpara.exp_ki * (watch.brightness - 128) : 0);`,动态调整曝光`set_exposure_time(int_time_us);`.
> 相关可调参数
> - setpara.camcfg.manbri  :0自动曝光,1手动曝光
> - setpara.camcfg.exptime  :曝光时间,手动曝光时才生效
> - setpara.exp_ki  :亮度增益,若图像闪烁调小一点

### 赛道识别
先确定一个基准行,通常为第20行,用函数`line_single(&lineinfo[watch.base_line], im[watch.base_line]); // ()`找出其左右边沿的坐标.

然后for循环`line_findnext(&lineinfo[y], im[y], &lineinfo[y + 1]);`,从基准行向下到第`setpara.camcfg.forward_near`行(可调参数),找到左右两边的赛道边沿横坐标.

for循环`line_findnext(&lineinfo[y], im[y], &lineinfo[y - 1]);`,从基准行向上到第`setpara.camcfg.forward_far`行(可调参数),找到左右两边的赛道边沿横坐标.

> 相关可调参数
> - setpara.camcfg.forward_far  :该行以上的图像不处理
> - setpara.camcfg.forward_near  :该行以下的图像不处理

执行完这一步,就可以通过`lineinfo[y].?`调用第y行的一些关键信息.

### 行信息结构体
```C
struct lineinfo_s
{
    unsigned char y;          // 第y行的行数值 0~119
    unsigned char left;       // 第y行的左边沿横坐标 0~160
    unsigned char right;      // 第y行的右边沿横坐标 0~160
    unsigned char edge_count; // 第y行的跳变沿数 0~160
    uint16_t persp_lx; // 逆透视得到的第y行的左边沿横坐标 0~255
    uint16_t persp_ly; // 逆透视得到的第y行的左边沿纵坐标 0~255
    uint16_t persp_rx; // 逆透视得到的第y行的右边沿横坐标 0~255
    uint16_t persp_ry; // 逆透视得到的第y行的右边沿纵坐标 0~255
    int angel_left;  // 第y行的左边打角值
    int angel_right; // 第y行的右边打角值
    int left_lost;   // 第y行的左边丢线值
    int right_lost;  // 第y行的右边丢线值
    unsigned char zebra_flag; // 第y行斑马线标志
    
    unsigned char black_width; // 第y行黑色宽度(三岔路)
    unsigned char junc_angle; // 第y行黑色中心值(三岔路)
};
```
可以通过结构体调用某一行的信息,比如
- `lineinfo[30].right`得第30行右边沿横坐标值- `lineinfo[y + 2].left_lost`得到第y+2行左边沿是否丢线(1丢线0否)



## Caminfo_Cal()函数说明

函数的程序顺序如图

![请添加图片描述](https://img-blog.csdnimg.cn/02c01d5ac50f4de6a9790458d66ab31e.png)



逻辑上,应该先进行元素检测,在进行补线操作,但由于获得每一帧图像时`Caminfo_Cal()`都会直行,实际顺序就变成

![请添加图片描述](https://img-blog.csdnimg.cn/dd0d9c38b9b54380827d212cbb038116.png)

# 4.元素识别

## 直线检测
搜索注释`新检测直线—21区—6-3`

我们在检测圆环,三岔等元素之前,先检测前方是否为直道
- 假如前方是直道,即左边沿和右边沿都是直线,是不可能有环岛三岔和十字这些元素的,相关的检测也不会执行;
- 假如左边是直线,右边不是,则可能出现右圆环或右边车库;
- 反之相似;
- 假如左边右边都不是直线,则可能出现三岔路口和十字等

检测原理是将左边划分为许多小线段,记下这些小线段的斜率,再相互之间作差,如果差值很小的情况占大多数,则左边为一段直线,如果差值很小的情况占得不多,则左边不是直线,右边类似.

> 相关可调参数
> - setpara.StrThres  当差值小得小线段数大于该值,这一边判断为直线

这一步后得到参数`watch.RLStraight`,可以为其他元素识别提供判据
- 0 左右都不是直线
- 1 左边直线
- 2 右边直线
- 3 左右都是直线

## 丢线检测
在底层搜线之后可通过`lineinfo[y].left_lost`和`lineinfo[y].right_lost`得到第y行是否丢线

- 丢线: 某一行的赛道边沿在摄像头视野以外,如下图 
- ![请添加图片描述](https://img-blog.csdnimg.cn/3c64d273e65045a1827131b807e28867.png)
- 得到的右边沿为最大值159
- ![请添加图片描述](https://img-blog.csdnimg.cn/3ffed5c38c9240b5bb5c25d44f9e7fe9.png)

用for循环遍历,可以统计到
> - watch.cross : 有多少行左右都丢线
> - watch.left_lost : 有多少行只是左边丢线
> - watch.right_lost : 有多少行只是右边丢线

这些参数可以为其他元素判断提供判据



## 环岛识别
搜索注释`新环岛识别2—21区602`

从入环到出环,圆环有一些明显的特征
![请添加图片描述](https://img-blog.csdnimg.cn/353bded4640644cba98d968de2e55051.png)

以下讲解以左圆环为例,右圆环原理相同.

### 入环识别1
在到达环岛之前,图像中可以看到环岛**第一个角点**,如图篮圈
![请添加图片描述](https://img-blog.csdnimg.cn/2bf5105d496c438f84ab7bd067d06f09.png)

比较明显的特征是,蓝线以上行左边沿都丢线,蓝线以下行左边沿都没有丢线,右边沿都没有丢线,在未丢线行,越往下左边坐标越小

> 检测特征
> - - 防干扰特征
> - !watch.Junc_flag 非三岔路特征(防干扰)
> - watch.InLoop == 0 圆环特征为0(在圆环外)
> - watch.RLStraight == 2 右边是直线左边不是
> - caminfo.zebra_count < 2 无斑马线特征
> - lineinfo[y + 2].right-lineinfo[y].left>49
> - lineinfo[y + 2].right-lineinfo[y].left<149 左右坐标差值不应太过离谱
> - - 判断特征
> - lineinfo[y + 3].left_lost y行以上3行左边丢线
> - !lineinfo[y - 3].left_lost y行以下右边3行不丢线
> - ...以此类推
> - abs_m(lineinfo[y - 1].left,lineinfo[y - 3].left)<setpara.LoopThres 斜率应小于一定值
> - !lineinfo[y + 3].right_lost 右边都不丢线

满足这些条件后,我们可以跟踪到**第一个角点**所在的行`watch.InLoopAngle = y`,随着车前进,这个值是越来越小的,当他小于可调参数`setpara.InLoopLine`时,准备进入圆环,这一行赋0,同时圆环标志位`watch.InLoop = 1`.

### 入环识别2
在准备入环岛之前,图像中可以看到环岛内圈有一段弧,如图蓝色横线左边,还有**第二个角点**,如图紫色线
![请添加图片描述](https://img-blog.csdnimg.cn/23bb0fd8cae34fdebc698afae2c1d058.png)

对于弧线端点,比较明显的特征有上方丢线,下方不丢线,斜率方向,在第二个角点下方等
> 部分检测特征
> - watch.InLoop == 1 已经过第一个角点
> - y > setpara.WhenInLoop-30 在第二角点的下方
> - lineinfo[y - 1].left < 19 弧线端点在很左边的地方
> - lineinfo[y - 2].right-lineinfo[y-2].left>49 左右两边不靠近(避免一些干扰)
> - lineinfo[y + 3].left_lost 该行上边第三行丢线
> - !lineinfo[y - 3].left_lost 该行下边第三行没丢线
> - lineinfo[y - 1].left < lineinfo[y - 3].left 往下坐标变大
> - !lineinfo[y + 2].right_lost 右边都没有丢线

满足这些条件后,可以通过`watch.InLoopCirc = y`跟踪这段**弧线端点**所在的行

对于**第二个角点**,特征和第一个角点上下相反,上边没有丢线,下方丢线了,同时它在**弧线端点**所在行的上方,这和**第一个角点**差不多,不再复述

满足条件后可以跟踪到**第二个角点**所在行`watch.InLoopAngle2 = y`,此时可以开始补线.

由于在进入环岛时几乎打满角度,**第二个角点**所在的行(纵坐标)向下变化较慢,而它所在的列(横坐标)迅速右移,所以我们用列来判断.当**第二个角点**所在列比可调参数大`lineinfo[watch.InLoopAngle2 + 2].left > (159-setpara.InLoopRow)`(159-该参数是为了左右对称),将该行清0`watch.InLoopAngle2 = 0`,同时赋给`watch.InLoop = 2`,标志着已经入环.

### 环内
在环岛内和弯道差不多,但可能存在旁边赛道或杂物影响,导致误判成出环等情况,故在`InLoop = 2`同时开启陀螺仪`gz`轴角速度积分得到偏航角`indata.YawAngle += gz_to_delta_angle*indata.gz`(文件data_input.c),当偏航角约为200度时,基本走到圆环出口附近,此时赋`watch.InLoop = 3`

### 出环识别
在准备出环岛之前,图像中可以看到一个明显的角
![请添加图片描述](https://img-blog.csdnimg.cn/c402732654b1468ca38b8d083309e47d.png)

比较明显的特征有左边丢线,该点往上坐标变大,往下坐标也变大
```C
if ((watch.InLoop == 3) && \
            lineinfo[y].left_lost && watch.zebra_flag == 0 && \
            lineinfo[y + 2].right > lineinfo[y].right && \
            lineinfo[y - 3].right > lineinfo[y - 1].right && \
            lineinfo[y + 4].right > lineinfo[y + 2].right && \
            lineinfo[y - 5].right > lineinfo[y - 3].right && \
            lineinfo[y].right > 30)
```

并且随着小车前进,这个角迅速向右边移动,赋`watch.OutLoopRight = lineinfo[y].right`可以跟踪这个角点的右边沿横坐标.

通过补线可以让小车保持左转,同时偏航角仍在增大,当他大于270°`indata.YawAngle > LoopOut`,赋圆环标志`watch.InLoop = 4`,并将偏航角清0.

### 出环识别2
出环时会再一次见到第二个角点,当他小于设定值`(watch.OutLoopAngle = y) < setpara.OutLoopThres`时,完全出环,清除所有标志位.

## 圆环补线
搜索注释`圆环入口补线1-21区602`

由于逆透视传入的是中间值`xl,xr`,我们在赋值`xl = lineinfo[y].left;
        xr = lineinfo[y].right;`后,逆透视变换之前,更改这两个值
### 过环补线
在入环前,我们先补一段线段,避免小车从错误入口入环

![请添加图片描述](https://img-blog.csdnimg.cn/07c42e7c1b6c4914b9f581e9a16ea3a4.png)

由于右边为直线,我们直接从右边坐标运算到左边`xl = lineinfo[y].right - 135 + y;`(先计算两个端点的关系,中间点可插值得到,这是我们已经确定摄像头高度和俯仰角后写死的,不能适应其他角度).

### 入环补线

![请添加图片描述](https://img-blog.csdnimg.cn/f7d24b64620d46c19921a3dbdc5b837a.png)

在入环检测时我们已经知道第二个角点的横纵坐标,从该点出发,分别和左上角和右下角连线,即可将赛道改为左行赛道.

计算方式是先求斜率`slopeL = (159.0 - lineinfo[watch.InLoopAngle2 + 2].left * 1.0) / (watch.InLoopAngle2);`,再逐点求出右边横坐标值`xr = (watch.InLoopAngle2 - y) * slopeL + lineinfo[watch.InLoopAngle2 + 2].left;`,同时为了更快入环,左边坐标`xl`全部赋0.

上图是**第二个角点**连接**弧线端点**所在行的右边坐标,但这样入环时机慢了,故改为右下角连线.
![请添加图片描述](https://img-blog.csdnimg.cn/3d3f831641f143588f84c77cb5646bcb.png)

### 出环补线

到出左环时,在检测右边角点时得到其横纵坐标,将该点连线到左上角点`xr = slopeOL * (watch.watch_line - y)`,可将赛道改为左行赛道.
![请添加图片描述](https://img-blog.csdnimg.cn/83468d43dacc404d883f75dacf730018.png)

当看到第二个角点时,将入环口封死,原理同过环补线.
![请添加图片描述](https://img-blog.csdnimg.cn/89f390f01c214db19132717af884b3b7.png)

> 重要参数
> - setpara.CamLoopKp 圆环内舵机pd
> - setpara.CamLoopKd 
> - setpara.InLoopLine 准备入环的行
> - setpara.WhenInLoop 确认入环的行
> - setpara.InLoopRow 入环所在列
> - setpara.LoopThres 比较用的斜率
> - setpara.OutLoopThres 出环行
> - watch.RLStraight;   //直线标志：左右直线0无1左2右3左右
> - watch.InLoopAngle;  //经过圆环第一个角所在行
> - watch.InLoopCirc;   //经过圆环上凸弧
> - watch.InLoopAngle2; //入环前看见第二个角点
> - watch.OutLoopAngle; //出环后看见第二个角点
> - **watch.InLoop**; //圆环状态0无环 左环1234 右环5678
> - watch.OutLoopRight;  //出环右角点横坐标
> - watch.OutLoopRightY; //出环右角点纵坐标
> - watch.OutLoopLeft;   //出环左角点横坐标
> - watch.OutLoopLeftY;  //出环左角点纵坐标


## 车库识别
### 出库
在车库内我们其实无法看到明显的特征,参见开篇动图,故我们出库直接打死角,并延时一段时间.

> 关键参数
> - mycar.status 小车运行状态,0常态3出库1行驶2刹车
> - mycar.RUNTIME 当前行驶时间,从发车开始计时
> - setpara.mode 设定的运行模式,0直行1左出库2右出库
> - setpara.settime 设定的总行驶时间
> - setpara.garage_outangle 设定的出库角度
> - setpara.garage_outtime 设定的出库角度锁定时间

相关代码见文件`2021TJU.c`,搜索注释`起步出库`

### 入库
16届无斑马线和入库,该部分参考附件`国赛改规则前805准备移植wifi`的文件`camera_process.c`

搜索注释`斑马线探测`

![请添加图片描述](https://img-blog.csdnimg.cn/328a82a85e394368b6ad7c96599d5ef3.png)

入库时可见到明显的斑马线,在底层首先算法对斑马线进行了提取和屏蔽,在边沿显示上是看不见的.斑马线明显的特征是黑白跳变沿非常多,在底层搜线时得到每行边沿数量`lineinfo->edge_count`,可以做一个初步判断,同时边沿的横坐标已存储在数组`edge_store[]`中,这个数组的[0246]奇数位都是黑色变白色边沿,[1357]偶数位都是白色变黑色边沿,用后者减去前者可以得到白色的宽度,可以作为后续判断.这样可以统计出符合条件的白色块数量,这个数量多于一定值(6)时,这一行标记为有斑马线特征的行,赋`lineinfo->zebra_flag = 1`.

这些工作都在函数`zebra_detect(~)`中完成,在搜索边沿函数`line_findnext(~)`已调用,之后就可以通过`lineinfo[y].zebra_flag`知道第y行是否为斑马线相关行(1是0否).

搜索注释`斑马线 & apriltag 检测`

通过循环可以知道当前一帧有多少行带有斑马线特征,如下图,左下角为`zebra_count`,靠近车库时这个值变大了(7->19)

![请添加图片描述](https://img-blog.csdnimg.cn/28762c1bfb434b0ea9d515c68e12b2eb.png)

![请添加图片描述](https://img-blog.csdnimg.cn/8929b23f942f48d789b24b72bf50b095.png)



当斑马线行数大于一定值时,通过检测上方角点补线即可,但华北赛时需要跑两圈,也就是第一次过斑马线时并不处理,这里定义标志第几圈`flag.CircCount`,在起步后这个值赋1,当第一次通过斑马线时这个值赋2,第二次来到时赋3,这时才开始补线,补线原理同圆环.

![请添加图片描述](https://img-blog.csdnimg.cn/dbfe7b24918d4d3cb0964cedafb8dce6.png)

搜索注释`入车库补线1-21区602`

> 关键参数
> - caminfo.zebra_count 带有斑马线特征的行数
> - setpara.ZebraCount 设定的值,斑马线行数大于该值时,下面参数赋1
> - watch.zebra_flag 斑马线标志,到斑马线处赋1,平时为0
> - flag.CircCount 当前在第几圈,值为3且上面参数为1时开始补线入库
> - watch.ZebraLine 上角点所在行

## 三岔路
### 三岔识别
三岔识别有比较明显的特征
![请添加图片描述](https://img-blog.csdnimg.cn/6ada7afb6679414f8afc34cbc0ae8108.png)

搜索注释`新三岔路识别2—21区602`

由于底层算法得到的信息限制,每一行只有一个左边沿和一个右边沿,搜索边沿是从左边开始的,故两段岔路里,`lineinfo`记录的左右坐标都只有左边路段,所以**假如完全不对三岔路进行处理,小车一定会向左走**.
![请添加图片描述](https://img-blog.csdnimg.cn/b1b287d08622475c89153375b51f2574.png)



单纯靠左下角和右下角两个特征可靠性不高,很可能和十字路口混淆,所以我们引入倒黑色三角形的检测.

搜索注释`每一行的三岔特征检测`

仿照斑马线的检测,我们在直行搜索边沿函数`line_findnext`时,检测三岔特征.通过`lineinfo->edge_count`可以知道当前行的跳变沿数量,通过`edge_store[2]`可以知道从左到右跳变沿的横坐标值.
![请添加图片描述](https://img-blog.csdnimg.cn/8cb611bcd83b49009751fcf7217a0f05.png)



在三岔分路点以下,只有两个边沿,`lineinfo->edge_count=2`,`edge_store`数组有两个值,在分路点以上,道路分开,`lineinfo->edge_count=4`,`edge_store`数组有四个值.

分路点的一个明显特征是它是倒黑色三角形,从一个点向上黑色宽度会变大,可以通过`JBlack_width = edge_store[2]-edge_store[1]`得到这一行中间部分黑色的宽度,同时通过`lineinfo->junc_angle = (edge_store[2]+edge_store[1])/2`得到这一行的黑色区域中间坐标.

搜索注释`新三岔路识别2—21区602`

可以通过分路点的特征:该点以上边沿数为4,该点以下边沿数为2,分路点往上黑色区域变宽,黑色区域中心基本在一条直线上,满足这样的条件可以得到角点纵坐标`watch.JuncAngle = y`和角点横坐标`watch.JuncAngleX = lineinfo[y+1].junc_angle`,这个特征可以作为后续判据和补线起点.

还有下边左右两个点的特征,也比较明显,左右两边都没有丢线,两个角点都在分路点下面,角点处是坐标的极大值或极小值,记一下上下的坐标,作差比较即可.小车不一定正着来到三岔路口前,所以我们对两个角点分别找它的行`watch.JuncLineR,watch.JuncLineL`,再求平均.

后面还做了一些防止误识别的检测,左右丢线或者特征行相差太大都可以视为误判.

`watch.Junc_flag`是三岔路标志位,在到达三岔路时可能并不知道向左走还是向右走,如16届智能视觉组华北赛在到达三岔路时要先停车识别数字,故在到达三岔路时这个值先赋3,判断之后1为向左,2为向右,这时候可以开始补线.

由于三岔路口一定有两个,在进入三岔路之前,一定走过了偶数个三岔口,进入岔路后,就走过了奇数个三岔口,这样可以判断小车在岔路内还是岔路外.从哪一边进岔路,就从哪一边出去,做相应补线即可,我们用`flag.JuncCount`记录这个数量.

在16届国赛时,三岔路只需和上一圈相反即可.在第一次到达三岔时,我们根据可调的`setpara.TempRL`决定先走左边还是右边,`flag.JuncDirection`是记录一个总的三岔方向标志,在平时不清0,而`watch.Junc_flag`时临时三岔方向标志,变化为0-3-1/2,然后实现补线.

在之后到达三岔路,和一圈进入方向相反,我们在每次出岔路时,都变换`flag.JuncDirection`,值为1时走左边,为2时走右边,下次准备入岔路时就可以知道怎么走了.

### 三岔补线
搜索注释`三岔路补线1-21区602`

在三岔口检测时已经知道分路点的横纵坐标,在分路点拉一条直线到左下角或右下角,同时另一边沿赋最值,则轨迹改为向右走或向左走.

![请添加图片描述](https://img-blog.csdnimg.cn/c4a75600b3e0418daf98c8ffdfca6c0c.png)
![请添加图片描述](https://img-blog.csdnimg.cn/a0251a2fa99c482891a10b738d01b19e.png)


由于经过岔路只是一瞬间的事,并且经过后也没有明显的特征,故这里补线一段时间后即取消补线,搜索注释`过三岔`

在走过岔路时响一下蜂鸣器,以验证是否正确检测到三岔路,但对于急弯进三岔,还是不能保证百分百检测到三岔,这时有可能会出现都走左边,或者在三岔路内打转.可以用其他手段补救以下:因为圆环不在三岔内,来到圆环时,必定已经走过偶数个三岔口，假如看漏了一个三岔，则补上.

> 关键参数
> - watch.Junc_flag 三岔标志,0无三岔3有三岔1左走2右走
> - flag.JuncDirection 三岔转向标志1左走2右走
> - watch.JuncAngle 三岔分路点所在行,纵坐标
> - watch.JuncAngleX 三岔分路点所在列,横坐标
> - watch.JuncLineR 右边角点所在行
> - watch.JuncLineL 左边角点所在行
> - watch.JuncLine 左右角点行平均值
> - lineinfo[y].edge_count 第y行跳变沿数量
> - lineinfo[y].black_width 第y行黑色区域宽度
> - lineinfo[y].junc_angle 第y行黑色区域中心横坐标
> - UpRDel,DownRDel,UpRDely,DownRDely 计算斜率时存的一些中间变量
> - setpara.WhenJuncStop 设定的行,watch.JuncAngle小于这个值时判定为前方有三岔
> - watch.JuncTime 此时三岔补线开始,记下这个时间
> - flag.JuncCount 走过了多少个三岔路口
> - setpara.JuncProcess 设定的三岔路补线多长时间


## 十字检测

我们的底层算法一个优势是可以不处理十字,到十字口都会默认直行,但由于左右边沿都是从基准行`watch.base_line`(20)开始搜索,来到十字口时,20行的左右两边会丢掉边沿,会丢失基准行,导致丢失轨迹,得到的轨迹会稍微向左偏,这是不理想的路径,见开篇的动图,如果十字处有路肩,就会撞上.

我们的处理方法是检测到十字时,基准行20行找不到了,将基准行改为80行.

搜索注释`十字处理`

检测方法是从near行到60行统计左右都丢线的行数`watch.cross_lost`,当这个值大于25就判断为十字,更改基准行,下面是处理前后对比.

![请添加图片描述](https://img-blog.csdnimg.cn/b3dc23874b9d4daab80706abce2f3796.gif)
![请添加图片描述](https://img-blog.csdnimg.cn/c15c513a852a4665a9f8c70c8f9dcdf3.gif)


## 坡道处理

我们的底层算法另一个优势,也可以不处理坡道,到坡道时都会默认直行.但假如速度过快,可能会飞车,可能会导致车体不稳定(智能视觉组装备比较多),我们一直通过陀螺仪对俯仰角做积分,实时补偿到速度上,在上坡时会有一定减速.

# 5.逆透视变换
对补线之后的左右坐标临时值`xl,xr`做逆透视变换

搜索注释`对补线后的边沿进行逆透视变换`

# 6.角度计算相关
搜索`d_xl = lineinfo[y].left;`和注释`舵机角度计算`

得到方向值`caminfo.dir`和`caminfo.ddir`,可做进一步舵机打角处理.

# 7.实时图传
为了了解小车运行实际状况,我们用python写了一个接收上位机.

## 发送

在小车上,我们用RT1064的DMA-UART传输到串口转无线模块,再传输到电脑.

搜索注释`准备发送图像数据`

只需要传第y行,左边坐标,右边坐标或想要显示的标志线即可,一直发送就可以得到实时图像,参考开篇三个动图.

## 上位机
我们用OpenCV-python写了一个简单的接收程序,开启串口接收,根据接收到第几行的左边右边坐标,用画线函数画两个点,只需接收一定行数就可以看出实时拍摄的边沿了.
